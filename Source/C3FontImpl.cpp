// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"

#include <C3FontImpl.h>
#include <C3CommonVertexDefs.h>

using namespace c3;



void Font::ParseFontDescIntoLOGFONT(const TCHAR *fontdesc, LOGFONT &lf)
{
	memset(&lf, 0, sizeof(LOGFONT));

	if (!fontdesc)
		return;

	TCHAR tmp[8];
	const TCHAR *s = fontdesc;
	TCHAR *d = lf.lfFaceName;

	while (*s && (*s != _T('(')))
		*(d++) = *(s++);

	s++;
	d = tmp;

	while (*s && (*s != _T(')')))
		*(d++) = *(s++);
	*d = _T('\0');

	lf.lfHeight = _ttoi(tmp);
}


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
	{
		m_Tex = m_pRend->CreateTexture2D(w, h, Renderer::TextureType::U8_1CH, 1);
	}

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
		m_Mtl->SetTexture(Material::TextureComponentType::TCT_DIFFUSE, m_Tex, 0);
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


void FontImpl::GetTextExtent(const TCHAR *text, math::FRect2D &extent, float tabwidth) const
{
	extent.left = extent.top = extent.right = extent.bottom = 0;

	if (!m_Initialized)
		return;

	tabwidth *= m_SpaceWidth;

	if (!text)
		return;

	const TCHAR *c = text;

	float x = 0.0f;
	float y = 0.0f;
	float lh = 0.0f;

	while (*c)
	{
		TGlyphInfoMap::const_iterator it = m_GlyphInfo.find(*c);

		switch (*c)
		{
			case _T('\n'):
				x = 0.0f;
				y -= (lh + m_ExtraLineSpacing);

				lh = 0.0f;
				break;

			case _T('\t'):
				x += tabwidth;
				break;

			default:
				if (it != m_GlyphInfo.end())
				{
					x += it->second.w + m_Kerning;
					extent.right = std::max<float>(extent.right, x);
					lh = std::max<float>(lh, (float)it->second.h);
				}
				break;
		}

		c++;
	}

	extent.bottom = y - lh;
}


size_t FontImpl::RenderText(const TCHAR *text, VertexBuffer *pverts, props::TFlags32 draw_flags, math::FRect2D *extents, float tabwidth) const
{
	if (!m_Initialized)
		return 0;

	if (!text || !*text)
		return 0;

	size_t length = 0;
	for (const TCHAR *c = text; *c != _T('\0'); c++)
		if (!_istspace(*c))
			length++;

	float sx = 0;
	float sy = 0;
	float rx = 0;

	float topshift = draw_flags.IsSet(FONTFLAG_ITALIC) ? 8.0f : 0.0f;

	tabwidth *= m_SpaceWidth;

	Vertex::PNYT1::s *v;

	glm::fvec2 corg(0, 0);

	math::FRect2D ext;
	GetTextExtent(text, ext, tabwidth);

	float hofs = 0.0f;
	float vofs = 0.0f;

	if (draw_flags.IsSet(FONTFLAG_HCENTER))
	{
		hofs = ext.Width() / 2.0f;
	}
	else if (draw_flags.IsSet(FONTFLAG_HRIGHT))
	{
		hofs = ext.Width();
	}

	if (draw_flags.IsSet(FONTFLAG_VCENTER))
	{
		vofs = ext.Height() / 2.0f;
	}
	else if (draw_flags.IsSet(FONTFLAG_VBOTTOM))
	{
		vofs = ext.Height();
	}

	corg.x -= hofs;
	corg.y += vofs;

	if (extents)
	{
		extents->left = ext.left - hofs;
		extents->right = ext.right - hofs;
		extents->top = ext.top + vofs;
		extents->bottom = ext.bottom + vofs;
	}

	size_t d = 0;

	glm::fvec3 vn(0, 0, 1);
	glm::fvec3 vt(0, 1, 0);
	glm::fvec3 vb(1, 0, 0);

	if (pverts->Lock((void **)&v, length * 6, Vertex::PNYT1::d, VBLOCKFLAG_WRITE | VBLOCKFLAG_DYNAMIC) == VertexBuffer::RETURNCODE::RET_OK)
	{
		while (*text)
		{
			switch (*text)
			{
				case _T('\n'):
				{
					corg.x = 0.0f;
					corg.y -= m_Size + m_ExtraLineSpacing;
					break;
				}

				case _T('\t'):
				{
					corg.x += tabwidth;
					break;
				}

				case _T(' '):
				{
					corg.x += m_SpaceWidth;
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
							memcpy(&(v->norm), &vn, sizeof(float) * 3);
							memcpy(&(v->tang), &vt, sizeof(float) * 3);
							memcpy(&(v->binorm), &vb, sizeof(float) * 3);
							v->uv.x = it->second.uv.ul.x;
							v->uv.y = it->second.uv.ul.y;
							v++;

							v->pos.x = corg.x + it->second.w + topshift;
							v->pos.y = corg.y;
							v->pos.z = 0.0f;
							memcpy(&(v->norm), &vn, sizeof(float) * 3);
							memcpy(&(v->tang), &vt, sizeof(float) * 3);
							memcpy(&(v->binorm), &vb, sizeof(float) * 3);
							v->uv.x = it->second.uv.lr.x;
							v->uv.y = it->second.uv.ul.y;
							v++;

							v->pos.x = corg.x;
							v->pos.y = corg.y - it->second.h;
							v->pos.z = 0.0f;
							memcpy(&(v->norm), &vn, sizeof(float) * 3);
							memcpy(&(v->tang), &vt, sizeof(float) * 3);
							memcpy(&(v->binorm), &vb, sizeof(float) * 3);
							v->uv.x = it->second.uv.ul.x;
							v->uv.y = it->second.uv.lr.y;
							v++;

							v->pos.x = corg.x;
							v->pos.y = corg.y - it->second.h;
							v->pos.z = 0.0f;
							memcpy(&(v->norm), &vn, sizeof(float) * 3);
							memcpy(&(v->tang), &vt, sizeof(float) * 3);
							memcpy(&(v->binorm), &vb, sizeof(float) * 3);
							v->uv.x = it->second.uv.ul.x;
							v->uv.y = it->second.uv.lr.y;
							v++;

							v->pos.x = corg.x + it->second.w + topshift;
							v->pos.y = corg.y;
							v->pos.z = 0.0f;
							memcpy(&(v->norm), &vn, sizeof(float) * 3);
							memcpy(&(v->tang), &vt, sizeof(float) * 3);
							memcpy(&(v->binorm), &vb, sizeof(float) * 3);
							v->uv.x = it->second.uv.lr.x;
							v->uv.y = it->second.uv.ul.y;
							v++;

							v->pos.x = corg.x + it->second.w;
							v->pos.y = corg.y - it->second.h;
							v->pos.z = 0.0f;
							memcpy(&(v->norm), &vn, sizeof(float) * 3);
							memcpy(&(v->tang), &vt, sizeof(float) * 3);
							memcpy(&(v->binorm), &vb, sizeof(float) * 3);
							v->uv.x = it->second.uv.lr.x;
							v->uv.y = it->second.uv.lr.y;
							v++;

							corg.x += it->second.w + m_Kerning;
						}
					}
					break;
			}

			text++;
		}

		// Unlock and render the vertex buffer
		pverts->Unlock();
	}

	return length;
}


const Material *FontImpl::GetMaterial() const
{
	return m_Mtl;
}