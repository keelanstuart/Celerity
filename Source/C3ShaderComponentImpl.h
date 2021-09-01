// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


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

	DEFINE_RESOURCETYPE(ShaderComponent, RTFLAG_RUNBYRENDERER, GUID({0x8c91d2f4, 0xb414, 0x47ef, {0x88, 0x23, 0xf9, 0x5e, 0xb6, 0xab, 0x22, 0x72}}), "ShaderComponent", "Shader Components", "esh;fsh;gsh;tsh;vsh", "glsl");

};