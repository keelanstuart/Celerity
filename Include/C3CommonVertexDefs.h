// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once


#include <C3.h>
#include <C3VertexBuffer.h>

namespace c3
{
	namespace Vertex
	{
		/// Position / Normal / Tangent / Binormal / 1x Texture Coords
		namespace PNYT1
		{
			constexpr VertexBuffer::ComponentDescription d[] =
			{
				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_POSITION},
				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_NORMAL},
				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_TANGENT},
				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_BINORMAL},
				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 2, c3::VertexBuffer::ComponentDescription::Usage::VU_TEXCOORD0},

				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_NONE, 0, c3::VertexBuffer::ComponentDescription::Usage::VU_NONE} // terminator
			};

#pragma pack(push, 1)
			typedef struct _s
			{
				glm::fvec3 pos;
				glm::fvec3 norm;
				glm::fvec3 tang;
				glm::fvec3 binorm;
				glm::fvec2 uv;
			} s;
#pragma pack(pop)
		};

		/// Position / Normal / 1x Texture Coords
		namespace PNT1
		{
			constexpr VertexBuffer::ComponentDescription d[] =
			{
				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_POSITION},
				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_NORMAL},
				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 2, c3::VertexBuffer::ComponentDescription::Usage::VU_TEXCOORD0},

				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_NONE, 0, c3::VertexBuffer::ComponentDescription::Usage::VU_NONE} // terminator
			};

#pragma pack(push, 1)
			typedef struct _s
			{
				glm::fvec3 pos;
				glm::fvec3 norm;
				glm::fvec2 uv;
			} s;
#pragma pack(pop)
		};

		/// Pre-transformed Position / 1x Texture Coords
		namespace WT1
		{
			constexpr VertexBuffer::ComponentDescription d[] =
			{
				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 4, c3::VertexBuffer::ComponentDescription::Usage::VU_POSITION},
				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 2, c3::VertexBuffer::ComponentDescription::Usage::VU_TEXCOORD0},

				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_NONE, 0, c3::VertexBuffer::ComponentDescription::Usage::VU_NONE} // terminator
			};

#pragma pack(push, 1)
			typedef struct _s
			{
				glm::fvec4 pos;
				glm::fvec2 uv;
			} s;
#pragma pack(pop)
		};

		/// Screen position / 1x Texture Coords
		namespace ST1
		{
			constexpr VertexBuffer::ComponentDescription d[] =
			{
				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 2, c3::VertexBuffer::ComponentDescription::Usage::VU_POSITION},
				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 2, c3::VertexBuffer::ComponentDescription::Usage::VU_TEXCOORD0},

				{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_NONE, 0, c3::VertexBuffer::ComponentDescription::Usage::VU_NONE} // terminator
			};

#pragma pack(push, 1)
			typedef struct _s
			{
				glm::fvec2 pos;
				glm::fvec2 uv;
			} s;
#pragma pack(pop)
		};
	};
};
