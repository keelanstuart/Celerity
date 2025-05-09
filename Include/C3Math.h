// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


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

		// Derives the rotation (in radians) about the Z axis, given an input quaternion
		float C3_API GetYaw(const glm::fquat *q);

		// Derives the rotation (in radians) about the X axis, given an input quaternion
		float C3_API GetPitch(const glm::fquat *q);

		// Derives the rotation (in radians) about the Y axis, given an input quaternion
		float C3_API GetRoll(const glm::fquat *q);

		float C3_API RandomRange(float lo, float hi);

		template <class T> struct Rect2D
		{
			T left, top, right, bottom;

			Rect2D() { }
			Rect2D(T l, T t, T r, T b) : left(l), top(t), right(r), bottom(b) { }

			inline T Width()
			{
				return std::abs(right - left);
			}

			inline T Height()
			{
				return std::abs(top - bottom);
			}
		};

		using FRect2D = Rect2D<float>;

	};

};