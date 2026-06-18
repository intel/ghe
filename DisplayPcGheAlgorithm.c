//===========================================================================
//
//   Copyright (c) Intel Corporation (2000 - 2019)
//
//   INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS LICENSED
//   ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT, ASSISTANCE,
//   INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT PROVIDE ANY UPDATES,
//   ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY DISCLAIMS ANY WARRANTY OF
//   MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY PARTICULAR PURPOSE, OR ANY
//   OTHER WARRANTY.  Intel disclaims all liability, including liability for
//   infringement of any proprietary rights, relating to use of the code. No license,
//   express or implied, by estoppel or otherwise, to any intellectual property
//   rights is granted herein.
//
//--------------------------------------------------------------------------

/**
 *
 * @file  DisplayPcGheAlgorithm.c
 * @brief  This file contains the GHE Algorithm and related functions.
 *
 */

#include "DisplayPcDpst.h"
#include "ghe.h"

#define DRM_MODE_HISTOGRAM_HSV_MAX_RGB                  (1 << 0)


static void ClipTxFuncBoost(double *pTxFunc, uint32_t NumSamples, double MinBoost, double MaxBoost);
static void ConvertLookupToMultiplierLut(double *pInput, double *pOutput, double MinBoost, double MaxBoost);
static bool DetectBinsOutOfRange(uint32_t *pHistogram, GHE_CONFIG *pGheCfg, GHE_PARAMS *pGheParams);
static bool AdjustExcessBins(uint32_t *pHistogram, GHE_CONFIG *pGheCfg, GHE_PARAMS *pGheParams);
static void DisplayGheAlgorithmCore_v1_0(DISPLAY_PC_XPST_CONTEXT *pDpstContext);

/***************************************************************
 * @brief To clip the input lut for controlling min and max boost factor
 *
 * @param pTxFunc
 * @param NumSamples
 * @param MinBoost
 * @param MaxBoost
 * @return void
 ***************************************************************/
static void ClipTxFuncBoost(double *pTxFunc, uint32_t NumSamples, double MinBoost, double MaxBoost)
{
    double StepSize = 1.0 / (double)(NumSamples - 1);

    for (uint32_t Index = 1; Index < NumSamples; Index++)
    {
        double Input       = (double)Index * StepSize;
        double BoostFactor = pTxFunc[Index] / Input;

        BoostFactor = DD_MIN(BoostFactor, MaxBoost);
        BoostFactor = DD_MAX(BoostFactor, MinBoost);

        pTxFunc[Index] = Input * BoostFactor;
    }
}

/*
 * Interpolate IET LUT from histogram based LUT.
 * This is required because IET has more entries than histogram LUT
 * Interpolation Logic => MinValue + (MaxValue - MinValue) * Interpolator.
 */
static double DisplayDpstCalculateInterpolated1DLUTValue(double inval, double *lut, double maxindex)
{
        uint32_t index1, index2;
        double val1, val2, interpolator;
        double DIndex = inval * maxindex;

        index1 = DIndex;
        index2 = ceil(DIndex);

        interpolator = (DIndex - (double)index1);
        val1         = lut[index1];
        val2         = lut[index2];

        return val1 + interpolator * (val2 - val1);
}

/***************************************************************
 * @brief To convert LUT of size XPST_BIN_COUNT to IET LUT of size XPST_IET_LUT_LENGTH
 *
 * @param pInput
 * @param pOutput
 * @param MinBoost
 * @param MaxBoost
 * @return void
 ***************************************************************/
static void ConvertLookupToMultiplierLut(double *pInput, double *pOutput, double MinBoost, double MaxBoost)
{
    double BinIndexNormalized, IetVal;
    const double MaxHistBinIndex = XPST_MAX_BIN_INDEX;
    const double IetLutStepSize  = 1.0 / (double)XPST_MAX_IET_INDEX;

    for (uint32_t BinIndex = 1; BinIndex < XPST_IET_LUT_LENGTH; BinIndex++)
    {
        BinIndexNormalized = (double)BinIndex * IetLutStepSize;
        IetVal             = DisplayDpstCalculateInterpolated1DLUTValue(BinIndexNormalized, pInput, MaxHistBinIndex);
        IetVal             = IetVal / BinIndexNormalized; // Compute sample for multiplier LUT
        IetVal             = DD_MIN(IetVal, MaxBoost);
        IetVal             = DD_MAX(IetVal, MinBoost);

        pOutput[BinIndex] = IetVal;
    }
}

/***************************************************************
 * @brief This function is to validate all bins are in range of
 *          (total pixel count > upperlimit) >= (total pixel count < lowerlimit)
 *          Where, Lower limit and upper limits are assigned during
 *              initialization based on boost factor and frame size
 *
 * @param pHistogram
 * @param pGheCfg
 * @param pGheParams
 * @return bool
 ***************************************************************/
static bool DetectBinsOutOfRange(uint32_t *pHistogram, GHE_CONFIG *pGheCfg, GHE_PARAMS *pGheParams)
{
    pGheParams->NumBinsBelowMinHistogramBinCount   = 0;
    pGheParams->NumPixelsAboveMaxHistogramBinCount = 0;
    pGheParams->NumPixelsBelowMinHistogramBinCount = 0;

    // Initialize with invalid data
    bool AreValidBins = FALSE;

    for (uint8_t BinIndex = 0; BinIndex < XPST_BIN_COUNT; BinIndex++)
    {
        uint32_t BinVal = pHistogram[BinIndex];

        if (BinVal < pGheCfg->MinHistBinCount)
        {
            pGheParams->NumPixelsBelowMinHistogramBinCount += pGheCfg->MinHistBinCount - BinVal;
            pGheParams->NumBinsBelowMinHistogramBinCount++;
        }
    }

    // Find values above bank of upper limits
    for (uint8_t LimitIndex = 0; LimitIndex < GHE_NUM_STEPS_MAX_LIMIT_SEARCH; LimitIndex++)
    {
        uint32_t UpperLimit                    = pGheCfg->MaxHistBinCounts[LimitIndex];
        uint32_t NumValuesAboveUpperLimitIndex = 0;

        for (uint8_t BinIndex = 0; BinIndex < XPST_BIN_COUNT; BinIndex++)
        {
            uint32_t BinVal = pHistogram[BinIndex];

            if (BinVal > UpperLimit)
            {
                NumValuesAboveUpperLimitIndex += BinVal - UpperLimit;
            }
        }

        // We got a case where values above limit can be adjusted to values below limit
        if (NumValuesAboveUpperLimitIndex >= pGheParams->NumPixelsBelowMinHistogramBinCount)
        {
            pGheParams->NumPixelsAboveMaxHistogramBinCount = NumValuesAboveUpperLimitIndex;
            pGheParams->MaxHistBinCount                    = pGheCfg->MaxHistBinCounts[LimitIndex];
            AreValidBins                                   = TRUE;
            break;
        }
    }

    return AreValidBins;
}

/***************************************************************
 * @brief This function is a preprocessing function distributes
 *         histogram bin values in a way that every bin has pixel count
 *         between some min and max limits. The limits are decided based on
 *         allowed max and min slope in the transfer function.
 *
 * @param pHistogram
 * @param pGheCfg
 * @param pGheParams
 * @return bool
 ***************************************************************/
static bool AdjustExcessBins(uint32_t *pHistogram, GHE_CONFIG *pGheCfg, GHE_PARAMS *pGheParams)
{
    if (0 == pGheParams->NumBinsBelowMinHistogramBinCount)
    {
        return FALSE;
    }

    // There can be NumBinsBelowMinHistogramBinCount = 0 with NumPixelsAboveMaxHistogramBinCount != 0
    // Need to improve Algorithm to handle those kind of images. Eg. Gradient images which is not having dark portion

    if (pGheParams->NumPixelsAboveMaxHistogramBinCount > pGheParams->NumPixelsBelowMinHistogramBinCount)
    {
        // Adjust the excess values uniformly to the lowest bins
        uint32_t NumValuesToAdjust        = pGheParams->NumPixelsAboveMaxHistogramBinCount - pGheParams->NumPixelsBelowMinHistogramBinCount; // Intended values to adjust over and avove limit capping
        double NumValuesAdjustedPerBin = ((double)NumValuesToAdjust) / (double)pGheParams->NumBinsBelowMinHistogramBinCount;              // Use a double container to keep the fractional part.
        double NumValuesAccumulatedForAdjustment = 0;
        uint32_t NumValuesAdjustedActual            = 0;

        for (uint8_t BinIndex = 0; BinIndex < XPST_BIN_COUNT; BinIndex++)
        {
            // Cap the limits
            pHistogram[BinIndex] = DD_CLAMP_MIN_MAX(pHistogram[BinIndex], pGheCfg->MinHistBinCount, pGheParams->MaxHistBinCount);

            // Adjust values (over and above limit capping) to the lowest bins
            if (NumValuesToAdjust && (pHistogram[BinIndex] == pGheCfg->MinHistBinCount))
            {
                // Accumulate fractional number into a sum. Keep adjusting the integer part of the unadjusted portion from accumulated value.
                NumValuesAccumulatedForAdjustment += NumValuesAdjustedPerBin;
                uint32_t Adjust = (uint32_t)(NumValuesAccumulatedForAdjustment - NumValuesAdjustedActual); // Integer part of unadjusted portion of accumlated value
                Adjust       = DD_MIN(Adjust, NumValuesToAdjust);

                pHistogram[BinIndex] += Adjust;
                NumValuesToAdjust -= Adjust;
                NumValuesAdjustedActual += Adjust;
            }
        }

        // Adjust remaining values into the last histogram bin
        if (NumValuesToAdjust)
        {
            pHistogram[XPST_MAX_BIN_INDEX] += NumValuesToAdjust;
        }
    }
    else if (pGheParams->NumPixelsAboveMaxHistogramBinCount == pGheParams->NumPixelsBelowMinHistogramBinCount)
    {
        // Cap the limits
        for (uint8_t BinIndex = 0; BinIndex < XPST_BIN_COUNT; BinIndex++)
        {
            pHistogram[BinIndex] = DD_CLAMP_MIN_MAX(pHistogram[BinIndex], pGheCfg->MinHistBinCount, pGheParams->MaxHistBinCount);
        }
    }

    return TRUE;
}

static double EstimateProbabilityOfFullScreenSolidColor(double *powerhistogram, double totalpower)
{
        const double solidcolorpowerthreshold = SOLID_COLOR_POWER_THRESHOLD * totalpower;
        double windowsizetoprobabilitymapping[SOLID_COLOR_SEARCH_WINDOW_SIZE] = {1, 1, 0.75, 0.375};
        uint8_t N;

        /*
         * Find N number of consecutive bins which contain SOLID_COLOR_POWER_THRESHOLD
         * amount of frame power.
         * N = 1 means solid color for sure.
         * Since SOLID_COLOR_POWER_THRESHOLD is not 1.0, it detects almost solid color.
         * N = 2 may mean near solid color. One pixel value shift will shift energy
         * to next or prev bin.
         * For example, image with solid color patches 247 and 248 will look almost single solid,
         * but  histogram will be spread across two bins.
         * N >= 3 means probability of solid color is less.
         * Return value is gradullay reduced to 0 for N >= 3.
         */

	for (N = 1; N <= SOLID_COLOR_SEARCH_WINDOW_SIZE; N++)
	{
		for (uint8_t BinIndex = 0; BinIndex <= (XPST_MAX_BIN_INDEX - N + 1); BinIndex++)
		{
			double SumPower = 0;
			for (uint8_t i = BinIndex; i < (BinIndex + N); i++)
			{
				SumPower += powerhistogram[i];

				if (SumPower >= solidcolorpowerthreshold)
				{
					return windowsizetoprobabilitymapping[N - 1];
				}
			}
		}
	}

    return 0;
}

double DetectFullScreenSolidColor(double *pPowerHistogram, double TotalPower)
{
        return EstimateProbabilityOfFullScreenSolidColor(pPowerHistogram, TotalPower);

}

void CreateUnityIET(uint32_t *pMultiplierLut)
{
    for (int BinIndex = 0; BinIndex < XPST_IET_LUT_LENGTH; BinIndex++)
    {
        pMultiplierLut[BinIndex] = XPST_IET_SCALE_FACTOR;
    }
}

/***************************************************************
 * @brief Calculates contrast enhanced LUT and covertes to IET LUT
 *
 * @param pDpstContext
 * @return void
 ***************************************************************/
static void DisplayGheAlgorithmCore_v1_0(DISPLAY_PC_XPST_CONTEXT *pDpstContext)
{
    uint32_t TotalNumOfPixel = 0;
    uint8_t BinIndex;
    double BinIndexNormalized, CDFRange, CdfNormalizingFactor, ProbabilityOfFullScreenSolidColor, SumPower = 0, MinCDFVal = 0;
    double AggressivenessFactor, DefaultIetWeight;
    GHE_PARAMS *pGheParams      = &pDpstContext->GheArgs.GheParams;
    uint32_t *pHistogram           = pDpstContext->Algorithm.XpstAlgorithmDynamicData.Histogram;
    uint32_t *pMultiplierLut       = pDpstContext->Algorithm.XpstAlgorithmDynamicData.MultiplierLutTarget;
    const double IetLutStepSize = 1.0 / (double)XPST_MAX_IET_INDEX;
    double MaxBoostFactor       = (double)pDpstContext->GheArgs.GheCfg.MaxBoostFactor / (double)GHE_SLOPE_PRECISION_FACTOR;
    double MinBoostFactor       = (double)pDpstContext->GheArgs.GheCfg.MinBoostFactor / (double)GHE_SLOPE_PRECISION_FACTOR;

    // We could not get a good adjustment option.
    if (FALSE == DetectBinsOutOfRange(pHistogram, &pDpstContext->GheArgs.GheCfg, &pDpstContext->GheArgs.GheParams))
    {
        CreateUnityIET(pMultiplierLut);
        return;
    }

    if (FALSE == AdjustExcessBins(pHistogram, &pDpstContext->GheArgs.GheCfg, &pDpstContext->GheArgs.GheParams))
    {
        CreateUnityIET(pMultiplierLut);
        return;
    }

    // CDF is calculated from the pre-processed histogram
    for (BinIndex = 0; BinIndex < XPST_BIN_COUNT; BinIndex++)
    {
        TotalNumOfPixel += pHistogram[BinIndex];
        pGheParams->NormalizedCDF[BinIndex] = TotalNumOfPixel;

        if ((0 == MinCDFVal) && (TotalNumOfPixel > 0))
        {
            MinCDFVal = TotalNumOfPixel;
        }
    }

    // Calculate histogram bin wise power distribution and total frame power
    for (BinIndex = 0; BinIndex < XPST_BIN_COUNT; BinIndex++)
    {
        double BinWeight                        = pDpstContext->Algorithm.XpstAlgorithmStaticData.DeGammaLUT[BinIndex];
        pGheParams->PowerDistribution[BinIndex] = BinWeight * (double)pHistogram[BinIndex];

        SumPower += pGheParams->PowerDistribution[BinIndex];
    }

    ProbabilityOfFullScreenSolidColor =
    DetectFullScreenSolidColor(pGheParams->PowerDistribution, SumPower);

    // Do not modify pixel values for Solid Color
    if (1 == ProbabilityOfFullScreenSolidColor)
    {
        CreateUnityIET(pMultiplierLut);
        return;
    }

    // Normalizing CDF
    CDFRange             = pGheParams->NormalizedCDF[XPST_MAX_BIN_INDEX] - MinCDFVal;
    CdfNormalizingFactor = 1.0 / CDFRange;

    for (BinIndex = 0; BinIndex < XPST_BIN_COUNT; BinIndex++)
    {
        double OutVal                       = (pGheParams->NormalizedCDF[BinIndex] - MinCDFVal) * CdfNormalizingFactor;
        pGheParams->NormalizedCDF[BinIndex] = DD_MAX(OutVal, 0);
    }

    ClipTxFuncBoost(pGheParams->NormalizedCDF, XPST_BIN_COUNT, MinBoostFactor, MaxBoostFactor);

    // Convert LUT of size XPST_BIN_COUNT to IET LUT of size XPST_IET_LUT_LENGTH
    ConvertLookupToMultiplierLut(pGheParams->NormalizedCDF, pGheParams->EnhancementTable, MinBoostFactor, MaxBoostFactor);

    // 0th multiplier sample can't be computed. Extend 1st sample to the 0th
    pGheParams->SmoothenedTable[0] = pGheParams->SmoothenedTable[1] = pGheParams->EnhancementTable[1];

    // First two and last two samples can not be processed using five-point filter
    pGheParams->SmoothenedTable[XPST_MAX_IET_INDEX - 1] = pGheParams->EnhancementTable[XPST_MAX_IET_INDEX - 1];
    pGheParams->SmoothenedTable[XPST_MAX_IET_INDEX]     = pGheParams->EnhancementTable[XPST_MAX_IET_INDEX];

    // Smoothen jerks in EnhancementTable by averaging current and nearby 4 samples.
    for (BinIndex = 2; BinIndex < XPST_MAX_IET_INDEX - 1; BinIndex++)
    {
        pGheParams->SmoothenedTable[BinIndex] = 0.2 * (pGheParams->EnhancementTable[BinIndex - 2] + pGheParams->EnhancementTable[BinIndex - 1] + pGheParams->EnhancementTable[BinIndex] +
                                                       pGheParams->EnhancementTable[BinIndex + 1] + pGheParams->EnhancementTable[BinIndex + 2]);
    }

    // First and last sample can not be processed
    pMultiplierLut[0] = (uint32_t)((double)XPST_IET_SCALE_FACTOR * pGheParams->SmoothenedTable[0] + 0.5);
    pMultiplierLut[0] = DD_MIN(pMultiplierLut[0], XPST_IET_MAX_VAL);

    pMultiplierLut[XPST_MAX_IET_INDEX] = (uint32_t)((double)XPST_IET_SCALE_FACTOR * pGheParams->SmoothenedTable[XPST_MAX_IET_INDEX] + 0.5);
    pMultiplierLut[XPST_MAX_IET_INDEX] = DD_MIN(pMultiplierLut[XPST_MAX_IET_INDEX], XPST_IET_MAX_VAL);

    AggressivenessFactor = pDpstContext->GheArgs.GheCfg.AggressivenessFactor;
    DefaultIetWeight     = 1.0 - AggressivenessFactor;

    // Smoothen jerks in FilteredEnhancementTable by averaging with nearby bins.
    for (BinIndex = 1; BinIndex < XPST_MAX_IET_INDEX; BinIndex++)
    {
        // Average out current and two nearby samples.
        double FilteredIetVal = 0.333333 * (pGheParams->SmoothenedTable[BinIndex - 1] + pGheParams->SmoothenedTable[BinIndex] + pGheParams->SmoothenedTable[BinIndex + 1]);

        BinIndexNormalized = (double)BinIndex * IetLutStepSize;
        FilteredIetVal     = DD_MIN(FilteredIetVal, 1.0 / BinIndexNormalized); // Cap IET val to the value that will not cause clipping.

        // AmbientLux based interpolation between 1.0 and calculated Iet value
        FilteredIetVal = AggressivenessFactor * FilteredIetVal + DefaultIetWeight;

        pMultiplierLut[BinIndex] = (uint32_t)((double)XPST_IET_SCALE_FACTOR * FilteredIetVal + 0.5);
        pMultiplierLut[BinIndex] = DD_MIN(pMultiplierLut[BinIndex], XPST_IET_MAX_VAL);
    }

    pMultiplierLut[0] = pMultiplierLut[1]; // 0th multiplier sample can't be computed. Extend 1st sample to the 0th

    return;
}

/***************************************************************
 * @brief GHE Algorithm version 1.0
 *
 * @param pDpstContext
 * @param Pipe
 * @param IsValidHistogram
 * @param pHistogramData
 * @return void
 ***************************************************************/
void DisplayGheAlgorithm(DISPLAY_PC_XPST_CONTEXT *pDpstContext)
{
    uint32_t ImageSize;

    ImageSize = pDpstContext->Algorithm.XpstAlgorithmStaticData.ImageSize;
    // Sanity checks...
    if (0 == ImageSize)
    {
        return;
    }

    DisplayGheAlgorithmCore_v1_0(pDpstContext);

    return;
}

void histogram_compute_generate_data_bin(struct globalhist_args *gheargs)
{
        if (gheargs->histogrammode != DRM_MODE_HISTOGRAM_HSV_MAX_RGB)
                return;

        DISPLAY_PC_XPST_CONTEXT *pDpstContext =
                (DISPLAY_PC_XPST_CONTEXT *)calloc(1, sizeof(DISPLAY_PC_XPST_CONTEXT));

        assert(pDpstContext != NULL);

        for (int i = 0; i < XPST_BIN_COUNT; i++)
                pDpstContext->Algorithm.XpstAlgorithmDynamicData.Histogram[i] = gheargs->histogram[i];

        uint32_t ImageSize = gheargs->resolution_x * gheargs->resolution_y;
        pDpstContext->Algorithm.XpstAlgorithmStaticData.ImageSize = ImageSize;

        /* Initialize GHE config — matching DisplayPcDpst.c driver initialization */
        GHE_CONFIG *pCfg = &pDpstContext->GheArgs.GheCfg;
        
        /* Boost factor limits (4.0x max, 0.5x min) */
        pCfg->MaxBoostFactor = GHE_MAX_BOOST_FACTOR;
        pCfg->MinBoostFactor = GHE_MIN_BOOST_FACTOR;
        
        /* Slope limits for histogram bin clamping computation */
        pCfg->MaxSlope = GHE_IET_MAX_SLOPE;    /* 4000 */
        pCfg->MinSlope = GHE_IET_MIN_SLOPE;    /* 500 */
        
        pCfg->AggressivenessFactor = 0.8;
        
        /* Compute histogram bin limits based on frame size and slopes */
        double FrameSize = (double)ImageSize;
        double DivisionFactor = (double)GHE_SLOPE_PRECISION_FACTOR * (double)XPST_MAX_BIN_INDEX;
        uint32_t BinLowerLimit = (uint32_t)(((double)pCfg->MinSlope * FrameSize) / DivisionFactor);
        uint32_t BinUpperLimit = (uint32_t)(((double)pCfg->MaxSlope * FrameSize) / DivisionFactor);
        uint32_t AvgBinSize = ImageSize / XPST_BIN_COUNT;
        uint32_t MaxSlopeSearchStep = (uint32_t)((BinUpperLimit - AvgBinSize) / (double)GHE_NUM_STEPS_MAX_LIMIT_SEARCH);

        /* One lower limit for all bins */
        pCfg->MinHistBinCount = BinLowerLimit;

        /* Generate bank of upper limits for bin clamping search */
        pCfg->MaxHistBinCounts[0] = BinUpperLimit;
        for (int i = 1; i < GHE_NUM_STEPS_MAX_LIMIT_SEARCH; i++)
                pCfg->MaxHistBinCounts[i] = BinUpperLimit - i * MaxSlopeSearchStep;

        /* De-gamma LUT: sRGB linear-light decoding (SDR mode)
         * Standard sRGB EOTF: 
         *   if (v <= 0.04045): v / 12.92
         *   else: pow((v + 0.055) / 1.055, 2.4)
         */
        for (int i = 0; i < XPST_BIN_COUNT; i++) {
                double norm = (double)i / (double)(XPST_BIN_COUNT - 1);
                double linear;
                
                if (norm <= 0.04045) {
                        linear = norm / 12.92;
                } else {
                        linear = pow((norm + 0.055) / 1.055, 2.4);
                }
                
                pDpstContext->Algorithm.XpstAlgorithmStaticData.DeGammaLUT[i] = linear;
        }

        DisplayGheAlgorithm(pDpstContext);

        for (int i = 0; i < XPST_IET_LUT_LENGTH; i++)
                gheargs->ietlutentries[i] = pDpstContext->Algorithm.XpstAlgorithmDynamicData.MultiplierLutTarget[i];

        free(pDpstContext);
}
