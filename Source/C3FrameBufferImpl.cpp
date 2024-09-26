// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3FrameBufferImpl.h>
#include <C3DepthBufferImpl.h>
#include <C3TextureImpl.h>
#include <C3ResourceImpl.h>


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

	for (size_t i = 0; i < MAX_COLORTARGETS; i++)
	{
		m_BlendMode[i] = Renderer::BlendMode::BM_NUMMODES;
		m_BlendEq[i] = Renderer::BlendEquation::BE_NUMMODES;
		m_ChannelMask[i] = CM_RED | CM_GREEN | CM_BLUE | CM_ALPHA;
	}

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

	m_ColorTarget.resize(numtargs);

	for (size_t i = 0; i < numtargs; i++)
	{
		Texture2D *pt = m_Rend->CreateTexture2D(w, h, ptargdescs[i].type, 1, ptargdescs[i].flags);
		if (!pt)
			return RETURNCODE::RET_UNKNOWN;

		pt->SetName(ptargdescs[i].name);

		// register the texture so it can be auto-bound in a shader
		ResourceManager *presman = m_Rend->GetSystem()->GetResourceManager();
		ResourceImpl *pres = (ResourceImpl *)presman->GetResource(ptargdescs[i].name, RESF_CREATEENTRYONLY, DefaultTexture2DResourceType::Type(), pt);

		ret = AttachColorTarget(pt, i);
		if (ret != RETURNCODE::RET_OK)
			return ret;

		m_ColorTarget[i].owns = true;
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
			//C3_SAFERELEASE(m_ColorTarget[i].tex);
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

		if (position >= m_ColorTarget.size())
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


void FrameBufferImpl::SetClearColor(size_t position, glm::fvec4 color)
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
				m_ColorTarget[position].clearcolor.ur = (uint8_t)(std::min<float>(std::max<float>(0, color.x), 1) * 255.0f);
				m_ColorTarget[position].clearcolor.ug = (uint8_t)(std::min<float>(std::max<float>(0, color.y), 1) * 255.0f);
				m_ColorTarget[position].clearcolor.ub = (uint8_t)(std::min<float>(std::max<float>(0, color.z), 1) * 255.0f);
				m_ColorTarget[position].clearcolor.ua = (uint8_t)(std::min<float>(std::max<float>(0, color.w), 1) * 255.0f);
				break;
			}

			case Renderer::TextureType::S8_1CH:
			case Renderer::TextureType::S8_2CH:
			case Renderer::TextureType::S8_3CH:
			case Renderer::TextureType::S8_4CH:
			{
				m_ColorTarget[position].clearcolor.sr = (int8_t)(std::min<float>(std::max<float>(-1, color.x), 1) * 128.0f + 128.0f);
				m_ColorTarget[position].clearcolor.sg = (int8_t)(std::min<float>(std::max<float>(-1, color.y), 1) * 128.0f + 128.0f);
				m_ColorTarget[position].clearcolor.sb = (int8_t)(std::min<float>(std::max<float>(-1, color.z), 1) * 128.0f + 128.0f);
				m_ColorTarget[position].clearcolor.sa = (int8_t)(std::min<float>(std::max<float>(-1, color.w), 1) * 128.0f + 128.0f);
				break;
			}

			case Renderer::TextureType::F16_1CH:
			case Renderer::TextureType::F16_2CH:
			case Renderer::TextureType::F16_3CH:
			case Renderer::TextureType::F16_4CH:
			case Renderer::TextureType::F32_1CH:
			case Renderer::TextureType::F32_2CH:
			case Renderer::TextureType::F32_3CH:
			case Renderer::TextureType::F32_4CH:
			{
				m_ColorTarget[position].clearcolor.fr = color.x;
				m_ColorTarget[position].clearcolor.fg = color.y;
				m_ColorTarget[position].clearcolor.fb = color.z;
				m_ColorTarget[position].clearcolor.fa = color.w;
				break;
			}
		}
	}
}


glm::fvec4 FrameBufferImpl::GetClearColor(size_t position) const
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
				glm::fvec4 ret;
				ret.x = (float)m_ColorTarget[position].clearcolor.ur / 255.0f;
				ret.y = (float)m_ColorTarget[position].clearcolor.ug / 255.0f;
				ret.z = (float)m_ColorTarget[position].clearcolor.ub / 255.0f;
				ret.w = (float)m_ColorTarget[position].clearcolor.ua / 255.0f;
				return ret;
			}

			case Renderer::TextureType::S8_1CH:
			case Renderer::TextureType::S8_2CH:
			case Renderer::TextureType::S8_3CH:
			case Renderer::TextureType::S8_4CH:
			{
				glm::fvec4 ret;
				ret.x = ((float)m_ColorTarget[position].clearcolor.ur - 128.0f) / 128.0f;
				ret.y = ((float)m_ColorTarget[position].clearcolor.ug - 128.0f) / 128.0f;
				ret.z = ((float)m_ColorTarget[position].clearcolor.ub - 128.0f) / 128.0f;
				ret.w = ((float)m_ColorTarget[position].clearcolor.ua - 128.0f) / 128.0f;
				return ret;
			}

			case Renderer::TextureType::F16_1CH:
			case Renderer::TextureType::F16_2CH:
			case Renderer::TextureType::F16_3CH:
			case Renderer::TextureType::F16_4CH:
			case Renderer::TextureType::F32_1CH:
			case Renderer::TextureType::F32_2CH:
			case Renderer::TextureType::F32_3CH:
			case Renderer::TextureType::F32_4CH:
			{
				return glm::fvec4(m_ColorTarget[position].clearcolor.f[0], m_ColorTarget[position].clearcolor.f[1],
								  m_ColorTarget[position].clearcolor.f[2], m_ColorTarget[position].clearcolor.f[3]);
			}
		}
	}

	return glm::fvec4(0, 0, 0, 0);
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


void FrameBufferImpl::Clear(props::TFlags64 flags, int target)
{
	if (flags.IsSet(UFBFLAG_CLEARCOLOR))
	{
		size_t mini = flags.IsSet(UFBFLAG_STRICTCOMPLIANCE) ? 1 : 0; // (target < 0) ? 0 : target;
		size_t maxi = m_ColorTarget.size(); // (target < 0) ? m_ColorTarget.size() : (size_t)mini + 1;

		for (size_t i = mini; i < maxi; i++)
		{
			switch (m_ColorTarget[i].tex->Format())
			{
				case Renderer::TextureType::U8_1CH:
				case Renderer::TextureType::U8_2CH:
				case Renderer::TextureType::U8_3CH:
				case Renderer::TextureType::U8_4CH:
				{
					GLuint c[4];
					c[0] = m_ColorTarget[i].clearcolor.ur;
					c[1] = m_ColorTarget[i].clearcolor.ug;
					c[2] = m_ColorTarget[i].clearcolor.ub;
					c[3] = m_ColorTarget[i].clearcolor.ua;
					m_Rend->gl.ClearBufferuiv(GL_COLOR, (GLint)i, (const GLuint *)c);
					break;
				}

				case Renderer::TextureType::S8_1CH:
				case Renderer::TextureType::S8_2CH:
				case Renderer::TextureType::S8_3CH:
				case Renderer::TextureType::S8_4CH:
				{
					GLint c[4];
					c[0] = m_ColorTarget[i].clearcolor.sr;
					c[1] = m_ColorTarget[i].clearcolor.sg;
					c[2] = m_ColorTarget[i].clearcolor.sb;
					c[3] = m_ColorTarget[i].clearcolor.sa;
					m_Rend->gl.ClearBufferiv(GL_COLOR, (GLint)i, (const GLint *)c);
					break;
				}

				case Renderer::TextureType::F16_1CH:
				case Renderer::TextureType::F16_2CH:
				case Renderer::TextureType::F16_3CH:
				case Renderer::TextureType::F16_4CH:
				case Renderer::TextureType::F32_1CH:
				case Renderer::TextureType::F32_2CH:
				case Renderer::TextureType::F32_3CH:
				case Renderer::TextureType::F32_4CH:
				{
					m_Rend->gl.ClearBufferfv(GL_COLOR, (GLint)i, m_ColorTarget[i].clearcolor.f);
					break;
				}
			}
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


void FrameBufferImpl::SetBlendMode(Renderer::BlendMode mode, int target)
{
	// if target is negative, then go through all attachments
	if (target < 0)
	{
		for (size_t i = 0; i < GetNumColorTargets(); i++)
			SetBlendMode(mode, (int)i);
		return;
	}
	else if (target >= GetNumColorTargets())
		return;

	if (mode != m_BlendMode[target])
	{
		// normally, we want to set these things when we change the active frame buffer, but if we change a setting while
		// the frame is already active, do it now.
		if (m_Rend->GetActiveFrameBuffer() == this)
		{
			_SetBlendMode((size_t)target, mode);
		}

		m_BlendMode[target] = mode;
	}
}


Renderer::BlendMode FrameBufferImpl::GetBlendMode(int target) const
{
	return m_BlendMode[std::min<int>(std::max<int>(0, target), MAX_COLORTARGETS)];
}


void FrameBufferImpl::SetBlendEquation(Renderer::BlendEquation eq, int target)
{
	// if target is negative, then go through all attachments
	if (target < 0)
	{
		for (size_t i = 0; i < GetNumColorTargets(); i++)
			SetBlendEquation(eq, (int)i);
		return;
	}
	else if (target >= GetNumColorTargets())
		return;

	if (eq != m_BlendEq[target])
	{
		// normally, we want to set these things when we change the active frame buffer, but if we change a setting while
		// the frame is already active, do it now.
		if (m_Rend->GetActiveFrameBuffer() == this)
		{
			_SetBlendEquation((size_t)target, eq);
		}

		m_BlendEq[target] = eq;
	}
}


Renderer::BlendEquation FrameBufferImpl::GetBlendEquation(int target) const
{
	return m_BlendEq[std::min<int>(std::max<int>(0, target), MAX_COLORTARGETS)];
}


void FrameBufferImpl::FrameBufferImpl::SetChannelWriteMask(Renderer::ChannelMask mask, int target)
{
	// if target is negative, then go through all attachments
	if (target < 0)
	{
		for (size_t i = 0; i < GetNumColorTargets(); i++)
			SetChannelWriteMask(mask, (int)i);
		return;
	}
	else if (target >= GetNumColorTargets())
		return;

	if (mask != m_ChannelMask[target])
	{
		// normally, we want to set these things when we change the active frame buffer, but if we change a setting while
		// the frame is already active, do it now.
		if (m_Rend->GetActiveFrameBuffer() == this)
		{
			_SetChannelWriteMask((size_t)target, mask);
		}

		m_ChannelMask[target] = mask;
	}
}


Renderer::ChannelMask FrameBufferImpl::GetChannelWriteMask(int target) const
{
	return m_ChannelMask[std::min<int>(std::max<int>(0, target), MAX_COLORTARGETS)];
}


void FrameBufferImpl::Subscribe(Subscription *sub)
{
	m_Pub->Subscribe(sub);
}


void FrameBufferImpl::Unsubscribe(Subscription *sub)
{
	m_Pub->Unsubscribe(sub);
}


void FrameBufferImpl::ApplySettings()
{
	for (size_t i = 0; i < GetNumColorTargets(); i++)
	{
		_SetBlendMode(i, m_BlendMode[i]);
		_SetBlendEquation(i, m_BlendEq[i]);
		_SetChannelWriteMask(i, m_ChannelMask[i]);
	}
}


void FrameBufferImpl::_SetBlendMode(size_t target, Renderer::BlendMode mode)
{
	static Renderer::BlendMode oldmode[MAX_COLORTARGETS] = {Renderer::BM_DISABLED};
	if (oldmode[target] == mode)
		return;

	if ((oldmode[target] != Renderer::BlendMode::BM_ALPHA) &&
		(oldmode[target] != Renderer::BlendMode::BM_ADD) &&
		(oldmode[target] != Renderer::BlendMode::BM_ADDALPHA))
	{
		m_Rend->gl.Enablei(GL_BLEND, (GLuint)target);
	}
	else if ((mode != Renderer::BlendMode::BM_ALPHA) &&
			 (mode != Renderer::BlendMode::BM_ADD) &&
			 (mode != Renderer::BlendMode::BM_ADDALPHA))
	{
		m_Rend->gl.Disablei(GL_BLEND, (GLuint)target);
	}

	switch (mode)
	{
		case Renderer::BlendMode::BM_ALPHA:
			m_Rend->gl.BlendFunci((GLuint)target, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;

		case Renderer::BlendMode::BM_ADD:
			m_Rend->gl.BlendFunci((GLuint)target, GL_ONE, GL_ONE);
			break;

		case Renderer::BlendMode::BM_ADDALPHA:
			m_Rend->gl.BlendFunci((GLuint)target, GL_SRC_ALPHA, GL_ONE);
			break;

		case Renderer::BlendMode::BM_REPLACE:
			m_Rend->gl.BlendFunci((GLuint)target, GL_ONE, GL_ZERO);
			break;

		case Renderer::BlendMode::BM_DISABLED:
			m_Rend->gl.BlendFunci((GLuint)target, GL_ZERO, GL_ZERO);
			break;
	}

	oldmode[target] = mode;
}


void FrameBufferImpl::_SetBlendEquation(size_t target, Renderer::BlendEquation eq)
{
	static Renderer::BlendEquation oldeq[MAX_COLORTARGETS] = {Renderer::BE_ADD};
	if (oldeq[target] == eq)
		return;

	switch (eq)
	{
		case Renderer::BlendEquation::BE_ADD:
			m_Rend->gl.BlendEquationi((GLuint)target, GL_FUNC_ADD);
			break;

		case Renderer::BlendEquation::BE_SUBTRACT:
			m_Rend->gl.BlendEquationi((GLuint)target, GL_FUNC_SUBTRACT);
			break;

		case Renderer::BlendEquation::BE_REVERSE_SUBTRACT:
			m_Rend->gl.BlendEquationi((GLuint)target, GL_FUNC_REVERSE_SUBTRACT);
			break;

		case Renderer::BlendEquation::BE_MIN:
			m_Rend->gl.BlendEquationi((GLuint)target, GL_MIN);
			break;

		case Renderer::BlendEquation::BE_MAX:
			m_Rend->gl.BlendEquationi((GLuint)target, GL_MAX);
			break;
	}

	oldeq[target] = eq;
}


void FrameBufferImpl::_SetChannelWriteMask(size_t target, Renderer::ChannelMask mask)
{
	static Renderer::ChannelMask oldmask[MAX_COLORTARGETS] = {CM_RED | CM_GREEN | CM_BLUE | CM_ALPHA};
	if (oldmask[target] == mask)
		return;

	GLboolean r = mask.IsSet(CM_RED);
	GLboolean g = mask.IsSet(CM_GREEN);
	GLboolean b = mask.IsSet(CM_BLUE);
	GLboolean a = mask.IsSet(CM_ALPHA);

	m_Rend->gl.ColorMaski((GLuint)target, r, g, b, a);

	oldmask[target] = mask;
}
