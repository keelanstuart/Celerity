// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


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

		glm::fvec3 m_p;
		glm::fquat m_o;
		glm::fvec3 m_s;
		glm::fmat4x4 m_m;

		ShaderComponent *m_VS_defobj;
		ShaderComponent *m_FS_defobj;
		ShaderProgram *m_SP_defobj;

		ShaderComponent *m_VS_shadowobj;
		ShaderComponent *m_FS_shadowobj;
		ShaderProgram *m_SP_shadowobj;

		typedef std::pair<Model *, Resource *> TModOrRes;
		TModOrRes m_Mod;

#define MRIF_REBUILDMATRIX		0x0001

		props::TFlags64 m_Flags;

	public:

		ModelRendererImpl();

		virtual ~ModelRendererImpl();

		virtual void Release();

		virtual ComponentType *GetType();

		virtual props::TFlags64 Flags();

		virtual bool Initialize(Object *pobject);

		virtual void Update(Object *pobject, float elapsed_time = 0.0f);

		virtual bool Prerender(Object *pobject, props::TFlags64 rendflags);

		virtual void Render(Object *pobject, props::TFlags64 rendflags);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool HitTest(glm::fvec3 *ray_pos, glm::fvec3 *rayvec) const;

		virtual void SetPos(float x, float y, float z);

		virtual const glm::fvec3 *GetPosVec(glm::fvec3 *pos = nullptr);

		virtual void SetOriQuat(const glm::fquat *ori);

		virtual const glm::fquat *GetOriQuat(glm::fquat *ori = nullptr);

		virtual void SetScl(float x, float y, float z);

		virtual const glm::fvec3 *GetScl(glm::fvec3 *scl = nullptr);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance) const;

	};

	DEFINE_COMPONENTTYPE(ModelRenderer, ModelRendererImpl, GUID({0x15558c71, 0xe301, 0x4911, { 0xa1, 0xa9, 0x8d, 0x88, 0x6c, 0x3c, 0x45, 0xd1 }}), "ModelRenderer", "ModelRenderer draws a Model if it's visible (requires Positionable)");

};