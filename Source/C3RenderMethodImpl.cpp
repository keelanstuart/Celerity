// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3RenderMethodImpl.h>
#include <C3Resource.h>

using namespace c3;


RenderMethodImpl::PassImpl::PassImpl()
{
	m_ShaderProg = nullptr;
	m_FrameBuffer = nullptr;
	m_FrameBufferFlags = 0;
}


RenderMethodImpl::PassImpl::~PassImpl()
{
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

	if (m_FrameBufferName.has_value())
	{
		if (!m_FrameBuffer)
			m_FrameBuffer = prend->FindFrameBuffer((*m_FrameBufferName).c_str());

		prend->UseFrameBuffer(m_FrameBuffer, m_FrameBufferFlags);
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
				{
					// THIS BUILDS A PATH THAT INCLUDES THE SHADER MODE (like "SKIN")
					tstring path = (m_ShaderCompFilename[i])->c_str();
					path += _T("|");
					path += m_ShaderMode;

					scres[i] = prend->GetSystem()->GetResourceManager()->GetResource(path.c_str(), RESF_DEMANDLOAD);
				}

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

	return ret;
}


void RenderMethodImpl::PassImpl::SetFrameBufferName(const TCHAR *name)
{
	m_FrameBufferName = std::make_optional<tstring>(name);
}


bool RenderMethodImpl::PassImpl::GetFrameBufferName(tstring &name) const
{
	if (!m_FrameBufferName.has_value())
		return false;

	name = *m_FrameBufferName;
	return true;
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


RenderMethodImpl::TechniqueImpl::TechniqueImpl(Renderer *prend)
{
	m_pRend = prend;
	m_Mode = TECHMODE_NORMAL;
	m_Passes.resize(TECHMODE_NUMTYPES);
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
	return m_Passes[m_Mode].size();
}


RenderMethod::Pass *RenderMethodImpl::TechniqueImpl::GetPass(size_t idx) const
{
	return (RenderMethod::Pass *)&m_Passes[m_Mode][idx];
}


RenderMethod::Pass *RenderMethodImpl::TechniqueImpl::AddPass()
{
	RenderMethodImpl::PassImpl &ret = m_Passes[m_Mode].emplace_back();

	return &ret;
}


void RenderMethodImpl::TechniqueImpl::SetMode(ETechMode mode)
{
	m_Mode = mode;
}


bool RenderMethodImpl::TechniqueImpl::Begin(size_t &passes) const
{
	if (m_Passes[m_Mode].empty())
		return false;

	passes = m_Passes[m_Mode].size();
	return true;
}


Renderer::RenderStateOverrideFlags RenderMethodImpl::TechniqueImpl::ApplyPass(size_t idx) const
{
	if (idx >= m_Passes[m_Mode].size())
		return 0;

	PassImpl &pr = (PassImpl &)m_Passes[m_Mode][idx];
	return pr.Apply(m_pRend);
}


void RenderMethodImpl::TechniqueImpl::End() const
{

}


RenderMethodImpl::RenderMethodImpl(Renderer *prend)
{
	m_pRend = prend;
	m_ActiveTech = -1;
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
	TechniqueImpl t(m_pRend);
	m_Techniques.push_back(t);

	return &(m_Techniques.back());
}


RenderMethod::Technique *RenderMethodImpl::GetTechnique(size_t idx) const
{
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


bool RenderMethodImpl::Load(const tinyxml2::XMLElement *proot)
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
		LoadTechnique(ptel);

		ptel = ptel->NextSiblingElement("technique");
	}

	return true;
}


void RenderMethodImpl::LoadTechnique(const tinyxml2::XMLElement *proot)
{
	m_Techniques.push_back(TechniqueImpl(m_pRend));

	const tinyxml2::XMLAttribute *paname = proot->FindAttribute("name");
	if (paname)
	{
		TCHAR *n;
		CONVERT_MBCS2TCS(paname->Value(), n);

		m_Techniques.back().SetName(n);
	}

	const tinyxml2::XMLElement *ppel = proot->FirstChildElement("pass");
	while (ppel)
	{
		LoadPass(&(m_Techniques.back()), ppel);

		ppel = ppel->NextSiblingElement("pass");
	}
}


bool RenderMethodImpl::FindTechnique(const TCHAR *name, size_t &idx) const
{
	for (idx = 0; idx < m_Techniques.size(); idx++)
	{
		if (!_tcsicmp(GetTechnique(idx)->GetName(), name))
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

	TCHAR *v;
	CONVERT_MBCS2TCS(pavalue->Value(), v);
	tstring value = v;

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
		m_ShaderCompFilename[Renderer::ShaderComponentType::ST_GEOMETRY] = std::make_optional<tstring>(value);
	}
	else if (name == _T("shader.vertex"))
	{
		m_ShaderCompFilename[Renderer::ShaderComponentType::ST_VERTEX] = std::make_optional<tstring>(value);
	}
	else if (name == _T("shader.fragment"))
	{
		m_ShaderCompFilename[Renderer::ShaderComponentType::ST_FRAGMENT] = std::make_optional<tstring>(value);
	}
	else if (name == _T("shader.tesseval"))
	{
		m_ShaderCompFilename[Renderer::ShaderComponentType::ST_TESSEVAL] = std::make_optional<tstring>(value);
	}
	else if (name == _T("shader.tesscontrol"))
	{
		m_ShaderCompFilename[Renderer::ShaderComponentType::ST_TESSCONTROL] = std::make_optional<tstring>(value);
	}
	else if (name == _T("blendmode"))
	{
		if (value == _T("add"))
		{
			m_BlendMode = std::make_optional<Renderer::BlendMode>(Renderer::BlendMode::BM_ADD);
		}
		else if (value == _T("alpha"))
		{
			m_BlendMode = std::make_optional<Renderer::BlendMode>(Renderer::BlendMode::BM_ALPHA);
		}
		else if (value == _T("addalpha"))
		{
			m_BlendMode = std::make_optional<Renderer::BlendMode>(Renderer::BlendMode::BM_ADDALPHA);
		}
		else if (value == _T("alphatocoverage"))
		{
			m_BlendMode = std::make_optional<Renderer::BlendMode>(Renderer::BlendMode::BM_ALPHATOCOVERAGE);
		}
		else if (value == _T("replace"))
		{
			m_BlendMode = std::make_optional<Renderer::BlendMode>(Renderer::BlendMode::BM_REPLACE);
		}
		else if (value == _T("disabled"))
		{
			m_BlendMode = std::make_optional<Renderer::BlendMode>(Renderer::BlendMode::BM_DISABLED);
		}
	}
	else if (name == _T("blendeq"))
	{
		if (value == _T("add"))
		{
			m_BlendEq = std::make_optional<Renderer::BlendEquation>(Renderer::BlendEquation::BE_ADD);
		}
		else if (value == _T("subtract"))
		{
			m_BlendEq = std::make_optional<Renderer::BlendEquation>(Renderer::BlendEquation::BE_SUBTRACT);
		}
		else if (value == _T("revsubtract"))
		{
			m_BlendEq = std::make_optional<Renderer::BlendEquation>(Renderer::BlendEquation::BE_REVERSE_SUBTRACT);
		}
		else if (value == _T("min"))
		{
			m_BlendEq = std::make_optional<Renderer::BlendEquation>(Renderer::BlendEquation::BE_MIN);
		}
		else if (value == _T("max"))
		{
			m_BlendEq = std::make_optional<Renderer::BlendEquation>(Renderer::BlendEquation::BE_MAX);
		}
	}
	else if (name == _T("cullmode"))
	{
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


void RenderMethodImpl::PassImpl::SetShaderMode(const TCHAR *mode)
{
	m_ShaderMode = mode;
}


void RenderMethodImpl::LoadPass(TechniqueImpl *ptech, const tinyxml2::XMLElement *proot)
{
	const TCHAR *techmode_names[RenderMethod::Technique::ETechMode::TECHMODE_NUMTYPES] = { _T(""), _T("SKIN") };

	for (size_t i = 0; i < RenderMethod::Technique::ETechMode::TECHMODE_NUMTYPES; i++)
	{
		ptech->SetMode((RenderMethod::Technique::ETechMode)i);

		RenderMethodImpl::PassImpl *ppass = (RenderMethodImpl::PassImpl *)(ptech->AddPass());
		ppass->SetShaderMode(techmode_names[i]);

		const tinyxml2::XMLElement *psel = proot->FirstChildElement("setting");
		while (psel)
		{
			((RenderMethodImpl::PassImpl *)ppass)->LoadSetting(psel);

			psel = psel->NextSiblingElement("setting");
		}
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
		((RenderMethodImpl *)prm)->Load(doc.FirstChildElement());

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
		((RenderMethodImpl *)prm)->Load(doc.FirstChildElement());

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
