// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


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
			PLANE,

			FORCE_64BIT = 0xFFFFFFFFFFFFFFFF
		};

		TParticleArray m_Particles;
		TIndexArray m_Active, m_Inactive;
		VertexBuffer *m_Verts;
		Material *m_pMaterial;

		float m_Time;
		float m_EmitTime;
		
		EmitterShape m_Shape;

		uint64_t m_MaxParticles;		// maximum number of particles in system

		props::TVec2F m_EmitRate;		// time between emitted particles (min / max)

		props::TVec2F m_EmitSpeed;		// initial particle speed (min / max)

		props::TVec2F m_ParticleLife;	// lifetime of a particle (min / max)

		props::TVec2F m_Acceleration;	// particle acceleration along emission vector (min / max)

		props::TVec2F m_Roll;			// roll applied to each texture, in degrees/sec (min / max)

		float m_StartScale;			// starting particle size
		float m_PeakScale;			// peak particle size
		float m_EndScale;			// ending particle size

		float m_Gravity;			// a multiplier for environmental gravity

		Color::SRGBAColor m_StartColor;		// starting particle color
		Color::SRGBAColor m_PeakColor;		// peak particle color
		Color::SRGBAColor m_EndColor;		// ending particle color

		props::TVec2F m_Radius;			// Inner / outer radius, means different things for different emitter shapes

		RenderMethod *m_pMethod;
		size_t m_TechIdx_G, m_TechIdx_GS, m_TechIdx_S;
		Texture2D *m_pTexture;

		float m_Peak;				// percentage of particle lifetime that it reaches the "peak" values


	public:

		ParticleEmitterImpl();

		virtual ~ParticleEmitterImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags, int draworder);

		virtual void Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance) const;

		virtual size_t GetNumValues(const props::IProperty *pprop) const;

		virtual const TCHAR *GetValue(const props::IProperty *pprop, size_t ordinal, TCHAR *buf = nullptr, size_t bufsize = 0) const;

	};

	DEFINE_COMPONENTTYPE(ParticleEmitter, ParticleEmitterImpl, GUID({0xbe81dc6a, 0x962, 0x4b77, {0xa6, 0xdf, 0xa9, 0x85, 0x2b, 0x5f, 0x8a, 0x82}}), "ParticleEmitter", "Emits particles (requires Positionable)", 0);

};