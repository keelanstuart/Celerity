// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#include "pch.h"

#include <C3DepthBufferImpl.h>


using namespace c3;


DepthBufferImpl::DepthBufferImpl(RendererImpl* prend, size_t width, size_t height, Renderer::EDepthType type)
{
	m_Rend = prend;
	m_Type = type;
	m_Width = width;
	m_Height = height;
	m_glID = NULL;

	if (m_Rend)
	{
		GLenum glct = GL_DEPTH_COMPONENT;

		switch (m_Type)
		{
			case Renderer::EDepthType::U16_D:
				glct = GL_DEPTH_COMPONENT16;
				break;

			case Renderer::EDepthType::U32_D:
				glct = GL_DEPTH_COMPONENT32;
				break;

			case Renderer::EDepthType::U32_DS:
				glct = GL_DEPTH24_STENCIL8;// GL_DEPTH_COMPONENT24;
				break;

			case Renderer::EDepthType::F32_D:
				glct = GL_DEPTH_COMPONENT32F; //GL_DEPTH_COMPONENT32F;
				break;

			case Renderer::EDepthType::F32_DS:
				glct = GL_DEPTH32F_STENCIL8; //GL_DEPTH_COMPONENT32F;
				break;
		}

#if 0
		if (m_Rend->isnv)
		{
			prend->gl.GenRenderbuffersEXT(1, &m_glID);
			prend->gl.BindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_glID);
			prend->gl.RenderbufferStorageEXT(GL_RENDERBUFFER_EXT, glct, (GLsizei)m_Width, (GLsizei)m_Height);
		}
		else
#endif
		{
			prend->gl.GenRenderbuffers(1, &m_glID);
			prend->gl.BindRenderbuffer(GL_RENDERBUFFER, m_glID);
			prend->gl.RenderbufferStorage(GL_RENDERBUFFER, glct, (GLsizei)m_Width, (GLsizei)m_Height);
		}

		prend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);
	}
}


DepthBufferImpl::~DepthBufferImpl()
{
	if (m_Rend && (m_glID != NULL))
	{
		m_Rend->gl.DeleteBuffers(1, &m_glID);
		m_glID = NULL;
	}
}


void DepthBufferImpl::Release()
{
	delete this;
}


Renderer::EDepthType DepthBufferImpl::Format()
{
	return m_Type;
}


size_t DepthBufferImpl::Width()
{
	return m_Width;
}


size_t DepthBufferImpl::Height()
{
	return m_Height;
}
