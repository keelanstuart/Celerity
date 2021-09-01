// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	class Frustum
	{

	public:

		typedef enum
		{
			FRUSTFACE_NEAR = 0,
			FRUSTFACE_FAR,
			FRUSTFACE_TOP,
			FRUSTFACE_BOTTOM,
			FRUSTFACE_LEFT,
			FRUSTFACE_RIGHT,

			FRUSTFACE_NUMFACES

		} FrustumFace;

		typedef enum
		{
			FRUSTCORN_xyz = 0,
			FRUSTCORN_Xyz,
			FRUSTCORN_xYz,
			FRUSTCORN_XYz,
			FRUSTCORN_xyZ,
			FRUSTCORN_XyZ,
			FRUSTCORN_xYZ,
			FRUSTCORN_XYZ,

			FRUSTCORN_NUMCORNERS

		} FrustumCorner;

		typedef enum
		{
			FRUSTEDGE_xyz_Xyz = 0,		// x to X edge
			FRUSTEDGE_xYz_XYz,			// x to X edge
			FRUSTEDGE_xyZ_XyZ,			// x to X edge
			FRUSTEDGE_xYZ_XYZ,			// x to X edge

			FRUSTEDGE_xyz_xYz,			// y to Y edge
			FRUSTEDGE_Xyz_XYz,			// y to Y edge
			FRUSTEDGE_xyZ_xYZ,			// y to Y edge
			FRUSTEDGE_XyZ_XYZ,			// y to Y edge

			FRUSTEDGE_xyz_xyZ,			// z to Z edge
			FRUSTEDGE_Xyz_XyZ,			// z to Z edge
			FRUSTEDGE_xYz_xYZ,			// z to Z edge
			FRUSTEDGE_XYz_XYZ,			// z to Z edge

			FRUSTEDGE_NUMEDGES

		} FrustumEdge;

		virtual void CalculateForView(const glm::fmat4x4 *viewmat, const glm::fmat4x4 *projmat) = NULL;

		virtual void CalculateForBounds(const glm::fvec3 *boundsmin, const glm::fvec3 *boundsmax, const glm::fmat4x4 *matrix) = NULL;

		virtual bool IsPointInside(float x, float y, float z) = NULL;

		virtual bool IsPointInside(const glm::fvec3 *point) = NULL;

		virtual bool IsFrustumInside(const Frustum *frust) = NULL;

	};

};
