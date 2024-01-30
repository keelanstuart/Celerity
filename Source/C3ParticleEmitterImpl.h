// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3ParticleEmitter.h>
#include <deque>
#include <queue>

namespace c3
{

	class ParticleEmitterImpl : public ParticleEmitter, props::IPropertyChangeListener, props::IProperty::IEnumProvider
	{

	protected:

		Object *m_pOwner;

		struct SParticle
		{
			glm::fvec3 pos;
			float lifetime;
			float time;
			float roll;

			glm::fvec3 vel;
			glm::fvec3 acc;
			float rvel;
			float racc;
			size_t texidx;
		};

		using TParticleArray = std::vector<SParticle>;
		using TIndexArray = std::vector<size_t>;
		using TTextureArray = std::deque<c3::Texture2D *>;

		using EmitterShape = enum
		{
			SPHERE,
			RAY,
			CONE,
			CYLINDER,
			PLANE
		};

		TParticleArray m_Particles;
		TIndexArray m_Active, m_Inactive;
		VertexBuffer *m_Verts;
		Material *m_pMaterial;

		float m_Time;
		float m_EmitTime;
		
		EmitterShape m_Shape;

		uint64_t m_MaxParticles;		// maximum number of particles in system

		float m_EmitRateMin;		// minimum time between emitted particles
		float m_EmitRateMax;		// maximum time between emitted particles

		float m_EmitSpeedMin;		// minimum initial particle speed
		float m_EmitSpeedMax;		// maximum initial particle speed

		float m_ParticleLifeMin;	// minimum lifetime of a particle
		float m_ParticleLifeMax;	// maximum lifetime of a particle

		float m_AccelerationMin;	// minimum particle acceleration along emission vector
		float m_AccelerationMax;	// maximum particle acceleration along emission vector

		float m_RollMin;			// roll applied to each texture, in degrees/sec
		float m_RollMax;			// roll applied to each texture, in degrees/sec

		float m_StartScale;			// starting particle size
		float m_PeakScale;			// peak particle size
		float m_EndScale;			// ending particle size

		float m_Gravity;			// a multiplier for environmental gravity

		Color::SRGBAColor m_StartColor;		// starting particle color
		Color::SRGBAColor m_PeakColor;		// peak particle color
		Color::SRGBAColor m_EndColor;		// ending particle color

		float m_InnerRadius;
		float m_OuterRadius;

		RenderMethod *m_pMethod;
		size_t m_TechIdx_G, m_TechIdx_GS, m_TechIdx_S;
		Texture2D *m_pTexture;

		float m_Peak;				// percentage of particle lifetime that it reaches the "peak" values

		//bool m_Billboard;

	public:

		ParticleEmitterImpl();

		virtual ~ParticleEmitterImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags);

		virtual void Render(Object::RenderFlags flags);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats, float *pDistance) const;

		virtual size_t GetNumValues(const props::IProperty *pprop) const;

		virtual const TCHAR *GetValue(const props::IProperty *pprop, size_t ordinal, TCHAR *buf = nullptr, size_t bufsize = 0) const;

	};

	DEFINE_COMPONENTTYPE(ParticleEmitter, ParticleEmitterImpl, GUID({0xbe81dc6a, 0x962, 0x4b77, {0xa6, 0xdf, 0xa9, 0x85, 0x2b, 0x5f, 0x8a, 0x82}}), "ParticleEmitter", "Emits particles (requires Positionable)", 0);

};