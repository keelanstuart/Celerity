// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3OmniLight.h>
#include <C3PositionableImpl.h>
#include <C3BoundingBoxImpl.h>

namespace c3
{

	class OmniLightImpl : public OmniLight, props::IPropertyChangeListener
	{

	protected:
		Object *m_pOwner;
		Positionable *m_pPos;
		RenderMethod *m_pMethod;
		size_t m_TechIdx_L;
		FrameBuffer *m_SourceFB;
		glm::fvec3 m_Color;
		int32_t m_uniPos;
		int32_t m_uniRadius;
		int32_t m_uniColor;
		int32_t m_uniScreenSize;
		int32_t m_uniSampDiff;
		int32_t m_uniSampNorm;
		int32_t m_uniSampPosDepth;
		int32_t m_uniSampEmisRough;
		int32_t m_uniTexAtten;
		BoundingBoxImpl m_Bounds;
		Resource *m_TexAttenRes;
		Material *m_Material;

		props::TFlags64 m_Flags;

	public:

		OmniLightImpl();

		virtual ~OmniLightImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags, int draworder);

		virtual void Render(Object::RenderFlags rendflags);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats, float *pDistance) const;

		virtual void SetSourceFrameBuffer(FrameBuffer *psource);

	};

	DEFINE_COMPONENTTYPE(OmniLight, OmniLightImpl, GUID({0xfc8732ad, 0x580b, 0x4a78, { 0xa2, 0xc9, 0xff, 0x1, 0xdf, 0x93, 0x95, 0x6f }}), "OmniLight", "OmniLight provides an omni-directional light source (requires Positionable)", 0);

};