// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3Math.h>

// Hermite Spline function...
// s is in the range of [0..1]
// ap is the start point (when s == 0, p == ap)
// at is the start direction vector
// bp is the end point (when s == 1, p == bp)
// bt is the end direction vector
// p is the output of the function
// pd is an optional output - first derivative of the function
// pdd is an optional output - second derivative of the function
void c3::math::Hermite(float s, glm::fvec3 &ap, glm::fvec3 &at, glm::fvec3 &bp, glm::fvec3 &bt, glm::fvec3 &p, glm::fvec3 *pd, glm::fvec3 *pdd)
{
	// There are some values which can be computed once for all equations here...
	float s_sq = s * s;
	float s_cu = s_sq * s;

	float two_s_cu = s_cu + s_cu;
	float two_s_sq = s_sq + s_sq;
	float three_s_sq = two_s_sq + s_sq;

	// There are repeated computations related to solving for the spline equation...
	// So, calculate them up front for performance.
	float pcomp[4];
	pcomp[0] = (two_s_cu - three_s_sq + 1);
	pcomp[1] = (three_s_sq - two_s_cu);
	pcomp[2] = (s_cu - two_s_sq + s);
	pcomp[3] = (s_cu - s_sq);

	// Calculate the spline point in each axis
	p.x = (pcomp[0] * ap.x) + (pcomp[1] * bp.x) + (pcomp[2] * at.x) + (pcomp[3] * bt.x);
	p.y = (pcomp[0] * ap.y) + (pcomp[1] * bp.y) + (pcomp[2] * at.y) + (pcomp[3] * bt.y);
	p.z = (pcomp[0] * ap.z) + (pcomp[1] * bp.z) + (pcomp[2] * at.z) + (pcomp[3] * bt.z);

	if (pd || pdd)
	{
		glm::fvec3 _pd;
		if (!pd)
			pd = &_pd;

		// There are also repeated computations related to solving for the spline equation's first derivative...
		// Calculate those up front for performance.
		float pdcomp[4];
		pdcomp[0] = (6 * (s_sq - s));
		pdcomp[1] = (6 * (s - s_sq));
		pdcomp[2] = (three_s_sq - 3);
		pdcomp[3] = (three_s_sq - 2);

		// Compute the x, y, and z components of the first derivative vector
		pd->x = (pdcomp[0] * ap.x) + (pdcomp[1] * bp.x) + (pdcomp[2] * at.x) + (pdcomp[3] * bt.x);
		pd->y = (pdcomp[0] * ap.y) + (pdcomp[1] * bp.y) + (pdcomp[2] * at.y) + (pdcomp[3] * bt.y);
		pd->z = (pdcomp[0] * ap.z) + (pdcomp[1] * bp.z) + (pdcomp[2] * at.z) + (pdcomp[3] * bt.z);

		if (pdd)
		{
			// Lastly, pre-calculate the repeated elements of the spline equation's second derivative...
			float pddcomp[2];
			pddcomp[0] = (12 * s);
			pddcomp[1] = (6 * s);

			// Compute the x, y, and z components of the second derivative vector
			pdd->x = (pddcomp[0] * (ap.x - bp.x)) + (pddcomp[1] * (at.x + bt.x)) + (6 * (bp.x + ap.x)) - ((4 * at.x) + (2 * bt.x));
			pdd->y = (pddcomp[0] * (ap.y - bp.y)) + (pddcomp[1] * (at.y + bt.y)) + (6 * (bp.y + ap.y)) - ((4 * at.y) + (2 * bt.y));
			pdd->z = (pddcomp[0] * (ap.z - bp.z)) + (pddcomp[1] * (at.z + bt.z)) + (6 * (bp.z + ap.z)) - ((4 * at.z) + (2 * bt.z));
		}
	}
}


float c3::math::GetYaw(const glm::fquat *q)
{
	// Extract the tangent of the yaw angle (rotation about the z-axis) from the orientation quaternion
	float tanyaw = (2 * ((q->x * q->y) + (q->w * q->z))) / ((q->w * q->w) + (q->x * q->x) - (q->y * q->y) - (q->z * q->z));

	return (float)atan(tanyaw);
}


float c3::math::GetPitch(const glm::fquat *q)
{
	// Extract the sine of the pitch angle (rotation about the x-axis) from the orientation quaternion
	float sinpitch = -2 * ((q->x * q->z) - (q->w * q->y));

	return (float)asin(sinpitch);
}


float c3::math::GetRoll(const glm::fquat *q)
{
	// Extract the tangent of the roll angle (rotation about the y-axis) from the orientation quaternion
	float tanroll = (2 * ((q->w * q->x) + (q->y * q->z))) / ((q->w * q->w) - (q->x * q->x) - (q->y * q->y) + (q->z * q->z));

	return (float)atan(tanroll);
}


float c3::math::RandomRange(float lo, float hi)
{
	return ((float)rand() / (float)RAND_MAX) * (hi - lo) + lo;
}
