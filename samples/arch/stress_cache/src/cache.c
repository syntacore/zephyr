
#include "stress.h"

/**
 *  @brief flush contents of instruction and/or data cache
 */
int shim_cacheflush(char *addr, int nbytes, int cache)
{
	if (cache == SHIM_ICACHE) {
		ifence();
		return 0;
	}
	if (cache == SHIM_DCACHE) {
		fence();
		return 0;
	}
	(void)cache;

	cache_data_flush_range((void *)addr, nbytes);
	return 0;
}
