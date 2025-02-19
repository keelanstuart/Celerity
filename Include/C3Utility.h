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

		using ObjectActionFunc = std::function<void(Object *)>;
		using ObjectActionFuncBreakable = std::function<bool(Object *)>;

		// Runs an ObjectActionFunc function on all objects in the hierarchy given, starting at proot
		void C3_API RecursiveObjectAction(Object *proot, ObjectActionFunc action_func);

		// Runs an ObjectActionFunc function on all objects in the hierarchy given, starting at proot... and it stops if you return false
		bool C3_API RecursiveObjectActionBreakable(Object *proot, ObjectActionFuncBreakable action_func);

		// Finds the best relative path to one of the given candidate paths
		bool C3_API FindShortestRelativePath(const std::vector<tstring> &relative_paths,
			const TCHAR *target_filename,
			TCHAR *shortest_path_buf,
			size_t buf_len);

	};

};