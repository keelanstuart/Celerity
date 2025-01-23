// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <c3.h>

namespace c3
{

	class VertexBuffer;
	class IndexBuffer;

	// A Mesh associates a VertexBuffer and an IndexBuffer, meaning you could re-use either for multiple Meshes

	class Mesh
	{

	public:

		enum RETURNCODE
		{
			RET_OK = 0,
			RET_NOTVISIBLE,
			RET_NULLBUFFER,
			RET_FAILED,
		};

		// Releases any memory allocated by this Mesh
		virtual void Release() = NULL;

		// Makes this Mesh use the given VertexBuffer for rendering
		virtual void AttachVertexBuffer(VertexBuffer *pvertexbuf) = NULL;

		// Returns the VertexBuffer that is currently active for this Mesh
		virtual VertexBuffer *GetVertexBuffer() const = NULL;

		// Makes this Mesh use the given IndexBuffer for rendering
		virtual void AttachIndexBuffer(IndexBuffer *pindexbuf) = NULL;

		// Returns the IndexBuffer that is currently active for this Mesh
		virtual IndexBuffer *GetIndexBuffer() const = NULL;

		// Returns the BoundingBox that represents the extents of the geometry referenced by the Mesh
		virtual const BoundingBox *GetBounds() const = NULL;

		// Renders the Mesh
		virtual RETURNCODE Draw(Renderer::PrimType type = Renderer::PrimType::TRILIST) const = NULL;

		// Casts a ray at this Mesh, returning whether a collision occurred
		// pDistance, if provided, will be filled with a float representing the distance away from pRayPos
		// pFaceIndex, if provided, will be filled with the index of the face on which the collision occurred
		// pUV, if provided, will be filled with the UV coordinates where the collision occurred
		// pMat, indicates how the Mesh may have been transformed
		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir,
			float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV,
			const glm::fmat4x4 *pMat = nullptr) const = NULL;

		// Returns true if the mesh is a skin
		virtual bool IsSkin() const = NULL;

	};

};