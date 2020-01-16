// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Renderer.h>

namespace c3
{

	class ShaderProgram
	{

	public:

		enum RETURNCODE
		{
			RET_OK = 0,

			RET_CREATE_FAILED,
			RET_NULL_SHADER,
			RET_BAD_TYPE,
			RET_PROGRAM_INCOMPLETE,
			RET_LINK_FAILED,
			RET_NOT_INITIALIZED
		};

		virtual void Release() = NULL;

		virtual RETURNCODE AttachShader(ShaderComponent *pshader) = NULL;
		virtual RETURNCODE Link() = NULL;
		virtual bool IsLinked() = NULL;

		virtual int64_t GetUniformLocation(const TCHAR *name) = NULL;
		virtual bool SetUniformMatrix(int64_t location, const glm::fmat4x4 *mat) = NULL;
		virtual bool SetUniform1(int64_t location, float f) = NULL;
		virtual bool SetUniform2(int64_t location, const glm::fvec2 *v2) = NULL;
		virtual bool SetUniform3(int64_t location, const glm::fvec3 *v3) = NULL;
		virtual bool SetUniform4(int64_t location, const glm::fvec4 *v4) = NULL;
		virtual bool SetUniformTexture(int64_t location, Texture *tex) = NULL;

	};

};
