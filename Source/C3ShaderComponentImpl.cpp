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


ShaderComponent::RETURNCODE ShaderComponentImpl::CompileProgram(const TCHAR *program)
{
	if (!program)
		return ShaderComponent::RETURNCODE::RET_NULL_PROGRAM;

	if (m_glID == GL_INVALID_VALUE)
		m_glID = m_Rend->gl.CreateShader(m_glType);

	if (m_glID == GL_INVALID_VALUE)
		return ShaderComponent::RETURNCODE::RET_CREATE_FAILED;

	char *ps;
	CONVERT_TCS2MBCS(program, ps);
	m_Rend->gl.ShaderSource(m_glID, 1, &ps, NULL);

	m_Rend->gl.CompileShader(m_glID);

	m_Compiled = true;

	GLint compiled = 0;
	m_Rend->gl.GetShaderiv(m_glID, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
	{
		GLint maxlen = 0;
		m_Rend->gl.GetShaderiv(m_glID, GL_INFO_LOG_LENGTH, &maxlen);

		// The maxLength includes the NULL character
		std::string log;
		log.resize(maxlen);
		m_Rend->gl.GetShaderInfoLog(m_glID, maxlen, &maxlen, &log[0]);

		m_Rend->GetSystem()->GetLog()->Print(_T("* shader compile error:\n"));
		m_Rend->GetSystem()->GetLog()->Print(_T("\t%s\n\n"), log.c_str());

		m_Compiled = false;
	}

	if (!m_Compiled)
		return ShaderComponent::RETURNCODE::RET_COMPILE_FAILED;

	return ShaderComponent::RETURNCODE::RET_OK;
}
