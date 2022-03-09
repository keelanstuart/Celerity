// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class C3_API MatrixStack
	{

	public:

		static MatrixStack *Create();

		virtual void Release() = NULL;

		virtual void Push(const glm::fmat4x4 *m = nullptr) = NULL;
		virtual bool Pop() = NULL;

		virtual const glm::fmat4x4 *Top(glm::fmat4x4 *m = nullptr) const = NULL;

	};

};
