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

#define GLOBALHIST_BIN_COUNT 32   // Total number of segments in GlobalHist
#define GLOBALHIST_IET_LUT_LENGTH 33 // Total number of IET entries

enum pipe_id {
	GlobalHist_PIPE_A = 0,
	GlobalHist_PIPE_B = 1,
	GlobalHist_PIPE_C = 2,
	GlobalHist_PIPE_D = 3
};

struct globalhist_args {
	enum pipe_id pipeid;
	bool isprogramdiet;
	uint32_t dietfactor[GLOBALHIST_IET_LUT_LENGTH];
	uint32_t histogram[GLOBALHIST_BIN_COUNT];
	uint32_t resolution_x;
	uint32_t resolution_y;
};

void set_histogram_data_bin(struct globalhist_args *gheargs);
#endif
