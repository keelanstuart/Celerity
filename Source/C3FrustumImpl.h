// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	class FrustumImpl : public Frustum
	{

	protected:
		glm::fvec4 face[FRUSTFACE_NUMFACES];			// a plane representing each of the six faces
		glm::fvec3 corner[FRUSTCORN_NUMCORNERS];		// a point in space representing each of the eight corners
		uint32_t edge[FRUSTEDGE_NUMEDGES][2];		// a line segment representing each of the twelve edges

	public:
		FrustumImpl();

		virtual ~FrustumImpl();

		virtual void CalculateForView(const glm::fmat4x4 *viewmat, const glm::fmat4x4 *projmat);

		virtual void CalculateForBounds(const glm::fvec3 *boundsmin, const glm::fvec3 *boundsmax, const glm::fmat4x4 *matrix);

		virtual void CalculateForBounds(float halfdim, const glm::fmat4x4 *matrix);

		virtual bool IsPointInside(float x, float y, float z);

		virtual bool IsPointInside(const glm::fvec3 *point);

		virtual bool IsFrustumInside(const Frustum *frust);

	};

};