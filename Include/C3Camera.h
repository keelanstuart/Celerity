// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Comportment.h>

namespace c3
{

	class Camera : public Comportment
	{

	public:

		typedef enum EViewMode
		{
			VM_POLAR = 0,
			VM_LOOKAT,

			VM_NUMMODES
		} ViewMode;

		typedef enum EProjectionMode
		{
			PM_PERSPECTIVE = 0,
			PM_ORTHOGRAPHIC,

			PM_NUMMODES
		} ProjectionMode;

		virtual void SetViewMode(ViewMode mode) = NULL;
		virtual ViewMode GetViewMode() = NULL;

		virtual void SetProjectionMode(ProjectionMode mode) = NULL;
		virtual ProjectionMode GetProjectionMode() = NULL;

		virtual void SetPolarDistance(float distance) = NULL;
		virtual float GetPolarDistance() = NULL;

		virtual void SetOrthoWidth(float width) = NULL;
		virtual float GetOrthoWidth() = NULL;

		virtual void SetOrthoHeight(float height) = NULL;
		virtual float GetOrthoHeight() = NULL;

		virtual void SetFOV(float height) = NULL;
		virtual float GetFOV() = NULL;

		virtual const C3VEC3 *GetEyePos(C3VEC3 *pos) = NULL;
		virtual const C3VEC3 *GetTargetPos(C3VEC3 *pos) = NULL;

		virtual const C3MATRIX *GetViewMatrix(C3MATRIX *mat) = NULL;
		virtual const C3MATRIX *GetProjectionMatrix(C3MATRIX *mat) = NULL;

	};

};