// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class VertexBuffer
	{

	public:

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

		typedef struct SComponentDescription
		{
			typedef enum EComponentType
			{
				VCT_NONE = 0,		// list terminator

				VCT_U8,
				VCT_S8,
				VCT_U32,
				VCT_F16,
				VCT_F32,

				VCT_NUM_TYPES

			} ComponentType;

			ComponentType m_Type;

			size_t m_Count;

			typedef enum EUsage
			{
				VU_NONE = 0,

				VU_POSITION,
				VU_NORMAL,
				VU_TEXCOORD0,
				VU_TEXCOORD1,
				VU_TEXCOORD2,
				VU_TEXCOORD3,
				VU_TANGENT,
				VU_BINORMAL,
				VU_INDEX,
				VU_WEIGHT,
				VU_COLOR0,
				VU_COLOR1,
				VU_COLOR2,
				VU_COLOR3,
				VU_SIZE,

				VU_NUM_USAGES

			} Usage;

			Usage m_Usage;

			size_t size() const
			{
				size_t ret = 0;

				switch (m_Type)
				{
					case VCT_U8:
					case VCT_S8:		ret = 1; break;
					case VCT_F16:		ret = 2; break;
					case VCT_U32:
					case VCT_F32:		ret = 4; break;

					default:			ret = 0; break;
				}

				ret *= m_Count;

				return ret;
			}
		} ComponentDescription;

		virtual void Release() = NULL;

		#define VBLOCKFLAG_READ			0x0001
		#define VBLOCKFLAG_WRITE		0x0002
		#define VBLOCKFLAG_DYNAMIC		0x0004
		#define VBLOCKFLAG_USERBUFFER	0x0008
		#define VBLOCKFLAG_UPDATENOW	0x0010

		virtual RETURNCODE Lock(void **buffer, size_t numverts, const ComponentDescription *components, props::TFlags64 flags) = NULL;
		virtual void Unlock() = NULL;

		virtual size_t Count() = NULL;

		virtual size_t NumComponents() = NULL;
		virtual const ComponentDescription *Component(size_t compidx) = NULL;

		virtual size_t VertexSize() = NULL;
	};

};