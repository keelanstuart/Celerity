// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3Mesh.h>
#include <C3RendererImpl.h>
#include <C3VertexBufferImpl.h>
#include <C3IndexBufferImpl.h>

namespace c3
{

	class MeshImpl : public Mesh
	{

	protected:
		RendererImpl *m_pRend;
		IndexBufferImpl *m_IB;
		VertexBufferImpl *m_VB;

	public:

		MeshImpl(RendererImpl *prend);

		virtual ~MeshImpl();

		virtual void Release();

		virtual void AttachVertexBuffer(VertexBuffer *pvertexbuf);

		virtual VertexBuffer *GetVertexBuffer();

		virtual void AttachIndexBuffer(IndexBuffer *pindexbuf);

		virtual IndexBuffer *GetIndexBuffer();

		virtual RETURNCODE Draw(Renderer::PrimType type);

	};

};