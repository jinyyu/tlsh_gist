extern "C"
{
#include "postgres.h"
#include "fmgr.h"
}

#define TLSH_HASH_LENGTH 70
#define TLSH_INTERNAL_LENGTH 35

#define DistanceStrategyNumber 1 // <->

extern int32 tlsh_dist_threshold;

extern void from_hex(const char *psrc, int len, unsigned char *pdest);
extern void to_hex(unsigned char *psrc, int len, char *pdest);
