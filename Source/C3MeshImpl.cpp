// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3MeshImpl.h>
#include <C3BoundingBoxImpl.h>


using namespace c3;


std::vector<MeshImpl::OctreeNode *> MeshImpl::s_OctreeNodeCache;

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


static glm::fmat4x4 identMat = glm::identity<glm::fmat4x4>();


bool MeshImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV, const glm::fmat4x4 *pMat) const
{
	if (!pRayPos || !pRayDir)
		return false;

	if (m_Octree.empty())
		InitializeOctree();

	if (!pMat)
		pMat = &identMat;

	// Inverse of the transformation matrix to transform the ray to local space
	glm::fmat4x4 invMat = glm::inverse(*pMat);

	// Transform ray position and direction to local space
	glm::vec3 localRayPos = glm::vec3(invMat * glm::vec4(*pRayPos, 1.0f));
	glm::vec3 localRayDir = glm::normalize(glm::vec3(invMat * glm::vec4(*pRayDir, 0.0f)));

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

			std::function<void(const OctreeNode *)> CollideOctree;
			CollideOctree = [&](const OctreeNode *root)
			{
				if (root && root->m_Bounds.CheckCollision(&localRayPos, &localRayDir))
				{
					for (size_t i = 0; i < OctreeNode::EOctreeChild::COUNT; i++)
						CollideOctree(root->m_Children[i]);

					for (auto face : root->m_Tris)
					{
						size_t baseidx = face * 3;

						// Assume the first element in the vertex is position
						glm::vec3 v[3];

						v[0] = *(glm::vec3 *)(pvb + (vsz * pib[baseidx]));
						v[1] = *(glm::vec3 *)(pvb + (vsz * pib[baseidx + 1]));
						v[2] = *(glm::vec3 *)(pvb + (vsz * pib[baseidx + 2]));

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
				}
			};

			CollideOctree(&m_Octree.front());
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


void MeshImpl::InitializeOctree() const
{
	if (!m_pBounds)
		((MeshImpl *)this)->ComputeBounds();

	// reserve some space for building our octree...
	// place the octree node we fall in the cache by face index, then split it out after that
	uint32_t face_count = (uint32_t)(m_IB->Count() / 3);
	if (face_count > s_OctreeNodeCache.max_size())
		s_OctreeNodeCache.reserve(face_count);
	s_OctreeNodeCache.clear();

	std::function<void(OctreeNode *, OctreeNode::EOctreeChild, size_t)> AddOctreeNode;
	AddOctreeNode = [&](OctreeNode *parent, OctreeNode::EOctreeChild ch, size_t depth) -> void
	{
		if (depth >= OCTREE_DEPTH)
			return;

		if (!parent)
			m_Octree.clear();

		m_Octree.emplace_back();
		OctreeNode *n = &m_Octree.back();
		if (parent)
			parent->m_Children[ch] = n;

		// copy the parent bounds (from either the whole mesh or from the parent octree node)
		n->m_Bounds = parent ? parent->m_Bounds : (const BoundingBoxImpl &)*m_pBounds;
		glm::fvec3 minb = *(n->m_Bounds.GetCorners() + BoundingBoxImpl::CORNER::xyz);
		glm::fvec3 maxb = *(n->m_Bounds.GetCorners() + BoundingBoxImpl::CORNER::XYZ);
		glm::fvec3 hd = (maxb - minb) / 2.0f;

		switch (ch)
		{
			case OctreeNode::xyz:
				maxb.x -= hd.x;
				maxb.y -= hd.y;
				maxb.z -= hd.z;
				break;

			case OctreeNode::Xyz:
				minb.x += hd.x;
				maxb.y -= hd.y;
				maxb.z -= hd.z;
				break;

			case OctreeNode::xYz:
				maxb.x -= hd.x;
				minb.y += hd.y;
				maxb.z -= hd.z;
				break;

			case OctreeNode::XYz:
				minb.x += hd.x;
				minb.y += hd.y;
				maxb.z -= hd.z;
				break;

			case OctreeNode::xyZ:
				maxb.x -= hd.x;
				maxb.y -= hd.y;
				minb.z += hd.z;
				break;

			case OctreeNode::XyZ:
				minb.x += hd.x;
				maxb.y -= hd.y;
				minb.z += hd.z;
				break;

			case OctreeNode::xYZ:
				minb.x += hd.x;
				maxb.y -= hd.y;
				minb.z += hd.z;
				break;

			case OctreeNode::XYZ:
				minb.x += hd.x;
				minb.y += hd.y;
				minb.z += hd.z;
				break;

			default:
				break;
		}

		if (parent)
			n->m_Bounds.SetExtents(&minb, &maxb);
		n->m_Bounds.Align(&identMat);

		if (depth < OCTREE_DEPTH)
		{
			for (size_t i = 0; i < OctreeNode::EOctreeChild::COUNT; i++)
			{
				AddOctreeNode(n, (OctreeNode::EOctreeChild)i, depth + 1);
			}
		}
	};

	std::function<OctreeNode *(OctreeNode *, OctreeNode *, const glm::fvec3 *p0, const glm::fvec3 *p1, const glm::fvec3 *p2)> FindOctreeSubNodeFromPoints;
	FindOctreeSubNodeFromPoints = [&](OctreeNode *parent, OctreeNode *root, const glm::fvec3 *p0, const glm::fvec3 *p1, const glm::fvec3 *p2) -> OctreeNode *
	{
		if (!root)
			return nullptr;

		for (size_t i = 0; i < OctreeNode::EOctreeChild::COUNT; i++)
		{
			OctreeNode *n = root->m_Children[i];
			if (!n)
				continue;

			OctreeNode *ret = FindOctreeSubNodeFromPoints(root, n, p0, p1, p2);
			if (ret)
				return ret;
		}

#if 1
		if (root->m_Bounds.IsPointInside(p0) && root->m_Bounds.IsPointInside(p1) && root->m_Bounds.IsPointInside(p2) || !parent)
#else
		const glm::fvec3 *pb = root->m_Bounds.GetCorners();
		const glm::fvec3 &bmin = pb[BoundingBoxImpl::CORNER::xyz];
		const glm::fvec3 &bmax = pb[BoundingBoxImpl::CORNER::XYZ];
		if ((p0->x >= bmin.x) && (p0->x >= bmin.y) && (p0->x >= bmin.z) &&
			(p0->x <= bmax.x) && (p0->x <= bmax.y) && (p0->x <= bmax.z) &&
			(p1->x >= bmin.x) && (p1->x >= bmin.y) && (p1->x >= bmin.z) &&
			(p1->x <= bmax.x) && (p1->x <= bmax.y) && (p1->x <= bmax.z) &&
			(p2->x >= bmin.x) && (p2->x >= bmin.y) && (p2->x >= bmin.z) &&
			(p2->x <= bmax.x) && (p2->x <= bmax.y) && (p2->x <= bmax.z) ||
			!parent)
#endif
			return root;

		return nullptr;
	};

	// Build the octree structure with bounds in memory...
	AddOctreeNode(nullptr, OctreeNode::COUNT, 0);
	if (m_Octree.empty())
		return;

	// ...now fill it out with triangle references
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

			for (uint32_t face = 0, i = 0; face < face_count; face++)
			{
				// Assume the first element in the vertex is position
				OctreeNode *n = FindOctreeSubNodeFromPoints(nullptr, &m_Octree.front(),
					(const glm::fvec3 *)(pvb + (vsz * pib[i++])),
					(const glm::fvec3 *)(pvb + (vsz * pib[i++])),
					(const glm::fvec3 *)(pvb + (vsz * pib[i++])));

				// store the node in the cache and increment the number of faces in that node
				s_OctreeNodeCache.push_back(n);
				n->m_TriCount++;
			}

			m_IB->Unlock();
		}

		m_VB->Unlock();
	}

	// reserve the space for the face vectors
	for (auto node : m_Octree)
	{
		if (node.m_TriCount)
			node.m_Tris.reserve(node.m_TriCount);
	}

	// now fill them out
	uint32_t fidx = 0;
	for (auto pnode : s_OctreeNodeCache)
	{
		pnode->m_Tris.push_back(fidx);
		fidx++;
	}
}
