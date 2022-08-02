// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Component.h>


namespace c3
{

	class C3_API OmniLight : public Component
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

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance) const = NULL;

		virtual void SetSourceFrameBuffer(FrameBuffer *psource) = NULL;

	};

};
