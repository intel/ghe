#include "ghe.h"
#include "stdio.h"
#include <inttypes.h>

uint32_t lut_data[32] = {5000, 8000, 12000, 20000, 35000, 55000, 80000, 110000, 145000, 175000, 200000, 215000, 220000, 215000, 200000, 175000, 145000, 110000, 80000, 55000, 35000, 20000, 12000, 8000, 5000, 3000, 2000, 1500, 1000, 800, 500, 200};

struct globalhist_args user;

int main()
{
	user.binscount = 32;
	user.histogrammode = 1; // DRM_MODE_HISTOGRAM_HSV_MAX_RGB
	user.resolution_x = 1920;
	user.resolution_y = 1080;

	for (int i=0; i< XPST_BIN_COUNT; i++)
		user.histogram[i] = lut_data[i];

	for (int i=0; i< XPST_BIN_COUNT; i++)
		printf("bin[%2d] = %u\n", i, user.histogram[i]);

	histogram_compute_generate_data_bin(&user);

	printf("\nIET LUT output (%d entries):\n", 33);
	for (int i=0; i< XPST_IET_LUT_LENGTH; i++)
                printf("iet[%2d] = %u\n", i, user.ietlutentries[i]);

	return 0;
}
