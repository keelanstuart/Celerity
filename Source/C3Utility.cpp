// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3Utility.h>
#include <C3Positionable.h>
#include <filesystem>

using namespace c3;


void util::ComputeFinalTransform(Object *proot, glm::fmat4x4 *pmat)
{
	assert(pmat);

	if (!proot)
		return;

	static glm::fmat4x4 imat = glm::identity<glm::fmat4x4>();
	Positionable *ppos = (Positionable *)proot->FindComponent(Positionable::Type());
	const glm::fmat4x4 *mat = ppos ? ppos->GetTransformMatrix() : &imat;
	*pmat = *mat * *pmat;

	ComputeFinalTransform(proot->GetParent(), pmat);
}


void util::RecursiveObjectAction(Object *proot, ObjectActionFunc action_func)
{
	assert(action_func);

	if (!proot)
		return;

	action_func(proot);

	for (size_t i = 0, maxi = proot->GetNumChildren(); i < maxi; i++)
	{
		Object *pchild = proot->GetChild(i);
		RecursiveObjectAction(pchild, action_func);
	}
}


bool util::RecursiveObjectActionBreakable(Object *proot, ObjectActionFuncBreakable action_func)
{
	assert(action_func);

	if (!proot)
		return true;

	bool ret = action_func(proot);

	for (size_t i = 0, maxi = proot->GetNumChildren(); (i < maxi) && ret; i++)
	{
		Object *pchild = proot->GetChild(i);
		ret &= RecursiveObjectActionBreakable(pchild, action_func);
	}

	return ret;
}


void util::ObjectArrayAction(TObjectArray &objs, ObjectActionFunc action_func)
{
	if (!objs.size() || !action_func)
		return;

	int64_t i = (int64_t)objs.size();
	do
	{
		action_func(objs[--i]);
	}
	while (i > 0);
}


bool util::FindShortestRelativePath(const std::vector<tstring> &relative_paths,
	const TCHAR *target_filename,
	TCHAR *shortest_path_buf,
	size_t buf_len)
{
	// Don't allow filenames bounded by [] pairs
	if (target_filename && (*target_filename == _T('[')))
	{
		size_t c = _tcslen(target_filename);
		if (target_filename[c - 1] == _T(']'))
			return false;
	}

#if 1
	constexpr size_t MAX_BUF = MAX_PATH * 2;
	TCHAR fullTarget[MAX_BUF];

	// Even though the target is supposed to be a full path, we call GetFullPathName to be safe.
	if (!GetFullPathName(target_filename, MAX_BUF, fullTarget, nullptr))
		return false;

	bool found = false;
	tstring bestRelative;
	size_t bestLength = std::numeric_limits<size_t>::max();

	// Iterate through each candidate directory
	for (std::vector<tstring>::const_iterator it = relative_paths.cbegin(); it != relative_paths.cend(); it++)
	{
		TCHAR fullDir[MAX_BUF];

		// Convert the candidate into an absolute path if it's not already one
		if (!GetFullPathName(it->c_str(), MAX_BUF, fullDir, nullptr))
			continue; // Skip if we cannot resolve the full path.

		// Prepare a buffer to receive the relative path.
		TCHAR relPath[MAX_BUF];

		// Since our candidate is a directory, we pass FILE_ATTRIBUTE_DIRECTORY.
		// And for the target file we assume a normal file, FILE_ATTRIBUTE_NORMAL.
		if (PathRelativePathTo(relPath, fullDir, FILE_ATTRIBUTE_DIRECTORY, fullTarget, FILE_ATTRIBUTE_NORMAL))
		{
			size_t len = _tcslen(relPath);

			// We choose the candidate with the smallest string length.
			if (len < bestLength)
			{
				bestLength = len;
				bestRelative = relPath;
				found = true;
			}
		}
	}

	if (found)
	{
		// remove ".\"
		const TCHAR *rp = bestRelative.c_str();
		if (bestRelative.find(_T(".\\")) == 0)
			rp += 2;

		_tcsncpy_s(shortest_path_buf, buf_len, rp, buf_len);
		return true;
	}

	// None of the candidate directories are related.
	// In that case, return the full target path.
	//shortest_path_to_target_filename = fullTarget;
	return false;

#else
	namespace fs = std::filesystem;
	std::error_code ec;

	// Resolve the absolute path for the target.
	fs::path full_target = fs::absolute(target_filename, ec);
	if (ec)
		return false;

	bool found = false;
	tstring bestRelative;
	size_t bestLength = std::numeric_limits<size_t>::max();

	// Iterate over candidate directories.
	for (const auto &relDir : relative_paths)
	{
		// Resolve the absolute path for the candidate directory.
		fs::path full_dir = fs::absolute(relDir, ec);
		if (ec)
			continue;

		// Attempt to compute the relative path from this directory to the target.
		fs::path candidate_relative = fs::relative(full_target, full_dir, ec);
		if (ec)
			continue; // Skip if a relative path can't be computed.

		// Convert the relative path to a tstring.
		tstring candidateStr = candidate_relative.native();
		if (candidateStr.length() < bestLength)
		{
			bestLength = candidateStr.length();
			bestRelative = candidateStr;
			found = true;
		}
	}

	if (found)
	{
		_tcsncpy_s(shortest_path_buf, buf_len, rp, buf_len);
		return true;
	}

	// If no candidate directory provided a valid relative path,
	// return the absolute target path.
	return false;

#endif
}
