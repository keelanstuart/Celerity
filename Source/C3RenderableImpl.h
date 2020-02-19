// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Renderable.h>
#include <C3PositionableImpl.h>

namespace c3
{

	class RenderableImpl : public Renderable, props::IPropertyChangeListener
	{

	protected:
		Positionable *m_pcpos;

		props::TFlags64 m_Flags;

	public:

		RenderableImpl();

		virtual ~RenderableImpl();

		virtual void Release();

		virtual FeatureType *GetType();

		virtual props::TFlags64 Flags();

		virtual bool Initialize(Object *pobject);

		virtual void Update(Object *pobject, float elapsed_time = 0.0f);

		virtual bool Prerender(Object *pobject, props::TFlags64 rendflags);

		virtual void Render(Object *pobject, props::TFlags64 rendflags);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool HitTest(glm::fvec3 *ray_pos, glm::fvec3 *rayvec) const;

		virtual void SetMesh(const Mesh *pmesh);

		virtual const Mesh *GetMesh() const;

		virtual void SetTexture(const Texture *ptex);

		virtual const Texture *GetTexture() const;

		virtual void SetMaterial(const Material *pmaterial);

		virtual const Material *GetMaterial() const;

		virtual void SetShaderProgram(const ShaderProgram *pshader);

		virtual const ShaderProgram *GetShaderProgram() const;

	};

	DEFINE_FEATURETYPE(Renderable, RenderableImpl, GUID({0x15558c71, 0xe301, 0x4911, { 0xa1, 0xa9, 0x8d, 0x88, 0x6c, 0x3c, 0x45, 0xd1 }}), "Renderable", "Renderable draws a Model if it's visible (requires Positionable)");

};