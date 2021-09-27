// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3OmniLight.h>
#include <C3PositionableImpl.h>

namespace c3
{

	class OmniLightImpl : public OmniLight, props::IPropertyChangeListener
	{

	protected:
		Positionable *m_pcpos;

		props::TFlags64 m_Flags;

	public:

		OmniLightImpl();

		virtual ~OmniLightImpl();

		virtual void Release();

		virtual FeatureType *GetType();

		virtual props::TFlags64 Flags();

		virtual bool Initialize(Object *pobject);

		virtual void Update(Object *pobject, float elapsed_time = 0.0f);

		virtual bool Prerender(Object *pobject, props::TFlags64 rendflags);

		virtual void Render(Object *pobject, props::TFlags64 rendflags);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool HitTest(glm::fvec3 *ray_pos, glm::fvec3 *rayvec) const;

		virtual void SetShaderProgram(const ShaderProgram *pshader);

		virtual void SetShaderProgram(const Resource *pshaderres);

		virtual const ShaderProgram *GetShaderProgram() const;

	};

	DEFINE_FEATURETYPE(OmniLight, OmniLightImpl, GUID({0xfc8732ad, 0x580b, 0x4a78, { 0xa2, 0xc9, 0xff, 0x1, 0xdf, 0x93, 0x95, 0x6f }}), "OmniLight", "OmniLight provides an omni-directional light source (requires Positionable)");

};