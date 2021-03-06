// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2020, Keelan Stuart


#pragma once

#include <c3.h>

namespace c3
{

	class VertexBuffer;
	class IndexBuffer;

	class Mesh
	{

	public:

		enum RETURNCODE
		{
			RET_OK = 0,
			RET_NULLBUFFER,
			RET_FAILED,
		};

		virtual void Release() = NULL;

		virtual void AttachVertexBuffer(VertexBuffer *pvertexbuf) = NULL;

		virtual VertexBuffer *GetVertexBuffer() const = NULL;

		virtual void AttachIndexBuffer(IndexBuffer *pindexbuf) = NULL;

		virtual IndexBuffer *GetIndexBuffer() const = NULL;

		virtual RETURNCODE Draw(Renderer::PrimType type = Renderer::PrimType::TRILIST) const = NULL;

	};

};