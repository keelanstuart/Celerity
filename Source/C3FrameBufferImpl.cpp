// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3FrameBufferImpl.h>
#include <C3DepthBufferImpl.h>
#include <C3TextureImpl.h>


using namespace c3;


FrameBufferImpl::FrameBufferImpl(RendererImpl* prend)
{
	m_Rend = prend;
	m_glID = GL_INVALID_VALUE;
	m_DepthTarget = nullptr;
	m_ColorTarget.reserve(MAX_COLORTARGETS);

	if (m_Rend)
	{
		prend->gl.GenFramebuffers(1, &m_glID);
	}
}


FrameBufferImpl::~FrameBufferImpl()
{
	if (m_Rend && (m_glID != GL_INVALID_VALUE))
	{
		m_Rend->gl.DeleteFramebuffers(1, &m_glID);
		m_glID = GL_INVALID_VALUE;
	}
}


void FrameBufferImpl::Release()
{
	delete this;
}


FrameBuffer::RETURNCODE FrameBufferImpl::AttachColorTarget(Texture2D *target, size_t position)
{
	static const GLuint targenum[MAX_COLORTARGETS] =
	{
		GL_COLOR_ATTACHMENT0,	GL_COLOR_ATTACHMENT1,	GL_COLOR_ATTACHMENT2,	GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,	GL_COLOR_ATTACHMENT5,	GL_COLOR_ATTACHMENT6,	GL_COLOR_ATTACHMENT7,
		GL_COLOR_ATTACHMENT8,	GL_COLOR_ATTACHMENT9,	GL_COLOR_ATTACHMENT10,	GL_COLOR_ATTACHMENT11,
		GL_COLOR_ATTACHMENT12,	GL_COLOR_ATTACHMENT13,	GL_COLOR_ATTACHMENT14,	GL_COLOR_ATTACHMENT15,
		GL_COLOR_ATTACHMENT16,	GL_COLOR_ATTACHMENT17,	GL_COLOR_ATTACHMENT18,	GL_COLOR_ATTACHMENT19,
		GL_COLOR_ATTACHMENT20,	GL_COLOR_ATTACHMENT21,	GL_COLOR_ATTACHMENT22,	GL_COLOR_ATTACHMENT23,
		GL_COLOR_ATTACHMENT24,	GL_COLOR_ATTACHMENT25,	GL_COLOR_ATTACHMENT26,	GL_COLOR_ATTACHMENT27,
		GL_COLOR_ATTACHMENT28,	GL_COLOR_ATTACHMENT29,	GL_COLOR_ATTACHMENT30,	GL_COLOR_ATTACHMENT31,
	};

	if (m_Rend && (m_glID != GL_INVALID_VALUE))
	{
		if (position <= m_ColorTarget.size())
			m_ColorTarget.resize(position + 1, nullptr);

		m_ColorTarget[position] = target;

		m_Rend->gl.BindFramebuffer(GL_FRAMEBUFFER, m_glID);
		m_Rend->gl.FramebufferTexture(GL_FRAMEBUFFER, targenum[position], (const Texture2DImpl &)*target, 0);

		m_Rend->gl.DrawBuffers((GLsizei)m_ColorTarget.size(), targenum);
	}

	return FrameBuffer::RETURNCODE::RET_OK;
}


size_t FrameBufferImpl::GetNumColorTargets()
{
	return m_ColorTarget.size();
}


Texture2D* FrameBufferImpl::GetColorTarget(size_t position)
{
	return m_ColorTarget[position];
}


FrameBuffer::RETURNCODE FrameBufferImpl::AttachDepthTarget(DepthBuffer* pdepth)
{
	if (!pdepth)
		return FrameBuffer::RETURNCODE::RET_NULLTARGET;

	m_Rend->gl.FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (const DepthBufferImpl &)*pdepth);

	return FrameBuffer::RETURNCODE::RET_OK;
}


DepthBuffer* FrameBufferImpl::GetDepthTarget()
{
	return m_DepthTarget;
}
