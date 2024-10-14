// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3TextRendererImpl.h>
#include <C3PositionableImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(TextRenderer, TextRendererImpl);


TextRendererImpl::TextRendererImpl()
{
	m_pImgMtl = m_pTextMtl = nullptr;

	m_pTextVB = nullptr;

	m_pMethodImage = m_pMethodText = nullptr;
	m_TechText = m_TechImage = 0;
	m_pFont = nullptr;
}


TextRendererImpl::~TextRendererImpl()
{
	C3_SAFERELEASE(m_pTextVB);
}


void TextRendererImpl::Release()
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


props::TFlags64 TextRendererImpl::Flags() const
{
	return m_Flags;
}


bool TextRendererImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	props::IPropertySet *ps = pobject->GetProperties();
	if (!ps)
		return false;

	props::IProperty *pp;

	if (pp = ps->CreateProperty(_T("Image"), 'IImg'))
	{
		pp->SetString(_T(""));
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FILENAME);
	}

	if (pp = ps->CreateProperty(_T("Image.RenderMethod"), 'IIrm'))
	{
		pp->SetString(_T("std.c3rm"));
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FILENAME);
	}

	m_ImgColor = Color::iWhite;
	if (pp = ps->CreateProperty(_T("Image.Color.Normal"), 'INCi'))
	{
		pp->SetInt(m_ImgColor.i);
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
		pp->SetString(_T("This is a\nMulti-line Test!"));
	}

	if (pp = ps->CreateProperty(_T("Font"), 'IFnt'))
	{
		pp->SetString(_T("Arial(14)"));
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FONT_DESC);
	}

	if (pp = ps->CreateProperty(_T("Text.RenderMethod"), 'ITrm'))
	{
		pp->SetString(_T("std.c3rm"));
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

	m_TextColor = Color::iLightGrey;
	if (pp = ps->CreateProperty(_T("Text.Color.Normal"), 'INCt'))
	{
		pp->SetInt(m_TextColor.i);
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	return true;
}


void TextRendererImpl::Update(float elapsed_time)
{
}


bool TextRendererImpl::Prerender(Object::RenderFlags flags, int draworder)
{
	return true;
}


void TextRendererImpl::Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat)
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
		{
			props::TFlags32 f = 0;
			switch (m_VAlign)
			{
				case VerticalAlignment::VA_TOP:
					f.Set(FONTFLAG_VTOP);
					break;

				case VerticalAlignment::VA_CENTER:
					f.Set(FONTFLAG_VCENTER);
					break;

				case VerticalAlignment::VA_BOTTOM:
					f.Set(FONTFLAG_VBOTTOM);
					break;
			}

			switch (m_HAlign)
			{
				case HorizontalAlignment::HA_LEFT:
					f.Set(FONTFLAG_HLEFT);
					break;

				case HorizontalAlignment::HA_CENTER:
					f.Set(FONTFLAG_HCENTER);
					break;

				case HorizontalAlignment::HA_RIGHT:
					f.Set(FONTFLAG_HRIGHT);
					break;
			}

			m_TextQuads = m_pFont->RenderText(s, m_pTextVB, f, &m_Rect);
		}

		m_Flags.Clear(IEF_UPDATETEXT);
	}

	if (!flags.IsSet(RF_LOCKMATERIAL))
	{
		if (!m_pTextMtl)
			m_pTextMtl = pr->GetMaterialManager()->CreateMaterial();
		if (m_pTextMtl)
		{
			m_pTextMtl->SetColor(Material::CCT_DIFFUSE, m_TextColor);
			if (m_pFont)
				m_pTextMtl->SetTexture(Material::TCT_DIFFUSE, m_pFont->GetMaterial()->GetTexture(Material::TCT_DIFFUSE));
			m_pTextMtl->SetDepthTest(Renderer::DT_LESSEREQUAL);
		}

		if (!m_pImgMtl)
			m_pImgMtl = pr->GetMaterialManager()->CreateMaterial();
		if (m_pImgMtl)
		{
			m_pImgMtl->SetColor(Material::CCT_DIFFUSE, m_ImgColor);
			m_pImgMtl->SetTexture(Material::TCT_DIFFUSE, m_pImage ? m_pImage : pr->GetWhiteTexture());
			m_pImgMtl->SetCullMode(Renderer::CM_DISABLED);
		}
	}

	if (!flags.IsSet(RF_LOCKSHADER))
	{
		if (!m_pMethodImage)
		{
			props::IProperty *pmethod = m_pOwner->GetProperties()->GetPropertyById('IIrm');
			if (prm)
			{
				c3::Resource *pres = prm->GetResource(pmethod ? pmethod->AsString() : _T("std.c3rm"), RESF_DEMANDLOAD);
				if (pres && (pres->GetStatus() == Resource::RS_LOADED))
					m_pMethodImage = (RenderMethod *)(pres->GetData());

				if (m_pMethodImage)
					m_pMethodImage->FindTechnique(_T("textbg"), m_TechImage);
			}
		}

		if (!m_pMethodText)
		{
			props::IProperty *pmethod = m_pOwner->GetProperties()->GetPropertyById('ITrm');
			if (prm)
			{
				c3::Resource *pres = prm->GetResource(pmethod ? pmethod->AsString() : _T("std.c3rm"), RESF_DEMANDLOAD);
				if (pres && (pres->GetStatus() == Resource::RS_LOADED))
					m_pMethodText = (RenderMethod *)(pres->GetData());

				if (m_pMethodText)
					m_pMethodText->FindTechnique(_T("text"), m_TechText);
			}
		}
	}

	static glm::fmat4x4 imat = glm::identity<glm::fmat4x4>();
	if (!pmat)
		pmat = &imat;

	Resource *pres = m_pOwner->GetSystem()->GetResourceManager()->GetResource(_T("[guirect.model]"));
	Model *pmod = (Model *)pres->GetData();

	if (!flags.IsSet(RF_LOCKSHADER))
	{
		m_pMethodImage->SetActiveTechnique(m_TechImage);
		pr->UseRenderMethod(m_pMethodImage);
	}

	if (!flags.IsSet(RF_LOCKMATERIAL))
		pr->UseMaterial(m_pImgMtl);

	glm::fmat4x4 transmat = glm::translate(glm::fvec3(m_Rect.left, 0.0f, m_Rect.top));
	glm::fmat4x4 sclmat = glm::scale(glm::fvec3(m_Rect.Width(), 1.0f, -m_Rect.Height()));
	glm::fmat4x4 mat = *pmat * (transmat * sclmat);
	pmod->Draw(&mat, false);

	if (m_pTextVB && *s && m_pFont)
	{
		if (!flags.IsSet(RF_LOCKSHADER))
		{
			m_pMethodText->SetActiveTechnique(m_TechText);
			pr->UseRenderMethod(m_pMethodText);
		}

		if (!flags.IsSet(RF_LOCKMATERIAL))
			pr->UseMaterial(m_pTextMtl);

		pr->UseVertexBuffer(m_pTextVB);
		pr->SetWorldMatrix(pmat);

		//pr->SetDepthBias(0.1f);

		Renderer::BlendMode bm = pr->GetActiveFrameBuffer()->GetBlendMode(0);
		pr->GetActiveFrameBuffer()->SetBlendMode(Renderer::BM_ALPHA, 0);

		Renderer::ChannelMask cm[4];
		for (size_t i = 0; i < 4; i++)
			cm[i] = pr->GetActiveFrameBuffer()->GetChannelWriteMask((int)i);

		pr->GetActiveFrameBuffer()->SetChannelWriteMask(CM_RED | CM_GREEN | CM_BLUE, 0);
		pr->GetActiveFrameBuffer()->SetChannelWriteMask(CM_ALPHA, 1);
		pr->GetActiveFrameBuffer()->SetChannelWriteMask(0, 2);
		pr->GetActiveFrameBuffer()->SetChannelWriteMask(CM_RED | CM_GREEN | CM_BLUE | CM_ALPHA, 3);

		pr->DrawPrimitives(Renderer::PrimType::TRILIST, m_TextQuads * 6);

		for (size_t i = 0; i < 4; i++)
			pr->GetActiveFrameBuffer()->SetChannelWriteMask(cm[i], (int)i);
		pr->GetActiveFrameBuffer()->SetBlendMode(bm, 0);
		//pr->SetDepthBias(0);
	}
}


void TextRendererImpl::PropertyChanged(const props::IProperty *pprop)
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
			m_Flags.Set(IEF_UPDATETEXT);
			break;

		case 'ITha':
			m_HAlign = (HorizontalAlignment)pprop->AsInt();
			m_Flags.Set(IEF_UPDATETEXT);
			break;

		case 'INCi':
			m_ImgColor.i = (uint32_t)pprop->AsInt();
			m_ImgColor.a = 255;
			break;

		case 'INCt':
			m_TextColor.i = (uint32_t)pprop->AsInt();
			m_TextColor.a = 255;
			break;
	}
}


bool TextRendererImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance) const
{
	if (!pRayPos || !pRayDir)
		return false;

	static glm::fmat4x4 imat = glm::identity<glm::fmat4x4>();
	if (!pmat)
		pmat = &imat;

	// Inverse of the transformation matrix to transform the ray to local space
	glm::fmat4x4 invMat = glm::inverse(*pmat);

	// Transform ray position and direction to local space
	glm::vec3 localRayPos = glm::vec3(invMat * glm::vec4(*pRayPos, 1.0f));
	glm::vec3 localRayDir = glm::normalize(glm::vec3(invMat * glm::vec4(*pRayDir, 0.0f)));

	bool ret = false;

	glm::fvec3 v_lt = glm::vec4(m_Rect.left, 0, m_Rect.top, 0);
	glm::fvec3 v_lb = glm::vec4(m_Rect.left, 0, m_Rect.bottom, 0);
	glm::fvec3 v_rb = glm::vec4(m_Rect.right, 0, m_Rect.bottom, 0);
	glm::fvec3 v_rt = glm::vec4(m_Rect.right, 0, m_Rect.top, 0);

	glm::vec2 luv;
	float ldist;

	// check for a collision
	bool hit = false;

	hit = glm::intersectRayTriangle(localRayPos, localRayDir, v_lt, v_lb, v_rt, luv, ldist);
	if (!hit)
		hit = glm::intersectRayTriangle(localRayPos, localRayDir, v_lt, v_rt, v_lb, luv, ldist);
	if (!hit)
		hit = glm::intersectRayTriangle(localRayPos, localRayDir, v_lb, v_rb, v_rt, luv, ldist);
	if (!hit)
		hit = glm::intersectRayTriangle(localRayPos, localRayDir, v_lb, v_rt, v_rb, luv, ldist);

	if (hit)
	{
		// Transform distance back to the original coordinate space
		glm::vec3 hitPoint = localRayPos + ldist * localRayDir;
		glm::vec3 transformedHitPoint = glm::vec3(*pmat * glm::vec4(hitPoint, 1.0f));
		float worldDistance = glm::length(transformedHitPoint - *pRayPos);

		float cdist = FLT_MAX, *pcdist = pDistance ? pDistance : &cdist;

		// Get the nearest collision
		if ((worldDistance >= 0) && (worldDistance < *pcdist))
		{
			*pcdist = worldDistance;
			ret = true;
		}
	}

	return ret;
}


std::vector<tstring> HorizontalAlignmentNames ={_T("Left"), _T("Center"), _T("Right")};
std::vector<tstring> VerticalAlignmentNames ={_T("Top"), _T("Center"), _T("Bottom")};
std::vector<tstring> ImageModeNames ={_T("Stretch"), _T("Wrap")};

size_t TextRendererImpl::GetNumValues(const props::IProperty *pprop) const
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


const TCHAR *TextRendererImpl::GetValue(const props::IProperty *pprop, size_t ordinal, TCHAR *buf, size_t bufsize) const
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
