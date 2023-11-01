// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class C3_API MatrixStack
	{

	public:

		// Creates a MatrixStack - useful for maintaining hierarchies of transformations for recursive traversal
		static MatrixStack *Create();

		// Releases the memory allocated by this MatrixStack
		virtual void Release() = NULL;

		// Pushes a transformation onto the stack, concatenating the results, accessible via a call to Top
		virtual void Push(const glm::fmat4x4 *m = nullptr) = NULL;

		// Removes a transformation from the top of the stack
		virtual bool Pop() = NULL;

		// Retrieves the concatenated transformation at the top of the stack
		virtual const glm::fmat4x4 *Top(glm::fmat4x4 *m = nullptr) const = NULL;

	};

};
