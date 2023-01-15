// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <c3.h>


namespace c3
{

	namespace Crc32
	{
		#define CRC32_INITVALUE		0xFFFFFFFF

		uint32_t C3_API Calculate(const uint8_t *data, size_t len, uint32_t initvalue = CRC32_INITVALUE);
		uint32_t C3_API CalculateString(const TCHAR *s, uint32_t initvalue = CRC32_INITVALUE);
	};

	namespace Crc16
	{
		#define CRC16_INITVALUE		0x0000

		uint16_t C3_API Calculate(const uint8_t *data, size_t len, uint16_t initvalue = CRC16_INITVALUE);
		uint16_t C3_API CalculateString(const TCHAR *s, uint16_t initvalue = CRC16_INITVALUE);
	};

};
