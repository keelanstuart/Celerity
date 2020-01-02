// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class C3_API MatrixStack
	{

	public:

		static MatrixStack *Create();

		virtual void Release() = NULL;

		virtual void Push(const C3MATRIX *m = nullptr) = NULL;
		virtual bool Pop() = NULL;

		virtual const C3MATRIX *Top(C3MATRIX *m) const = NULL;

	};

};
