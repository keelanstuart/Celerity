// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <c3.h>

namespace c3
{

	class IndexBuffer
	{

	public:

		typedef enum EIndexSize
		{
			IS_NONE = 0,
			IS_8BIT = 1,
			IS_16BIT = 2,
			IS_32BIT = 4

		} IndexSize;

		enum RETURNCODE
		{
			RET_OK = 0,
			RET_ALREADY_LOCKED,
			RET_ZERO_ELEMENTS,
			RET_NULL_BUFFER,
			RET_BAD_VERTEX_DESCRIPTION,
			RET_GENBUFFER_FAILED,
			RET_MAPBUFFER_FAILED,
			RET_UPDATENOW_NEEDS_USERBUFFER,
		};

		virtual void Release() = NULL;

		#define IBLOCKFLAG_READ			0x0001
		#define IBLOCKFLAG_WRITE		0x0002
		#define IBLOCKFLAG_DYNAMIC		0x0004
		#define IBLOCKFLAG_USERBUFFER	0x0008
		#define IBLOCKFLAG_UPDATENOW	0x0010
		#define IBLOCKFLAG_CACHE		0x0020

		// Locks the IndexBuffer's memory so that it can be read from or written to
		// Requires an eventual, corresponding Unlock call
		virtual RETURNCODE Lock(void **buffer, size_t numindices, IndexSize sz, props::TFlags64 flags) = NULL;

		// Unlocks the IndexBuffer's memory, possibly updating GPU memory
		virtual void Unlock() = NULL;

		// Returns the number of indices in the buffer
		virtual size_t Count() = NULL;

		// Returns the size of an individual index in bytes, corresponds to EIndexSize
		virtual size_t GetIndexSize() = NULL;

	};

};
