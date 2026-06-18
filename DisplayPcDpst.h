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

#define DD_MAX(a, b) ((a) < (b) ? (b) : (a))
#define DD_MIN(a, b) ((a) < (b) ? (a) : (b))
#define DD_CLAMP_MIN_MAX(a, min, max) ((a) < (min) ? (min) : DD_MIN((a), (max)))
#define SOLID_COLOR_SEARCH_WINDOW_SIZE 4
#define SOLID_COLOR_POWER_THRESHOLD 0.95

#define TRUE true
#define FALSE false

// XPST Common algorithm constants
#define XPST_BIN_COUNT 32
#define XPST_IET_LUT_LENGTH 33 // Total number of IET entries
#define XPST_MAX_BIN_INDEX (XPST_BIN_COUNT - 1)    // Index of last histogram bin
#define XPST_IET_SCALE_FACTOR 512                    // IET factor programmed in 1.9 format, so fraction shift is (1 << 9)
#define XPST_IET_MAX_VAL 1023                        // IET values are in 1.9 format (1 bit integer, 9 bit fraction)
#define XPST_MAX_IET_INDEX (XPST_IET_LUT_LENGTH - 1) // Index of last histogram bin
// GHE Algorithm constants
#define GHE_SLOPE_PRECISION_FACTOR 1000 // Slope precision factor
#define GHE_IET_MIN_SLOPE (0.5 * GHE_SLOPE_PRECISION_FACTOR)
#define GHE_IET_MAX_SLOPE (4 * GHE_SLOPE_PRECISION_FACTOR)
#define GHE_MAX_BOOST_FACTOR (((double)XPST_IET_MAX_VAL / (double)XPST_IET_SCALE_FACTOR) * GHE_SLOPE_PRECISION_FACTOR)
#define GHE_MIN_BOOST_FACTOR 1.0 * GHE_SLOPE_PRECISION_FACTOR
#define GHE_NUM_STEPS_MAX_LIMIT_SEARCH 32

typedef struct _XPST_ALGORITHM_STATIC_DATA
{
    uint32_t ImageSize; // Source image size (pixels)
    double DeGammaLUT[XPST_BIN_COUNT];
} XPST_ALGORITHM_STATIC_DATA;

typedef struct _XPST_ALGORITHM_DYNAMIC_DATA
{
    uint32_t Histogram[XPST_BIN_COUNT]; // Bin wise histogram data for current frame.
    uint32_t MultiplierLutTarget[XPST_IET_LUT_LENGTH];
} XPST_ALGORITHM_DYNAMIC_DATA;

typedef struct _XPST_ALGORITHM_PARAMS
{
    XPST_ALGORITHM_DYNAMIC_DATA XpstAlgorithmDynamicData;
    XPST_ALGORITHM_STATIC_DATA XpstAlgorithmStaticData;
} XPST_ALGORITHM_PARAMS;

typedef struct _GHE_CONFIG
{
    uint16_t MaxSlope;         // Derived via experiments
    uint16_t MinSlope;         // Derived via experiments
    uint16_t MaxBoostFactor;   // Derived via experiments
    uint16_t MinBoostFactor;   // Derived via experiments
    uint32_t MinHistBinCount;
    uint32_t MaxHistBinCounts[GHE_NUM_STEPS_MAX_LIMIT_SEARCH];
    double AggressivenessFactor;
} GHE_CONFIG;

typedef struct _GHE_PARAMS
{
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
    GHE_CONFIG GheCfg;
    GHE_PARAMS GheParams;
} DD_GHE_ARGS;

typedef struct _DISPLAY_PC_XPST_CONTEXT
{
    XPST_ALGORITHM_PARAMS Algorithm;
    DD_GHE_ARGS GheArgs;
} DISPLAY_PC_XPST_CONTEXT;


// Algorithm supporting functions
void CreateUnityIET(uint32_t *pMultiplierLut);
double DetectFullScreenSolidColor(double *pPowerHistogram, double TotalPower);

// GHE
void DisplayGheAlgorithm(DISPLAY_PC_XPST_CONTEXT *pDpstContext);
