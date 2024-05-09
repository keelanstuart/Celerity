// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	class C3_API BoundingBox
	{

	public:

		static BoundingBox *Create();
		virtual void Release() = NULL;

		virtual void SetFromBounds(const BoundingBox *box) = NULL;

		virtual void SetExtents(const glm::fvec3 *min_bounds, const glm::fvec3 *max_bounds) = NULL;

		virtual void SetAsFrustum(const glm::fmat4x4 *viewmat, const glm::fmat4x4 *projmat, const RECT *viewport) = NULL;

		virtual void Align(const glm::fmat4x4 *matrix) = NULL;

		virtual bool IsPointInside(const glm::fvec3 *ppt) const = NULL;
		virtual bool IsBoxInside(const BoundingBox *pbox) const = NULL;
		virtual bool IsSphereInside(const glm::fvec3 *centroid, float radius) const = NULL;

		virtual bool CheckCollision(const glm::fvec3 *raypos, const glm::fvec3 *rayvec, float *dist = nullptr) const = NULL;

		// this will adjust the current bounding box by pbox's aligned bounds
		virtual void IncludeBounds(const BoundingBox *pbox) = NULL;

		virtual const uint16_t *GetIndices() const = NULL;
		virtual const glm::fvec3 *GetCorners() const = NULL;
		virtual const glm::fvec3 *GetAlignedCorners() const = NULL;
	};

};
