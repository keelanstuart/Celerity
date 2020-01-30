// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3ShaderProgram.h>
#include <C3RendererImpl.h>

namespace c3
{

	class ShaderComponentImpl;

	class ShaderProgramImpl : public ShaderProgram
	{

	protected:
		RendererImpl *m_Rend;
		GLuint m_glID;
		bool m_Linked;
		ShaderComponentImpl *m_Comp[Renderer::ShaderComponentType::ST_NUMTYPES];

	public:

		ShaderProgramImpl(RendererImpl *prend);
		virtual ~ShaderProgramImpl();

		virtual void Release();

		virtual ShaderProgram::RETURNCODE AttachShader(ShaderComponent *pshader);
		virtual ShaderProgram::RETURNCODE Link();
		virtual bool IsLinked();

		virtual int64_t GetUniformLocation(const TCHAR *name);
		virtual bool SetUniformMatrix(int64_t location, const glm::fmat4x4 *mat);
		virtual bool SetUniform1(int64_t location, float f);
		virtual bool SetUniform2(int64_t location, const glm::fvec2 *v2);
		virtual bool SetUniform3(int64_t location, const glm::fvec3 *v3);
		virtual bool SetUniform4(int64_t location, const glm::fvec4 *v4);
		virtual bool SetUniformTexture(int64_t location, uint64_t sampler, Texture *tex);

		operator GLuint() const { return m_glID; }

	};

};
