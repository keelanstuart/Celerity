// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	class BoundingBox
	{

	public:

		static BoundingBox *Create();
		virtual void Release() = NULL;

		virtual void SetFromBounds(const BoundingBox *box) = NULL;

		virtual void SetOrigin(float x, float y, float z) = NULL;
		virtual void SetOrigin(const glm::fvec3 *org) = NULL;
		virtual const glm::fvec3 *GetOrigin(glm::fvec3 *org = nullptr) const = NULL;

		virtual void SetExtents(float x, float y, float z) = NULL;
		virtual void SetExtents(const glm::fvec3 *ext) = NULL;
		virtual const glm::fvec3 *GetExtents(glm::fvec3 *ext = nullptr) const = NULL;

		virtual void SetAsFrustum(const glm::fmat4x4 *viewmat, const glm::fmat4x4 *projmat, const RECT *viewport) = NULL;

		virtual void Align(const glm::fmat4x4 *matrix) = NULL;

		virtual bool IsPointInside(const glm::fvec3 *ppt) = NULL;
		virtual bool IsBoxInside(const BoundingBox *pbox) = NULL;
		virtual bool IsSphereInside(const glm::fvec3 *centroid, float radius) = NULL;

		virtual bool CheckCollision(const glm::fvec3 *raypos, const glm::fvec3 *rayvec, float *dist = nullptr) = NULL;

		// this will adjust the current bounding box by pbox's aligned bounds
		virtual void IncludeBounds(const BoundingBox *pbox) = NULL;

		virtual const uint16_t *GetIndices() const = NULL;
		virtual const glm::fvec3 *GetCorners() const = NULL;
	};

};
