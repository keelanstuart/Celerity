// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3ModelRenderer.h>
#include <C3PositionableImpl.h>

namespace c3
{

	class ModelRendererImpl : public ModelRenderer, props::IPropertyChangeListener
	{

	protected:
		Object *m_pOwner;
		Positionable *m_pPos;
		ShaderComponent *m_VS_defobj;
		ShaderComponent *m_FS_defobj;
		ShaderProgram *m_SP_defobj;

		typedef std::pair<Model *, Resource *> TModOrRes;
		TModOrRes m_Mod;

		props::TFlags64 m_Flags;

	public:

		ModelRendererImpl();

		virtual ~ModelRendererImpl();

		virtual void Release();

		virtual FeatureType *GetType();

		virtual props::TFlags64 Flags();

		virtual bool Initialize(Object *pobject);

		virtual void Update(Object *pobject, float elapsed_time = 0.0f);

		virtual bool Prerender(Object *pobject, props::TFlags64 rendflags);

		virtual void Render(Object *pobject, props::TFlags64 rendflags);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool HitTest(glm::fvec3 *ray_pos, glm::fvec3 *rayvec) const;

	};

	DEFINE_FEATURETYPE(ModelRenderer, ModelRendererImpl, GUID({0x15558c71, 0xe301, 0x4911, { 0xa1, 0xa9, 0x8d, 0x88, 0x6c, 0x3c, 0x45, 0xd1 }}), "ModelRenderer", "ModelRenderer draws a Model if it's visible (requires Positionable)");

};