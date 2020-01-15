// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3ShaderProgramImpl.h>
#include <C3ShaderComponentImpl.h>


using namespace c3;


ShaderProgramImpl::ShaderProgramImpl(RendererImpl *prend)
{
	m_Rend = prend;
	m_glID = GL_INVALID_VALUE;
	memset(m_Comp, 0, sizeof(ShaderComponentImpl *) * Renderer::ShaderComponentType::ST_NUMTYPES);
}


ShaderProgramImpl::~ShaderProgramImpl()
{
	if (m_Rend && (m_glID != GL_INVALID_VALUE))
	{
		for (UINT i = (Renderer::ShaderComponentType::ST_NONE + 1), maxi = Renderer::ShaderComponentType::ST_NUMTYPES; i < maxi; i++)
		{
			if (!m_Comp[i])
				continue;

			m_Rend->gl.DetachShader(m_glID, (ShaderComponentImpl &)*m_Comp[i]);
			m_Comp[i] = nullptr;
		}

		m_Rend->gl.DeleteProgram(m_glID);
		m_glID = GL_INVALID_VALUE;
	}
}


void ShaderProgramImpl::Release()
{
	delete this;
}


ShaderProgram::RETURNCODE ShaderProgramImpl::AttachShader(ShaderComponent *pshader)
{
	if (!pshader)
		return ShaderProgram::RETURNCODE::RET_NULL_SHADER;

	int shtype = pshader->Type();
	if ((shtype < 0) || (shtype >= c3::Renderer::ShaderComponentType::ST_NUMTYPES))
		return ShaderProgram::RETURNCODE::RET_BAD_TYPE;

	if (m_glID == GL_INVALID_VALUE)
		m_glID = m_Rend->gl.CreateProgram();

	if (m_glID == GL_INVALID_VALUE)
		return ShaderProgram::RETURNCODE::RET_CREATE_FAILED;

	if (m_Comp[shtype])
		m_Rend->gl.DetachShader(m_glID, (ShaderComponentImpl &)*m_Comp[shtype]);
	m_Comp[shtype] = (ShaderComponentImpl *)pshader;

	m_Rend->gl.AttachShader(m_glID, (ShaderComponentImpl &)*pshader);

	return ShaderProgram::RETURNCODE::RET_OK;
}


ShaderProgram::RETURNCODE ShaderProgramImpl::Link()
{
	if (m_glID == GL_INVALID_VALUE)
		return ShaderProgram::RETURNCODE::RET_CREATE_FAILED;

	m_Rend->gl.LinkProgram(m_glID);

	return ShaderProgram::RETURNCODE::RET_OK;
}
