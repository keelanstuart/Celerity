// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3ShaderComponent.h>
#include <C3RendererImpl.h>

namespace c3
{

	class ShaderComponentImpl : public ShaderComponent
	{

	protected:
		RendererImpl *m_Rend;
		GLuint m_glID;
		Renderer::ShaderComponentType m_Type;
		GLuint m_glType;
		bool m_Compiled;
		tstring m_ProgramText;

	public:
		ShaderComponentImpl(RendererImpl *prend, Renderer::ShaderComponentType type);
		virtual ~ShaderComponentImpl();

		virtual void Release();

		virtual Renderer::ShaderComponentType Type();

		virtual ShaderComponent::RETURNCODE CompileProgram(const TCHAR *program, const TCHAR *preamble);

		virtual const TCHAR *GetProgramText();

		virtual bool IsCompiled();

		operator GLuint() const { return m_glID; }

	};

};