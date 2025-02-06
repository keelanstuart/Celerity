// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3ModelRenderer.h>
#include <C3PositionableImpl.h>
#include <C3RenderMethod.h>
#include <C3ModelImpl.h>

namespace c3
{

	class ModelRendererImpl : public ModelRenderer, props::IPropertyChangeListener
	{

	protected:
		Object *m_pOwner;
		Positionable *m_pPos;

		glm::fvec3 m_Pos;
		glm::fquat m_Ori;
		glm::fvec3 m_Scl;
		mutable glm::fmat4x4 m_Mat;

		RenderMethod *m_pMethod;
		size_t m_TechIdx_G, m_TechIdx_GS, m_TechIdx_S;
		std::optional<size_t> m_TechIdx_Override;

		typedef std::pair<Model *, Resource *> TModOrRes;
		TModOrRes m_Mod;

		ModelImpl::InstanceDataImpl *m_Inst;

#define MRIF_REBUILDMATRIX		0x0001

		mutable props::TFlags64 m_Flags;

	public:

		ModelRendererImpl();

		virtual ~ModelRendererImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags, int draworder);

		virtual void Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat = nullptr);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual void SetPos(float x, float y, float z);

		virtual const glm::fvec3 *GetPosVec(glm::fvec3 *pos = nullptr);

		virtual void SetOriQuat(const glm::fquat *ori);

		virtual const glm::fquat *GetOriQuat(glm::fquat *ori = nullptr);

		virtual void SetScl(float x, float y, float z);

		virtual const glm::fvec3 *GetScl(glm::fvec3 *scl = nullptr);

		virtual const glm::fmat4x4 *GetMatrix(glm::fmat4x4 *mat = nullptr) const;

		virtual const Model *GetModel() const;

		virtual Model::InstanceData *GetModelInstanceData();

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance, bool force) const;

	};

	DEFINE_COMPONENTTYPE(ModelRenderer, ModelRendererImpl, GUID({0x15558c71, 0xe301, 0x4911, { 0xa1, 0xa9, 0x8d, 0x88, 0x6c, 0x3c, 0x45, 0xd1 }}), "ModelRenderer", "ModelRenderer draws a Model if it's visible (requires Positionable)", 0);

};