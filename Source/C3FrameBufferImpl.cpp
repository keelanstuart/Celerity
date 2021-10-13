// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#include "pch.h"

#include <C3FrameBufferImpl.h>
#include <C3DepthBufferImpl.h>
#include <C3TextureImpl.h>


using namespace c3;


const GLuint FrameBufferImpl::targenum[MAX_COLORTARGETS] =
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


FrameBufferImpl::FrameBufferImpl(RendererImpl* prend)
{
	m_Rend = prend;
	m_glID = GL_INVALID_VALUE;
	m_DepthTarget = nullptr;
	m_BlendMode = Renderer::BlendMode::BM_NUMMODES;

	GLint maxAttach = 0;
	m_Rend->gl.GetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);

	m_ColorTarget.reserve(maxAttach);

	if (m_Rend)
	{
		prend->gl.GenFramebuffers(1, &m_glID);
		prend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);
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
	if (m_Rend && (m_glID != GL_INVALID_VALUE))
	{
		FrameBuffer *curfb = m_Rend->GetActiveFrameBuffer();
		m_Rend->UseFrameBuffer(this);

		if (position <= m_ColorTarget.size())
			m_ColorTarget.resize(position + 1, nullptr);

		m_ColorTarget[position] = target;

		if (target)
		{
			m_Rend->gl.FramebufferTexture(GL_FRAMEBUFFER, targenum[position], (const Texture2DImpl &)*target, 0);
			m_Rend->FlushErrors(_T("AttachColorTarget: %s"), __FILEW__, __LINE__);
		}

		m_Rend->UseFrameBuffer(curfb);
	}

	return FrameBuffer::RETURNCODE::RET_OK;
}


size_t FrameBufferImpl::GetNumColorTargets()
{
	return m_ColorTarget.size();
}


Texture2D *FrameBufferImpl::GetColorTarget(size_t position)
{
	if (position < m_ColorTarget.size())
		return m_ColorTarget[position];

	return nullptr;
}


Texture2D *FrameBufferImpl::GetColorTargetByName(const TCHAR *name)
{
	for (auto targ : m_ColorTarget)
	{
		if (!targ)
			continue;

		if (!_tcsicmp(name, targ->GetName()))
			return targ;
	}

	return nullptr;
}


FrameBuffer::RETURNCODE FrameBufferImpl::AttachDepthTarget(DepthBuffer* pdepth)
{
	if (!pdepth)
		return FrameBuffer::RETURNCODE::RET_NULLTARGET;

	if (m_Rend)
	{
		FrameBuffer *curfb = m_Rend->GetActiveFrameBuffer();
		m_Rend->UseFrameBuffer(this);

		GLenum attach_type = ((pdepth->Format() == Renderer::DepthType::F32_DS) || (pdepth->Format() == Renderer::DepthType::U32_DS)) ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;

		m_Rend->gl.FramebufferRenderbuffer(GL_FRAMEBUFFER, attach_type, GL_RENDERBUFFER, (const DepthBufferImpl &)*pdepth);
		m_Rend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);
		m_DepthTarget = pdepth;

		m_Rend->UseFrameBuffer(curfb);
	}

	return FrameBuffer::RETURNCODE::RET_OK;
}


DepthBuffer* FrameBufferImpl::GetDepthTarget()
{
	return m_DepthTarget;
}


FrameBuffer::RETURNCODE FrameBufferImpl::Seal()
{
	RETURNCODE ret = RETURNCODE::RET_INCOMPLETE;

	if (m_Rend)
	{
		FrameBuffer *curfb = m_Rend->GetActiveFrameBuffer();
		m_Rend->UseFrameBuffer(this);

		m_Rend->gl.DrawBuffers((GLsizei)m_ColorTarget.size(), targenum);
		m_Rend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);

		m_Rend->GetSystem()->GetLog()->Print(_T("FrameBuffer::Seal - "));
		GLenum status = (GLenum) m_Rend->gl.CheckFramebufferStatus(GL_FRAMEBUFFER);
		switch(status)
		{
			case GL_FRAMEBUFFER_COMPLETE:
				ret = RETURNCODE::RET_OK;
				break;

			case GL_FRAMEBUFFER_UNSUPPORTED:
				m_Rend->GetSystem()->GetLog()->Print(_T("Unsupported framebuffer format\n"));
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				m_Rend->GetSystem()->GetLog()->Print(_T("Framebuffer incomplete, missing attachment\n"));
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				m_Rend->GetSystem()->GetLog()->Print(_T("Framebuffer incomplete, missing draw buffer\n"));
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				m_Rend->GetSystem()->GetLog()->Print(_T("Framebuffer incomplete, missing read buffer\n"));
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				m_Rend->GetSystem()->GetLog()->Print(_T("Framebuffer incomplete, incomplete attachment\n"));
				break;

			default:
				m_Rend->GetSystem()->GetLog()->Print(_T("Error %x\n"), status);
				break;
		}

		m_Rend->UseFrameBuffer(curfb);
	}

	return ret;
}


void FrameBufferImpl::SetBlendMode(Renderer::BlendMode mode)
{
	if (mode != m_BlendMode)
	{
		if ((m_BlendMode != Renderer::BlendMode::BM_ALPHA) && (m_BlendMode != Renderer::BlendMode::BM_ADD) && (m_BlendMode != Renderer::BlendMode::BM_ADDALPHA))
			m_Rend->gl.Enablei(GL_BLEND, m_glID);
		else if ((mode != Renderer::BlendMode::BM_ALPHA) && (mode != Renderer::BlendMode::BM_ADD) && (mode != Renderer::BlendMode::BM_ADDALPHA))
			m_Rend->gl.Disablei(GL_BLEND, m_glID);

		switch (mode)
		{
			case Renderer::BlendMode::BM_ALPHA:
				m_Rend->gl.BlendFunci(m_glID, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;

			case Renderer::BlendMode::BM_ADD:
				m_Rend->gl.BlendFunci(m_glID, GL_ONE, GL_ONE);
				break;

			case Renderer::BlendMode::BM_ADDALPHA:
				m_Rend->gl.BlendFunci(m_glID, GL_SRC_ALPHA, GL_ONE);
				break;

			case Renderer::BlendMode::BM_REPLACE:
				m_Rend->gl.BlendFunci(m_glID, GL_ONE, GL_ZERO);
				break;

			case Renderer::BlendMode::BM_DISABLED:
				m_Rend->gl.BlendFunci(m_glID, GL_ZERO, GL_ZERO);
				break;
		}

		m_BlendMode = mode;
	}
}


Renderer::BlendMode FrameBufferImpl::GetBlendMode() const
{
	return m_BlendMode;
}


void FrameBufferImpl::SetBlendEquation(Renderer::BlendEquation eq)
{
	if (eq != m_BlendEq)
	{
		switch (eq)
		{
			case Renderer::BlendEquation::BE_ADD:
				m_Rend->gl.BlendEquationi(m_glID, GL_FUNC_ADD);
				break;

			case Renderer::BlendEquation::BE_SUBTRACT:
				m_Rend->gl.BlendEquationi(m_glID, GL_FUNC_SUBTRACT);
				break;

			case Renderer::BlendEquation::BE_REVERSE_SUBTRACT:
				m_Rend->gl.BlendEquationi(m_glID, GL_FUNC_REVERSE_SUBTRACT);
				break;

			case Renderer::BlendEquation::BE_MIN:
				m_Rend->gl.BlendEquationi(m_glID, GL_MIN);
				break;

			case Renderer::BlendEquation::BE_MAX:
				m_Rend->gl.BlendEquationi(m_glID, GL_MAX);
				break;
		}

		m_BlendEq = eq;
	}
}


Renderer::BlendEquation FrameBufferImpl::GetBlendEquation() const
{
	return m_BlendEq;
}
