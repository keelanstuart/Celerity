// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3FontImpl.h>

using namespace c3;



FontImpl::FontImpl(Renderer *prend, const TCHAR *fontname, size_t fontsize)
{
	m_pRend = prend;
	m_Name = fontname;
	m_Size = fontsize;

	m_Tex = nullptr;
	m_Mtl = nullptr;

	m_Initialized = false;

	m_Kerning = 0.0f;
	m_ExtraLineSpacing = 0.0f;
}

bool FontImpl::Initialize()
{
	if (m_Initialized)
		return true;

	size_t w = 1024;
	size_t h = 1024;

	if (!m_Tex)
		m_Tex = m_pRend->CreateTexture2D(w, h, Renderer::TextureType::U8_1CH, 1);

	if (!m_Tex)
	{
		m_pRend->GetSystem()->GetLog()->Print(_T("Failed to create a Texture2D for the requested font (\"%s\")!"), m_Name.c_str());
		return false;
	}

	if (!m_Mtl)
		m_Mtl = m_pRend->GetMaterialManager()->CreateMaterial();

	if (m_Mtl)
	{
		m_Mtl->SetWindingOrder(Renderer::WindingOrder::WO_CCW);
		m_Mtl->SetTexture(Material::TextureComponentType::TCT_DIFFUSE, m_Tex);
		m_Mtl->SetColor(Material::ColorComponentType::CCT_DIFFUSE, &Color::fWhite);
	}
	else
	{
		m_pRend->GetSystem()->GetLog()->Print(_T("Failed to create a Material for the requested font (\"%s\")!"), m_Name.c_str());
		return false;
	}

	// Prepare to create a bitmap
    uint32_t *psrc;
    BITMAPINFO bmi;
    ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth =  (int)w;
    bmi.bmiHeader.biHeight = -(int)h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biBitCount = 32;

    // Create a DC and a bitmap for the font
    HDC hDC = CreateCompatibleDC(NULL);
    HBITMAP hbmBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (VOID **)&psrc, NULL, 0);
    SetMapMode(hDC, MM_TEXT);

    // Create a font.  By specifying ANTIALIASED_QUALITY, we might get an
    // antialiased font, but this is not guaranteed.
    INT nHeight = -MulDiv((int)m_Size, (INT)(GetDeviceCaps(hDC, LOGPIXELSY)), 72);
	DWORD dwBold = FALSE; // m_crFlags.IsSet(FONTCFLAG_BOLD) ? FW_BOLD : FW_NORMAL;
    DWORD dwItalic = FALSE;
    HFONT hFont = CreateFont(nHeight, 0, 0, 0, dwBold, dwItalic,
                          FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                          VARIABLE_PITCH, m_Name.c_str());
	if (hFont)
	{
		HGDIOBJ oldbmp = SelectObject(hDC, hbmBitmap);
		HGDIOBJ oldfont = SelectObject(hDC, hFont);

		// Set text properties
		SetTextColor(hDC, RGB(255, 255, 255));
		SetBkColor(hDC, RGB(0, 0, 0));
		SetTextAlign(hDC, TA_TOP);

		TEXTMETRIC textmetric;
		GetTextMetrics(hDC, &textmetric);

		// Loop through all printable character and output them to the bitmap..
		// Meanwhile, keep track of the corresponding tex coords for each character.
		DWORD x = 0;
		DWORD y = 0;
		TCHAR str[2] = _T("x");

		SIZE charsize;

		uint8_t *font_pixels = nullptr;
		Texture2D::SLockInfo li;

		GlyphInfo gi;

		// Lock the surface and write the alpha values for the set pixels
		if (m_Tex->Lock((void **)&font_pixels, li) == Texture::RETURNCODE::RET_OK)
		{

			TCHAR c = _T(' ');

			while (true)
			{
				str[0] = c;

				GetTextExtentPoint32(hDC, str, 1, &charsize);

				if ((DWORD)(x + charsize.cx + 10) > w)
				{
					x  = 0;
					y += charsize.cy + 3;
					if (y >= (m_Tex->Height() - (m_Size * 2)))
						break;
				}

				ExtTextOut(hDC, x, y, 0, NULL, str, 1, NULL);

				gi.w = charsize.cx;
				gi.h = charsize.cy;
				gi.uv.ul.x = (float)x / (float)w;
				gi.uv.ul.y = (float)y / (float)h;
				gi.uv.lr.x = (float)(x + gi.w) / (float)w;
				gi.uv.lr.y = (float)(y + gi.h) / (float)h;

				m_GlyphInfo.emplace(TGlyphInfoMap::value_type(c, gi));

				x += (DWORD)gi.w + 5;

				c++;
			}

			// take each pixel and, given that it is rendered by GDI in black and white, use just the red channel to derive the alpha channel, filling in white for the rest
			for (y = 0; y < h; y++)
			{
				for (x = 0; x < w; x++)
				{
					*(font_pixels++) = *(psrc++) & 0xff;
				}
			}

			m_Tex->Unlock();
		}

		auto it = m_GlyphInfo.find(_T(' '));
		if (it != m_GlyphInfo.end())
			m_SpaceWidth = (float)it->second.w;

		SelectObject(hDC, oldbmp);
		SelectObject(hDC, oldfont);

		m_Initialized = true;
	}
	else
	{
		m_pRend->GetSystem()->GetLog()->Print(_T("Failed to create the requested font \"%s\""), m_Name.c_str());
	}

    DeleteObject(hFont);
	DeleteObject(hbmBitmap);
	DeleteDC(hDC);

	return m_Initialized;
}


FontImpl::~FontImpl()
{
	C3_SAFERELEASE(m_Tex);
}


void FontImpl::GetTextExtent(const TCHAR *text, RECT &extent, float tabwidth) const
{
	extent.left = extent.top = extent.right = extent.bottom = 0;

	if (!m_Initialized)
		return;

	tabwidth *= m_SpaceWidth;

	if(!text || !*text)
		return;

	const TCHAR *c = text;

	glm::fvec2 corg(0, 0);

	while (!*c)
	{
		TGlyphInfoMap::const_iterator it = m_GlyphInfo.find(*c);

		switch (*c)
		{
			case _T('\n'):
				corg.x = 0.0f;
				corg.y += m_Size + m_ExtraLineSpacing;
				break;

			case _T('\t'):
				corg.x += tabwidth;
				break;

			default:
				if (it != m_GlyphInfo.end())
				{
					extent.right = std::max<LONG>(extent.right, (LONG)(corg.x + it->second.w));
					extent.bottom = std::max<LONG>(extent.bottom, (LONG)(corg.y + it->second.h));

					corg.x = it->second.w + m_Kerning;
				}
				break;
		}

		c++;
	}
}


size_t FontImpl::RenderText(const TCHAR *text, VertexBuffer *pverts, props::TFlags32 draw_flags, float tabwidth) const
{
	if (!m_Initialized)
		return 0;

	if (!text || !*text)
		return 0;

	UINT32 length = 0;
	for (const TCHAR *c = text; *c != _T('\0'); c++)
		if (std::isprint(*c))
			length++;

	float sx = 0;
	float sy = 0;
	float rx = 0;

	float topshift = draw_flags.IsSet(FONTFLAG_ITALIC) ? 8.0f : 0.0f;

	tabwidth *= m_SpaceWidth;

	constexpr VertexBuffer::ComponentDescription vd[] =
	{
		{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_POSITION},
		{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 2, c3::VertexBuffer::ComponentDescription::Usage::VU_TEXCOORD0},

		{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_NONE, 0, c3::VertexBuffer::ComponentDescription::Usage::VU_NONE},
	};

#pragma pack(push, 1)
	struct STextVert
	{
		glm::fvec3 pos;
		glm::fvec2 tex0;
	} *v;
#pragma pack(pop)

	glm::fvec2 corg(0, 0);

	if (pverts->Lock((void **)&v, length * 4, vd, VBLOCKFLAG_WRITE | VBLOCKFLAG_DYNAMIC))
	{
		while (*text)
		{
			switch (*text)
			{
				case _T('\n'):
				{
					corg.x = 0.0f;
					corg.y += m_Size + m_ExtraLineSpacing;
					break;
				}

				case _T('\t'):
				{
					corg.x += tabwidth;
					break;
				}

				default:
				{
					auto it = m_GlyphInfo.find(*text);

					if (it != m_GlyphInfo.end())
					{
						v->pos.x = corg.x + topshift;
						v->pos.y = corg.y;
						v->pos.z = 0.0f;
						v->tex0.x = it->second.uv.ul.x;
						v->tex0.y = it->second.uv.ul.y;

						v++;
						v->pos.x = corg.x;
						v->pos.y = corg.y + it->second.h;
						v->pos.z = 0.0f;
						v->tex0.x = it->second.uv.ul.x;
						v->tex0.y = it->second.uv.lr.y;

						v++;
						v->pos.x = corg.x + it->second.w + topshift;
						v->pos.y = corg.y;
						v->pos.z = 0.0f;
						v->tex0.x = it->second.uv.lr.x;
						v->tex0.y = it->second.uv.ul.y;

						v++;
						v->pos.x = corg.x;
						v->pos.y = corg.y + it->second.h;
						v->pos.z = 0.0f;
						v->tex0.x = it->second.uv.ul.x;
						v->tex0.y = it->second.uv.lr.y;

						v++;
						v->pos.x = corg.x + it->second.w;
						v->pos.y = corg.y + it->second.h;
						v->pos.z = 0.0f;
						v->tex0.x = it->second.uv.lr.x;
						v->tex0.y = it->second.uv.lr.y;

						v++;
						v->pos.x = corg.x + it->second.w + topshift;
						v->pos.y = corg.y;
						v->pos.z = 0.0f;
						v->tex0.x = it->second.uv.lr.x;
						v->tex0.y = it->second.uv.ul.y;

						corg.x += it->second.w + m_Kerning;

						length++;
					}

					text++;
				}

				// Unlock and render the vertex buffer
				pverts->Unlock();
			}
		}
	}

	return length;
}


const Material *FontImpl::GetMaterial() const
{
	return m_Mtl;
}