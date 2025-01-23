// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Component.h>


namespace c3
{

	/// <summary>
	/// Physical is a Component that allows motion simulation
	///
	/// Requires: none
	/// 
	/// Recognized Properties:
	///		- 'POS' : "Position" 
	///			+ FVEC3 - The translation of the Object away from it's parent
	/// 
	///		- 'LVEL' : "LinearVelocity"
	///			+ FVEC3 - The velocity of the Object along a line
	/// 
	///		- 'LACC' : "LinearAcceleration"
	///			+ FVEC3 - The acceleration of the Object along a line
	/// 
	///		- 'LSFF' : "LinearSpeedFalloffFactor"
	///			+ FLOAT - Works similarly to friction; degrades the linear speed (scalar)
	/// 
	///		- 'MXLS' : "MaxLinearSpeed"
	///			+ FLOAT - The maximum speed the Object can obtain
	/// 
	///		- 'RVEL' : "RotationalVelocity"
	///			+ FVEC3 - The rotational velocity of the Object in radians
	/// 
	///		- 'DPOS' : "DeltaPosition"
	///			+ FVEC3 - hidden; the amount that the Object moved in the last Update
	/// 
	///		- 'RACC' : "RotationalAcceleration"
	///			+ FVEC3 - The rotational acceleration of the Object in radians
	/// 
	///		- 'RVFF' : "RotationalVelocityFalloffFactor"
	///			+ FVEC3 - The amount that the rotational valocity degrades once acceleration is no longer applied
	/// 
	///		- 'MXRS' : "MaxRotationalSpeed"
	///			+ FVEC3 - The maximum rotational speed that the Object can have (in radians)
	/// 
	///		- 'MASS' : "Mass"
	///			+ float - The Object's mass
	/// 
	/// </summary>


	class C3_API Physical : public Component
	{

	public:

		static const ComponentType *Type();

		using ColliderShape = enum
		{
			NONE = 0,

			MODEL,
			PLANE,
			BOX,
			SPHERE,
			CYLINDER,
			CAPSULE,

			COLLIDER_SHAPE_COUNT
		};

		using CollisionMode = enum
		{
			STATIC,
			KINEMATIC,
			DYNAMIC,

			COLLISION_MODE_COUNT
		};

		virtual void SetColliderShape(ColliderShape t) = NULL;
		virtual ColliderShape GetColliderShape() const = NULL;

		virtual void SetCollisionMode(CollisionMode m) = NULL;
		virtual CollisionMode GetCollisionMode() const = NULL;

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

		virtual void SetMaxLinSpeed(float speed = FLT_MAX) = NULL;
		virtual float GetMaxLinSpeed() = NULL;

		virtual void SetLinSpeedFalloffFactor(float factor = 0.0f) = NULL;


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
		virtual float GetRotVelY() = NULL;
		virtual float GetRotVelP() = NULL;
		virtual float GetRotVelR() = NULL;

		virtual void AdjustRotVel(float yadj = 0.0f, float padj = 0.0f, float radj = 0.0f) = NULL;

		virtual void SetMaxRotSpeed(glm::fvec3 *speed = nullptr) = NULL;
		virtual glm::fvec3 *GetMaxRotSpeed(glm::fvec3 *speed = nullptr) = NULL;

		virtual void SetRotVelFalloffFactor(glm::fvec3 *factor = nullptr) = NULL;
		virtual glm::fvec3 *GetRotVelFalloffFactor(glm::fvec3 *factor = nullptr) = NULL;


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


	};

};