#include "tlsh_gist.h"

static int length_mult = 12;
static int qratio_mult = 12;

// tlsh library
extern int h_distance(int len, const unsigned char x[], const unsigned char y[]);
extern int mod_diff(unsigned int x, unsigned int y, unsigned int R);

int tlsh_dist_impl(lsh_bin *lsh1, lsh_bin *lsh2, bool len_diff)
{
    int diff = 0;

    if (len_diff)
    {
        int ldiff = mod_diff(lsh1->Lvalue, lsh2->Lvalue, RANGE_LVALUE);
        if (ldiff == 0)
            diff = 0;
        else if (ldiff == 1)
            diff = 1;
        else
            diff += ldiff * length_mult;
    }

    int q1diff = mod_diff(lsh1->Q.QR.Q1ratio, lsh2->Q.QR.Q1ratio, RANGE_QRATIO);
    if (q1diff <= 1)
        diff += q1diff;
    else
        diff += (q1diff - 1) * qratio_mult;

    int q2diff = mod_diff(lsh1->Q.QR.Q2ratio, lsh2->Q.QR.Q2ratio, RANGE_QRATIO);
    if (q2diff <= 1)
        diff += q2diff;
    else
        diff += (q2diff - 1) * qratio_mult;

    for (int k = 0; k < TLSH_CHECKSUM_LEN; k++)
    {
        if (lsh1->checksum[k] != lsh2->checksum[k])
        {
            diff++;
            break;
        }
    }

    diff += h_distance(CODE_SIZE, lsh1->tmp_code, lsh2->tmp_code);

    return (diff);
}

int tlsh_cmp(Datum d1, Datum d2)
{
    return memcmp((void *)d1, (void *)d2, TLSH_INTERNAL_LENGTH);
}
