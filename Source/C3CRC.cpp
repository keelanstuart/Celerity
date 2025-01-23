// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3CRC.h>

using namespace c3;


uint32_t Crc32Table[256];

void InitializeCrc32Table()
{
	// terms of polynomial defining this crc (except x^32):
	static const uint8_t p[] = {0, 1, 2, 4, 5, 7, 8, 10, 11, 12, 16, 22, 23, 26};	

	// polynomial exclusive-or pattern
	// make exclusive-or pattern from polynomial (0xedb88320L)
	uint32_t poly = 0;

	uint32_t n;

	for (n = 0; n < sizeof(p) / sizeof(uint8_t); n++)
		poly |= 1L << (31 - p[n]);

	for (n = 0; n < 256; n++)
	{
		uint32_t c = (uint32_t)n;

		for (uint32_t k = 0; k < 8; k++)
			c = c & 1 ? poly ^ (c >> 1) : c >> 1;

		Crc32Table[n] = c;
	}
}


uint16_t Crc16Table[256];

void InitializeCrc16Table()
{
	for (uint32_t i = 0; i < 256; i++)
	{
		uint32_t k = 0xC0C1;

		for (uint32_t j = 1; j < 256; j <<= 1)
		{
			if (i & j)
				Crc16Table[i] ^= k;

			k = (k << 1) ^ 0x4003;
		}
	}
}


uint32_t Crc32::Calculate(const uint8_t *data, size_t len, uint32_t initvalue)
{
	uint32_t ret = initvalue;

	while (len) 
	{
		ret = Crc32Table[((int)ret ^ (*data++)) & 0xff] ^ (ret >> 8);

		--len;
	}

	return ret;
}


uint32_t Crc32::CalculateString(const TCHAR *s, uint32_t initvalue)
{
	uint32_t ret = initvalue;

	if (s)
	{
#ifdef _UNICODE
		uint8_t *b = (uint8_t *)s;
#endif
		while (*s)
		{
#ifdef _UNICODE

			ret = Crc32Table[((int)ret ^ (*b++)) & 0xff] ^ (ret >> 8);
			ret = Crc32Table[((int)ret ^ (*b++)) & 0xff] ^ (ret >> 8);
			s++;
#else
			ret = Crc32Table[((int)ret ^ (*s++)) & 0xff] ^ (ret >> 8);
#endif
		}
	}

	return ret;
}


uint16_t Crc16::Calculate(const uint8_t *data, size_t len, uint16_t initvalue)
{
	uint16_t ret = initvalue;

	while (len)
	{
		ret = (ret >> 8) ^ Crc16Table[(ret ^ *data++) & 0xff];

		--len;
	}

	return ret;
}


uint16_t Crc16::CalculateString(const TCHAR *s, uint16_t initvalue)
{
	uint16_t ret = initvalue;

	if (s)
	{
		while (*s)
		{
			BYTE *b = (BYTE *)s;
			for (int i = 0; i < sizeof(TCHAR); i++)
				ret = (ret >> 8) ^ Crc16Table[(ret ^ *b++) & 0xff];

			s++;
		}
	}

	return ret;
}