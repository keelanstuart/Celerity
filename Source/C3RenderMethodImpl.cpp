// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3RenderMethodImpl.h>
#include <C3Resource.h>
#include <C3FrameBufferImpl.h>

using namespace c3;


RenderMethodImpl::DrawOrderCountMap RenderMethodImpl::s_DrawOrders;
void RenderMethod::ForEachOrderedDrawDo(RenderMethod::OrderedDrawFunction func)
{
	auto it = RenderMethodImpl::s_DrawOrders.begin();

	if (!RenderMethodImpl::s_DrawOrders.empty())
	{
		while (it->first < 0)
		{
			func(it->first);
			it++;
		}
	}

	func(0);	// 0 should never be in the list itself

	if (!RenderMethodImpl::s_DrawOrders.empty())
	{
		while (it != RenderMethodImpl::s_DrawOrders.end())
		{
			func(it->first);
			it++;
		}
	}
}


RenderMethodImpl::PassImpl::PassImpl()
{
	m_ShaderProg = nullptr;
	m_FrameBuffer = nullptr;
	m_FBSub = true;
	m_FrameBufferFlags = 0;
	m_pOwner = nullptr;
	m_StateRestorationMask.Set(-1);
}


RenderMethodImpl::PassImpl::~PassImpl()
{
	// m_FBSub being null means that the FrameBuffer already went away... so we don't need to Unsubscribe
	if (!m_FBSub && m_FrameBuffer)
		((FrameBufferImpl *)m_FrameBuffer)->Unsubscribe(&m_FBSub);

	if (m_ShaderProg)
	{
		m_ShaderProg->Release();
		m_ShaderProg = nullptr;
	}
}


Renderer::RenderStateOverrideFlags RenderMethodImpl::PassImpl::Apply(Renderer *prend)
{
	assert(prend);

	Renderer::RenderStateOverrideFlags ret;

	if (m_FrameBufferName.has_value())
	{
		if (m_FBSub)
		{
			m_FrameBuffer = prend->FindFrameBuffer((*m_FrameBufferName).c_str());
			if (m_FrameBuffer)
			{
				((FrameBufferImpl *)m_FrameBuffer)->Subscribe(&m_FBSub);
				m_FBSub = false;
			}
		}
	}

	// If we have a frame buffer, we need to check for target-specific settings (blending and color masking stuff)
	if (m_FrameBuffer)
	{
		size_t ct = m_FrameBuffer->GetNumColorTargets();
		for (size_t i = 0; i < ct; i++)
		{
			if (m_BlendModeCh[i].has_value())
			{
				m_FrameBuffer->SetBlendMode(*m_BlendModeCh[i], (int)i);
				ret.Set(RSOF_BLENDMODE);
			}

			if (m_BlendEqCh[i].has_value())
			{
				m_FrameBuffer->SetBlendEquation(*m_BlendEqCh[i], (int)i);
				ret.Set(RSOF_BLENDEQ);
			}

			if (m_ChannelWriteMaskCh[i].has_value())
			{
				m_FrameBuffer->SetChannelWriteMask(*m_ChannelWriteMaskCh[i], (int)i);
				ret.Set(RSOF_COLORMASK);
			}
		}
	}

	if (m_FrameBuffer)
		prend->UseFrameBuffer(m_FrameBuffer, m_FrameBufferFlags);

	if (m_BlendMode.has_value())
	{
		prend->SetBlendMode(*m_BlendMode);
		ret.Set(RSOF_BLENDMODE);
	}

	if (m_BlendEq.has_value())
	{
		prend->SetBlendEquation(*m_BlendEq);
		ret.Set(RSOF_BLENDEQ);
	}

	if (m_ChannelWriteMask.has_value())
	{
		prend->SetChannelWriteMask(*m_ChannelWriteMask);
		ret.Set(RSOF_COLORMASK);
	}

	if (m_CullMode.has_value())
	{
		prend->SetCullMode(*m_CullMode);
		ret.Set(RSOF_CULLMODE);
	}

	if (m_WindingOrder.has_value())
	{
		prend->SetWindingOrder(*m_WindingOrder);
		ret.Set(RSOF_WINDINGORDER);
	}

	if (m_DepthMode.has_value())
	{
		prend->SetDepthMode(*m_DepthMode);
		ret.Set(RSOF_DEPTHMODE);
	}

	if (m_FillMode.has_value())
	{
		prend->SetFillMode(*m_FillMode);
		ret.Set(RSOF_FILLMODE);
	}

	bool need_shader = false;
	for (size_t i = 0; i < Renderer::ShaderComponentType::ST_NUMTYPES; i++)
	{
		if (m_ShaderCompFilename[i].has_value())
		{
			need_shader = true;
			break;
		}
	}

	if (need_shader)
	{
		if (!m_ShaderProg)
		{
			Resource *scres[Renderer::ShaderComponentType::ST_NUMTYPES] = {0};
			for (size_t i = 0; i < Renderer::ShaderComponentType::ST_NUMTYPES; i++)
				if (m_ShaderCompFilename[i].has_value())
					scres[i] = prend->GetSystem()->GetResourceManager()->GetResource(m_ShaderCompFilename[i]->c_str(), RESF_DEMANDLOAD);

			bool has_all_comps = true;
			for (size_t i = 0; i < Renderer::ShaderComponentType::ST_NUMTYPES; i++)
				has_all_comps &= m_ShaderCompFilename[i].has_value() ? (scres[i]->GetStatus() == Resource::RS_LOADED) : true;

			if (has_all_comps)
			{
				m_ShaderProg = prend->CreateShaderProgram();
				if (m_ShaderProg)
				{
					for (size_t i = 0; i < Renderer::ShaderComponentType::ST_NUMTYPES; i++)
						if (scres[i])
							m_ShaderProg->AttachShader((ShaderComponent *)(scres[i]->GetData()));

					m_ShaderProg->Link();
				}
			}
		}

		if (m_ShaderProg && m_ShaderProg->IsLinked())
			prend->UseProgram(m_ShaderProg);
	}

	return ret & m_StateRestorationMask;
}


void RenderMethodImpl::PassImpl::SetFrameBufferName(const TCHAR *name)
{
	if (m_FrameBuffer)
		((FrameBufferImpl *)m_FrameBuffer)->Unsubscribe(&m_FBSub);

	if (!name)
	{
		m_FrameBufferName.reset();
		return;
	}

	m_FrameBufferName = std::make_optional<tstring>(name);
}


bool RenderMethodImpl::PassImpl::GetFrameBufferName(tstring &name) const
{
	if (!m_FrameBufferName.has_value())
		return false;

	name = *m_FrameBufferName;
	return true;
}


void RenderMethodImpl::PassImpl::InvalidateFrameBuffer()
{
	m_FrameBuffer = nullptr;
}


void RenderMethodImpl::PassImpl::SetFrameBufferFlags(props::TFlags64 flags)
{
	m_FrameBufferFlags = flags;
}


props::TFlags64 RenderMethodImpl::PassImpl::GetFrameBufferFlags() const
{
	return m_FrameBufferFlags;
}


void RenderMethodImpl::PassImpl::SetShaderComponentFilename(Renderer::ShaderComponentType type, const TCHAR *filename)
{
	m_ShaderCompFilename[type] = std::make_optional<tstring>(filename);
}


bool RenderMethodImpl::PassImpl::GetShaderComponentFilename(Renderer::ShaderComponentType type, tstring &filename) const
{
	if (!m_ShaderCompFilename[type].has_value())
		return false;

	filename = *m_ShaderCompFilename[type];
	return true;
}


ShaderProgram *RenderMethodImpl::PassImpl::GetShader() const
{
	return m_ShaderProg;
}


void RenderMethodImpl::PassImpl::SetBlendMode(Renderer::BlendMode blendmode)
{
	m_BlendMode = std::make_optional<Renderer::BlendMode>(blendmode);
}


bool RenderMethodImpl::PassImpl::GetBlendMode(Renderer::BlendMode &blendmode) const
{
	if (!m_BlendMode.has_value())
		return false;

	blendmode = *m_BlendMode;
	return true;
}


void RenderMethodImpl::PassImpl::SetEquation(Renderer::BlendEquation blendeq)
{
	m_BlendEq = std::make_optional<Renderer::BlendEquation>(blendeq);
}


bool RenderMethodImpl::PassImpl::GetBlendEquation(Renderer::BlendEquation &blendeq) const
{
	if (!m_BlendEq.has_value())
		return false;

	blendeq = *m_BlendEq;
	return true;
}


void RenderMethodImpl::PassImpl::SetCullMode(Renderer::CullMode cullmode)
{
	m_CullMode = std::make_optional<Renderer::CullMode>(cullmode);
}


bool RenderMethodImpl::PassImpl::GetCullMode(Renderer::CullMode &cullmode) const
{
	if (!m_CullMode.has_value())
		return false;

	cullmode = *m_CullMode;
	return true;
}


void RenderMethodImpl::PassImpl::SetWindingOrder(Renderer::WindingOrder order)
{
	m_WindingOrder = std::make_optional<Renderer::WindingOrder>(order);
}


bool RenderMethodImpl::PassImpl::GetWindingOrder(Renderer::WindingOrder &order) const
{
	if (!m_WindingOrder.has_value())
		return false;

	order = *m_WindingOrder;
	return true;
}


void RenderMethodImpl::PassImpl::SetDepthMode(Renderer::DepthMode depthmode)
{
	m_DepthMode = std::make_optional<Renderer::DepthMode>(depthmode);
}


bool RenderMethodImpl::PassImpl::GetDepthMode(Renderer::DepthMode &depthmode) const
{
	if (!m_DepthMode.has_value())
		return false;

	depthmode = *m_DepthMode;
	return true;
}


void RenderMethodImpl::PassImpl::SetFillMode(Renderer::FillMode fillmode)
{
	m_FillMode = std::make_optional<Renderer::FillMode>(fillmode);
}


bool RenderMethodImpl::PassImpl::GetFillMode(Renderer::FillMode &fillmode) const
{
	if (!m_FillMode.has_value())
		return false;

	fillmode = *m_FillMode;
	return true;
}


RenderMethodImpl::TechniqueImpl::TechniqueImpl(RenderMethodImpl *powner)
{
	m_pOwner = powner;
}


void RenderMethodImpl::TechniqueImpl::SetName(const TCHAR *name)
{
	m_Name = name;
}


const TCHAR *RenderMethodImpl::TechniqueImpl::GetName() const
{
	return m_Name.c_str();
}


size_t RenderMethodImpl::TechniqueImpl::GetNumPasses() const
{
	return m_Passes.size();
}


RenderMethod::Pass *RenderMethodImpl::TechniqueImpl::GetPass(size_t idx) const
{
	return (RenderMethod::Pass *)&m_Passes[idx];
}


RenderMethod::Pass *RenderMethodImpl::TechniqueImpl::AddPass()
{
	RenderMethodImpl::PassImpl &ret = m_Passes.emplace_back();
	ret.m_pOwner = m_pOwner;

	return &ret;
}


bool RenderMethodImpl::TechniqueImpl::Begin(size_t &passes, bool restore_old)
{
	if (m_Passes.empty())
		return false;

	m_RestoreOld = restore_old;
	if (m_RestoreOld)
	{
		c3::Renderer *r = m_pOwner->m_pRend;
		m_OldState = 0;
		m_OldBlendMode = r->GetBlendMode();
		m_OldBlendEq = r->GetBlendEquation();
		m_OldCullMode = r->GetCullMode();
		m_OldWindingOrder = r->GetWindingOrder();
		m_OldDepthMode = r->GetDepthMode();
		m_OldFillMode = r->GetFillMode();
	}

	passes = m_Passes.size();
	return true;
}


Renderer::RenderStateOverrideFlags RenderMethodImpl::TechniqueImpl::ApplyPass(size_t idx)
{
	if (idx >= m_Passes.size())
		return 0;

	PassImpl &pr = (PassImpl &)m_Passes[idx];

	Renderer::RenderStateOverrideFlags changed_states = pr.Apply(((RenderMethodImpl *)m_pOwner)->m_pRend);
	m_OldState |= changed_states;

	return changed_states;
}


void RenderMethodImpl::TechniqueImpl::End()
{
	if (m_RestoreOld)
	{
		if (m_OldState.IsSet(RSOF_BLENDMODE))
			m_pOwner->m_pRend->SetBlendMode(m_OldBlendMode);

		if (m_OldState.IsSet(RSOF_BLENDEQ))
			m_pOwner->m_pRend->SetBlendEquation(m_OldBlendEq);

		if (m_OldState.IsSet(RSOF_CULLMODE))
			m_pOwner->m_pRend->SetCullMode(m_OldCullMode);

		if (m_OldState.IsSet(RSOF_WINDINGORDER))
			m_pOwner->m_pRend->SetWindingOrder(m_OldWindingOrder);

		if (m_OldState.IsSet(RSOF_DEPTHMODE))
			m_pOwner->m_pRend->SetDepthMode(m_OldDepthMode);

		if (m_OldState.IsSet(RSOF_FILLMODE))
			m_pOwner->m_pRend->SetFillMode(m_OldFillMode);
	}
}


void RenderMethodImpl::TechniqueImpl::SetDrawOrder(int order)
{
	if (m_DrawOrder.value_or(0) != order)
	{
		if (m_DrawOrder.has_value() && *m_DrawOrder)
		{
			auto it = s_DrawOrders.find(*m_DrawOrder);
			if (it != s_DrawOrders.end())
			{
				if (it->second)
					it->second--;

				if (!it->second)
					s_DrawOrders.erase(it);
			}
		}
	}

	if (order)
	{
		m_DrawOrder = order;
		std::pair<RenderMethodImpl::DrawOrderCountMap::iterator, bool> insret = RenderMethodImpl::s_DrawOrders.insert(RenderMethodImpl::DrawOrderCountMap::value_type(order, 1));
		if (!insret.second)
			insret.first->second++;
	}
	else
		m_DrawOrder.reset();
}


int RenderMethodImpl::TechniqueImpl::GetDrawOrder() const
{
	return m_DrawOrder.value_or(0);
}


RenderMethodImpl::RenderMethodImpl(Renderer *prend)
{
	m_pRend = prend;
	m_ActiveTech = -1;
}


const TCHAR *RenderMethodImpl::GetShaderMode()
{
	return m_ShaderMode.c_str();
}


void RenderMethodImpl::SetShaderMode(const TCHAR *mode)
{
	m_ShaderMode = mode;
}


RenderMethodImpl::~RenderMethodImpl()
{
}


void RenderMethodImpl::Release()
{
	delete this;
}


void RenderMethodImpl::SetName(const TCHAR *name)
{
	m_Name = name;
}


const TCHAR *RenderMethodImpl::GetName() const
{
	return m_Name.c_str();
}


RenderMethod::Technique *RenderMethodImpl::AddTechnique()
{
	TechniqueImpl t(this);
	m_Techniques.push_back(t);

	return &(m_Techniques.back());
}


RenderMethod::Technique *RenderMethodImpl::GetTechnique(size_t idx) const
{
	if (idx >= m_Techniques.size())
		return nullptr;

	return (RenderMethod::Technique *)&m_Techniques[idx];
}


size_t RenderMethodImpl::GetNumTechniques() const
{
	return m_Techniques.size();
}


bool RenderMethodImpl::GetActiveTechniqueIndex(size_t &idx) const
{
	if (m_ActiveTech >= m_Techniques.size())
		return false;

	idx = m_ActiveTech;
	return true;
}


RenderMethod::Technique *RenderMethodImpl::GetActiveTechnique() const
{
	size_t idx;
	if (GetActiveTechniqueIndex(idx))
		return GetTechnique(idx);

	return nullptr;
}


bool RenderMethodImpl::SetActiveTechnique(size_t idx)
{
	if (idx >= m_Techniques.size())
		return false;

	m_ActiveTech = idx;
	return true;
}


bool RenderMethodImpl::SetActiveTechniqueByName(const TCHAR *name)
{
	size_t idx;
	if (FindTechnique(name, idx))
		return SetActiveTechnique(idx);

	return false;
}


bool RenderMethodImpl::Load(const tinyxml2::XMLElement *proot, const TCHAR *options)
{
	if (!proot)
		return false;

	if (_stricmp(proot->Value(), "render_method"))
		return false;

	const tinyxml2::XMLAttribute *paname = proot->FindAttribute("name");
	if (paname)
	{
		TCHAR *n;
		CONVERT_MBCS2TCS(paname->Value(), n);
		m_Name = n;
	}

	const tinyxml2::XMLElement *ptel = proot->FirstChildElement("technique");
	while (ptel)
	{
		LoadTechnique(ptel, options);

		ptel = ptel->NextSiblingElement("technique");
	}

	return true;
}


void RenderMethodImpl::LoadTechnique(const tinyxml2::XMLElement *proot, const TCHAR *options)
{
	m_Techniques.push_back(TechniqueImpl(this));

	const tinyxml2::XMLAttribute *paname = proot->FindAttribute("name");
	if (paname)
	{
		TCHAR *n;
		CONVERT_MBCS2TCS(paname->Value(), n);

		tstring name;
		name = n;

		m_Techniques.back().SetName(name.c_str());
	}

	const tinyxml2::XMLAttribute *porder = proot->FindAttribute("draworder");
	if (porder)
	{
		m_Techniques.back().SetDrawOrder(porder->IntValue());
	}

	tstring techopts;
	const tinyxml2::XMLAttribute *poptions = proot->FindAttribute("options");
	if (poptions)
	{
		TCHAR *n;
		CONVERT_MBCS2TCS(poptions->Value(), n);

		techopts = n;
	}

	const tinyxml2::XMLElement *ppel = proot->FirstChildElement("pass");
	while (ppel)
	{
		LoadPass(&(m_Techniques.back()), ppel, techopts.c_str());

		ppel = ppel->NextSiblingElement("pass");
	}
}


bool RenderMethodImpl::FindTechnique(const TCHAR *name, size_t &idx) const
{
	for (idx = 0; idx < m_Techniques.size(); idx++)
	{
		auto pt = GetTechnique(idx);

		if (!_tcsicmp(pt->GetName(), name))
			return true;
	}

	return false;
}


bool RenderMethodImpl::PassImpl::LoadSetting(const tinyxml2::XMLElement *proot)
{
	const tinyxml2::XMLAttribute *paname = proot->FindAttribute("name");
	if (!paname)
		return false;

	TCHAR *n;
	CONVERT_MBCS2TCS(paname->Value(), n);
	tstring name = n;

	const tinyxml2::XMLAttribute *pavalue = proot->FindAttribute("value");
	if (!pavalue)
		return false;

	const tinyxml2::XMLAttribute *papersist = proot->FindAttribute("persist");
	bool persist = false;
	if (papersist)
	{
		TCHAR *pp;
		CONVERT_MBCS2TCS(papersist->Value(), pp);
		if (!_tcsicmp(_T("yes"), pp) || !_tcsicmp(_T("1"), pp) || !_tcsicmp(_T("true"), pp))
			persist = true;
	}

	const tinyxml2::XMLAttribute *pafbtarget = proot->FindAttribute("fbtarget");
	int fbtarget = -1;
	if (pafbtarget)
	{
		fbtarget = pafbtarget->IntValue();
		if (fbtarget >= FrameBuffer::MAX_COLORTARGETS)
		{
			// ??? ->GetSystem()->GetLog()->Print(_T("FBO target out of range!"));
		}
	}

	TCHAR *v;
	CONVERT_MBCS2TCS(pavalue->Value(), v);
	tstring value = v;

	auto SetShaderToLoad = [&](Renderer::ShaderComponentType shtype, const TCHAR *filename)
	{
		tstring modval = filename;

		const TCHAR * sms = m_pOwner->GetShaderMode();
		if (sms && *sms)
		{
			modval += _T("|");
			modval += m_pOwner->GetShaderMode();
		}

		m_ShaderCompFilename[shtype] = std::make_optional<tstring>(modval);
	};

	if (name == _T("framebuffer"))
	{
		m_FrameBufferName = std::make_optional<tstring>(value);
	}
	else if (name == _T("framebuffer.flags"))
	{
		props::TFlags64 flags = 0;
		m_FrameBufferFlags = flags;
		if (v)
		{
			while (*v)
			{
				while (*v && !__iswcsym(*v))
					v++;

				TCHAR *vp = v + 1;
				while (*vp && __iswcsym(*vp))
					vp++;

				if (*vp && (v != vp))
				{
					if (!_tcsnicmp(_T("clear_color"), v, vp - v))
					{
						m_FrameBufferFlags |= UFBFLAG_CLEARCOLOR;
					}
					else if (!_tcsnicmp(_T("clear_depth"), v, vp - v))
					{
						m_FrameBufferFlags |= UFBFLAG_CLEARDEPTH;
					}
					else if (!_tcsnicmp(_T("clear_stencil"), v, vp - v))
					{
						m_FrameBufferFlags |= UFBFLAG_CLEARSTENCIL;
					}
					else if (!_tcsnicmp(_T("finish_last"), v, vp - v))
					{
						m_FrameBufferFlags |= UFBFLAG_FINISHLAST;
					}
					else if (!_tcsnicmp(_T("update_viewport"), v, vp - v))
					{
						m_FrameBufferFlags |= UFBFLAG_UPDATEVIEWPORT;
					}
				}

				v = vp;
			}
		}
	}
	else if (name == _T("shader.geometry"))
	{
		SetShaderToLoad(Renderer::ShaderComponentType::ST_GEOMETRY, value.c_str());
	}
	else if (name == _T("shader.vertex"))
	{
		SetShaderToLoad(Renderer::ShaderComponentType::ST_VERTEX, value.c_str());
	}
	else if (name == _T("shader.fragment"))
	{
		SetShaderToLoad(Renderer::ShaderComponentType::ST_FRAGMENT, value.c_str());
	}
	else if (name == _T("shader.tesseval"))
	{
		SetShaderToLoad(Renderer::ShaderComponentType::ST_TESSEVAL, value.c_str());
	}
	else if (name == _T("shader.tesscontrol"))
	{
		SetShaderToLoad(Renderer::ShaderComponentType::ST_TESSCONTROL, value.c_str());
	}
	else if (name == _T("blendmode"))
	{
		if (persist)
			m_StateRestorationMask.Clear(RSOF_BLENDMODE);

		std::optional<Renderer::BlendMode> &blendmode = (fbtarget < 0) ? m_BlendMode : m_BlendModeCh[fbtarget];

		if (value == _T("add"))
		{
			blendmode = std::make_optional<Renderer::BlendMode>(Renderer::BlendMode::BM_ADD);
		}
		else if (value == _T("alpha"))
		{
			blendmode = std::make_optional<Renderer::BlendMode>(Renderer::BlendMode::BM_ALPHA);
		}
		else if (value == _T("addalpha"))
		{
			blendmode = std::make_optional<Renderer::BlendMode>(Renderer::BlendMode::BM_ADDALPHA);
		}
		else if (value == _T("alphatocoverage"))
		{
			blendmode = std::make_optional<Renderer::BlendMode>(Renderer::BlendMode::BM_ALPHATOCOVERAGE);
		}
		else if (value == _T("replace"))
		{
			blendmode = std::make_optional<Renderer::BlendMode>(Renderer::BlendMode::BM_REPLACE);
		}
		else if (value == _T("disabled"))
		{
			blendmode = std::make_optional<Renderer::BlendMode>(Renderer::BlendMode::BM_DISABLED);
		}
	}
	else if (name == _T("blendeq"))
	{
		if (persist)
			m_StateRestorationMask.Clear(RSOF_BLENDEQ);

		std::optional<Renderer::BlendEquation> &blendeq = (fbtarget < 0) ? m_BlendEq : m_BlendEqCh[fbtarget];

		if (value == _T("add"))
		{
			blendeq = std::make_optional<Renderer::BlendEquation>(Renderer::BlendEquation::BE_ADD);
		}
		else if (value == _T("subtract"))
		{
			blendeq = std::make_optional<Renderer::BlendEquation>(Renderer::BlendEquation::BE_SUBTRACT);
		}
		else if (value == _T("revsubtract"))
		{
			blendeq = std::make_optional<Renderer::BlendEquation>(Renderer::BlendEquation::BE_REVERSE_SUBTRACT);
		}
		else if (value == _T("min"))
		{
			blendeq = std::make_optional<Renderer::BlendEquation>(Renderer::BlendEquation::BE_MIN);
		}
		else if (value == _T("max"))
		{
			blendeq = std::make_optional<Renderer::BlendEquation>(Renderer::BlendEquation::BE_MAX);
		}
	}
	else if (name == _T("channelmask"))
	{
		if (persist)
			m_StateRestorationMask.Clear(RSOF_COLORMASK);

		std::optional<Renderer::ChannelMask> &chmask = (fbtarget < 0) ? m_ChannelWriteMask : m_ChannelWriteMaskCh[fbtarget];

		bool r = (_tcschr(value.c_str(), _T('R')) != nullptr) || (_tcschr(value.c_str(), _T('r')) != nullptr);
		bool g = (_tcschr(value.c_str(), _T('G')) != nullptr) || (_tcschr(value.c_str(), _T('g')) != nullptr);
		bool b = (_tcschr(value.c_str(), _T('B')) != nullptr) || (_tcschr(value.c_str(), _T('b')) != nullptr);
		bool a = (_tcschr(value.c_str(), _T('A')) != nullptr) || (_tcschr(value.c_str(), _T('a')) != nullptr);

		chmask = std::make_optional<Renderer::ChannelMask>((r ? CM_RED : 0) | (g ? CM_GREEN : 0) | (b ? CM_BLUE : 0) | (a ? CM_ALPHA : 0));
	}
	else if (name == _T("cullmode"))
	{
		if (persist)
			m_StateRestorationMask.Clear(RSOF_CULLMODE);

		if (value == _T("front"))
		{
			m_CullMode = std::make_optional<Renderer::CullMode>(Renderer::CullMode::CM_FRONT);
		}
		else if (value == _T("back"))
		{
			m_CullMode = std::make_optional<Renderer::CullMode>(Renderer::CullMode::CM_BACK);
		}
		else if (value == _T("disabled"))
		{
			m_CullMode = std::make_optional<Renderer::CullMode>(Renderer::CullMode::CM_DISABLED);
		}
		else if (value == _T("all"))
		{
			m_CullMode = std::make_optional<Renderer::CullMode>(Renderer::CullMode::CM_ALL);
		}
	}
	else if (name == _T("windingorder"))
	{
		if (persist)
			m_StateRestorationMask.Clear(RSOF_WINDINGORDER);

		if (value == _T("cw"))
		{
			m_WindingOrder = std::make_optional<Renderer::WindingOrder>(Renderer::WindingOrder::WO_CW);
		}
		else if (value == _T("ccw"))
		{
			m_WindingOrder = std::make_optional<Renderer::WindingOrder>(Renderer::WindingOrder::WO_CCW);
		}
	}
	else if (name == _T("depthmode"))
	{
		if (persist)
			m_StateRestorationMask.Clear(RSOF_DEPTHMODE);

		if (value == _T("readwrite"))
		{
			m_DepthMode = std::make_optional<Renderer::DepthMode>(Renderer::DepthMode::DM_READWRITE);
		}
		else if (value == _T("readonly"))
		{
			m_DepthMode = std::make_optional<Renderer::DepthMode>(Renderer::DepthMode::DM_READONLY);
		}
		else if (value == _T("writeonly"))
		{
			m_DepthMode = std::make_optional<Renderer::DepthMode>(Renderer::DepthMode::DM_WRITEONLY);
		}
		else if (value == _T("disabled"))
		{
			m_DepthMode = std::make_optional<Renderer::DepthMode>(Renderer::DepthMode::DM_DISABLED);
		}
	}
	else if (name == _T("fillmode"))
	{
		if (persist)
			m_StateRestorationMask.Clear(RSOF_FILLMODE);

		if (value == _T("fill"))
		{
			m_FillMode = std::make_optional<Renderer::FillMode>(Renderer::FillMode::FM_FILL);
		}
		else if (value == _T("point"))
		{
			m_FillMode = std::make_optional<Renderer::FillMode>(Renderer::FillMode::FM_POINT);
		}
		else if (value == _T("wire"))
		{
			m_FillMode = std::make_optional<Renderer::FillMode>(Renderer::FillMode::FM_WIRE);
		}
	}
	return true;
}


void RenderMethodImpl::LoadPass(TechniqueImpl *ptech, const tinyxml2::XMLElement *proot, const TCHAR *options)
{
	RenderMethodImpl::PassImpl *ppass = (RenderMethodImpl::PassImpl *)(ptech->AddPass());

	const tinyxml2::XMLElement *psel = proot->FirstChildElement("setting");
	while (psel)
	{
		((RenderMethodImpl::PassImpl *)ppass)->LoadSetting(psel);

		psel = psel->NextSiblingElement("setting");
	}
}

DECLARE_RESOURCETYPE(RenderMethod);

c3::ResourceType::LoadResult RESOURCETYPENAME(RenderMethod)::ReadFromFile(c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const
{
	if (returned_data)
	{
		*returned_data = nullptr;

		char *s;
		CONVERT_TCS2MBCS(filename, s);

		tinyxml2::XMLDocument doc;
		if (doc.LoadFile(s) != tinyxml2::XMLError::XML_SUCCESS)
			return ResourceType::LoadResult::LR_ERROR;

		RenderMethod *prm = psys->GetRenderer()->CreateRenderMethod();

		((RenderMethodImpl *)prm)->SetShaderMode(options);

		((RenderMethodImpl *)prm)->Load(doc.FirstChildElement(), options);

		*returned_data = (void *)prm;
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(RenderMethod)::ReadFromMemory(c3::System *psys, const TCHAR *contextname, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
{
	if (returned_data)
	{
		*returned_data = nullptr;

		tinyxml2::XMLDocument doc;
		if (doc.Parse((const char *)buffer, buffer_length) != tinyxml2::XMLError::XML_SUCCESS)
			return ResourceType::LoadResult::LR_ERROR;

		RenderMethod *prm = psys->GetRenderer()->CreateRenderMethod();

		((RenderMethodImpl *)prm)->SetShaderMode(options);

		((RenderMethodImpl *)prm)->Load(doc.FirstChildElement(), options);

		*returned_data = (void *)prm;
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


bool RESOURCETYPENAME(RenderMethod)::WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const
{
	return false;
}


void RESOURCETYPENAME(RenderMethod)::Unload(void *data) const
{
	((RenderMethod *)data)->Release();
}
