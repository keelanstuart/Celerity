// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Physical.h>

namespace c3
{

	class PhysicalImpl : public Physical, props::IPropertyChangeListener
	{

	protected:
		glm::vec3 m_LinVel;
		glm::vec3 m_LinAcc;
		float m_maxLinSpeed;

		glm::vec3 m_RotVel;
		glm::vec3 m_RotAcc;
		glm::vec3 m_maxRotSpeed;

		props::TFlags64 m_Flags;

		Positionable *m_pPositionable;

	public:

		PhysicalImpl();

		virtual ~PhysicalImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(Object *pobject, float elapsed_time = 0.0f);

		virtual bool Prerender(Object *pobject, Object::RenderFlags flags);

		virtual void Render(Object *pobject, Object::RenderFlags flags);

		virtual void PropertyChanged(const props::IProperty *pprop);

#if 0
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
		virtual float GetRoitVelY();
		virtual float GetRoitVelP();
		virtual float GetRoitVelR();

		virtual void AdjustRotVel(float yadj = 0.0f, float padj = 0.0f, float radj = 0.0f);


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
#endif


		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance) const;

	};

	DEFINE_COMPONENTTYPE(Physical, PhysicalImpl, GUID({0x26d446ca, 0xda21, 0x4272, {0xb6, 0x66, 0x16, 0xc2, 0xcb, 0xf3, 0x5f, 0x69}}), "Physical", "Allows the Object to move through space, requires Positional");

};