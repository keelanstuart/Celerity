// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3Utility.h>
#include <C3Positionable.h>
#include <filesystem>
#include <TRaster.h>
#include <C3BlobImpl.h>

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


void util::RecursiveObjectAction(Object *proot, ObjectActionFunc action_func, bool action_first)
{
	assert(action_func);

	if (!proot)
		return;

	if (action_first)
		action_func(proot);

	size_t i = proot->GetNumChildren();
	while (i)
	{
		Object *pchild = proot->GetChild(--i);
		RecursiveObjectAction(pchild, action_func, action_first);
	}

	if (!action_first)
		action_func(proot);
}


bool util::RecursiveObjectActionBreakable(Object *proot, ObjectActionFuncBreakable action_func, bool action_first)
{
	assert(action_func);

	if (!proot)
		return true;

	bool ret = action_first ? action_func(proot) : true;

	size_t i = proot->GetNumChildren();
	while (ret && i)
	{
		Object *pchild = proot->GetChild(--i);
		ret &= RecursiveObjectActionBreakable(pchild, action_func, action_first);
	}

	if (!action_first && ret)
		ret &= action_func(proot);

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


bool util::IsInHeirarchyOf(Object *obj, Object *parent, size_t *depth)
{
	size_t ret = 0;
	while (obj && (obj != parent))
	{
		obj = obj->GetParent();
		ret++;
	}

	if (obj == parent)
	{
		if (depth)
			*depth = ret;
		return true;
	}

	return false;
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


void util::LoadU8Image(ResourceManager *rm, const TCHAR *filename, U8Raster &img)
{
	Resource *hr = rm->GetResource(filename, RESF_DEMANDLOAD, RESOURCETYPE(Blob));

	if (hr && (hr->GetStatus() == Resource::RS_LOADED))
	{
		c3::Blob *pblob = dynamic_cast<Blob *>((Blob *)hr->GetData());

		int w, h, c;
		if (stbi_info_from_memory(pblob->Data(), (int)pblob->Size(), &w, &h, &c) && w && h && (c == 1))
		{
			stbi_uc *src = stbi_load_from_memory(pblob->Data(), (int)pblob->Size(), &w, &h, &c, 0);
			if (src)
			{
				img.Resize(w, h);
				memcpy(img.m_Image.data(), src, img.m_Image.size());
				free(src);
			}
		}

		hr->DelRef();
	}
}


void util::LoadRGBImage(c3::ResourceManager *rm, const TCHAR *filename, RGBRaster &img)
{
	Resource *hr = rm->GetResource(filename, RESF_DEMANDLOAD);

	Texture2D *readtex = nullptr;
	void *readbuf = nullptr;
	Texture2D::SLockInfo readli;

	if (hr && (hr->GetStatus() == Resource::RS_LOADED))
	{
		readtex = dynamic_cast<Texture2D *>((Texture2D *)(hr->GetData()));
		if (readtex && ((readtex->Width() > 1) && (readtex->Height() > 1) && (readtex->Format() == Renderer::U8_3CH)) &&
			(readtex->Lock(&readbuf, readli, 0, TEXLOCKFLAG_READ | TEXLOCKFLAG_CACHE) == Texture::RET_OK))
		{
			img.Resize(readtex->Width(), readtex->Height());

			if (readtex->Format() == Renderer::U8_3CH)
			{
				memcpy(img.m_Image.data(), readbuf, img.m_Image.size() * sizeof(glm::u8vec3));
			}
			else if (readtex->Format() == Renderer::U8_1CH)
			{
				uint8_t *s = (uint8_t *)readbuf;
				glm::u8vec3 *d = img.m_Image.data();
				for (size_t i = 0; i < img.m_Image.size(); i++, s++, d++)
					*d = glm::u8vec3(*s, *s, *s);
			}
			else if (readtex->Format() == Renderer::U8_4CH)
			{
				glm::u8vec4 *s = (glm::u8vec4 *)readbuf;
				glm::u8vec3 *d = img.m_Image.data();
				for (size_t i = 0; i < img.m_Image.size(); i++, s++, d++)
					*d = *s;
			}

			readtex->Unlock();
		}

		hr->DelRef();
	}
	else
	{
		img.Resize(16, 16);
	}
}

void util::LoadRGBAImage(c3::ResourceManager *rm, const TCHAR *filename, RGBARaster &img)
{
	Resource *hr = rm->GetResource(filename, RESF_DEMANDLOAD);

	Texture2D *readtex = nullptr;
	void *readbuf = nullptr;
	Texture2D::SLockInfo readli;

	if (hr && (hr->GetStatus() == Resource::RS_LOADED))
	{
		readtex = dynamic_cast<Texture2D *>((Texture2D *)(hr->GetData()));
		if (readtex && ((readtex->Width() > 1) && (readtex->Height() > 1)) &&
			(readtex->Lock(&readbuf, readli, 0, TEXLOCKFLAG_READ | TEXLOCKFLAG_CACHE) == Texture::RET_OK))
		{
			img.Resize(readtex->Width(), readtex->Height());

			if (readtex->Format() == Renderer::U8_4CH)
			{
				memcpy(img.m_Image.data(), readbuf, img.m_Image.size() * sizeof(glm::u8vec4));
			}
			else if (readtex->Format() == Renderer::U8_1CH)
			{
				uint8_t *s = (uint8_t *)readbuf;
				glm::u8vec4 *d = img.m_Image.data();
				for (size_t i = 0; i < img.m_Image.size(); i++, s++, d++)
					*d = glm::u8vec4(*s, *s, *s, 255);
			}
			else if (readtex->Format() == Renderer::U8_3CH)
			{
				glm::u8vec3 *s = (glm::u8vec3 *)readbuf;
				glm::u8vec4 *d = img.m_Image.data();
				for (size_t i = 0; i < img.m_Image.size(); i++, s++, d++)
					*d = glm::u8vec4(*s, 255);
			}

			readtex->Unlock();
		}

		hr->DelRef();
	}
	else
	{
		img.Resize(16, 16);
	}
}
