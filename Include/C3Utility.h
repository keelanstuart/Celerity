// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	namespace util
	{

		// Recursively builds the transform for the given Object using its parent(s)
		void C3_API ComputeFinalTransform(Object *proot, glm::fmat4x4 *pmat);

		void C3_API RecursiveObjectAction(Object *proot, std::function<void(Object *)> action_func);

	};

};