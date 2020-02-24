// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3ShaderComponentImpl.h>


using namespace c3;


ShaderComponentImpl::ShaderComponentImpl(RendererImpl *prend, Renderer::ShaderComponentType type)
{
	m_Rend = prend;

	m_Type = type;
	switch (m_Type)
	{
		case Renderer::ShaderComponentType::ST_VERTEX:
			m_glType = GL_VERTEX_SHADER;
			break;

		case Renderer::ShaderComponentType::ST_FRAGMENT:
			m_glType = GL_FRAGMENT_SHADER;
			break;

		case Renderer::ShaderComponentType::ST_GEOMETRY:
			m_glType = GL_GEOMETRY_SHADER;
			break;

		case Renderer::ShaderComponentType::ST_TESSEVAL:
			m_glType = GL_TESS_EVALUATION_SHADER;
			break;

		case Renderer::ShaderComponentType::ST_TESSCONTROL:
			m_glType = GL_TESS_CONTROL_SHADER;
			break;
	}

	m_glID = GL_INVALID_VALUE;
	m_Compiled = false;
}


ShaderComponentImpl::~ShaderComponentImpl()
{
	if (m_Rend && (m_glID != GL_INVALID_VALUE))
	{
		m_Rend->gl.DeleteShader(m_glID);
		m_glID = GL_INVALID_VALUE;
	}
}


void ShaderComponentImpl::Release()
{
	delete this;
}


Renderer::ShaderComponentType ShaderComponentImpl::Type()
{
	return m_Type;
}


ShaderComponent::RETURNCODE ShaderComponentImpl::CompileProgram(const TCHAR *program, const TCHAR *preamble)
{
	if (!program)
		return ShaderComponent::RETURNCODE::RET_NULL_PROGRAM;

	if (m_glID == GL_INVALID_VALUE)
		m_glID = m_Rend->gl.CreateShader(m_glType);

	if (m_glID == GL_INVALID_VALUE)
		return ShaderComponent::RETURNCODE::RET_CREATE_FAILED;

	m_ProgramText = program;

	size_t progidx = preamble ? 1 : 2;
	char *ps[3] = {"#version 410", nullptr, nullptr};
	CONVERT_TCS2MBCS(program, ps[progidx]);
	if (preamble)
	{
		CONVERT_TCS2MBCS(preamble, ps[1]);
	}
	m_Rend->gl.ShaderSource(m_glID, preamble ? 3 : 2, ps, NULL);

	m_Rend->gl.CompileShader(m_glID);

	m_Compiled = true;

	GLint compiled = 0;
	m_Rend->gl.GetShaderiv(m_glID, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
	{
		m_Rend->GetSystem()->GetLog()->Print(_T("* shader compile error:"));

		GLint maxlen = 0;
		m_Rend->gl.GetShaderiv(m_glID, GL_INFO_LOG_LENGTH, &maxlen);

		if (maxlen)
		{
			// The maxLength includes the NULL character
			char *pserr = (char *)_alloca(maxlen);
			m_Rend->gl.GetShaderInfoLog(m_glID, maxlen, &maxlen, pserr);
			TCHAR *tpserr;
			CONVERT_MBCS2TCS(pserr, tpserr);

			m_Rend->GetSystem()->GetLog()->Print(_T("\n\t%s\n\n"), tpserr);
		}
		else
		{
			m_Rend->GetSystem()->GetLog()->Print(_T(" unspecified\n\n"));
		}

		m_Compiled = false;
	}

	if (!m_Compiled)
		return ShaderComponent::RETURNCODE::RET_COMPILE_FAILED;

	return ShaderComponent::RETURNCODE::RET_OK;
}


const TCHAR *ShaderComponentImpl::GetProgramText()
{
	return m_ProgramText.c_str();
}


bool ShaderComponentImpl::IsCompiled()
{
	return m_Compiled;
}
