// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3MeshImpl.h>


using namespace c3;


MeshImpl::MeshImpl(RendererImpl *prend)
{
	m_pRend = prend;
	m_IB = nullptr;
	m_VB = nullptr;
	m_pBounds = nullptr;

	assert(m_pRend);
}


MeshImpl::~MeshImpl()
{
	m_pRend = nullptr;
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

	if (m_pBounds)
	{
		m_pBounds->Release();
		m_pBounds = nullptr;
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


const BoundingBox *MeshImpl::GetBounds()
{
	if (!m_pBounds)
	{
		this->m_pBounds = BoundingBox::Create();

		if (m_VB)
		{
			void *vbuf = nullptr;
			glm::fvec3 *ppos;

			if (m_VB->Lock(&vbuf, -1, nullptr, VBLOCKFLAG_READ) == VertexBuffer::RETURNCODE::RET_OK)
			{
				size_t vofs = 0; // TODO: find the POS component's offset; currently assume POS is the first component

				size_t vsz = m_VB->VertexSize();
				glm::fvec3 minb(FLT_MAX, FLT_MAX, FLT_MAX), maxb(-FLT_MAX, -FLT_MAX, -FLT_MAX);

				if (m_IB)
				{
					void *ibuf = nullptr;
					if (m_IB->Lock(&ibuf, -1, IndexBuffer::IndexSize::IS_NONE, IBLOCKFLAG_READ))
					{
						switch (m_IB->GetIndexSize())
						{
							case IndexBuffer::IS_8BIT:
							{
								uint8_t *pidx = (uint8_t *)ibuf;
								for (size_t ii = 0; ii < m_IB->Count(); ii++)
								{
									ppos = (glm::fvec3 *)((char *)vbuf + (pidx[ii] * vsz) + vofs);

									if (ppos->x < minb.x)
										minb.x = ppos->x;
									if (ppos->y < minb.y)
										minb.y = ppos->y;
									if (ppos->z < minb.z)
										minb.z = ppos->z;

									if (ppos->x > maxb.x)
										maxb.x = ppos->x;
									if (ppos->y > maxb.y)
										maxb.y = ppos->y;
									if (ppos->z > maxb.z)
										maxb.z = ppos->z;
								}

								break;
							}
							case IndexBuffer::IS_16BIT:
							{
								uint16_t *pidx = (uint16_t *)ibuf;
								for (size_t ii = 0; ii < m_IB->Count(); ii++)
								{
									ppos = (glm::fvec3 *)((char *)vbuf + (pidx[ii] * vsz) + vofs);

									if (ppos->x < minb.x)
										minb.x = ppos->x;
									if (ppos->y < minb.y)
										minb.y = ppos->y;
									if (ppos->z < minb.z)
										minb.z = ppos->z;

									if (ppos->x > maxb.x)
										maxb.x = ppos->x;
									if (ppos->y > maxb.y)
										maxb.y = ppos->y;
									if (ppos->z > maxb.z)
										maxb.z = ppos->z;
								}

								break;
							}
							case IndexBuffer::IS_32BIT:
							{
								uint32_t *pidx = (uint32_t *)ibuf;
								for (size_t ii = 0; ii < m_IB->Count(); ii++)
								{
									ppos = (glm::fvec3 *)((char *)vbuf + (pidx[ii] * vsz) + vofs);

									if (ppos->x < minb.x)
										minb.x = ppos->x;
									if (ppos->y < minb.y)
										minb.y = ppos->y;
									if (ppos->z < minb.z)
										minb.z = ppos->z;

									if (ppos->x > maxb.x)
										maxb.x = ppos->x;
									if (ppos->y > maxb.y)
										maxb.y = ppos->y;
									if (ppos->z > maxb.z)
										maxb.z = ppos->z;
								}

								break;
							}
						}

						m_IB->Unlock();
					}
				}
				else
				{
					for (size_t vi = 0; vi < m_VB->Count(); vi++)
					{
						ppos = (glm::fvec3 *)((char *)vbuf + vofs);

						if (ppos->x < minb.x)
							minb.x = ppos->x;
						if (ppos->y < minb.y)
							minb.y = ppos->y;
						if (ppos->z < minb.z)
							minb.z = ppos->z;

						if (ppos->x > maxb.x)
							maxb.x = ppos->x;
						if (ppos->y > maxb.y)
							maxb.y = ppos->y;
						if (ppos->z > maxb.z)
							maxb.z = ppos->z;

						vofs += vsz;
					}
				}

				m_VB->Unlock();

				m_pBounds->SetOrigin(&minb);
				m_pBounds->SetExtents(&maxb);
			}
		}
	}

	return m_pBounds;
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


bool MeshImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV, const glm::fmat4x4 *pMat) const
{
	if (!pRayPos || !pRayDir)
		return false;

	static glm::fmat4x4 identMat = glm::identity<glm::fmat4x4>();
	if (!pMat)
		pMat = &identMat;

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
				// assume the first element in the vertex is position
				glm::vec3 v[3];

				v[0] = *pMat * glm::vec4(*(glm::vec3 *)(pvb + (vsz * pib[i++])), 1);
				v[1] = *pMat * glm::vec4(*(glm::vec3 *)(pvb + (vsz * pib[i++])), 1);
				v[2] = *pMat * glm::vec4(*(glm::vec3 *)(pvb + (vsz * pib[i++])), 1);

				glm::vec2 luv;
				float ldist;

#if 0
				// ignore backfacing triangles
				glm::fvec3 na = glm::normalize(*v[1] - *v[0]);
				glm::fvec3 nb = glm::normalize(*v[2] - *v[0]);
				glm::fvec3 n = glm::normalize(glm::cross(na, nb));
				if (glm::dot(n, *pRayDir) < 0)
					continue;
#endif

				// check for a collision
				bool hit = glm::intersectRayTriangle(*pRayPos, *pRayDir, v[0], v[2], v[1], luv, ldist);

				if (hit)
				{
					// get the nearest collision
					if (ldist < *pcdist)
					{
						if (pcdist)
							*pcdist = ldist;

						if (pcface)
							*pcface = face;

						if (pcuv)
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
