/* SPDX-License-Identifier: MIT
 *
 * Copyright © 2024 Intel Corporation
 */

#ifndef GHE_H_
#define GHE_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define XPST_BIN_COUNT 32
#define XPST_IET_LUT_LENGTH 33

struct globalhist_args {
	uint8_t histogrammode;
	uint32_t binscount;
	uint32_t ietlutentries[XPST_IET_LUT_LENGTH];
	uint32_t histogram[XPST_BIN_COUNT];
	uint32_t resolution_x;
	uint32_t resolution_y;
};

void histogram_compute_generate_data_bin(struct globalhist_args *gheargs);
#endif
