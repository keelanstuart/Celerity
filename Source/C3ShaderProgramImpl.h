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
		ShaderComponentImpl *m_Comp[Renderer::ShaderComponentType::ST_NUMTYPES];

	public:

		ShaderProgramImpl(RendererImpl *prend);
		virtual ~ShaderProgramImpl();

		virtual void Release();

		virtual ShaderProgram::RETURNCODE AttachShader(ShaderComponent *pshader);
		virtual ShaderProgram::RETURNCODE Link();

		operator GLuint() const { return m_glID; }

	};

};
