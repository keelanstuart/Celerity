// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Component.h>


namespace c3
{

	class C3_API ModelRenderer : public Component
	{

	public:

		static const ComponentType *Type();

		virtual void Release() = NULL;

		virtual ComponentType *GetType() = NULL;

		virtual props::TFlags64 Flags() = NULL;

		virtual bool Initialize(Object *pobject) = NULL;

		virtual void Update(Object *pobject, float elapsed_time = 0.0f) = NULL;

		virtual bool Prerender(Object *pobject, props::TFlags64 rendflags) = NULL;

		virtual void Render(Object *pobject, props::TFlags64 rendflags) = NULL;

		virtual void PropertyChanged(const props::IProperty *pprop) = NULL;

		virtual bool HitTest(glm::fvec3 *ray_pos, glm::fvec3 *rayvec) const = NULL;

		/*
		virtual void SetModel(const Model *pmodel) = NULL;

		virtual void SetModel(const Resource *pmodelres) = NULL;

		virtual const Model *GetModel() const = NULL;

		virtual void SetShaderProgram(const ShaderProgram *pshader) = NULL;

		virtual const ShaderProgram *GetShaderProgram() const = NULL;
		*/
	};

};