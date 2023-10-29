// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <c3.h>


namespace c3
{

	namespace math
	{

		/// Computes the hermite spline position at percent_traveled between points A and B, given tangent vectors through those points
		/// Optionally computes the first and second derivatives
		/// percent_traveled: [0 .. 1]
		/// p: position along the spline
		/// pd: where the first derivative should be stored, or nullptr if you don't want it
		/// pdd: where the second derivative should be stored, or nullptr if you don't want it
		void C3_API Hermite(float percent_traveled,
					 glm::fvec3 &point_A, glm::fvec3 &tangent_A,
					 glm::fvec3 &point_B, glm::fvec3 &tangent_B,
					 glm::fvec3 &p, glm::fvec3 *pd = nullptr, glm::fvec3 *pdd = nullptr);

		float C3_API GetYaw(const glm::fquat *q);

		float C3_API GetPitch(const glm::fquat *q);

		float C3_API GetRoll(const glm::fquat *q);

	};

};