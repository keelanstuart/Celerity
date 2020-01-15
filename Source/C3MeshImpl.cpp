// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3MeshImpl.h>


using namespace c3;


MeshImpl::MeshImpl(RendererImpl *prend)
{
	m_pRend = prend;
	m_IB = nullptr;
	m_VB = nullptr;

	assert(m_pRend);
}


MeshImpl::~MeshImpl()
{
	m_pRend = nullptr;
	m_IB = nullptr;
	m_VB = nullptr;
}


void MeshImpl::Release()
{
	delete this;
}


void MeshImpl::AttachVertexBuffer(VertexBuffer *pvertexbuf)
{
	m_VB = (VertexBufferImpl *)pvertexbuf;
}


VertexBuffer *MeshImpl::GetVertexBuffer()
{
	return m_VB;
}


void MeshImpl::AttachIndexBuffer(IndexBuffer *pindexbuf)
{
	m_IB = (IndexBufferImpl *)pindexbuf;
}


IndexBuffer *MeshImpl::GetIndexBuffer()
{
	return m_IB;
}


Mesh::RETURNCODE MeshImpl::Draw(Renderer::PrimType type)
{
	if (m_VB)
	{
		m_pRend->UseVertexBuffer(m_VB);

		if (m_IB)
		{
			m_pRend->UseIndexBuffer(m_IB);
			m_pRend->DrawIndexedPrimitives(c3::Renderer::PrimType::TRILIST);
		}
		else
		{
			m_pRend->UseIndexBuffer(nullptr);
			m_pRend->DrawPrimitives(c3::Renderer::PrimType::TRILIST);
		}

		return Mesh::RETURNCODE::RET_OK;
	}

	return Mesh::RETURNCODE::RET_NULLBUFFER;
}
