// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Component.h>

namespace c3
{

	#define POSFLAG_POSCHANGED			0x0001
	#define POSFLAG_ORICHANGED			0x0002
	#define POSFLAG_SCLCHANGED			0x0004

	#define POSFLAG_REBUILDMATRIX		(POSFLAG_POSCHANGED | POSFLAG_ORICHANGED | POSFLAG_SCLCHANGED)
	#define POSFLAG_MATRIXCHANGED		0x0008

	class C3_API Positionable : public Component
	{

	public:

		static const ComponentType *Type();

		// *** POSITION FUNCTIONS *******************************

		virtual void SetPos(float x, float y, float z) = NULL;
		virtual void SetPosVec(const glm::fvec3 *pos) = NULL;
		virtual void SetPosX(float x) = NULL;
		virtual void SetPosY(float y) = NULL;
		virtual void SetPosZ(float z) = NULL;

		virtual const glm::fvec3 *GetPosVec(glm::fvec3 *pos = nullptr) = NULL;
		virtual float GetPosX() = NULL;
		virtual float GetPosY() = NULL;
		virtual float GetPosZ() = NULL;

		virtual void AdjustPos(float xadj = 0.0f, float yadj = 0.0f, float zadj = 0.0f) = NULL;


		// *** ORIENTATION FUNCTIONS *******************************

		// Set the orientation by providing each element of the quaternion directly
		virtual void SetOri(float x, float y, float z, float w) = NULL;

		// Set the orientation by providing each element of the quaternion directly
		virtual void SetOriQuat(const glm::fquat *ori) = NULL;
		virtual void SetYawPitchRoll(float y, float p, float r) = NULL;

		virtual const glm::fquat *GetOriQuat(glm::fquat *ori = nullptr) = NULL;

		// returns the yaw (in radians)
		virtual float GetYaw() = NULL;

		// returns the pitch (in radians)
		virtual float GetPitch() = NULL;

		// returns the roll (in radians)
		virtual float GetRoll() = NULL;

		// adjust the yaw (in radians)
		virtual void AdjustYaw(float dy) = NULL;

		// adjust the yaw (in radians) about the world-space up axis
		virtual void AdjustYawFlat(float dy) = NULL;

		// adjust the pitch (in radians)
		virtual void AdjustPitch(float dp) = NULL;

		// adjust the roll (in radians)
		virtual void AdjustRoll(float dr) = NULL;

		virtual const glm::fvec3 *GetFacingVector(glm::fvec3 *vec = nullptr) = NULL;
		virtual const glm::fvec3 *GetLocalUpVector(glm::fvec3 *vec = nullptr) = NULL;
		virtual const glm::fvec3 *GetLocalRightVector(glm::fvec3 *vec = nullptr) = NULL;


		// *** SCALE FUNCTIONS *******************************

		virtual void SetScl(float x, float y, float z) = NULL;
		virtual void SetSclVec(const glm::fvec3 *scl) = NULL;
		virtual void SetSclX(float x) = NULL;
		virtual void SetSclY(float y) = NULL;
		virtual void SetSclZ(float z) = NULL;

		virtual const glm::fvec3 *GetScl(glm::fvec3 *scl = nullptr) = NULL;
		virtual float GetSclX() = NULL;
		virtual float GetSclY() = NULL;
		virtual float GetSclZ() = NULL;

		virtual void AdjustScl(float dx = 0.0f, float dy = 0.0f, float dz = 0.0f) = NULL;


		// *** COMBINED TRANSFORM FUNCTIONS ******************

		virtual const glm::fmat4x4 *GetTransformMatrix(glm::fmat4x4 *mat = nullptr) = NULL;

		// Returns the inverse transpose transform
		virtual const glm::fmat4x4 *GetTransformMatrixNormal(glm::fmat4x4 *matn = nullptr) = NULL;

		/// Casts a ray against the object
		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance) const = NULL;

	};

};