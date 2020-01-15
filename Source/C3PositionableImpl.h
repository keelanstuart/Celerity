// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Positionable.h>

namespace c3
{

	class PositionableImpl : public Positionable, props::IPropertyChangeListener
	{

	protected:
		props::IProperty *m_ppos;
		glm::vec3 m_Pos;

		props::IProperty *m_pori;
		glm::quat m_Ori;

		props::IProperty *m_pscl;
		glm::vec3 m_Scl;

		bool m_RecalcMatrix;
		glm::mat4x4 m_Mat;

		C3VEC3 m_Facing, m_LocalUp, m_LocalLeft;

	public:

		PositionableImpl();

		virtual ~PositionableImpl();

		virtual void Release();

		virtual ComportmentType *GetType();

		virtual props::TFlags64 Flags();

		virtual bool Initialize(Object *pobject);

		virtual void Update(Object *pobject, float elapsed_time = 0.0f);

		virtual bool Prerender(Object *pobject, props::TFlags64 rendflags);

		virtual void Render(Object *pobject, props::TFlags64 rendflags);

		virtual void PropertyChanged(const props::IPropertySet *ppropset, const props::IProperty *pprop);

		// *** POSITION FUNCTIONS *******************************

		virtual void SetPos(float x, float y, float z);
		virtual void SetPosVec(const C3VEC3 *pos);
		virtual void SetPosX(float x);
		virtual void SetPosY(float y);
		virtual void SetPosZ(float z);

		virtual const C3VEC3 *GetPosVec(C3VEC3 *pos = nullptr);
		virtual float GetPosX();
		virtual float GetPosY();
		virtual float GetPosZ();

		virtual void AdjustPos(float xadj = 0.0f, float yadj = 0.0f, float zadj = 0.0f);


		// *** ROTATION CENTER FUNCTIONS *******************************

		virtual void SetRotCenter(float x, float y, float z);
		virtual void SetRotCenterVec(const C3VEC3 *rotc);
		virtual void SetRotCenterX(float x);
		virtual void SetRotCenterY(float y);
		virtual void SetRotCenterZ(float z);

		virtual const C3VEC3 *GetRotCenterVec(C3VEC3 *rotc = nullptr);
		virtual float GetRotCenterX();
		virtual float GetRotCenterY();
		virtual float GetRotCenterZ();

		virtual void AdjustRotCenter(float dx = 0.0f, float dy = 0.0f, float dz = 0.0f);


		// *** ORIENTATION FUNCTIONS *******************************

		// Set the orientation by providing each element of the quaternion directly
		virtual void SetOri(float x, float y, float z, float w);

		// Set the orientation by providing each element of the quaternion directly
		virtual void SetOriQuat(const C3QUAT *ori);
		virtual void SetYawPitchRoll(float y, float p, float r);

		virtual const C3QUAT *GetOriQuat(C3QUAT *ori = nullptr);

		// returns the yaw (in radians)
		virtual float GetYaw();

		// returns the pitch (in radians)
		virtual float GetPitch();

		// returns the roll (in radians)
		virtual float GetRoll();

		// adjust the yaw (in radians)
		virtual void AdjustYaw(float dy);

		// adjust the pitch (in radians)
		virtual void AdjustPitch(float dp);

		// adjust the roll (in radians)
		virtual void AdjustRoll(float dr);

		virtual const C3VEC3 *GetFacingVector(C3VEC3 *vec = nullptr);
		virtual const C3VEC3 *GetLocalUpVector(C3VEC3 *vec = nullptr);
		virtual const C3VEC3 *GetLocalLeftVector(C3VEC3 *vec = nullptr);


		// *** SCALE FUNCTIONS *******************************

		virtual void SetScl(float x, float y, float z);
		virtual void SetSclVec(const C3VEC3 *scl);
		virtual void SetSclX(float x);
		virtual void SetSclY(float y);
		virtual void SetSclZ(float z);

		virtual const C3VEC3 *GetScl(C3VEC3 *scl = nullptr);
		virtual float GetSclX();
		virtual float GetSclY();
		virtual float GetSclZ();

		virtual void AdjustScl(float dx = 0.0f, float dy = 0.0f, float dz = 0.0f);

	};

	DEFINE_COMPORTMENTTYPE(Positionable, PositionableImpl, GUID({0x26d446ca, 0xda21, 0x4272, {0xb6, 0x66, 0x16, 0xc2, 0xcb, 0xf3, 0x5f, 0x69}}), _T("Positionable"), _T("Allows the Object to be positioned and oriented in space"));

};