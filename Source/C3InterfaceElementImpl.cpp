// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3InterfaceElementImpl.h>
#include <C3PositionableImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(InterfaceElement, InterfaceElementImpl);


InterfaceElementImpl::InterfaceElementImpl()
{
	m_pImgMtl = m_pTextMtl = nullptr;
	m_State = IS_NORMAL;

	m_pTextVB = nullptr;

	m_pMethodImage = m_pMethodText = nullptr;
	m_TechText = m_TechImage = 0;
	m_pFont = nullptr;

	m_Dims.x = 100.0f;
	m_Dims.y = 75.0f;
}


InterfaceElementImpl::~InterfaceElementImpl()
{
	C3_SAFERELEASE(m_pTextVB);
}


void InterfaceElementImpl::Release()
{
	props::IPropertySet *pps = m_pOwner->GetProperties();
	props::IProperty *pp;

	pp = pps->GetPropertyById('ITva');
	if (pp) pp->ExternalizeReference();

	pp = pps->GetPropertyById('ITha');
	if (pp) pp->ExternalizeReference();

	pp = pps->GetPropertyById('ITMi');
	if (pp) pp->ExternalizeReference();

	delete this;
}


props::TFlags64 InterfaceElementImpl::Flags() const
{
	return m_Flags;
}


bool InterfaceElementImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	props::IPropertySet *ps = pobject->GetProperties();
	if (!ps)
		return false;

	props::IProperty *pp;

	pp = ps->CreateProperty(_T("Dimensions"), 'DIMS');
	if (pp)
	{
		pp->SetVec2F(m_Dims);
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_DIMENSIONS);
	}

	if (pp = ps->CreateProperty(_T("Image"), 'IImg'))
	{
		pp->SetString(_T(""));
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FILENAME);
	}

	if (pp = ps->CreateProperty(_T("Image.RenderMethod"), 'IIrm'))
	{
		pp->SetString(_T("ui.c3rm"));
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FILENAME);
	}

	m_ImgColor[IS_NORMAL] = Color::iWhite;
	if (pp = ps->CreateProperty(_T("Image.Color.Normal"), 'INCi'))
	{
		pp->SetInt(m_ImgColor[IS_NORMAL].i);
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_ImgColor[IS_HOVER] = Color::iLightGrey;
	if (pp = ps->CreateProperty(_T("Image.Color.Hover"), 'IHCi'))
	{
		pp->SetInt(m_ImgColor[IS_HOVER].i);
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_ImgColor[IS_DOWN] = Color::iGrey;
	if (pp = ps->CreateProperty(_T("Image.Color.Down"), 'IDCi'))
	{
		pp->SetInt(m_ImgColor[IS_DOWN].i);
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	if (pp = ps->CreateProperty(_T("Image.Mode"), 'ITMi'))
	{
		pp->SetEnumProvider(this);
		pp->SetEnumVal(m_VAlign);
	}


	if (pp = ps->CreateProperty(_T("Text"), 'ITxt'))
	{
		pp->SetString(_T(""));
	}

	if (pp = ps->CreateProperty(_T("Font"), 'IFnt'))
	{
		pp->SetString(_T("Arial(14)"));
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FONT_DESC);
	}

	if (pp = ps->CreateProperty(_T("Text.RenderMethod"), 'ITrm'))
	{
		pp->SetString(_T("ui.c3rm"));
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FILENAME);
	}

	if (pp = ps->CreateProperty(_T("Text.VerticalAlignment"), 'ITva'))
	{
		pp->SetEnumProvider(this);
		pp->SetEnumVal(m_VAlign);
	}

	if (pp = ps->CreateProperty(_T("Text.HorizontalAlignment"), 'ITha'))
	{
		pp->SetEnumProvider(this);
		pp->SetEnumVal(m_HAlign);
	}

	m_TextColor[IS_NORMAL] = Color::iLightGrey;
	if (pp = ps->CreateProperty(_T("Text.Color.Normal"), 'INCt'))
	{
		pp->SetInt(m_TextColor[IS_NORMAL].i);
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_TextColor[IS_HOVER] = Color::iWhite;
	if (pp = ps->CreateProperty(_T("Text.Color.Hover"), 'IHCt'))
	{
		pp->SetInt(m_TextColor[IS_HOVER].i);
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_TextColor[IS_DOWN] = Color::iGrey;
	if (pp = ps->CreateProperty(_T("Text.Color.Down"), 'IDCt'))
	{
		pp->SetInt(m_TextColor[IS_DOWN].i);
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	return true;
}


void InterfaceElementImpl::Update(float elapsed_time)
{
	PositionableImpl *ppos = dynamic_cast<PositionableImpl *>(m_pOwner->FindComponent(Positionable::Type()));
	if (ppos)
	{
		m_Rect.left = ppos->GetPosX();
		m_Rect.right = m_Rect.left + ppos->GetSclX();
		m_Rect.top = ppos->GetPosY();
		m_Rect.bottom = m_Rect.top + ppos->GetSclY();
	}

	InputManager *pim = m_pOwner->GetSystem()->GetInputManager();

	int32_t mx, my;
	pim->GetMousePos(mx, my);
	if ((my >= m_Rect.top) && (my <= m_Rect.bottom) && (mx >= m_Rect.left) && (mx <= m_Rect.right))
	{
		m_State = IS_HOVER;
	}

	if (m_State == IS_HOVER)
	{
		bool ldown = false, lup = false, rdown = false, rup = false;

		if (pim->ButtonPressed(InputDevice::VirtualButton::BUTTON1))
			ldown = true;
		else if (pim->ButtonReleased(InputDevice::VirtualButton::BUTTON1))
			lup = true;

		if (pim->ButtonPressed(InputDevice::VirtualButton::BUTTON2))
			rdown = true;
		else if (pim->ButtonReleased(InputDevice::VirtualButton::BUTTON2))
			rup = true;

		if (ldown || lup || rdown || rup)
		{
			ScriptableImpl *pscr = dynamic_cast<ScriptableImpl *>(m_pOwner->FindComponent(Scriptable::Type()));
			if (pscr)
			{
				if (ldown)
					pscr->Execute(_T("OnPress(1);"));
				else if (lup)
					pscr->Execute(_T("OnRelease(1);"));

				if (rdown)
					pscr->Execute(_T("OnPress(2);"));
				else if (rup)
					pscr->Execute(_T("OnRelease(2);"));
			}
		}
	}
}


bool InterfaceElementImpl::Prerender(Object::RenderFlags flags, int draworder)
{
	if (!flags.IsSet(RF_GUI))
		return false;

	return true;
}


void InterfaceElementImpl::Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat)
{
	Renderer *pr = m_pOwner->GetSystem()->GetRenderer();
	ResourceManager *prm = m_pOwner->GetSystem()->GetResourceManager();

	if (m_Flags.IsSet(IEF_UPDATEIMAGE))
	{
		if (props::IProperty *pp = m_pOwner->GetProperties()->GetPropertyById('IImg'))
		{
			Resource *pres = prm->GetResource(pp->AsString(), RESF_DEMANDLOAD);
			if (pres && (pres->GetStatus() == Resource::Status::RS_LOADED))
			{
				m_pImage = dynamic_cast<Texture2D *>((Texture2D *)(pres->GetData()));
				if (!m_pImage)
					m_pImage = pr->GetWhiteTexture();
			}
			m_Flags.Clear(IEF_UPDATEIMAGE);
		}
	}

	if (!m_pFont)
	{
		props::IProperty *pfp = m_pOwner->GetProperties()->GetPropertyById('IFnt');

		static LOGFONT lf;
		Font::ParseFontDescIntoLOGFONT((const TCHAR * const)pfp->AsString(), lf);
		m_pFont = pr->GetFont(lf.lfFaceName, lf.lfHeight);
	}

	props::IProperty *pp = m_pOwner->GetProperties()->GetPropertyById('ITxt');
	const TCHAR *s = pp ? pp->AsString() : _T("");

	if (*s && m_pFont && m_Flags.IsSet(IEF_UPDATETEXT))
	{
		if (!m_pTextVB)
			m_pTextVB = pr->CreateVertexBuffer();

		if (m_pTextVB)
			m_TextQuads = m_pFont->RenderText(s, m_pTextVB);

		m_Flags.Clear(IEF_UPDATETEXT);
	}

	if (!m_pTextMtl)
		m_pTextMtl = pr->GetMaterialManager()->CreateMaterial();
	if (m_pTextMtl)
	{
		m_pTextMtl->SetColor(Material::CCT_DIFFUSE, m_TextColor[m_State]);
		if (m_pFont)
			m_pTextMtl->SetTexture(Material::TCT_DIFFUSE, m_pFont->GetMaterial()->GetTexture(Material::TCT_DIFFUSE));
	}

	if (!m_pImgMtl)
		m_pImgMtl = pr->GetMaterialManager()->CreateMaterial();
	if (m_pImgMtl)
	{
		m_pImgMtl->SetColor(Material::CCT_DIFFUSE, m_ImgColor[m_State]);
		m_pImgMtl->SetTexture(Material::TCT_DIFFUSE, m_pImage ? m_pImage : pr->GetWhiteTexture());
	}

	if (!m_pMethodImage)
	{
		props::IProperty *pmethod = m_pOwner->GetProperties()->GetPropertyById('IIrm');
		if (prm)
		{
			c3::Resource *pres = prm->GetResource(pmethod ? pmethod->AsString() : _T("ui.c3rm"), RESF_DEMANDLOAD);
			if (pres && (pres->GetStatus() == Resource::RS_LOADED))
				m_pMethodImage = (RenderMethod *)(pres->GetData());

			if (m_pMethodImage)
				m_pMethodImage->FindTechnique(_T("image"), m_TechImage);
		}
	}

	if (!m_pMethodText)
	{
		props::IProperty *pmethod = m_pOwner->GetProperties()->GetPropertyById('ITrm');
		if (prm)
		{
			c3::Resource *pres = prm->GetResource(pmethod ? pmethod->AsString() : _T("ui.c3rm"), RESF_DEMANDLOAD);
			if (pres && (pres->GetStatus() == Resource::RS_LOADED))
				m_pMethodText = (RenderMethod *)(pres->GetData());

			if (m_pMethodText)
				m_pMethodImage->FindTechnique(_T("text"), m_TechText);
		}
	}

	static glm::fmat4x4 imat = glm::identity<glm::fmat4x4>();
	if (!pmat)
		pmat = &imat;

	Resource *pres = m_pOwner->GetSystem()->GetResourceManager()->GetResource(_T("[guirect.model]"));
	Model *pmod = (Model *)pres->GetData();

	RECT vr;
	pr->GetViewport(&vr);
	float L = (float)vr.left;
	float R = (float)vr.right;
	float T = (float)vr.top;
	float B = (float)vr.bottom;
	glm::fmat4x4 ortho_projection =
	{
		{ 2.0f/(L-R),   0.0f,         0.0f,   0.0f },
		{ 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
		{ 0.0f,         0.0f,        -1.0f,   0.0f },
		{ (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
	};

	m_pMethodImage->SetActiveTechnique(m_TechImage);
	pr->UseRenderMethod(m_pMethodImage);
	pr->SetProjectionMatrix(&ortho_projection);
	pr->UseMaterial(m_pImgMtl);
	glm::fmat4x4 mat = *pmat * glm::scale(glm::fvec3(m_Dims.x, 1.0f, m_Dims.y));
	pmod->Draw(&mat, false);

	if (m_pTextVB && *s && m_pFont)
	{
		m_pMethodImage->SetActiveTechnique(m_TechText);
		pr->UseRenderMethod(m_pMethodText);
		pr->UseMaterial(m_pTextMtl);
		pr->UseVertexBuffer(m_pTextVB);
		pr->SetWorldMatrix(pmat);
		pr->DrawPrimitives(Renderer::PrimType::TRILIST, m_TextQuads * 6);
	}
}


void InterfaceElementImpl::PropertyChanged(const props::IProperty *pprop)
{
	if (!pprop)
		return;

	switch (pprop->GetID())
	{
		case 'IImg':
			m_pImage = nullptr;
			m_Flags.Set(IEF_UPDATEIMAGE);
			break;

		case 'IIrm':
			m_pMethodImage = nullptr;
			break;

		case 'ITrm':
			m_pMethodText = nullptr;
			break;

		case 'IFnt':
			m_pFont = nullptr;
		case 'ITxt':
			m_Flags.Set(IEF_UPDATETEXT);
			break;

		case 'ITva':
			m_VAlign = (VerticalAlignment)pprop->AsInt();
			break;

		case 'ITha':
			m_HAlign = (HorizontalAlignment)pprop->AsInt();
			break;

		case 'INCi':
			m_ImgColor[IS_NORMAL].i = (uint32_t)pprop->AsInt();
			m_ImgColor[IS_NORMAL].a = 255;
			break;

		case 'IHCi':
			m_ImgColor[IS_HOVER].i = (uint32_t)pprop->AsInt();
			m_ImgColor[IS_HOVER].a = 255;
			break;

		case 'IDCi':
			m_ImgColor[IS_DOWN].i = (uint32_t)pprop->AsInt();
			m_ImgColor[IS_DOWN].a = 255;
			break;

		case 'INCt':
			m_TextColor[IS_NORMAL].i = (uint32_t)pprop->AsInt();
			m_TextColor[IS_NORMAL].a = 255;
			break;

		case 'IHCt':
			m_TextColor[IS_HOVER].i = (uint32_t)pprop->AsInt();
			m_TextColor[IS_HOVER].a = 255;
			break;

		case 'IDCt':
			m_TextColor[IS_DOWN].i = (uint32_t)pprop->AsInt();
			m_TextColor[IS_DOWN].a = 255;
			break;

		case 'DIMS':
			pprop->AsVec2F(&m_Dims);
			break;
	}
}


bool InterfaceElementImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance) const
{
	if (!pRayPos || !pRayDir)
		return false;

	static glm::fmat4x4 imat = glm::identity<glm::fmat4x4>();
	if (!pmat)
		pmat = &imat;

	bool ret = false;

	glm::fvec3 v_lt = *pmat * glm::vec4(m_Rect.left, m_Rect.top, 0, 1);
	glm::fvec3 v_lb = *pmat * glm::vec4(m_Rect.left, m_Rect.bottom, 0, 1);
	glm::fvec3 v_rb = *pmat * glm::vec4(m_Rect.right, m_Rect.bottom, 0, 1);
	glm::fvec3 v_rt = *pmat * glm::vec4(m_Rect.right, m_Rect.top, 0, 1);

	glm::vec2 luv;
	float ldist;

	// check for a collision
	bool hit = glm::intersectRayTriangle(*pRayPos, *pRayDir, v_lt, v_lb, v_rt, luv, ldist);
	if (!hit)
		hit = glm::intersectRayTriangle(*pRayPos, *pRayDir, v_lb, v_rb, v_rt, luv, ldist);

	if (hit)
	{
		float cdist = FLT_MAX, *pcdist = pDistance ? pDistance : &cdist;

		// get the nearest collision
		if ((ldist >= 0) && (ldist < *pcdist))
		{
			if (pDistance)
				*pcdist = ldist;

			ret = true;
		}
	}

	return ret;
}


std::vector<tstring> HorizontalAlignmentNames ={_T("Left"), _T("Center"), _T("Right")};
std::vector<tstring> VerticalAlignmentNames ={_T("Top"), _T("Center"), _T("Bottom")};
std::vector<tstring> ImageModeNames ={_T("Stretch"), _T("Wrap")};

size_t InterfaceElementImpl::GetNumValues(const props::IProperty *pprop) const
{
	if (pprop)
	{
		switch (pprop->GetID())
		{
			case 'ITva':
				return VerticalAlignmentNames.size();
				break;

			case 'ITha':
				return HorizontalAlignmentNames.size();
				break;

			case 'ITMi':
				return ImageModeNames.size();
				break;
		}
	}

	return 0;
}


const TCHAR *InterfaceElementImpl::GetValue(const props::IProperty *pprop, size_t ordinal, TCHAR *buf, size_t bufsize) const
{
	if (pprop)
	{
		switch (pprop->GetID())
		{
			case 'ITva':
				if (buf && (bufsize >= ((VerticalAlignmentNames[ordinal].length() + 1) * sizeof(TCHAR))))
					_tcscpy_s(buf, bufsize, VerticalAlignmentNames[ordinal].c_str());
				return VerticalAlignmentNames[ordinal].c_str();
				break;

			case 'ITha':
				if (buf && (bufsize >= ((HorizontalAlignmentNames[ordinal].length() + 1) * sizeof(TCHAR))))
					_tcscpy_s(buf, bufsize, HorizontalAlignmentNames[ordinal].c_str());
				return HorizontalAlignmentNames[ordinal].c_str();
				break;

			case 'ITMi':
				if (buf && (bufsize >= ((ImageModeNames[ordinal].length() + 1) * sizeof(TCHAR))))
					_tcscpy_s(buf, bufsize, ImageModeNames[ordinal].c_str());
				return ImageModeNames[ordinal].c_str();
				break;
		}
	}

	return nullptr;
}
