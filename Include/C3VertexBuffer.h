// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


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
			RET_NULL_VERTEX_DESCRIPTION,
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

		#define VBLOCKFLAG_READ			0x0001		// need read access to the memory
		#define VBLOCKFLAG_WRITE		0x0002		// need write access to the memory
		#define VBLOCKFLAG_DYNAMIC		0x0004		// hint to the driver that the data will change frequently
		#define VBLOCKFLAG_USERBUFFER	0x0008		// we want the memory in the provided buffer
		#define VBLOCKFLAG_UPDATENOW	0x0010		// update the buffer data (used with USERBUFFER) - requires no unlock
		#define VBLOCKFLAG_CACHE		0x0020		// create a cache of the data so that it can be queried quickly later

		// Lock the vertex buffer's memory so you can read from or write to it, depending on how it was created and the flags you supply here
		virtual RETURNCODE Lock(void **buffer, size_t numverts, const ComponentDescription *components, props::TFlags64 flags) = NULL;

		// Unlocks the vertex buffer's memory once you are done doing whatever it is you were doing to it. Writes will typically be
		// finalized by updating GPU memory
		virtual void Unlock() = NULL;

		// Returns the number of vertices in this buffer
		virtual size_t Count() = NULL;

		// Returns the number of components on these vertices
		virtual size_t NumComponents() = NULL;

		// Gets the vertex component type by index
		virtual const ComponentDescription *Component(size_t compidx) = NULL;

		// Returns the size in bytes of each vertex
		virtual size_t VertexSize() = NULL;

		// Returns true if the underlying structures of the vertices have been configured
		virtual bool Configured() const = NULL;
	};

};