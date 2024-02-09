// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	class AnimKey
	{

	public:

		virtual void SetComposedTransform(const glm::fmat4 &mat) = 0;

		virtual void SetTranslation(float x, float y, float z) = 0;

		virtual void SetTranslationVec(const glm::fvec3 &trans) = 0;

		virtual void SetRotation(float y, float p, float r) = 0;

		virtual void SetRotationVec(const glm::fvec3 &rot) = 0;

		virtual void SetScaling(float x, float y, float z) = 0;

		virtual void SetScalingVec(const glm::fvec3 &scl) = 0;

		virtual void SetNote(const TCHAR *note) = 0;

		virtual const glm::fmat4 *GetComposedTransform() const = 0;

		virtual const glm::fvec3 *GetTranslationVec() const = 0;

		virtual const glm::fvec3 *GetRotationVec() const = 0;

		virtual const glm::fvec3 *GetScalingVec() const = 0;

		virtual const TCHAR *GetNote() const = 0;

	};

};
