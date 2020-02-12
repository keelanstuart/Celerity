// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Comportment.h>


namespace c3
{

	class C3_API Renderable : public Comportment
	{

	public:

		static const ComportmentType *Type();

		virtual void Release() = NULL;

		virtual ComportmentType *GetType() = NULL;

		virtual props::TFlags64 Flags() = NULL;

		virtual bool Initialize(Object *pobject) = NULL;

		virtual void Update(Object *pobject, float elapsed_time = 0.0f) = NULL;

		virtual bool Prerender(Object *pobject, props::TFlags64 rendflags) = NULL;

		virtual void Render(Object *pobject, props::TFlags64 rendflags) = NULL;

		virtual void PropertyChanged(const props::IProperty *pprop) = NULL;

		virtual bool HitTest(glm::fvec3 *ray_pos, glm::fvec3 *rayvec) const = NULL;

		virtual void SetMesh(const Mesh *pmesh) = NULL;

		virtual const Mesh *GetMesh() const = NULL;

		virtual void SetTexture(const Texture *ptex) = NULL;

		virtual const Texture *GetTexture() const = NULL;

		virtual void SetMaterial(const Material *pmaterial) = NULL;

		virtual const Material *GetMaterial() const = NULL;

		virtual void SetShaderProgram(const ShaderProgram *pshader) = NULL;

		virtual const ShaderProgram *GetShaderProgram() const = NULL;

	};

};
