// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Renderer.h>
#include <C3Texture.h>

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

		// The texture name (Texture::SetName), if correspondent to the uniform name, will determine the location if -1 is given there
		// a particular sampler can be used, but if one is not given, one will be automatically assigned
		virtual bool SetUniformTexture(Texture *tex, int32_t location = -1, int32_t texunit = -1, props::TFlags32 texflags = TEXFLAG_WRAP_U | TEXFLAG_WRAP_V | TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPLINEAR) = NULL;

		/// Actually sets the uniform values in the program
		virtual void ApplyUniforms(bool update_globals = true) = NULL;

		// Builds a ResourceManager ID string from shader component filenames
		static const TCHAR *ResourceIdentifier(TCHAR *ret, size_t retlc, const TCHAR *vs, const TCHAR *fs, const TCHAR *gs = nullptr, const TCHAR *es = nullptr, const TCHAR *cs = nullptr);

		// Builds a ResourceManager ID string from shader component pointers
		static const TCHAR *ResourceIdentifier(TCHAR *ret, size_t retlc,
			const ShaderComponent *vs, const ShaderComponent *fs,
			const ShaderComponent *gs, const ShaderComponent *es, const ShaderComponent *cs);

	};

};
