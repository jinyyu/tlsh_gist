#include "tlsh_gist.h"
#include <tlsh/tlsh.h>



extern "C"
{
	PG_MODULE_MAGIC;
	PG_FUNCTION_INFO_V1(tlsh_in);
	PG_FUNCTION_INFO_V1(tlsh_out);
}

void to_hex(unsigned char *psrc, int len, char *pdest)
{
	static unsigned char HexLookup[513] = {
		"000102030405060708090A0B0C0D0E0F"
		"101112131415161718191A1B1C1D1E1F"
		"202122232425262728292A2B2C2D2E2F"
		"303132333435363738393A3B3C3D3E3F"
		"404142434445464748494A4B4C4D4E4F"
		"505152535455565758595A5B5C5D5E5F"
		"606162636465666768696A6B6C6D6E6F"
		"707172737475767778797A7B7C7D7E7F"
		"808182838485868788898A8B8C8D8E8F"
		"909192939495969798999A9B9C9D9E9F"
		"A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
		"B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
		"C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
		"D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
		"E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
		"F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF"};
	unsigned short *pwHex = (unsigned short *)HexLookup;
	unsigned short *pwDest = (unsigned short *)pdest;

	for (int i = 0; i < len; i++)
	{
		*pwDest = pwHex[*psrc];
		pwDest++;
		psrc++;
	}
	*((unsigned char *)pwDest) = 0; // terminate the string
}

void from_hex(const char *psrc, int len, unsigned char *pdest)
{
	static unsigned char DecLookup[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // gap before first hex digit
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,		   // 0123456789
		0, 0, 0, 0, 0, 0, 0,				   // :;<=>?@ (gap)
		10, 11, 12, 13, 14, 15,				   // ABCDEF
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // GHIJKLMNOPQRS (gap)
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // TUVWXYZ[/]^_` (gap)
		10, 11, 12, 13, 14, 15				   // abcdef
	};

	for (int i = 0; i < len; i += 2)
	{
		unsigned d = DecLookup[*(unsigned char *)(psrc + i)] << 4;
		d |= DecLookup[*(unsigned char *)(psrc + i + 1)];
		*pdest++ = d;
	}
}



Datum tlsh_in(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();

	char *str = PG_GETARG_CSTRING(0);
	if (strlen(str) != TLSH_HASH_LENGTH) {
		elog(ERROR, "invalid tlash hash");
	}

	unsigned char* buffer = (unsigned char*) palloc(TLSH_INTERNAL_LENGTH);
	from_hex(str, TLSH_HASH_LENGTH, buffer);

	PG_RETURN_POINTER(buffer);
}

Datum tlsh_out(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	unsigned char *buffer = (unsigned char *)PG_GETARG_POINTER(0);
	char *str = (char*) palloc(TLSH_HASH_LENGTH + 1);
	to_hex(buffer, TLSH_INTERNAL_LENGTH, str);
	str[TLSH_HASH_LENGTH] = 0;

	PG_RETURN_CSTRING(str);
}
