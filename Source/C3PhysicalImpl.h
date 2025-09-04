// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Physical.h>

#if defined(ODE_SUPPORT) && ODE_SUPPORT
#include <ode\ode.h>
#include <ode\objects.h>
#include <ode\collision_space.h>
#endif

#define USE_PHYSICS_MANAGER		FALSE

namespace c3
{

	class PhysicalImpl : public Physical, props::IPropertyChangeListener, props::IProperty::IEnumProvider
	{
		friend class PhysicsManagerImpl;

	protected:

		Object *m_pOwner;

		glm::vec3 m_LinVel;
		glm::vec3 m_LinAcc;
		float m_LinSpeedFalloff;
		float m_maxLinSpeed;

		glm::vec3 m_RotVel;
		glm::vec3 m_RotAcc;
		glm::vec3 m_RotVelFalloff;
		glm::vec3 m_maxRotSpeed;

		props::TFlags64 m_Flags;

		Positionable *m_pPositionable;
		glm::fvec3 m_DeltaPos;

		ColliderShape m_ColliderShape;
		CollisionMode m_CollisionMode;

#if defined(ODE_SUPPORT) && ODE_SUPPORT
		union
		{
			dBodyID u_ODEBody;
			dGeomID u_ODEGeom;
		};

		// TODO
		dMass m_ODEMass;
#endif
		float m_Mass;

	public:

		PhysicalImpl();

		virtual ~PhysicalImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags, int draworder);

		virtual void Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance, glm::fvec3 *pNormal, bool force) const;

		virtual size_t GetNumValues(const props::IProperty *pprop) const;

		virtual const TCHAR *GetValue(const props::IProperty *pprop, size_t ordinal, TCHAR *buf = nullptr, size_t bufsize = 0) const;

		virtual void SetColliderShape(ColliderShape t);
		virtual ColliderShape GetColliderShape() const;

		virtual void SetCollisionMode(CollisionMode m);
		virtual CollisionMode GetCollisionMode() const;

		// *** LINEAR VELOCITY FUNCTIONS *******************************

		virtual void SetLinVel(float x, float y, float z);
		virtual void SetLinVelVec(const glm::fvec3 *lvel);
		virtual void SetLinVelX(float x);
		virtual void SetLinVelY(float y);
		virtual void SetLinVelZ(float z);

		virtual const glm::fvec3 *GetLinVel(glm::fvec3 *vel = nullptr);
		virtual float GetLinVelX();
		virtual float GetLinVelY();
		virtual float GetLinVelZ();

		virtual void AdjustLinVel(float xadj = 0.0f, float yadj = 0.0f, float zadj = 0.0f);

		virtual void SetMaxLinSpeed(float speed = FLT_MAX);
		virtual float GetMaxLinSpeed();

		virtual void SetLinSpeedFalloffFactor(float factor = 0.0f);


		// *** LINEAR ACCELERATION FUNCTIONS *******************************

		virtual void SetLinAcc(float x, float y, float z);
		virtual void SetLinAccVec(const glm::fvec3 *lacc);
		virtual void SetLinAccX(float x);
		virtual void SetLinAccY(float y);
		virtual void SetLinAccZ(float z);

		virtual const glm::fvec3 *GetLinAcc(glm::fvec3 *lacc = nullptr);
		virtual float GetLinAccX();
		virtual float GetLinAccY();
		virtual float GetLinAccZ();

		virtual void AdjustLinAcc(float xadj = 0.0f, float yadj = 0.0f, float zadj = 0.0f);


		// *** ROTATIONAL VELOCITY FUNCTIONS *******************************

		virtual void SetRotVel(float y, float p, float r);
		virtual void SetRotVelVec(const glm::fvec3 *rvel);
		virtual void SetRotVelY(float y);
		virtual void SetRotVelP(float p);
		virtual void SetRotVelR(float r);

		virtual const glm::fvec3 *GetRotVel(glm::fvec3 *rvel = nullptr);
		virtual float GetRotVelY();
		virtual float GetRotVelP();
		virtual float GetRotVelR();

		virtual void AdjustRotVel(float yadj = 0.0f, float padj = 0.0f, float radj = 0.0f);

		virtual void SetMaxRotSpeed(glm::fvec3 *speed = nullptr);
		virtual glm::fvec3 *GetMaxRotSpeed(glm::fvec3 *speed = nullptr);

		virtual void SetRotVelFalloffFactor(glm::fvec3 *factor = nullptr);
		virtual glm::fvec3 *GetRotVelFalloffFactor(glm::fvec3 *factor = nullptr);


		// *** ROTATIONAL ACCELERATION FUNCTIONS *******************************

		virtual void SetRotAcc(float y, float p, float r);
		virtual void SetRotAccVec(const glm::fvec3 *racc);
		virtual void SetRotAccY(float y);
		virtual void SetRotAccP(float p);
		virtual void SetRotAccR(float r);

		virtual const glm::fvec3 *GetRotAcc(glm::fvec3 *racc = nullptr);
		virtual float GetRotAccY();
		virtual float GetRotAccP();
		virtual float GetRotAccR();

		virtual void AdjustRotAcc(float yadj = 0.0f, float padj = 0.0f, float radj = 0.0f);

	};

	DEFINE_COMPONENTTYPE(Physical, PhysicalImpl, GUID({0xf8641b51, 0xa0fc, 0x4d93, {0x89, 0x2f, 0x48, 0xb, 0x7, 0xba, 0x28, 0x61}}), "Physical", "Allows the Object to move through space, requires Positional", 0);

};