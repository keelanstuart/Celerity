// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <deque>


namespace c3
{
	typedef std::deque<Object *> TObjectArray;

	namespace util
	{

		// Recursively builds the transform for the given Object using its parent(s)
		void C3_API ComputeFinalTransform(Object *proot, glm::fmat4x4 *pmat);

		using ObjectActionFunc = std::function<void(Object *)>;
		using ObjectActionFuncBreakable = std::function<bool(Object *)>;

		// Runs an ObjectActionFunc function on all objects in the hierarchy given, starting at proot
		void C3_API RecursiveObjectAction(Object *proot, ObjectActionFunc action_func, bool action_first = true);

		// Runs an ObjectActionFunc function on all objects in the hierarchy given, starting at proot... and it stops if you return false
		bool C3_API RecursiveObjectActionBreakable(Object *proot, ObjectActionFuncBreakable action_func, bool action_first = true);

		// Runs an ObjectActionFunc function on all objects in give list
		void C3_API ObjectArrayAction(TObjectArray &objs, ObjectActionFunc action_func);

		// Finds the best relative path to one of the given candidate paths
		bool C3_API FindShortestRelativePath(const std::vector<tstring> &relative_paths,
			const TCHAR *target_filename,
			TCHAR *shortest_path_buf,
			size_t buf_len);

		// Returns if the obj is in the hierarchy of parent (and optionally the depth)
		bool C3_API IsInHeirarchyOf(Object *obj, Object *parent, size_t *depth = nullptr);

		using SnapDirection = enum
		{
			POSZ = 0,
			NEGZ,

			POSY,
			NEGY,

			POSX,
			NEGX,

			NUMDIRS
		};

		// Moves the given Object to a position that is on the surface of the near collidable Object in the given direction
		bool C3_API SnapTo(Object *obj, SnapDirection sd);

	};

};