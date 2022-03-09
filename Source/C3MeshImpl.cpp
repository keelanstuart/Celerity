// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


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


const BoundingBox *MeshImpl::GetBounds() const
{
	return nullptr;
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
	if (!pRayPos || !pRayDir)
		return false;

	bool ret = false;

	BYTE *pvb = nullptr;
	if (m_VB)
		m_VB->Lock((void **)&pvb, 0, nullptr, VBLOCKFLAG_READ);

	if (pvb)
	{
		size_t vsz = m_VB->VertexSize();

		uint16_t *pib = nullptr;
		if (m_IB)
			m_IB->Lock((void **)&pib, 0, IndexBuffer::IS_16BIT, IBLOCKFLAG_READ);

		if (pib)
		{

			float cdist, *pcdist = pDistance ? pDistance : &cdist;
			*pcdist = FLT_MAX;

			size_t cface, *pcface = pFaceIndex ? pFaceIndex : &cface;
			*pcface = 0;

			glm::vec2 cuv, *pcuv = pUV ? pUV : &cuv;
			*pcuv = glm::vec2(0, 0);

			for (size_t face = 0, max_face = m_IB->Count() / 3, i = 0; face < max_face; face++)
			{
				glm::vec3 *v[3];
				v[0] = (glm::vec3 *)(pvb + (vsz * pib[i++]));
				v[1] = (glm::vec3 *)(pvb + (vsz * pib[i++]));
				v[2] = (glm::vec3 *)(pvb + (vsz * pib[i++]));

				glm::vec2 luv;
				float ldist;

				bool hit = glm::intersectRayTriangle(*pRayPos, *pRayDir, *v[0], *v[1], *v[2], luv, ldist);

				if (hit)
				{
					if (ldist < *pcdist)
					{
						*pcdist = ldist;
						*pcface = face;
						*pcuv = luv;
					}

					ret = true;
				}
			}

			m_IB->Unlock();
		}

		m_VB->Unlock();
	}

	return ret;
}
