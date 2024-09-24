
#include "mwc.h"

stress_mwc_t mwc = {
	STRESS_MWC_SEED_W,
	STRESS_MWC_SEED_Z
};

uint32_t stress_mwc32(void)
{
	mwc.z = 36969 * (mwc.z & 65535) + (mwc.z >> 16);
	mwc.w = 18000 * (mwc.w & 65535) + (mwc.w >> 16);
	return (mwc.z << 16) + mwc.w;
}

uint64_t stress_mwc64(void)
{
	return (((uint64_t)stress_mwc32()) << 32) | stress_mwc32();
}

uint64_t stress_mwc64modn_nonzero(const uint64_t max)
{
	register uint64_t threshold = max;
	register uint64_t val;

	if ((max & (max - 1)) == 0)
		return stress_mwc64() & (max - 1);

	while (threshold < 0x8000000000000000ULL) {
		threshold <<= 1;
	}
	do {
		val = stress_mwc64();
	} while (val >= threshold);

	return val % max;
}

uint64_t stress_mwc64modn(const uint64_t max)
{
	return (max > 0) ? stress_mwc64modn_nonzero(max) : 0;
}