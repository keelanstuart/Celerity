// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


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
		BoundingBox *m_pBounds;
		bool m_IsSkin;

	public:

		MeshImpl(RendererImpl *prend);

		virtual ~MeshImpl();

		virtual void Release();

		virtual void AttachVertexBuffer(VertexBuffer *pvertexbuf);

		virtual VertexBuffer *GetVertexBuffer() const;

		virtual void AttachIndexBuffer(IndexBuffer *pindexbuf);

		virtual IndexBuffer *GetIndexBuffer() const;

		virtual const BoundingBox *GetBounds();

		void SetBounds(const glm::fvec3 &vmin, const glm::fvec3 &vmax);

		virtual RETURNCODE Draw(Renderer::PrimType type) const;

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir,
			float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV,
			const glm::fmat4x4 *pMat = nullptr) const;

		virtual bool IsSkin() const;

		void MakeSkinned();

	};

};