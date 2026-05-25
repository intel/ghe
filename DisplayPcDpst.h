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
 * @file  DisplayPcDpst.h
 * @brief  This file contains all the Display Pc Dpst related interface functions
 *
 */
#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define BIT_ULL(n) (1ULL << n)
#define DD_MAX(a, b) ((a) < (b) ? (b) : (a))
#define DD_MIN(a, b) ((a) < (b) ? (a) : (b))
#define DD_CLAMP_MIN_MAX(a, min, max) ((a) < (min) ? (min) : DD_MIN((a), (max)))
#define DD_ABS(x) ((x) < 0 ? -(x) : (x))

#define DPST_DEFAULT_BOOST 1.0
#define DD_BLC_PWM_LOW_PRECISION_FACTOR 100
#define SOLID_COLOR_SEARCH_WINDOW_SIZE 4
#define SOLID_COLOR_POWER_THRESHOLD 0.95

#ifndef IN
#define IN
#endif

#define TRUE true
#define FALSE false

// XPST Common algorithm constants
#define XPST_BIN_COUNT 32
#define XPST_IET_LUT_LENGTH 33 // Total number of IET entries
#define GHE_MAX_NUM_ENTRIES_IN_LUX_AGGR_MAP 129     // Maximum number of Entries supported in LuxVsAggr map
#define XPST_MAX_BIN_INDEX (XPST_BIN_COUNT - 1)    // Index of last histogram bin
#define XPST_ENTRIES_PER_BIN 8                     // Number of entries in each bin
#define XPST_GUARDBAND_INTERRUPT_DELAY 4           // Guardband interrupt delay
#define XPST_LOW_THRESHOLD_GUARDBAND_MAX_LIMIT 300 // 12.0% of the pipe's current pixel count. HW does x4 of programmed value
#define XPST_THRESHOLD_GUARDBAND_24BIT (XPST_LOW_THRESHOLD_GUARDBAND_MAX_LIMIT >> 2)
#define XPST_HIGH_THRESHOLD_GUARDBAND_DEFAULT 1000   // 10.0% of the pipe's current pixel count
#define XPST_IET_SCALE_FACTOR 512                    // IET factor programmed in 1.9 format, so fraction shift is (1 << 9)
#define XPST_IET_MAX_VAL 1023                        // IET values are in 1.9 format (1 bit integer, 9 bit fraction)
#define XPST_MAX_IET_INDEX (XPST_IET_LUT_LENGTH - 1) // Index of last histogram bin
#define XPST_THRESHOLD_GUARDBAND_CURRENT XPST_THRESHOLD_GUARDBAND_24BIT
#define XPST_MAX_BIN_INDEX_DOUBLE (double)(XPST_BIN_COUNT - 1)
#define XPST_IIR_FILTER_ORDER 3                // Default third order filter
#define XPST_SMOOTHENING_MIN_SPEED_DEFAULT 500 // Default XPST temporal filter Min cutoff frequency in Milli Hz
#define XPST_SMOOTHENING_MAX_SPEED_DEFAULT 800 // Default XPST temporal filter Min cutoff frequency in Milli Hz
#define XPST_SMOOTHENING_SPEED_1000 1000       // Default XPST temporal filter frequency in Milli Hz ~1s
#define XPST_DCE_SPEED_MULTIPLIER_MIN 1
#define XPST_DCE_SPEED_MULTIPLIER_MAX 2
#define XPST_DCE_MIN_BRIGHTNESS 10                  // limit min brightness percent to 10%
#define XPST_DCE_MAX_BRIGHTNESS 100                 // limit max brightness percent to 100%
#define XPST_SMOOTHENING_TOLERANCE_DEFAULT 50       // Default Minimum step percent to stop the timer. In 100 * percent
#define XPST_SMOOTHENING_INF_RANGE_FACTOR 3         // INF Range - Value can be > Default / 3 or < Default * 3
#define XPST_MAX_HISTOGRAM_CHANGE_FACTOR_DOUBLE 2.0 // On frame change pixels getting reduced from one bin are added to another
#define XPST_MAX_HISTOGRAM_CHANGE_PERCENT_DOUBLE 100.0
#define XPST_GUARDBAND_INTERRUPT_DELAY_FOR_PSR2 1 // Guardband interrupt delay value while MBO or PSR2
#define XPST_FIRST_INDEX_IN_BIN(i) (XPST_ENTRIES_PER_BIN * (i))
#define XPST_LAST_INDEX_IN_BIN(i) (XPST_FIRST_INDEX_IN_BIN(i) + XPST_ENTRIES_PER_BIN - 1)
#define XPST_BLC_MAX_VALUE (100 * DD_BLC_PWM_LOW_PRECISION_FACTOR)
// Bpc support mask
#define XPST_IS_6BPC_PANEL(BpcMask)                                                                                                                                     \
    ((0 == BpcMask.Supports8BitsPerColor) && (0 == BpcMask.Supports10BitsPerColor) && (0 == BpcMask.Supports12BitsPerColor) && (0 == BpcMask.Supports14BitsPerColor) && \
     (0 == BpcMask.Supports16BitsPerColor))
// XST + FSSC
#define XPST_SOLID_COLOR_THRESHOLD_DEFAULT_PERCENT 98.0 // Threshold defined for FSSC. An image with >= 98% of pixels accumulated in two consecutive bins is considered as a full screen solid color.
#define XPST_MAX_NON_SOLID_PIXEL_PERCENT (100.0 - XPST_SOLID_COLOR_THRESHOLD_DEFAULT_PERCENT)

// DPST Common algorithm Constants
#define DPST_SLOPE_PRECISION_FACTOR 1000   // Slope precision factor
#define DPST_HIGHLIGHT_THRESHOLD_DEFAULT 8 // Default Highlight pixel threshold permil
// DPST min and max aggressiveness levels (0-5) within driver
// corresponding to CUI (1-6)
#define LFP_DEFAULT_PANEL_GAMMA_IN_HUNDREDS 220 // Default LFP Panel Gamma Value in hundreds precision

// DIET Adjustment status
#define INIT_DIET_ADJ_STATUS (BIT_ULL(XPST_IET_LUT_LENGTH) - 1)
#define DIET_ADJ_DONE(i) (~(BIT_ULL(i)))

// Half transition length of bezier curve in case of FSSC. Considering 4 minimum points to join the half curve in a smooth way. Also 4 points are used to push solid bin to linear region
// so that the pixels are restored completely without any distortion.
#define DPST_MIN_HALF_WINDOW_SIZE_FOR_BEZIER 4
#define DPST_MAX_BRIGHT_BIN_INDEX_POSSIBLE_FSSC (XPST_MAX_BIN_INDEX - (2 * DPST_MIN_HALF_WINDOW_SIZE_FOR_BEZIER + 1)) // Max bright bin index possible in case full screen solid color.
#define DPST_MIN_SHADOW_SLOPE_SEARCH_STEP_SIZE (1.0 / (double)DPST_NUM_STEPS_TO_FIND_OPTIMAL_LOWER_SLOPE)

// To map DPST UI aggressiveness level 1-based index values to DPST array 0-based index values
#define DPST_GET_AGGR_LEVEL_INDEX_FROM_UI_LEVELS_8_0(AggrLevel) ((AggrLevel > 0 && AggrLevel <= DPST_8_0_UI_LEVEL_MAX) ? (AggrLevel - 1) : 0)

// RGB weights for Luma histogram
// MTL HW follows BT601 standard. And this is not programmable.
// From LNL, HW gives an option for driver to program the R, G and B weights. The default value from LNL follows
// BT709 standard. To keep the behavior on par with MTL, values will now be programmed as per BT601 standard and
// can be fine tuned later by doing power measuremnts and VQ testing.
// BT601 standard: Red channel coef: 0.299 , Green channel coef: 0.587 , Blue channel coef: 0.114
// All coefficients have a precision of 0.9. Coefficients are multiplied by 2^9 to get programming value.
#define DISPLAY_PC_OPST_LUMA_HIST_RED_COEFFICIENT 153
#define DISPLAY_PC_OPST_LUMA_HIST_GREEN_COEFFICIENT 301
#define DISPLAY_PC_OPST_LUMA_HIST_BLUE_COEFFICIENT 58

// GHE Algorithm constants
#define GHE_SLOPE_PRECISION_FACTOR 1000 // Slope precision factor
#define GHE_IET_MIN_SLOPE (0.5 * GHE_SLOPE_PRECISION_FACTOR)
#define GHE_IET_MAX_SLOPE (4 * GHE_SLOPE_PRECISION_FACTOR)
#define GHE_MAX_BOOST_FACTOR (((double)XPST_IET_MAX_VAL / (double)XPST_IET_SCALE_FACTOR) * GHE_SLOPE_PRECISION_FACTOR)
#define GHE_MIN_BOOST_FACTOR 1.0 * GHE_SLOPE_PRECISION_FACTOR
#define GHE_NUM_STEPS_MAX_LIMIT_SEARCH 32
#define GHE_DEFAULT_NUM_ENTRIES_IN_LUX_AGGR_MAP 11 // Driver supported default number of entries in GHE LuxVsAggr map
#define GHE_INVALID_LUX_THRESOLD MAX_32BIT_VALUE

typedef enum _XPST_EVENT
{
    XPST_EVENT_DPST_PHASEOUT_STARTED = 1,
    XPST_EVENT_NO_PHASEOUT,
    XPST_EVENT_DPST_PHASEOUT_COMPLETED,
    XPST_EVENT_ENABLED,
    XPST_EVENT_NO_ALGO_RUN_REQUIRED,
    XPST_EVENT_NO_CHANGE_IN_GAMMA_ADJUST,
    XPST_EVENT_DISABLED
} XPST_EVENT;

// DPST Event Handler parameters
typedef enum _XPST_OPERATION
{
    XPST_OPERATION_UNKNOWN = 0,
    XPST_OPERATION_ALS     = 2,
    XPST_OPERATON_BLC,
    XPST_OPERATION_SET_GUARDBAND_INTERRUPT_DELAY,
    XPST_OPERATION_BLC_OPTIMIZATION_CHANGE,
    XPST_OPERATION_RESTORE_GUARDBAND_INTERRUPT_DELAY_COUNTER,
    XPST_OPERATION_SW_TRIGGERED,
    XPST_OPERATION_UPDATE_SFSU_DPST_HANDLING,
    XPST_OPERATION_GHE,
    XPST_OPERATION_DPST,
    XPST_OPERATION_HISTOGRAM_INTERRUPT_TOGGLE,
    XPST_OPERATION_MAX
} XPST_OPERATION;

typedef enum _DPST_ALS_LUX_RANGE
{
    DPST_LUX_AT_BELOW_T0,
    DPST_LUX_ABOVE_T0
} DPST_ALS_LUX_RANGE;


typedef struct _XPST_ALGORITHM_STATIC_DATA
{
    uint32_t ImageSize; // Source image size (pixels)
    double DeGammaLUT[XPST_BIN_COUNT];
    double GammaLUT[XPST_BIN_COUNT];
    uint32_t FSSCPixelCountThreshold;
    bool PlatformSupportForFssc;
} XPST_ALGORITHM_STATIC_DATA;

typedef struct _XPST_ALGORITHM_DYNAMIC_DATA
{
    uint32_t Histogram[XPST_BIN_COUNT]; // Bin wise histogram data for current frame.
    uint32_t PrevHistogram[XPST_BIN_COUNT];
    double HistogramChangePercentage;
    double NormalizedHistogram[XPST_BIN_COUNT];
    double TargetBoost;
    uint16_t BacklightAdjust; // Backlight adjustment
    union
    {
        uint32_t MultiplierLutTarget[XPST_IET_LUT_LENGTH]; // UnfilteredTarget for IIR based phase in - used in DPST v7+ algorithms.
        uint32_t NewIetFactor[XPST_IET_LUT_LENGTH];        // New Image enhancement factor - used in DPST 6.3 version.
    };

    bool PhaseOutInProgress; // Indicates whether the current DPST adjustments are getting phased out.

    // TODO: Current Lut/ brightness values to be moved to Phasemanager context.
    union
    {
        uint32_t MultiplierLutCurrent[XPST_IET_LUT_LENGTH]; // New Image enhancement factor - used in DPST v7+ algorithms.
        uint32_t CurrentIetFactor[XPST_IET_LUT_LENGTH];     // Old Image enhancement factor - used in DPST 6.3 version.
    };
} XPST_ALGORITHM_DYNAMIC_DATA;

typedef struct _XPST_ALGORITHM_PARAMS
{
    XPST_ALGORITHM_DYNAMIC_DATA XpstAlgorithmDynamicData;
    XPST_ALGORITHM_STATIC_DATA XpstAlgorithmStaticData;
} XPST_ALGORITHM_PARAMS;

typedef enum _PIPE_ID  {
	NULL_PIPE = 0x7F,
	PIPE_A = 0,
	PIPE_B = 1,
	PIPE_C = 2,
	MAX_INTEL_PIPES = 3
} PIPE_ID;

typedef struct _XPST_GAMMA_SIG
{
    union
    {
        uint64_t Data; // A 64-bit encoding of the following fields
        struct
        {
            uint64_t V1 : 8; // 1st Value
            uint64_t V2 : 8; // 2nd Value
            uint64_t V3 : 8; // 3rd Value
            uint64_t V4 : 8; // 4th Value
            uint64_t V5 : 8; // 5th Value
            uint64_t V6 : 8; // 6th Value
            uint64_t V7 : 8; // 7th Value
            uint64_t V8 : 8; // 8th Value
        };
    };
} XPST_GAMMA_SIG;

typedef struct _DISPLAY_XPST_ALS_PARAM
{
    // DPST ALS Thresholds
    uint32_t Lux; // Current ALS lux value.
} DISPLAY_XPST_ALS_PARAM;

typedef struct _DISPLAY_XPST_GUARDBAND_PARAM
{
    bool RfbUpdateMode; // Flag to check if Media playback if active while in PSR2 mode
} DISPLAY_XPST_GUARDBAND_PARAM;

typedef struct _DISPLAY_XPST_EVENT_PARAMS
{
    XPST_OPERATION Operation;
    PIPE_ID PipeId;
    union
    {
        DISPLAY_XPST_ALS_PARAM AlsParam;
        DISPLAY_XPST_GUARDBAND_PARAM GuardbandParam;
        bool IsSfsuDpstHandlingNeeded;
        bool InvokeSwDpst;
        bool IsGheEnabled;
        bool IsHistogramInterruptEnable;
    };
} DISPLAY_XPST_EVENT_PARAMS;

// XPST Algorithm function pointer.
typedef void (*PFN_XPSTSETUPIEPHASEOUT)(IN void *);
typedef void (*PFN_XPSTRESETALGORITHM)(IN void *);
typedef void (*PFN_XPSTGETIETADJUST)(IN void *, IN void *);
typedef void (*PFN_OPSTFILLALGOPARAMS)(IN void *, IN uint32_t);
typedef void (*PFN_OPSTCALCPOWERCURVEANDDIMMINGLUT)(IN void *);
// XPST Eventhandler function pointer.
typedef void (*PFN_DPSTEVENTHANDLER)(IN void *);

// GHE Algorithm function pointer.
typedef void (*PFN_GHEALGORITHM)(IN void *, IN PIPE_ID, IN bool, IN void *);

// Function table for DPST functions.
typedef struct _XPST_FUNCTBL
{
    PFN_DPSTEVENTHANDLER pDpstEventHandler;
    PFN_XPSTSETUPIEPHASEOUT pXpstResetTargetParams;
    uint32_t *pXpstSetAdjustments;
    PFN_XPSTRESETALGORITHM pXpstResetAlgorithm;
    PFN_XPSTGETIETADJUST pXpstGetIetAdjust;
    PFN_OPSTFILLALGOPARAMS pOpstFillAlgoParams;
    PFN_OPSTCALCPOWERCURVEANDDIMMINGLUT pCalculatePowerCurveAndDimmingLUT;
} XPST_FUNCTBL;

typedef struct _XPST_TEMPORAL_FILTER_PARAMS
{
    uint64_t SmootheningIteration;                     // TODO: Remove and depend on ElapsedTime
    uint64_t IterationInIncreasingCutoffFreqState;     // TODO: Remove and depend on IncreasingCutoffFrequencyTimeStamp
    uint32_t MinCutOffFreqInMilliHz;                   // Value from the INF or Default
    uint32_t MaxCutOffFreqInMilliHz;                   // Value from the INF or Default
    uint32_t CurrentMinCutOffFreqInMilliHz;
    uint32_t CurrentMaxCutOffFreqInMilliHz;
    double IETHistory[XPST_IET_LUT_LENGTH][XPST_IIR_FILTER_ORDER];
    double BlcHistory[XPST_IIR_FILTER_ORDER];
    double MinimumStepPercent;
    double AdaptiveCutOffFreq;
} XPST_TEMPORAL_FILTER_PARAMS;

typedef struct _DD_LUX_AGGR_MAP_ENTRY
{
    uint32_t Lux;                  //  Ambient lux
    uint8_t AggressivenessPercent; //  Pixel boost Aggressiveness Percent
} DD_LUX_AGGR_MAP_ENTRY;

typedef enum _GHE_TRIGGER {
    GHE_TRIGGER_INVALID             =0,
    GHE_TRIGGER_AMBIENT_LIGHT       =1, // GHE enhancement depends on Ambient Light
    GHE_TRIGGER_FIXED_AGGRESSIVENESS =2, // GHE enhancement is as per given Fixed Aggressiveness Percent
}GHE_TRIGGER;

typedef struct _GHE_TRIGGER_CONFIG
{
    GHE_TRIGGER Trigger; // GHE mode to be triggered in
    uint32_t NumEntries;    // Number of entries in the given LuxVsAggr mapping table and applicable for GHE_TRIGGER_AMBIENT_LIGHT
    union
    {
        uint8_t FixedAggressivenessPercent;                                                  // Fixed aggressiveness level, applicable for DD_GHE_TRIGGER_FIXED_AGGRESSIVENESS
        DD_LUX_AGGR_MAP_ENTRY LuxToAggrMappingTable[GHE_MAX_NUM_ENTRIES_IN_LUX_AGGR_MAP]; // Lux to enhancement mapping table, applicable for DD_GHE_TRIGGER_AMBIENT_LIGHT
    };
} GHE_TRIGGER_CONFIG;

typedef struct _GHE_CONFIG
{
    bool IsGheSupportedByPanel;
    bool IsGheSupported; // Depends on SDR/HDR restriction, pannel support (CABC/6 bpc restriction) and primary lfp restriction
    uint16_t MaxSlope;         // Derived via experiments
    uint16_t MinSlope;         // Derived via experiments
    uint16_t MaxBoostFactor;   // Derived via experiments
    uint16_t MinBoostFactor;   // Derived via experiments
    uint32_t MinHistBinCount;
    uint32_t MaxHistBinCounts[GHE_NUM_STEPS_MAX_LIMIT_SEARCH];
    GHE_TRIGGER_CONFIG GheTriggerConfig;
    double AggressivenessFactor;
    bool IsGheConfigChanged; // Indicates if lux or trigger config changed
} GHE_CONFIG;

typedef struct _GHE_RESTRICTIONS
{
    union
    {
        uint8_t Restrictions;
        struct
        {
            uint8_t InvalidData : 1;
            uint8_t DisableAdvancedColorProcessing : 1;
            uint8_t Reserved : 6;
        };
    };
} GHE_RESTRICTIONS;

typedef struct _GHE_PARAMS
{
    uint32_t GheTriggerLuxThreshold;
    uint8_t NumBinsBelowMinHistogramBinCount;
    uint32_t NumPixelsBelowMinHistogramBinCount;
    uint32_t NumPixelsAboveMaxHistogramBinCount;
    uint32_t MaxHistBinCount;
    double PowerDistribution[XPST_BIN_COUNT];
    double NormalizedCDF[XPST_IET_LUT_LENGTH];
    double EnhancementTable[XPST_IET_LUT_LENGTH];
    double SmoothenedTable[XPST_IET_LUT_LENGTH];
} GHE_PARAMS;

typedef struct _DD_GHE_ARGS
{
    uint32_t *pGheAlgorithm;
    GHE_CONFIG GheCfg;
    GHE_PARAMS GheParams;
    bool IsGheActiveAndNeedFullCorrLutToComputeIetPhase; // Flag to indicate if GHE is active and needs full Correction Lut to calculate next phase of GHE IET.
    bool PipeRestrictionsSatisfied;                      // GHE Pipe Restrictions
} DD_GHE_ARGS;

typedef struct _DD_DCE_PARAMS
{
    uint32_t TargetBrightness;
    bool Enable;
    bool IsSupported;
} DD_DCE_PARAMS;

typedef enum _DD_XPST_REG_OPERATION
{
    XPST_REG_OPERATION_UNDEFINED        = 0,
    XPST_REG_OPERATION_HIST_IET_ENABLE  = 1,
    XPST_REG_OPERATION_HIST_IET_DISABLE = 2,
    XPST_REG_OPERATION_HIST_ENABLE      = 3,
    XPST_REG_OPERATION_HIST_DISABLE     = 4,
} XPST_REG_OPERATION;

typedef struct _DISPLAY_PC_CONTEXT
{
        uint8_t BDFData[4];

} DISPLAY_PC_CONTEXT;

typedef struct _DISPLAY_PC_XPST_CONTEXT
{
    // Hardware dependent variables //
    uint32_t GuardbandIntrDelay_PSR2;   // Guardband Interrupt delay while MBO or PSR2 video playback is active
    XPST_FUNCTBL XpstFuncTable;      // DPST Algorithm Function Table
    bool IsDpstSupportedByPanel;
    bool IsSourceSideDitheringSupported;
    bool DisableDpstOn6BpcPanel; // By default DPST is disabled on 6BPC panel and override is possible. For features other than DPST, no support on panels with BPP <= 6
    PIPE_ID Pipe;
    bool IsSecondaryInternalTarget;
    uint32_t LUT[XPST_BIN_COUNT];
    XPST_ALGORITHM_PARAMS Algorithm;
    bool EnableComplete; // DPST enable completion state

    // Runtime variables

    uint32_t Dpst6_FactorScalar[XPST_IET_LUT_LENGTH]; // Factor Scalar
    uint64_t Dpst6_DietAdjNotDone;                    // Flag for 33 bin to check if Lumma adjust done for specific bin while DPST phase-in
    bool Xpst_IsTargetIetReached;

    XPST_GAMMA_SIG LastSignature;              // Last gamma signature
    bool ApplyBkltCeiling;
    uint16_t PanelGammaInHundreds;
    uint32_t CurrAggressivenessLevel;
    bool EpsmRunningState; // (Enhanced Power Savings Mode) Indicates application of backlight ceiling for DPST 6.3
    bool IsEpsmSupported;
    bool IsSwXpstControlTrigger;
    uint32_t LowThresholdGuardBand;
    bool EnableDithering;
    bool IsDPSTConfigChanged;
    double SolidColorThreshold;
    bool IsFullScreenSolidColor;

    XPST_TEMPORAL_FILTER_PARAMS FilterParams;

    DD_GHE_ARGS GheArgs;

    DD_DCE_PARAMS Dce;

    // Pointers to useful contexts //
    DISPLAY_PC_CONTEXT *pDisplayPcContext; // Back Pointer to Power Conservation Context.
    double DpstEpsmWeight;

    bool IsSfsuDpstHandlingNeeded;
    uint32_t MaxSupportedImageSize;
    uint8_t XpstSmootheningPeriodInMilliSec;
} DISPLAY_PC_XPST_CONTEXT;


// XPST/algorithm supporting function
void CreateUnityIET(uint32_t *pMultiplierLut);
void DisplaySetXpstRegsState(DISPLAY_PC_CONTEXT *pDisplayPcContext, XPST_REG_OPERATION Operation, bool IsImmediatePhaseOut, bool EnableHwSfSuDpst, PIPE_ID Pipe);
uint8_t DisplayReadXpstSmootheningPeriod(DISPLAY_PC_CONTEXT *pDisplayPcContext, PIPE_ID PipeId);
double DetectFullScreenSolidColor(double *pPowerHistogram, double TotalPower);

// DPST Avg Luminance functions
bool DisplayDpstDceSaveHistogram(DISPLAY_PC_XPST_CONTEXT *pDpstContext, PIPE_ID Pipe, bool IsValidHistogram, uint32_t *pHistogramData);

void DisplayDpstResetTargetParams_Dce_v1_0(DISPLAY_PC_XPST_CONTEXT *pDpstContext);

void DisplayDpstResetDcePhaseInParams(DISPLAY_PC_XPST_CONTEXT *pDpstContext);
void DisplayPcDpstPipePhaseAdjustComplete(DISPLAY_PC_CONTEXT *pDisplayPcContext, uint8_t Pipe);

// OPST
bool DisplayOpstAlgorithm(DISPLAY_PC_XPST_CONTEXT *pDpstContext, PIPE_ID Pipe, bool IsValidHistogram, uint32_t *pHistogramData);
void CalculatePowerCurveAndDimmingLUTOpst_v1_0(DISPLAY_PC_XPST_CONTEXT *pDpstContext);
void CalculatePowerCurveAndDimmingLUTOpst_v1_1(DISPLAY_PC_XPST_CONTEXT *pDpstContext);
void DisplayOpstFillAlgoParams_Opst_v1_0(DISPLAY_PC_XPST_CONTEXT *pDpstContext, uint32_t NewAggrLevel);
void DisplayOpstFillAlgoParams_Opst_v1_1(DISPLAY_PC_XPST_CONTEXT *pDpstContext, uint32_t NewAggrLevel);
uint32_t DisplayOpstGetAggressiveness(DISPLAY_PC_CONTEXT *pDisplayPcContext, PIPE_ID Pipe);

// OPST 2.0
bool IsFullScreenSolidColorHdr(uint32_t *pHistogram, uint32_t ImageSize);
uint8_t GetHighestPopulatedBinIndex(DISPLAY_PC_XPST_CONTEXT *pDpstContext);
void CalculateDimmingLUT_Opst_v2_0(DISPLAY_PC_XPST_CONTEXT *pDpstContext);
void DisplayOpstFillAlgoParams_Opst_v2_0(DISPLAY_PC_XPST_CONTEXT *pDpstContext, uint32_t NewAggrLevel);

// GHE
void DisplayPcGheEventCallback(DISPLAY_PC_CONTEXT *pDisplayPcContext, uint8_t PipeId);
void DisplayGheAlgorithm(DISPLAY_PC_XPST_CONTEXT *pDpstContext);
