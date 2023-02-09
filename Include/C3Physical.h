// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Component.h>


namespace c3
{

	class C3_API Physical : public Component
	{

	public:

		static const ComponentType *Type();

#if 0
		// *** LINEAR VELOCITY FUNCTIONS *******************************

		virtual void SetLinVel(float x, float y, float z) = NULL;
		virtual void SetLinVelVec(const glm::fvec3 *lvel) = NULL;
		virtual void SetLinVelX(float x) = NULL;
		virtual void SetLinVelY(float y) = NULL;
		virtual void SetLinVelZ(float z) = NULL;

		virtual const glm::fvec3 *GetLinVel(glm::fvec3 *vel = nullptr) = NULL;
		virtual float GetLinVelX() = NULL;
		virtual float GetLinVelY() = NULL;
		virtual float GetLinVelZ() = NULL;

		virtual void AdjustLinVel(float xadj = 0.0f, float yadj = 0.0f, float zadj = 0.0f) = NULL;


		// *** LINEAR ACCELERATION FUNCTIONS *******************************

		virtual void SetLinAcc(float x, float y, float z) = NULL;
		virtual void SetLinAccVec(const glm::fvec3 *lacc) = NULL;
		virtual void SetLinAccX(float x) = NULL;
		virtual void SetLinAccY(float y) = NULL;
		virtual void SetLinAccZ(float z) = NULL;

		virtual const glm::fvec3 *GetLinAcc(glm::fvec3 *lacc = nullptr) = NULL;
		virtual float GetLinAccX() = NULL;
		virtual float GetLinAccY() = NULL;
		virtual float GetLinAccZ() = NULL;

		virtual void AdjustLinAcc(float xadj = 0.0f, float yadj = 0.0f, float zadj = 0.0f) = NULL;


		// *** ROTATIONAL VELOCITY FUNCTIONS *******************************

		virtual void SetRotVel(float y, float p, float r) = NULL;
		virtual void SetRotVelVec(const glm::fvec3 *rvel) = NULL;
		virtual void SetRotVelY(float y) = NULL;
		virtual void SetRotVelP(float p) = NULL;
		virtual void SetRotVelR(float r) = NULL;

		virtual const glm::fvec3 *GetRotVel(glm::fvec3 *rvel = nullptr) = NULL;
		virtual float GetRoitVelY() = NULL;
		virtual float GetRoitVelP() = NULL;
		virtual float GetRoitVelR() = NULL;

		virtual void AdjustRotVel(float yadj = 0.0f, float padj = 0.0f, float radj = 0.0f) = NULL;


		// *** ROTATIONAL ACCELERATION FUNCTIONS *******************************

		virtual void SetRotAcc(float y, float p, float r) = NULL;
		virtual void SetRotAccVec(const glm::fvec3 *racc) = NULL;
		virtual void SetRotAccY(float y) = NULL;
		virtual void SetRotAccP(float p) = NULL;
		virtual void SetRotAccR(float r) = NULL;

		virtual const glm::fvec3 *GetRotAcc(glm::fvec3 *racc = nullptr) = NULL;
		virtual float GetRotAccY() = NULL;
		virtual float GetRotAccP() = NULL;
		virtual float GetRotAccR() = NULL;

		virtual void AdjustRotAcc(float yadj = 0.0f, float padj = 0.0f, float radj = 0.0f) = NULL;
#endif


		/// Casts a ray against the object
		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance) const = NULL;

	};

};