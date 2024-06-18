// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


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


FrameBufferImpl::FrameBufferImpl(RendererImpl* prend, const TCHAR *name)
{
	m_Pub = Publisher::Create(name);

	m_Rend = prend;
	m_Name = name;
	m_glID = NULL;
	m_DepthTarget = nullptr;
	m_BlendMode = Renderer::BlendMode::BM_NUMMODES;

	if (m_Rend)
	{
		GLint maxAttach = 0;
		m_Rend->gl.GetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);

		m_ColorTarget.reserve(maxAttach);

		m_Rend->gl.GenFramebuffers(1, &m_glID);
		m_Rend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);
	}

	m_ClearDepth = 1.0f;
	m_ClearStencil = 0;
}


FrameBufferImpl::~FrameBufferImpl()
{
	C3_SAFERELEASE(m_Pub);

	if (m_Rend && (m_glID != NULL))
	{
		m_Rend->gl.DeleteFramebuffers(1, &m_glID);
		m_glID = NULL;
	}
}


void FrameBufferImpl::Release()
{
	m_Pub->Deliver();

	m_Rend->RemoveFrameBuffer(m_Name.c_str());
	Teardown();

	delete this;
}


const TCHAR *FrameBufferImpl::GetName() const
{
	return m_Name.c_str();
}


FrameBuffer::RETURNCODE FrameBufferImpl::Setup(size_t numtargs, const TargetDesc *ptargdescs, c3::DepthBuffer *pdb, RECT &r)
{
	size_t w = r.right - r.left;
	size_t h = r.bottom - r.top;

	m_OwnsDepth = (pdb == nullptr);
	if (!pdb)
		pdb = m_Rend->CreateDepthBuffer(w, h, c3::Renderer::DepthType::U32_DS, 0);

	RETURNCODE ret = RETURNCODE::RET_UNKNOWN;

	if (pdb)
		ret = AttachDepthTarget(pdb);
	if (ret != RETURNCODE::RET_OK)
		return ret;

#if 0
	// on nVidia systems, actually follow the openGL spec and normalize texture types to the largest
	if (m_Rend->isnv)
	{
		Renderer::TextureType h = Renderer::TextureType::P8_3CH;
		for (size_t i = 0; i < numtargs; i++)
			if (ptargdescs[i].type > h)
				h = ptargdescs[i].type;

		for (size_t i = 0; i < numtargs; i++)
		{
			if ((ptargdescs[i].type >= Renderer::TextureType::U8_1CH) && (h >= Renderer::TextureType::F16_1CH))
			{
				*((uint8_t *)ptargdescs[i].type) += 4;
			}

			if ((ptargdescs[i].type >= Renderer::TextureType::F16_1CH) && (h >= Renderer::TextureType::F32_1CH))
			{
				*((uint8_t *)ptargdescs[i].type) += 4;
			}
		}
	}
#endif

	for (size_t i = 0; i < numtargs; i++)
	{
		c3::Texture2D* pt = m_Rend->CreateTexture2D(w, h, ptargdescs[i].type, 1, ptargdescs[i].flags);
		if (!pt)
			return RETURNCODE::RET_UNKNOWN;

		ret = AttachColorTarget(pt, i);
		if (ret != RETURNCODE::RET_OK)
			return ret;

		m_ColorTarget[i].owns = true;

		pt->SetName(ptargdescs[i].name);
	}

	return Seal();
}


FrameBuffer::RETURNCODE FrameBufferImpl::Teardown()
{
	if (m_OwnsDepth)
	{
		C3_SAFERELEASE(m_DepthTarget);
	}

	for (size_t i = 0; i < m_ColorTarget.size(); i++)
	{
		if (m_ColorTarget[i].owns)
		{
			C3_SAFERELEASE(m_ColorTarget[i].tex);
		}
	}

	m_ColorTarget.clear();

	return FrameBuffer::RETURNCODE::RET_OK;
}


FrameBuffer::RETURNCODE FrameBufferImpl::AttachColorTarget(Texture2D *target, size_t position)
{
	if (m_Rend && (m_glID != NULL))
	{
		m_Rend->UseFrameBuffer(this, 0);

		if (position <= m_ColorTarget.size())
			m_ColorTarget.resize(position + 1);

		m_ColorTarget[position].tex = target;
		m_ColorTarget[position].clearcolor.pu = 0;
		m_ColorTarget[position].owns = false;

		if (target)
		{
			m_Rend->gl.FramebufferTexture2D(GL_FRAMEBUFFER, targenum[position], GL_TEXTURE_2D, (const Texture2DImpl &)*target, 0);

			m_Rend->FlushErrors(_T("AttachColorTarget: %s"), __FILEW__, __LINE__);
		}
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
		return m_ColorTarget[position].tex;

	return nullptr;
}


Texture2D *FrameBufferImpl::GetColorTargetByName(const TCHAR *name)
{
	for (size_t i = 0; i < m_ColorTarget.size(); i++)
	{
		TColorTargetArray::value_type &targ = m_ColorTarget[i];
		if (!targ.tex)
			continue;

		if (!_tcsicmp(name, targ.tex->GetName()))
			return targ.tex;
	}

	return nullptr;
}


FrameBuffer::RETURNCODE FrameBufferImpl::AttachDepthTarget(DepthBuffer* pdepth)
{
	if (!pdepth)
		return FrameBuffer::RETURNCODE::RET_NULLTARGET;

	if (m_Rend)
	{
		m_Rend->UseFrameBuffer(this, 0);

		GLenum attach_type = ((pdepth->Format() == Renderer::DepthType::F32_DS) || (pdepth->Format() == Renderer::DepthType::U32_DS)) ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;

		if (pdepth->Format() < Renderer::DepthType::F16_SHADOW)
			m_Rend->gl.FramebufferRenderbuffer(GL_FRAMEBUFFER, attach_type, GL_RENDERBUFFER, (const DepthBufferImpl &)*pdepth);
		else
			m_Rend->gl.FramebufferTexture(GL_FRAMEBUFFER, attach_type, (const DepthBufferImpl &)*pdepth, 0);

		m_Rend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);
		m_DepthTarget = pdepth;
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
		m_Rend->UseFrameBuffer(this, 0);

		// There might be no color targets if we're setting up a shadow map
		if (m_ColorTarget.size() > 0)
		{
			m_Rend->gl.DrawBuffers((GLsizei)m_ColorTarget.size(), targenum);
			m_Rend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);
		}
		else
		{
			m_Rend->gl.DrawBuffer(GL_NONE);
			//m_Rend->gl.ReadBuffer(GL_NONE);
		}

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
	}

	return ret;
}


void FrameBufferImpl::SetClearColor(size_t position, uint32_t color)
{
	if (position < m_ColorTarget.size())
	{
		glm::vec4 c;
		if (((m_ColorTarget[position].tex->Format() >= Renderer::TextureType::F16_1CH) &&
			(m_ColorTarget[position].tex->Format() <= Renderer::TextureType::F16_4CH)) ||
			((m_ColorTarget[position].tex->Format() >= Renderer::TextureType::F32_1CH) &&
			(m_ColorTarget[position].tex->Format() <= Renderer::TextureType::F32_4CH)))
		{
			c.x = float(color & 0xff) / 255.0f;
			c.y = float((color >> 8) & 0xff) / 255.0f;
			c.z = float((color >> 16) & 0xff) / 255.0f;
			c.w = float((color >> 24) & 0xff) / 255.0f;
		}

		switch (m_ColorTarget[position].tex->Format())
		{
			case Renderer::TextureType::U8_1CH:
			case Renderer::TextureType::U8_2CH:
			case Renderer::TextureType::U8_3CH:
			case Renderer::TextureType::U8_4CH:
			{
				m_ColorTarget[position].clearcolor.pu = color;
				break;
			}

			case Renderer::TextureType::F16_1CH:
			case Renderer::TextureType::F16_2CH:
			case Renderer::TextureType::F16_3CH:
			case Renderer::TextureType::F16_4CH:
			{
				m_ColorTarget[position].clearcolor.ph = glm::packHalf4x16(c);
				break;
			}

			case Renderer::TextureType::F32_1CH:
			case Renderer::TextureType::F32_2CH:
			case Renderer::TextureType::F32_3CH:
			case Renderer::TextureType::F32_4CH:
			{
				m_ColorTarget[position].clearcolor.fr = c.x;
				m_ColorTarget[position].clearcolor.fg = c.y;
				m_ColorTarget[position].clearcolor.fb = c.z;
				m_ColorTarget[position].clearcolor.fa = c.w;
				break;
			}
		}
	}
}


uint32_t FrameBufferImpl::GetClearColor(size_t position) const
{
	if (position < m_ColorTarget.size())
	{
		switch (m_ColorTarget[position].tex->Format())
		{
			case Renderer::TextureType::U8_1CH:
			case Renderer::TextureType::U8_2CH:
			case Renderer::TextureType::U8_3CH:
			case Renderer::TextureType::U8_4CH:
			{
				return m_ColorTarget[position].clearcolor.pu;
			}

			case Renderer::TextureType::F16_1CH:
			case Renderer::TextureType::F16_2CH:
			case Renderer::TextureType::F16_3CH:
			case Renderer::TextureType::F16_4CH:
			{
				glm::vec4 c = glm::unpackHalf4x16(m_ColorTarget[position].clearcolor.ph);
				return uint32_t(uint8_t(c.x * 255.0f) | (uint8_t(c.y * 255.0f) << 8) | (uint8_t(c.z * 255.0f) << 16) | (uint8_t(c.w * 255.0f) << 24));
				break;
			}

			case Renderer::TextureType::F32_1CH:
			case Renderer::TextureType::F32_2CH:
			case Renderer::TextureType::F32_3CH:
			case Renderer::TextureType::F32_4CH:
			{
				glm::vec4 *c = (glm::vec4 *)&(m_ColorTarget[position].clearcolor.f[0]);
				return uint32_t(uint8_t(c->x * 255.0f) | (uint8_t(c->y * 255.0f) << 8) | (uint8_t(c->z * 255.0f) << 16) | (uint8_t(c->w * 255.0f) << 24));
			}
		}
	}

	return 0;
}


void FrameBufferImpl::SetClearDepth(float depth)
{
	m_ClearDepth = depth;
}


float FrameBufferImpl::GetClearDepth() const
{
	return m_ClearDepth;
}


void FrameBufferImpl::SetClearStencil(int8_t stencil)
{
	m_ClearStencil = stencil;
}


int8_t FrameBufferImpl::GetClearStencil() const
{
	return m_ClearStencil;
}


void FrameBufferImpl::Clear(props::TFlags64 flags)
{
	if (flags.IsSet(UFBFLAG_CLEARCOLOR))
	{
		for (size_t i = 0; i < m_ColorTarget.size(); i++)
		{
			GLuint c[4];
			c[0] = m_ColorTarget[i].clearcolor.pu & 0xff;
			c[1] = (m_ColorTarget[i].clearcolor.pu >> 8) & 0xff;
			c[2] = (m_ColorTarget[i].clearcolor.pu >> 16) & 0xff;
			c[3] = (m_ColorTarget[i].clearcolor.pu >> 24) & 0xff;

			m_Rend->gl.ClearBufferuiv(GL_COLOR, (GLint)i, (const GLuint *)c);
		}
	}

	if (m_DepthTarget && flags.AnySet(UFBFLAG_CLEARDEPTH | UFBFLAG_CLEARSTENCIL))
	{
		switch (m_DepthTarget->Format())
		{
			case Renderer::DepthType::U32_DS:
			case Renderer::DepthType::F32_DS:
				if (flags.IsSet(UFBFLAG_CLEARDEPTH | UFBFLAG_CLEARSTENCIL))
					m_Rend->gl.ClearBufferfi(GL_DEPTH_STENCIL, 0, m_ClearDepth, m_ClearStencil);
				else if (flags.IsSet(UFBFLAG_CLEARDEPTH))
					m_Rend->gl.ClearBufferfv(GL_DEPTH, 0, &m_ClearDepth);
				break;

			case Renderer::DepthType::U16_D:
			case Renderer::DepthType::F16_SHADOW:
			case Renderer::DepthType::U32_D:
			case Renderer::DepthType::F32_D:
			case Renderer::DepthType::F32_SHADOW:
				m_Rend->gl.ClearBufferfv(GL_DEPTH, 0, &m_ClearDepth);
				break;
		}
	}
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


void FrameBufferImpl::Subscribe(Subscription *sub)
{
	m_Pub->Subscribe(sub);
}


void FrameBufferImpl::Unsubscribe(Subscription *sub)
{
	m_Pub->Unsubscribe(sub);
}
