// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


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
}


void MeshImpl::Release()
{
	if (m_VB)
	{
		m_VB->Release();
		m_VB = nullptr;
	}

	if (m_IB)
	{
		m_IB->Release();
		m_IB = nullptr;
	}

	delete this;
}


void MeshImpl::AttachVertexBuffer(VertexBuffer *pvertexbuf)
{
	m_VB = (VertexBufferImpl *)pvertexbuf;
}


VertexBuffer *MeshImpl::GetVertexBuffer() const
{
	return m_VB;
}


void MeshImpl::AttachIndexBuffer(IndexBuffer *pindexbuf)
{
	m_IB = (IndexBufferImpl *)pindexbuf;
}


IndexBuffer *MeshImpl::GetIndexBuffer() const
{
	return m_IB;
}


Mesh::RETURNCODE MeshImpl::Draw(Renderer::PrimType type) const
{
	if (m_VB)
	{
		m_pRend->UseVertexBuffer(m_VB);

		if (m_IB)
		{
			m_pRend->UseIndexBuffer(m_IB);
			m_pRend->DrawIndexedPrimitives(type);
		}
		else
		{
			m_pRend->UseIndexBuffer(nullptr);
			m_pRend->DrawPrimitives(type);
		}

		return Mesh::RETURNCODE::RET_OK;
	}

	return Mesh::RETURNCODE::RET_NULLBUFFER;
}


bool MeshImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir,
					   float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV) const
{
	return false;
}
