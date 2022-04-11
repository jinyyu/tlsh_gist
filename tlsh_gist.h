#ifndef TLSH_GIST_H
#define TLSH_GIST_H

extern "C"
{
#include "postgres.h"
#include "fmgr.h"
}

#define TLSH_HASH_LENGTH 70
#define TLSH_INTERNAL_LENGTH 35

#define TLSHEqualStrategyNumber 1    // =
#define TLSHDistanceStrategyNumber 2 // <->

extern int32 tlsh_dist_threshold;



#define RANGE_LVALUE 256
#define RANGE_QRATIO 16

#ifndef BUCKETS_128
#define BUCKETS_128
#endif

#if defined BUCKETS_128
  #define EFF_BUCKETS         128
  #define CODE_SIZE           32   // 128 * 2 bits = 32 bytes
  #if defined CHECKSUM_3B
    #define INTERNAL_TLSH_STRING_LEN 74
    #define TLSH_CHECKSUM_LEN 3
    // defined in tlsh.h   #define TLSH_STRING_LEN   74   // 2 + 3 + 32 bytes = 74 hexidecimal chars
  #else
    #define INTERNAL_TLSH_STRING_LEN 70
    #define TLSH_CHECKSUM_LEN 1
    // defined in tlsh.h   #define TLSH_STRING_LEN   70   // 2 + 1 + 32 bytes = 70 hexidecimal chars
  #endif
#endif


typedef    struct lsh_bin_struct {
        unsigned char checksum[TLSH_CHECKSUM_LEN];  // 1 to 3 bytes
        unsigned char Lvalue;                       // 1 byte
        union {
#if defined(__SPARC) || defined(_AIX)
		#pragma pack(1)
#endif
        unsigned char QB;
            struct{
#if defined(__SPARC) || defined(_AIX)
		unsigned char Q2ratio : 4;
		unsigned char Q1ratio : 4;
#else
                unsigned char Q1ratio : 4;
                unsigned char Q2ratio : 4;
#endif
            } QR;
        } Q;                                        // 1 bytes
        unsigned char tmp_code[CODE_SIZE];          // 32/64 bytes
    } lsh_bin;


// extren tlsh library
extern int h_distance(int len, const unsigned char x[], const unsigned char y[]);
extern int mod_diff(unsigned int x, unsigned int y, unsigned int R);
extern void from_hex(const char *psrc, int len, unsigned char *pdest);
extern void to_hex(unsigned char *psrc, int len, char *pdest);
extern unsigned char swap_byte( const unsigned char in );


extern int tlsh_cmp(Datum d1, Datum d2);
extern int tlsh_dist_impl(lsh_bin* lsh1, lsh_bin* lsh2, bool len_diff);

#endif
