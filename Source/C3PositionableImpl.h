// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Positionable.h>

namespace c3
{

#define POSFLAG_POSCHANGED			0x0001
#define POSFLAG_ORICHANGED			0x0002
#define POSFLAG_SCLCHANGED			0x0004

#define POSFLAG_REBUILDMATRIX		(POSFLAG_POSCHANGED | POSFLAG_ORICHANGED | POSFLAG_SCLCHANGED)
#define POSFLAG_MATRIXCHANGED		0x0008

	class PositionableImpl : public Positionable, props::IPropertyChangeListener
	{

	protected:

		Object *m_pOwner;

		glm::vec3 m_Pos;
		glm::fquat m_Ori;
		glm::vec3 m_Scl;

		props::TFlags64 m_Flags;
		mutable glm::fmat4x4 m_Mat, m_MatN;

		mutable glm::fvec3 m_Facing, m_LocalUp, m_LocalRight;

		inline void UpdateTransformsAndVectors();

	public:

		PositionableImpl();

		virtual ~PositionableImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags, int draworder);

		virtual void Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat);

		virtual void PropertyChanged(const props::IProperty *pprop);

		// *** POSITION FUNCTIONS *******************************

		virtual void SetPos(float x, float y, float z);
		virtual void SetPosVec(const glm::fvec3 *pos);
		virtual void SetPosX(float x);
		virtual void SetPosY(float y);
		virtual void SetPosZ(float z);

		virtual const glm::fvec3 *GetPosVec(glm::fvec3 *pos = nullptr);
		virtual float GetPosX();
		virtual float GetPosY();
		virtual float GetPosZ();

		virtual void AdjustPos(float xadj = 0.0f, float yadj = 0.0f, float zadj = 0.0f);


		// *** ORIENTATION FUNCTIONS *******************************

		// Set the orientation by providing each element of the quaternion directly
		virtual void SetOri(float x, float y, float z, float w);

		// Set the orientation by providing each element of the quaternion directly
		virtual void SetOriQuat(const glm::fquat *ori);
		virtual void SetYawPitchRoll(float y, float p, float r);

		virtual const glm::fquat *GetOriQuat(glm::fquat *ori = nullptr);

		// returns the yaw (in radians)
		virtual float GetYaw();

		// returns the pitch (in radians)
		virtual float GetPitch();

		// returns the roll (in radians)
		virtual float GetRoll();

		// adjust the yaw (in radians)
		virtual void AdjustYaw(float dy);

		// adjust the yaw (in radians) about the world-space up axis
		virtual void AdjustYawFlat(float dy);

		// adjust the pitch (in radians)
		virtual void AdjustPitch(float dp);

		// adjust the roll (in radians)
		virtual void AdjustRoll(float dr);

		virtual const glm::fvec3 *GetFacingVector(glm::fvec3 *vec = nullptr) const;
		virtual const glm::fvec3 *GetLocalUpVector(glm::fvec3 *vec = nullptr) const;
		virtual const glm::fvec3 *GetLocalRightVector(glm::fvec3 *vec = nullptr) const;


		// *** SCALE FUNCTIONS *******************************

		virtual void SetScl(float x, float y, float z);
		virtual void SetSclVec(const glm::fvec3 *scl);
		virtual void SetSclX(float x);
		virtual void SetSclY(float y);
		virtual void SetSclZ(float z);

		virtual const glm::fvec3 *GetScl(glm::fvec3 *scl = nullptr);
		virtual float GetSclX();
		virtual float GetSclY();
		virtual float GetSclZ();

		virtual void AdjustScl(float dx = 0.0f, float dy = 0.0f, float dz = 0.0f);


		// *** COMBINED TRANSFORM FUNCTIONS ******************

		virtual const glm::fmat4x4 *GetTransformMatrix(glm::fmat4x4 *mat = nullptr);

		virtual const glm::fmat4x4 *GetTransformMatrixNormal(glm::fmat4x4 *matn = nullptr);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance, bool force) const;

	};

	DEFINE_COMPONENTTYPE(Positionable, PositionableImpl, GUID({0x26d446ca, 0xda21, 0x4272, {0xb6, 0x66, 0x16, 0xc2, 0xcb, 0xf3, 0x5f, 0x69}}), "Positionable", "Allows the Object to be positioned and oriented in space", 0);

};