
#include <zephyr/kernel.h>
#include <stdint.h>

#ifndef	_MWC_H
#define	_MWC_H
#endif

#define STRESS_MWC_SEED_Z	(362436069UL)
#define STRESS_MWC_SEED_W	(521288629UL)

typedef struct {
	uint32_t w;
	uint32_t z;
} stress_mwc_t;

uint32_t stress_mwc32(void);
uint64_t stress_mwc64(void);
uint64_t stress_mwc64modn_nonzero(const uint64_t max);
uint64_t stress_mwc64modn(const uint64_t max);
