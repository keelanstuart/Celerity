// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


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

		enum { INVALID_UNIFORM = -1 };

		virtual int32_t GetUniformLocation(const TCHAR *name) = NULL;
		virtual bool SetUniformMatrix(int32_t location, const glm::fmat4x4 *mat) = NULL;
		virtual bool SetUniform1(int32_t location, float f) = NULL;
		virtual bool SetUniform2(int32_t location, const glm::fvec2 *v2) = NULL;
		virtual bool SetUniform3(int32_t location, const glm::fvec3 *v3) = NULL;
		virtual bool SetUniform4(int32_t location, const glm::fvec4 *v4) = NULL;
		virtual bool SetUniformTexture(int32_t location, uint32_t sampler, Texture *tex) = NULL;

		/// Actually sets the uniform values in the program
		virtual void ApplyUniforms(bool update_globals = true) = NULL;

	};

};
