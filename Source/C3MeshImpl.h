// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3Mesh.h>
#include <C3RendererImpl.h>
#include <C3VertexBufferImpl.h>
#include <C3IndexBufferImpl.h>
#include <C3BoundingBoxImpl.h>

namespace c3
{

	class MeshImpl : public Mesh
	{

	protected:
		RendererImpl *m_pRend;
		IndexBufferImpl *m_IB;
		VertexBufferImpl *m_VB;
		BoundingBox *m_pBounds;
		glm::fvec3 m_BoundingCentroid;
		float m_BoundingRadius;
		bool m_IsSkin;

		struct OctreeNode
		{
			OctreeNode()
			{
				memset(m_Children, 0, sizeof(OctreeNode *) * EOctreeChild::COUNT);
				m_TriCount = 0;
			}

			using EOctreeChild = enum { xyz, Xyz, xYz, XYz, xyZ, XyZ, xYZ, XYZ, COUNT };
			BoundingBoxImpl m_Bounds;							// the bounds for this level in the octree
			OctreeNode *m_Children[EOctreeChild::COUNT];		// the child octants
			using TriangleIndexArray = std::vector<uint32_t>;	// a list of triangle faces (multiply by 3 to get start in IB)
			TriangleIndexArray m_Tris;
			size_t m_TriCount;
		};

		using Octree = std::deque<OctreeNode>;
		mutable Octree m_Octree;
		enum { OCTREE_DEPTH = 2 };
		virtual void InitializeOctree() const;
		static std::vector<OctreeNode *> s_OctreeNodeCache;

	public:

		MeshImpl(RendererImpl *prend);

		virtual ~MeshImpl();

		virtual void Release();

		virtual void AttachVertexBuffer(VertexBuffer *pvertexbuf);

		virtual VertexBuffer *GetVertexBuffer() const;

		virtual void AttachIndexBuffer(IndexBuffer *pindexbuf);

		virtual IndexBuffer *GetIndexBuffer() const;

		virtual const BoundingBox *GetBounds() const;

		void ComputeBounds();

		void SetBounds(const glm::fvec3 &vmin, const glm::fvec3 &vmax);

		virtual RETURNCODE Draw(Renderer::PrimType type) const;

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir,
			float *pDistance, glm::fvec3 *pNormal, size_t *pFaceIndex, glm::vec2 *pUV,
			const glm::fmat4x4 *pMat = nullptr) const;

		virtual bool IsSkin() const;

		void MakeSkinned();

	};

};