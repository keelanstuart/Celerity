// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3MeshImpl.h>
#include <C3BoundingBoxImpl.h>


using namespace c3;


MeshImpl::MeshImpl(RendererImpl *prend)
{
	m_pRend = prend;
	m_IB = nullptr;
	m_VB = nullptr;
	m_pBounds = nullptr;
	m_IsSkin = false;

	assert(m_pRend);
}


MeshImpl::~MeshImpl()
{
	m_pRend = nullptr;
}


void MeshImpl::Release()
{
	C3_SAFERELEASE(m_VB);
	C3_SAFERELEASE(m_IB);
	C3_SAFERELEASE(m_pBounds);

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
	// I know this is "questionable", since this is a const function... but we're not changing
	// the real data here, so I don't feel too bad about it.
	if (!m_pBounds)
		((MeshImpl *)this)->ComputeBounds();

	return m_pBounds;
}


void MeshImpl::ComputeBounds()
{
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
				if (m_IB->Lock(&ibuf, -1, IndexBuffer::IndexSize::IS_NONE, IBLOCKFLAG_READ) == IndexBuffer::RETURNCODE::RET_OK)
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

			SetBounds(minb, maxb);
		}
	}
}


void MeshImpl::SetBounds(const glm::fvec3 &vmin, const glm::fvec3 &vmax)
{
	if (!m_pBounds)
		m_pBounds = BoundingBox::Create();

	m_pBounds->SetExtents(&vmin, &vmax);
}


Mesh::RETURNCODE MeshImpl::Draw(Renderer::PrimType type) const
{
	if (m_pBounds)
	{
		glm::fmat4x4 m;
		m_pRend->GetWorldMatrix(&m);

		BoundingBoxImpl bb(m_pBounds);
		bb.Align(&m);

		if (!m_pRend->GetClipFrustum()->IsBoxInside(&bb))
			return Mesh::RETURNCODE::RET_NOTVISIBLE;
	}

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

	// Inverse of the transformation matrix to transform the ray to local space
	glm::fmat4x4 invMat = glm::inverse(*pMat);

	// Transform ray position and direction to local space
	glm::vec3 localRayPos = glm::vec3(invMat * glm::vec4(*pRayPos, 1.0f));
	glm::vec3 localRayDir = glm::normalize(glm::vec3(invMat * glm::vec4(*pRayDir, 0.0f)));

	// Check bounding box collision in local space if m_pBounds is valid
	if (m_pBounds)
	{
		BoundingBoxImpl bb(m_pBounds);
		bb.Align(&identMat);
		if (!bb.CheckCollision(&localRayPos, &localRayDir))
			return false;
	}

	bool ret = false;

	BYTE *pvb = nullptr;
	if (m_VB && m_VB->Lock((void **)&pvb, 0, nullptr, VBLOCKFLAG_READ) == S_OK)
	{
		size_t vsz = m_VB->VertexSize();

		uint16_t *pib = nullptr;
		if (m_IB && m_IB->Lock((void **)&pib, 0, IndexBuffer::IS_16BIT, IBLOCKFLAG_READ) == S_OK)
		{
			float closestDistance = FLT_MAX;
			size_t closestFace = 0;
			glm::vec2 closestUV(0, 0);

			for (size_t face = 0, max_face = m_IB->Count() / 3, i = 0; face < max_face; face++)
			{
				// Assume the first element in the vertex is position
				glm::vec3 v[3];

				v[0] = *(glm::vec3 *)(pvb + (vsz * pib[i++]));
				v[1] = *(glm::vec3 *)(pvb + (vsz * pib[i++]));
				v[2] = *(glm::vec3 *)(pvb + (vsz * pib[i++]));

				glm::vec2 uv;
				float distance;

				// Check for a collision using the local ray
				bool hit = glm::intersectRayTriangle(localRayPos, localRayDir, v[0], v[1], v[2], uv, distance);

				if (hit)
				{
					// Transform distance back to the original coordinate space
					glm::vec3 hitPoint = localRayPos + distance * localRayDir;
					glm::vec3 transformedHitPoint = glm::vec3(*pMat * glm::vec4(hitPoint, 1.0f));
					float worldDistance = glm::length(transformedHitPoint - *pRayPos);

					// Get the nearest collision
					if ((worldDistance >= 0) && (worldDistance < closestDistance))
					{
						closestDistance = worldDistance;
						closestFace = face;
						closestUV = uv;
						ret = true;
					}
				}
			}

			// Update the output parameters if an intersection was found
			if (ret)
			{
				if (pDistance)
					*pDistance = closestDistance;
				if (pFaceIndex)
					*pFaceIndex = closestFace;
				if (pUV)
					*pUV = closestUV;
			}

			m_IB->Unlock();
		}

		m_VB->Unlock();
	}

	return ret;
}


bool MeshImpl::IsSkin() const
{
	return m_IsSkin;
}


void MeshImpl::MakeSkinned()
{
	m_IsSkin = true;
}
