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
				glct = GL_DEPTH24_STENCIL8;
				break;

			case Renderer::EDepthType::F32_D:
				glct = GL_DEPTH_COMPONENT32F;
				break;

			case Renderer::EDepthType::F32_DS:
				glct = GL_DEPTH32F_STENCIL8;
				break;

			case Renderer::EDepthType::F16_SHADOW:
				glct = GL_DEPTH_COMPONENT16;
				break;

			case Renderer::EDepthType::F32_SHADOW:
				glct = GL_DEPTH_COMPONENT32;
				break;
		}

		if (m_Type < Renderer::EDepthType::F16_SHADOW)
		{
			prend->gl.GenRenderbuffers(1, &m_glID);
			prend->gl.BindRenderbuffer(GL_RENDERBUFFER, m_glID);
			prend->gl.RenderbufferStorage(GL_RENDERBUFFER, glct, (GLsizei)m_Width, (GLsizei)m_Height);
		}
		else
		{
			prend->gl.GenTextures(1, &m_glID);
			prend->gl.BindTexture(GL_TEXTURE_2D, m_glID);
			prend->gl.TexImage2D(GL_TEXTURE_2D, 0, glct, (GLsizei)m_Width, (GLsizei)m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			prend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			prend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			prend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			prend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			prend->UseTexture(0, nullptr);

		}

		prend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);
	}
}


DepthBufferImpl::~DepthBufferImpl()
{
	if (m_Rend && (m_glID != NULL))
	{
		if (m_Type < Renderer::DepthType::F16_SHADOW)
			m_Rend->gl.DeleteBuffers(1, &m_glID);
		else
			m_Rend->gl.DeleteTextures(1, &m_glID);

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
