// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Component.h>

namespace c3
{

	#define CAMFLAG_REBUILDMATRICES		0x0001

	class C3_API Camera : public Component
	{

	public:

		static const ComponentType *Type();

		using ViewMode = enum EViewMode
		{
			VM_POLAR = 0,
			VM_LOOKAT,

			VM_NUMMODES,

			VM_FORCE64BIT = 0xFFFFFFFFFFFFFFFF

		};

		using ProjectionMode = enum EProjectionMode
		{
			PM_PERSPECTIVE = 0,
			PM_ORTHOGRAPHIC,

			PM_NUMMODES,

			PM_FORCE64BIT = 0xFFFFFFFFFFFFFFFF

		};

		virtual void SetViewMode(ViewMode mode) = NULL;
		virtual ViewMode GetViewMode() = NULL;

		virtual void SetProjectionMode(ProjectionMode mode) = NULL;
		virtual ProjectionMode GetProjectionMode() = NULL;

		virtual void SetPolarDistance(float distance) = NULL;
		virtual float GetPolarDistance() = NULL;

		virtual void SetOrthoDimensions(float dimx, float dimy) = NULL;
		virtual void SetOrthoDimensionsVec(const glm::fvec2 *dim) = NULL;
		virtual const glm::fvec2 *GetOrthoDimensions(glm::fvec2 *dim = nullptr) = NULL;
		virtual float GetOrthoWidth() = NULL;
		virtual float GetOrthoHeight() = NULL;

		virtual void SetFOV(float height) = NULL;
		virtual float GetFOV() = NULL;

		virtual const glm::fvec3 *GetEyePos(glm::fvec3 *pos = nullptr) = NULL;
		virtual const glm::fvec3 *GetTargetPos(glm::fvec3 *pos = nullptr) = NULL;

		virtual const glm::fmat4x4 *GetViewMatrix(glm::fmat4x4 *mat = nullptr) = NULL;
		virtual const glm::fmat4x4 *GetProjectionMatrix(glm::fmat4x4 *mat = nullptr) = NULL;

	};

};