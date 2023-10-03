// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3GuiImpl.h>
#include <imgui.h>


using namespace c3;


GuiImpl::GuiImpl(Renderer *prend)
{
	m_pRend = prend;

	m_ImGui = nullptr;
	m_FontTex = nullptr;
	m_Prog = nullptr;
	m_VS = nullptr;
	m_FS = nullptr;
	m_VB = nullptr;
	m_IB = nullptr;

	if (!m_pRend->Initialized())
	{
		m_pRend->GetSystem()->GetLog()->Print(_T("Gui Error: You must Initialize your Renderer before calling GetGui\n"));
		return;
	}

	m_ImGui = ImGui::CreateContext();

	// Setup back-end capabilities flags
	ImGuiIO &io = ImGui::GetIO();
	io.BackendRendererName = "imgui_impl_celerity";

	c3::ResourceManager *rm = prend->GetSystem()->GetResourceManager();

	c3::Resource *pvsres = rm->GetResource(_T("ui.vsh"), RESF_DEMANDLOAD);
	if (!pvsres)
	{
		prend->GetSystem()->GetLog()->Print(_T("GUI Missing VSH\n"));
	}

	c3::Resource *pfsres = rm->GetResource(_T("ui.fsh"), RESF_DEMANDLOAD);
	if (!pfsres)
	{
		prend->GetSystem()->GetLog()->Print(_T("GUI Missing FSH\n"));
	}

	if (pfsres && pvsres)
	{
		m_VS = (c3::ShaderComponent *)(pvsres->GetData());
		m_FS = (c3::ShaderComponent *)(pfsres->GetData());

		m_Prog = m_pRend->CreateShaderProgram();
		m_Prog->AttachShader(m_VS);
		m_Prog->AttachShader(m_FS);
		m_Prog->Link();
	}

	// Create buffers
	m_VB = m_pRend->CreateVertexBuffer();
	m_IB = m_pRend->CreateIndexBuffer();

	// Build texture atlas
	unsigned char *pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

	m_FontTex = m_pRend->CreateTexture2D(width, height, c3::Renderer::TextureType::U8_4CH, 1);
	m_FontTex->SetName(_T("uSamplerDiffuse"));
	c3::Texture2D::SLockInfo li;
	void *ubuf;
	if (m_FontTex->Lock(&ubuf, li, 0) == c3::Texture::RETURNCODE::RET_OK)
	{
		memcpy(ubuf, pixels, sizeof(uint32_t) * width * height);
		m_FontTex->Unlock();
	}

	// Store our identifier
	io.Fonts->TexID = (void *)m_FontTex;
}


GuiImpl::~GuiImpl()
{
	if (m_FontTex)
	{
		ImGuiIO &io = ImGui::GetIO();
		io.Fonts->TexID = 0;
		m_FontTex->Release();
	}

	if (m_VB)
	{
		m_VB->Release();
		m_VB = nullptr;
	}

	if (m_IB)
	{
		m_IB->Release();
		m_IB = nullptr;
	}

	if (m_Prog)
	{
		m_Prog->Release();
		m_Prog = nullptr;
	}

	ImGui::DestroyContext(m_ImGui);
}


void GuiImpl::SetupRenderState(ImDrawData *draw_data, int fb_width, int fb_height)
{
	if (!m_pRend->Initialized())
		return;

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill

	m_pRend->SetDepthMode(c3::Renderer::DepthMode::DM_DISABLED);
	m_pRend->SetCullMode(c3::Renderer::CullMode::CM_DISABLED);
	m_pRend->SetBlendMode(Renderer::BlendMode::BM_ALPHA);

#if 0
	// Setup viewport, orthographic projection matrix
	// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
	g_imgui_sys->glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
#endif
	float L = draw_data->DisplayPos.x;
	float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
	float T = draw_data->DisplayPos.y;
	float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

	glm::fmat4x4 ortho_projection =
	{
		{ 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
		{ 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
		{ 0.0f,         0.0f,        -1.0f,   0.0f },
		{ (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
	};

	m_pRend->UseProgram(m_Prog);
	m_pRend->SetProjectionMatrix(&ortho_projection);
	m_Prog->SetUniformTexture(m_FontTex);

	// Bind vertex/index buffers and setup attributes for ImDrawVert
	m_pRend->UseVertexBuffer(m_VB);
	m_pRend->UseIndexBuffer(m_IB);
}


void GuiImpl::Render()
{
	if (!m_pRend->Initialized())
		return;

	ImGui::Render();

	ImDrawData *draw_data = ImGui::GetDrawData();

	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	if ((fb_width <= 0) || (fb_height <= 0))
		return;

	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	static const c3::VertexBuffer::ComponentDescription comps[4] ={
		{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 2, c3::VertexBuffer::ComponentDescription::Usage::VU_POSITION},
		{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 2, c3::VertexBuffer::ComponentDescription::Usage::VU_TEXCOORD0},
		{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_U8, 4, c3::VertexBuffer::ComponentDescription::Usage::VU_COLOR0},

		{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_NONE, 0, c3::VertexBuffer::ComponentDescription::Usage::VU_NONE} // terminator
	};

	// Render command lists
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList *cmd_list = draw_data->CmdLists[n];

		if (!cmd_list->VtxBuffer.Size || !cmd_list->IdxBuffer.Size)
			continue;

		// Upload vertex/index buffers
		if (c3::VertexBuffer::RETURNCODE::RET_OK == m_VB->Lock((void **)&(cmd_list->VtxBuffer.Data), cmd_list->VtxBuffer.Size, comps, VBLOCKFLAG_DYNAMIC | VBLOCKFLAG_UPDATENOW | VBLOCKFLAG_USERBUFFER | VBLOCKFLAG_WRITE))
			m_VB->Unlock();

		if (c3::IndexBuffer::RETURNCODE::RET_OK == m_IB->Lock((void **)&(cmd_list->IdxBuffer.Data), cmd_list->IdxBuffer.Size, c3::IndexBuffer::IS_16BIT, IBLOCKFLAG_DYNAMIC | IBLOCKFLAG_UPDATENOW | IBLOCKFLAG_USERBUFFER | VBLOCKFLAG_WRITE))
			m_IB->Unlock();

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			SetupRenderState(draw_data, fb_width, fb_height);

			const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback != NULL)
			{
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if (pcmd->UserCallback != ImDrawCallback_ResetRenderState)
					pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				// Project scissor/clipping rectangles into framebuffer space
				ImVec4 clip_rect;
				clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
				clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
				clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
				clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

				if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
				{
#if 0
					// Apply scissor/clipping rectangle
					if (clip_origin_lower_left)
						glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));
					else
						glScissor((int)clip_rect.x, (int)clip_rect.y, (int)clip_rect.z, (int)clip_rect.w); // Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)
#endif

					m_pRend->DrawIndexedPrimitives(c3::Renderer::PrimType::TRILIST, pcmd->IdxOffset, cmd_list->IdxBuffer.Size);
				}
			}
		}
	}
}


void GuiImpl::BeginFrame()
{
	ImGuiIO &io = ImGui::GetIO();
	io.DeltaTime = m_pRend->GetSystem()->GetElapsedTime();

	ImGui::NewFrame();
}


void GuiImpl::EndFrame()
{
	ImGui::EndFrame();
}


void GuiImpl::SetDisplaySize(float w, float h)
{
	ImGuiIO &io = ImGui::GetIO();
	io.DisplaySize.x = w;
	io.DisplaySize.y = h;
}


void GuiImpl::AddMouseButtonEvent(MouseButton but, bool down)
{
	ImGuiIO &io = ImGui::GetIO();
	io.AddMouseButtonEvent(but, down);
}


bool GuiImpl::Begin(const TCHAR *name, bool *p_openptr, props::TFlags64 flags)
{
	char *_name;
	CONVERT_TCS2MBCS(name, _name);
	return ImGui::Begin(_name, p_openptr, (ImGuiWindowFlags)(uint64_t)flags);
}


void GuiImpl::End()
{
	ImGui::End();
}


bool GuiImpl::BeginChild(const TCHAR *str_id, const glm::fvec2 &size, bool border, props::TFlags64 window_flags)
{
	char *_str_id;
	CONVERT_TCS2MBCS(str_id, _str_id);
	return ImGui::BeginChild(_str_id, size, border, (ImGuiWindowFlags)(uint64_t)window_flags);
}


bool GuiImpl::BeginChild(uint64_t id, const glm::fvec2 &size, bool border, props::TFlags64 window_flags)
{
	return ImGui::BeginChild((ImGuiID)id, size, border, (ImGuiWindowFlags)(uint64_t)window_flags);
}


void GuiImpl::EndChild()
{
	ImGui::EndChild();
}


bool GuiImpl::IsWindowAppearing()
{
	return ImGui::IsWindowAppearing();
}


bool GuiImpl::IsWindowCollapsed()
{
	return ImGui::IsWindowCollapsed();
}


bool GuiImpl::IsWindowFocused(props::TFlags64 focus_flags)
{
	return ImGui::IsWindowFocused((ImGuiFocusedFlags)(uint64_t)focus_flags);
}


bool GuiImpl::IsWindowHovered(props::TFlags64 hover_flags)
{
	return ImGui::IsWindowHovered((ImGuiFocusedFlags)(uint64_t)hover_flags);
}


glm::fvec2 GuiImpl::GetWindowPos()
{
	return ImGui::GetWindowPos();
}


glm::fvec2 GuiImpl::GetWindowSize()
{
	return ImGui::GetWindowSize();
}


float GuiImpl::GetWindowWidth()
{
	return ImGui::GetWindowWidth();
}


float GuiImpl::GetWindowHeight()
{
	return ImGui::GetWindowHeight();
}


void GuiImpl::SetNextWindowPos(const glm::fvec2 &pos, int64_t cond, const glm::fvec2 &pivot)
{
	ImGui::SetNextWindowPos(pos, (ImGuiCond)cond, pivot);
}


void GuiImpl::SetNextWindowSize(const glm::fvec2 &size, int64_t cond)
{
	ImGui::SetNextWindowSize(size, (ImGuiCond)cond);
}


void GuiImpl::SetNextWindowSizeConstraints(const glm::fvec2 &size_min, const glm::fvec2 &size_max)
{
	ImGui::SetNextWindowSizeConstraints(size_min, size_max);
}


void GuiImpl::SetNextWindowContentSize(const glm::fvec2 &size)
{
	ImGui::SetNextWindowContentSize(size);
}


void GuiImpl::SetNextWindowCollapsed(bool collapsed, int64_t cond)
{
	ImGui::SetNextWindowCollapsed(collapsed, (ImGuiCond)cond);
}


void GuiImpl::SetNextWindowFocus()
{
	ImGui::SetNextWindowFocus();
}


void GuiImpl::SetNextWindowBgAlpha(float alpha)
{
	ImGui::SetNextWindowBgAlpha(alpha);
}


void GuiImpl::SetWindowPos(const glm::fvec2 &pos, int64_t cond)
{
	ImGui::SetWindowPos(pos, (ImGuiCond)cond);
}


void GuiImpl::SetWindowSize(const glm::fvec2 &size, int64_t cond)
{
	ImGui::SetWindowSize(size, (ImGuiCond)cond);
}


void GuiImpl::SetWindowCollapsed(bool collapsed, int64_t cond)
{
	ImGui::SetWindowCollapsed(collapsed, (ImGuiCond)cond);
}


void GuiImpl::SetWindowFocus()
{
	ImGui::SetWindowFocus();
}


void GuiImpl::SetWindowFontScale(float scale)
{
	ImGui::SetWindowFontScale(scale);
}


void GuiImpl::SetWindowPos(const TCHAR *name, const glm::fvec2 &pos, int64_t cond)
{
	char *_name;
	CONVERT_TCS2MBCS(name, _name);
	ImGui::SetWindowPos(_name, pos, (ImGuiCond)cond);
}


void GuiImpl::SetWindowSize(const TCHAR *name, const glm::fvec2 &size, int64_t cond)
{
	char *_name;
	CONVERT_TCS2MBCS(name, _name);
	ImGui::SetWindowSize(_name, size, (ImGuiCond)cond);
}


void GuiImpl::SetWindowCollapsed(const TCHAR *name, bool collapsed, int64_t cond)
{
	char *_name;
	CONVERT_TCS2MBCS(name, _name);
	ImGui::SetWindowCollapsed(_name, collapsed, (ImGuiCond)cond);
}


void GuiImpl::SetWindowFocus(const TCHAR *name)
{
	char *_name;
	CONVERT_TCS2MBCS(name, _name);
	ImGui::SetWindowFocus(_name);
}


glm::fvec2 GuiImpl::GetContentRegionMax()
{
	return ImGui::GetContentRegionMax();
}


glm::fvec2 GuiImpl::GetContentRegionAvail()
{
	return ImGui::GetContentRegionAvail();
}


glm::fvec2 GuiImpl::GetWindowContentRegionMin()
{
	return ImGui::GetWindowContentRegionMin();
}


glm::fvec2 GuiImpl::GetWindowContentRegionMax()
{
	return ImGui::GetWindowContentRegionMax();
}


float GuiImpl::GetWindowContentRegionWidth()
{
	//return ImGui::GetWindowContentRegionWidth();
	return 0.0f;
}


float GuiImpl::GetScrollX()
{
	return ImGui::GetScrollX();
}


float GuiImpl::GetScrollY()
{
	return ImGui::GetScrollY();
}


float GuiImpl::GetScrollMaxX()
{
	return ImGui::GetScrollMaxX();
}


float GuiImpl::GetScrollMaxY()
{
	return ImGui::GetScrollMaxY();
}


void GuiImpl::SetScrollX(float scroll_x)
{
	ImGui::SetScrollX(scroll_x);
}


void GuiImpl::SetScrollY(float scroll_y)
{
	ImGui::SetScrollY(scroll_y);
}


void GuiImpl::SetScrollHereX(float center_x_ratio)
{
	ImGui::SetScrollHereX(center_x_ratio);
}


void GuiImpl::SetScrollHereY(float center_y_ratio)
{
	ImGui::SetScrollHereY(center_y_ratio);
}


void GuiImpl::SetScrollFromPosX(float local_x, float center_x_ratio)
{
	ImGui::SetScrollFromPosX(local_x, center_x_ratio);
}


void GuiImpl::SetScrollFromPosY(float local_y, float center_y_ratio)
{
	ImGui::SetScrollFromPosY(local_y, center_y_ratio);
}


void GuiImpl::PushFont(ImFont *font)
{
	ImGui::PushFont(font);
}


void GuiImpl::PopFont()
{
	ImGui::PopFont();
}


void GuiImpl::SetColorScheme(ColorScheme scheme)
{
}


Gui::ColorScheme GuiImpl::GetColorScheme()
{
	return ColorScheme::GS_NORMAL;
}


void GuiImpl::PushStyleColor(ColorType idx, uint32_t col)
{
	ImGui::PushStyleColor(idx, col);
}


void GuiImpl::PushStyleColor(ColorType idx, const glm::fvec4 &col)
{
	ImGui::PushStyleColor(idx, col);
}


void GuiImpl::PopStyleColor(int count)
{
	ImGui::PopStyleColor(count);
}


void GuiImpl::PushStyleVar(StyleVar idx, float val)
{
	ImGui::PushStyleVar(idx, val);
}


void GuiImpl::PushStyleVar(StyleVar idx, const glm::fvec2 &val)
{
	ImGui::PushStyleVar(idx, val);
}


void GuiImpl::PopStyleVar(int count)
{
	ImGui::PopStyleVar(count);
}


const glm::fvec4 GuiImpl::GetStyleColorVec4(ColorType idx)
{
	return ImGui::GetStyleColorVec4(idx);
}


ImFont *GuiImpl::GetFont()
{
	return ImGui::GetFont();
}


float GuiImpl::GetFontSize()
{
	return ImGui::GetFontSize();
}


glm::fvec2 GuiImpl::GetFontTexUvWhitePixel()
{
	return ImGui::GetFontTexUvWhitePixel();
}


uint32_t GuiImpl::GetColorU32(ColorType idx, float alpha_mul)
{
	return ImGui::GetColorU32(idx, alpha_mul);
}


uint32_t GuiImpl::GetColorU32(const glm::fvec4 &col)
{
	return ImGui::GetColorU32(col);
}


uint32_t GuiImpl::GetColorU32(uint32_t col)
{
	return ImGui::GetColorU32(col);
}


void GuiImpl::PushItemWidth(float item_width)
{
	ImGui::PushItemWidth(item_width);
}


void GuiImpl::PopItemWidth()
{
	ImGui::PopItemWidth();
}


void GuiImpl::SetNextItemWidth(float item_width)
{
	ImGui::SetNextItemWidth(item_width);
}


float GuiImpl::CalcItemWidth()
{
	return ImGui::CalcItemWidth();
}


void GuiImpl::PushTextWrapPos(float wrap_local_pos_x)
{
	ImGui::PushTextWrapPos(wrap_local_pos_x);
}


void GuiImpl::PopTextWrapPos()
{
	ImGui::PopTextWrapPos();
}


void GuiImpl::PushAllowKeyboardFocus(bool allow_keyboard_focus)
{
	ImGui::PushAllowKeyboardFocus(allow_keyboard_focus);
}


void GuiImpl::PopAllowKeyboardFocus()
{
	ImGui::PopAllowKeyboardFocus();
}


void GuiImpl::PushButtonRepeat(bool repeat)
{
	ImGui::PushButtonRepeat(repeat);
}


void GuiImpl::PopButtonRepeat()
{
	ImGui::PopButtonRepeat();
}


void GuiImpl::Separator()
{
	ImGui::Separator();
}


void GuiImpl::SameLine(float offset_from_start_x, float spacing)
{
	ImGui::SameLine(offset_from_start_x, spacing);
}


void GuiImpl::NewLine()
{
	ImGui::NewLine();
}


void GuiImpl::Spacing()
{
	ImGui::Spacing();
}


void GuiImpl::Dummy(const glm::fvec2 &size)
{
	ImGui::Dummy(size);
}


void GuiImpl::Indent(float indent_w)
{
	ImGui::Indent(indent_w);
}


void GuiImpl::Unindent(float indent_w)
{
	ImGui::Unindent(indent_w);
}


void GuiImpl::BeginGroup()
{
	ImGui::BeginGroup();
}


void GuiImpl::EndGroup()
{
	ImGui::EndGroup();
}


glm::fvec2 GuiImpl::GetCursorPos()
{
	return ImGui::GetCursorPos();
}


float GuiImpl::GetCursorPosX()
{
	return ImGui::GetCursorPosX();
}


float GuiImpl::GetCursorPosY()
{
	return ImGui::GetCursorPosY();
}


void GuiImpl::SetCursorPos(const glm::fvec2 &local_pos)
{
	ImGui::SetCursorPos(local_pos);
}


void GuiImpl::SetCursorPosX(float local_x)
{
	ImGui::SetCursorPosX(local_x);
}


void GuiImpl::SetCursorPosY(float local_y)
{
	ImGui::SetCursorPosY(local_y);
}


glm::fvec2 GuiImpl::GetCursorStartPos()
{
	return ImGui::GetCursorStartPos();
}


glm::fvec2 GuiImpl::GetCursorScreenPos()
{
	return ImGui::GetCursorScreenPos();
}


void GuiImpl::SetCursorScreenPos(const glm::fvec2 &pos)
{
	ImGui::SetCursorScreenPos(pos);
}


void GuiImpl::AlignTextToFramePadding()
{
	ImGui::AlignTextToFramePadding();
}


float GuiImpl::GetTextLineHeight()
{
	return ImGui::GetTextLineHeight();
}


float GuiImpl::GetTextLineHeightWithSpacing()
{
	return ImGui::GetTextLineHeightWithSpacing();
}


float GuiImpl::GetFrameHeight()
{
	return ImGui::GetFrameHeight();
}


float GuiImpl::GetFrameHeightWithSpacing()
{
	return ImGui::GetFrameHeightWithSpacing();
}


void GuiImpl::PushID(const TCHAR *str_id)
{
	char *_str_id;
	CONVERT_TCS2MBCS(str_id, _str_id);
	ImGui::PushID(_str_id);
}


void GuiImpl::PushID(const TCHAR *str_id_begin, const TCHAR *str_id_end)
{
	char *_str_id_begin, *_str_id_end;
	CONVERT_TCS2MBCS(str_id_begin, _str_id_begin);
	CONVERT_TCS2MBCS(str_id_end, _str_id_end);
	ImGui::PushID(_str_id_begin, _str_id_end);
}


void GuiImpl::PushID(const void *ptr_id)
{
	ImGui::PushID(ptr_id);
}


void GuiImpl::PushID(int int_id)
{
	ImGui::PushID(int_id);
}


void GuiImpl::PopID()
{
	ImGui::PopID();
}


uint64_t GuiImpl::GetID(const TCHAR *str_id)
{
	char *_str_id;
	CONVERT_TCS2MBCS(str_id, _str_id);
	return ImGui::GetID(_str_id);
}


uint64_t GuiImpl::GetID(const TCHAR *str_id_begin, const TCHAR *str_id_end)
{
	char *_str_id_begin, *_str_id_end;
	CONVERT_TCS2MBCS(str_id_begin, _str_id_begin);
	CONVERT_TCS2MBCS(str_id_end, _str_id_end);
	return ImGui::GetID(_str_id_begin, _str_id_end);
}


uint64_t GuiImpl::GetID(const void *ptr_id)
{
	return ImGui::GetID(ptr_id);
}


void GuiImpl::TextUnformatted(const TCHAR *text, const TCHAR *text_endptr)
{
	char *_text, *_text_endptr;
	CONVERT_TCS2MBCS(text, _text);
	_text_endptr = _text + (text_endptr - text);
	ImGui::TextUnformatted(_text, _text_endptr);
}


void GuiImpl::Text(const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	TextV(fmt, args);

	va_end(args);
}


void GuiImpl::TextV(const TCHAR *fmt, va_list args)
{
	char *_fmt;
	CONVERT_TCS2MBCS(fmt, _fmt);
	ImGui::TextV(_fmt, args);
}


void GuiImpl::TextColored(const glm::fvec4 &col, const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	TextColoredV(col, fmt, args);

	va_end(args);
}


void GuiImpl::TextColoredV(const glm::fvec4 &col, const TCHAR *fmt, va_list args)
{
	char *_fmt;
	CONVERT_TCS2MBCS(fmt, _fmt);
	ImGui::TextColoredV(col, _fmt, args);
}


void GuiImpl::TextDisabled(const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	TextDisabledV(fmt, args);

	va_end(args);
}


void GuiImpl::TextDisabledV(const TCHAR *fmt, va_list args)
{
	char *_fmt;
	CONVERT_TCS2MBCS(fmt, _fmt);
	ImGui::TextDisabledV(_fmt, args);
}


void GuiImpl::TextWrapped(const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	TextWrappedV(fmt, args);

	va_end(args);
}


void GuiImpl::TextWrappedV(const TCHAR *fmt, va_list args)
{
	char *_fmt;
	CONVERT_TCS2MBCS(fmt, _fmt);
	ImGui::TextWrappedV(_fmt, args);
}


void GuiImpl::LabelText(const TCHAR *label, const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	LabelTextV(label, fmt, args);

	va_end(args);
}


void GuiImpl::LabelTextV(const TCHAR *label, const TCHAR *fmt, va_list args)
{
	char *_label, *_fmt;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(fmt, _fmt);
	ImGui::LabelTextV(_label, _fmt, args);
}


void GuiImpl::BulletText(const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	BulletTextV(fmt, args);

	va_end(args);
}


void GuiImpl::BulletTextV(const TCHAR *fmt, va_list args)
{
	char *_fmt;
	CONVERT_TCS2MBCS(fmt, _fmt);
	ImGui::BulletTextV(_fmt, args);
}


bool GuiImpl::Button(const TCHAR *label, const glm::fvec2 &size)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::Button(_label, size);
}


bool GuiImpl::SmallButton(const TCHAR *label)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::SmallButton(_label);
}


bool GuiImpl::InvisibleButton(const TCHAR *str_id, const glm::fvec2 &size)
{
	char *_str_id;
	CONVERT_TCS2MBCS(str_id, _str_id);
	return ImGui::InvisibleButton(_str_id, size);
}


bool GuiImpl::ArrowButton(const TCHAR *str_id, Direction dir)
{
	char *_str_id;
	CONVERT_TCS2MBCS(str_id, _str_id);
	return ImGui::ArrowButton(_str_id, dir);

}


void GuiImpl::Image(Texture2D *user_texture, const glm::fvec2 &size, const glm::fvec2 &uv0, const glm::fvec2 &uv1, const glm::fvec4 &tint_col, const glm::fvec4 &border_col)
{
	ImGui::Image(user_texture, size, uv0, uv1, tint_col, border_col);
}


bool GuiImpl::ImageButton(Texture2D *user_texture, const glm::fvec2 &size, const glm::fvec2 &uv0, const glm::fvec2 &uv1, int frame_padding, const glm::fvec4 &bg_col, const glm::fvec4 &tint_col)
{
	return ImGui::ImageButton(user_texture, size, uv0, uv1, frame_padding, bg_col, tint_col);
}


bool GuiImpl::Checkbox(const TCHAR *label, bool *v)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::Checkbox(_label, v);
}


bool GuiImpl::CheckboxFlags(const TCHAR *label, unsigned int *flags, unsigned int flags_value)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::CheckboxFlags(_label, flags, flags_value);
}


bool GuiImpl::RadioButton(const TCHAR *label, bool active)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::RadioButton(_label, active);
}


bool GuiImpl::RadioButton(const TCHAR *label, int *v, int v_button)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::RadioButton(_label, v, v_button);
}


void GuiImpl::ProgressBar(float fraction, const glm::fvec2 &size_arg, const TCHAR *overlayptr)
{
	char *_overlayptr;
	CONVERT_TCS2MBCS(overlayptr, _overlayptr);
	ImGui::ProgressBar(fraction, size_arg, _overlayptr);
}


void GuiImpl::Bullet()
{
	ImGui::Bullet();
}


bool GuiImpl::BeginCombo(const TCHAR *label, const TCHAR *preview_value, props::TFlags64 combo_flags)
{
	char *_label, *_preview_value;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(preview_value, _preview_value);
	return ImGui::BeginCombo(_label, _preview_value, (ImGuiComboFlags)(uint64_t)combo_flags);
}


void GuiImpl::EndCombo()
{
	ImGui::EndCombo();
}


bool GuiImpl::Combo(const TCHAR *label, int *current_item, const TCHAR *const items[], int items_count, int popup_max_height_in_items)
{
	// TODO
#if 0
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::Combo(_label, current_item, items, items_count, popup_max_height_in_items);
#else
	return false;
#endif
}


bool GuiImpl::Combo(const TCHAR *label, int *current_item, const TCHAR *items_separated_by_zeros, int popup_max_height_in_items)
{
	char *_label, *_items;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(items_separated_by_zeros, _items);
	return ImGui::Combo(_label, current_item, _items, popup_max_height_in_items);
}


bool GuiImpl::Combo(const TCHAR *label, int *current_item, bool(*items_getter)(void *data, int idx, const char **out_text), void *data, int items_count, int popup_max_height_in_items)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::Combo(_label, current_item, items_getter, data, items_count, popup_max_height_in_items);
}


#if 0
bool GuiImpl::DragFloat(const TCHAR *label, float *v, float v_speed, float v_min, float v_max, const TCHAR *format, float power)
{
	// TODO
}


bool GuiImpl::DragFloat2(const TCHAR *label, float v[2], float v_speed, float v_min, float v_max, const TCHAR *format, float power)
{
	// TODO
}


bool GuiImpl::DragFloat3(const TCHAR *label, float v[3], float v_speed, float v_min, float v_max, const TCHAR *format, float power)
{
	// TODO
}


bool GuiImpl::DragFloat4(const TCHAR *label, float v[4], float v_speed, float v_min, float v_max, const TCHAR *format, float power)
{
	// TODO
}


bool GuiImpl::DragFloatRange2(const TCHAR *label, float *v_current_min, float *v_current_max, float v_speed, float v_min, float v_max, const TCHAR *format, const TCHAR *format_maxptr, float power)
{
	// TODO
}


bool GuiImpl::DragInt(const TCHAR *label, int *v, float v_speed, int v_min, int v_max, const TCHAR *format)
{
	// TODO
}


bool GuiImpl::DragInt2(const TCHAR *label, int v[2], float v_speed, int v_min, int v_max, const TCHAR *format)
{
	// TODO
}


bool GuiImpl::DragInt3(const TCHAR *label, int v[3], float v_speed, int v_min, int v_max, const TCHAR *format)
{
	// TODO
}


bool GuiImpl::DragInt4(const TCHAR *label, int v[4], float v_speed, int v_min, int v_max, const TCHAR *format)
{
	// TODO
}


bool GuiImpl::DragIntRange2(const TCHAR *label, int *v_current_min, int *v_current_max, float v_speed, int v_min, int v_max, const TCHAR *format, const TCHAR *format_maxptr)
{
	// TODO
}


bool GuiImpl::DragScalar(const TCHAR *label, DataType data_type, void *p_data, float v_speed, const void *p_minptr, const void *p_maxptr, const TCHAR *formatptr, float power)
{
	// TODO
}


bool GuiImpl::DragScalarN(const TCHAR *label, DataType data_type, void *p_data, int components, float v_speed, const void *p_minptr, const void *p_maxptr, const TCHAR *formatptr, float power)
{
	// TODO
}
#endif


bool GuiImpl::SliderFloat(const TCHAR *label, float *v, float v_min, float v_max, const TCHAR *format, float power)
{
	char *_label, *_format;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(format, _format);
	//return ImGui::SliderFloat(_label, v, v_min, v_max, _format, power);
	return false;
}


bool GuiImpl::SliderFloat2(const TCHAR *label, float v[2], float v_min, float v_max, const TCHAR *format, float power)
{
	char *_label, *_format;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(format, _format);
	//return ImGui::SliderFloat2(_label, v, v_min, v_max, _format, power);
	return false;
}


bool GuiImpl::SliderFloat3(const TCHAR *label, float v[3], float v_min, float v_max, const TCHAR *format, float power)
{
	char *_label, *_format;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(format, _format);
	//return ImGui::SliderFloat(_label, v, v_min, v_max, _format, power);
	return false;
}


bool GuiImpl::SliderFloat4(const TCHAR *label, float v[4], float v_min, float v_max, const TCHAR *format, float power)
{
	char *_label, *_format;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(format, _format);
	//return ImGui::SliderFloat(_label, v, v_min, v_max, _format, power);
	return false;
}


bool GuiImpl::SliderAngle(const TCHAR *label, float *v_rad, float v_degrees_min, float v_degrees_max, const TCHAR *format)
{
	char *_label, *_format;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(format, _format);
	return ImGui::SliderAngle(_label, v_rad, v_degrees_min, v_degrees_max, _format);
}


bool GuiImpl::SliderInt(const TCHAR *label, int *v, int v_min, int v_max, const TCHAR *format)
{
	char *_label, *_format;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(format, _format);
	return ImGui::SliderInt(_label, v, v_min, v_max, _format);
}


bool GuiImpl::SliderInt2(const TCHAR *label, int v[2], int v_min, int v_max, const TCHAR *format)
{
	char *_label, *_format;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(format, _format);
	return ImGui::SliderInt(_label, v, v_min, v_max, _format);
}


bool GuiImpl::SliderInt3(const TCHAR *label, int v[3], int v_min, int v_max, const TCHAR *format)
{
	char *_label, *_format;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(format, _format);
	return ImGui::SliderInt(_label, v, v_min, v_max, _format);
}


bool GuiImpl::SliderInt4(const TCHAR *label, int v[4], int v_min, int v_max, const TCHAR *format)
{
	char *_label, *_format;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(format, _format);
	return ImGui::SliderInt(_label, v, v_min, v_max, _format);
}


bool GuiImpl::SliderScalar(const TCHAR *label, DataType data_type, void *p_data, const void *p_min, const void *p_max, const TCHAR *formatptr, float power)
{
	char *_label, *_formatptr;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(formatptr, _formatptr);
	//return ImGui::SliderScalar(_label, data_type, p_data, p_min, p_max, _formatptr, power);
	return false;
}


bool GuiImpl::SliderScalarN(const TCHAR *label, DataType data_type, void *p_data, int components, const void *p_min, const void *p_max, const TCHAR *formatptr, float power)
{
	char *_label, *_formatptr;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(formatptr, _formatptr);
	//return ImGui::SliderScalar(_label, data_type, p_data, p_min, p_max, _formatptr, power);
	return false;
}


bool GuiImpl::VSliderFloat(const TCHAR *label, const glm::fvec2 &size, float *v, float v_min, float v_max, const TCHAR *format, props::TFlags64 flags)
{
	char *_label, *_format;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(format, _format);
	//return ImGui::VSliderFloat(_label, size, v, v_min, v_max, _format, power);
	return false;
}


bool GuiImpl::VSliderInt(const TCHAR *label, const glm::fvec2 &size, int *v, int v_min, int v_max, const TCHAR *format)
{
	char *_label, *_format;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(format, _format);
	return ImGui::VSliderInt(_label, size, v, v_min, v_max, _format);
}


bool GuiImpl::VSliderScalar(const TCHAR *label, const glm::fvec2 &size, DataType data_type, void *p_data, const void *p_min, const void *p_max, const TCHAR *formatptr, props::TFlags64 flags)
{
	char *_label, *_formatptr;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(formatptr, _formatptr);
	//return ImGui::VSliderScalar(_label, size, data_type, p_data, p_min, p_max, _formatptr, power);
	return false;
}


#if 0
bool GuiImpl::InputText(const TCHAR *label, TCHAR *buf, size_t buf_size, props::TFlags64 inputtext_flags = 0, ImGuiInputTextCallback callbackptr, void *user_dataptr)
{
	// TODO
}


bool GuiImpl::InputTextMultiline(const TCHAR *label, TCHAR *buf, size_t buf_size, const glm::fvec2 &size, props::TFlags64 inputtext_flags, ImGuiInputTextCallback callbackptr, void *user_dataptr)
{
	// TODO
}


bool GuiImpl::InputTextWithHint(const TCHAR *label, const TCHAR *hint, TCHAR *buf, size_t buf_size, props::TFlags64 inputtext_flags, ImGuiInputTextCallback callbackptr, void *user_dataptr)
{
	// TODO
}


bool GuiImpl::InputFloat(const TCHAR *label, float *v, float step, float step_fast, const TCHAR *format, props::TFlags64 inputtext_flags)
{
	// TODO
}


bool GuiImpl::InputFloat2(const TCHAR *label, float v[2], const TCHAR *format, props::TFlags64 inputtext_flags)
{
	// TODO
}


bool GuiImpl::InputFloat3(const TCHAR *label, float v[3], const TCHAR *format, props::TFlags64 inputtext_flags)
{
	// TODO
}


bool GuiImpl::InputFloat4(const TCHAR *label, float v[4], const TCHAR *format, props::TFlags64 inputtext_flags)
{
	// TODO
}


bool GuiImpl::InputInt(const TCHAR *label, int *v, int step, int step_fast, props::TFlags64 inputtext_flags)
{
	// TODO
}


bool GuiImpl::InputInt2(const TCHAR *label, int v[2], props::TFlags64 inputtext_flags)
{
	// TODO
}


bool GuiImpl::InputInt3(const TCHAR *label, int v[3], props::TFlags64 inputtext_flags)
{
	// TODO
}


bool GuiImpl::InputInt4(const TCHAR *label, int v[4], props::TFlags64 inputtext_flags)
{
	// TODO
}


bool GuiImpl::InputDouble(const TCHAR *label, double *v, double step, double step_fast, const TCHAR *format, props::TFlags64 inputtext_flags)
{
	// TODO
}


bool GuiImpl::InputScalar(const TCHAR *label, DataType data_type, void *p_data, const void *p_stepptr, const void *p_step_fastptr, const TCHAR *formatptr, props::TFlags64 inputtext_flags)
{
	// TODO
}


bool GuiImpl::InputScalarN(const TCHAR *label, DataType data_type, void *p_data, int components, const void *p_stepptr, const void *p_step_fastptr, const TCHAR *formatptr, props::TFlags64 inputtext_flags)
{
	// TODO
}
#endif

bool GuiImpl::ColorEdit3(const TCHAR *label, float col[3], props::TFlags64 coloredit_flags)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::ColorEdit3(_label, col, (ImGuiColorEditFlags)(uint64_t)coloredit_flags);
}


bool GuiImpl::ColorEdit4(const TCHAR *label, float col[4], props::TFlags64 coloredit_flags)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::ColorEdit4(_label, col, (ImGuiColorEditFlags)(uint64_t)coloredit_flags);
}


bool GuiImpl::ColorPicker3(const TCHAR *label, float col[3], props::TFlags64 coloredit_flags)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::ColorPicker3(_label, col, (ImGuiColorEditFlags)(uint64_t)coloredit_flags);
}


bool GuiImpl::ColorPicker4(const TCHAR *label, float col[4], props::TFlags64 coloredit_flags, const float *ref_colptr)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::ColorPicker4(_label, col, (ImGuiColorEditFlags)(uint64_t)coloredit_flags);
}


bool GuiImpl::ColorButton(const TCHAR *desc_id, const glm::fvec4 &col, props::TFlags64 coloredit_flags, glm::fvec2 size)
{
	char *_desc_id;
	CONVERT_TCS2MBCS(desc_id, _desc_id);
	return ImGui::ColorButton(_desc_id, col, (ImGuiColorEditFlags)(uint64_t)coloredit_flags, size);
}


void GuiImpl::SetColorEditOptions(props::TFlags64 coloredit_flags)
{
	ImGui::SetColorEditOptions((ImGuiColorEditFlags)(uint64_t)coloredit_flags);
}


bool GuiImpl::TreeNode(const TCHAR *label)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::TreeNode(_label);
}


bool GuiImpl::TreeNode(const TCHAR *str_id, const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	bool ret = TreeNodeV(str_id, fmt, args);

	va_end(args);

	return ret;
}


bool GuiImpl::TreeNode(const void *ptr_id, const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	bool ret = TreeNodeV(ptr_id, fmt, args);

	va_end(args);

	return ret;
}


bool GuiImpl::TreeNodeV(const TCHAR *str_id, const TCHAR *fmt, va_list args)
{
	char *_str_id, *_fmt;
	CONVERT_TCS2MBCS(str_id, _str_id);
	CONVERT_TCS2MBCS(fmt, _fmt);
	return ImGui::TreeNodeV(_str_id, _fmt, args);
}


bool GuiImpl::TreeNodeV(const void *ptr_id, const TCHAR *fmt, va_list args)
{
	char *_fmt;
	CONVERT_TCS2MBCS(fmt, _fmt);
	return ImGui::TreeNodeV(ptr_id, _fmt, args);
}


bool GuiImpl::TreeNodeEx(const TCHAR *label, props::TFlags64 treenode_flags)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::TreeNodeEx(_label, (ImGuiTreeNodeFlags)(uint64_t)treenode_flags);
}


bool GuiImpl::TreeNodeEx(const TCHAR *str_id, props::TFlags64 treenode_flags, const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	bool ret = TreeNodeEx(str_id, treenode_flags, fmt, args);

	va_end(args);

	return ret;
}


bool GuiImpl::TreeNodeEx(const void *ptr_id, props::TFlags64 treenode_flags, const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	bool ret = TreeNodeEx(ptr_id, treenode_flags, fmt, args);

	va_end(args);

	return ret;
}


bool GuiImpl::TreeNodeExV(const TCHAR *str_id, props::TFlags64 treenode_flags, const TCHAR *fmt, va_list args)
{
	char *_str_id, *_fmt;
	CONVERT_TCS2MBCS(str_id, _str_id);
	CONVERT_TCS2MBCS(fmt, _fmt);
	return ImGui::TreeNodeExV(_str_id, (ImGuiTreeNodeFlags)(uint64_t)treenode_flags, _fmt, args);
}


bool GuiImpl::TreeNodeExV(const void *ptr_id, props::TFlags64 treenode_flags, const TCHAR *fmt, va_list args)
{
	char *_fmt;
	CONVERT_TCS2MBCS(fmt, _fmt);
	return ImGui::TreeNodeExV(ptr_id, (ImGuiTreeNodeFlags)(uint64_t)treenode_flags, _fmt, args);
}


void GuiImpl::TreePush(const TCHAR *str_id)
{
	char *_str_id;
	CONVERT_TCS2MBCS(str_id, _str_id);
	ImGui::TreePush(_str_id);
}


void GuiImpl::TreePush(const void *ptr_idptr)
{
	ImGui::TreePush(ptr_idptr);
}


void GuiImpl::TreePop()
{
	ImGui::TreePop();
}


float GuiImpl::GetTreeNodeToLabelSpacing()
{
	return ImGui::GetTreeNodeToLabelSpacing();
}


bool GuiImpl::CollapsingHeader(const TCHAR *label, props::TFlags64 treenode_flags)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::CollapsingHeader(_label, (ImGuiTreeNodeFlags)(uint64_t)treenode_flags);
}


bool GuiImpl::CollapsingHeader(const TCHAR *label, bool *p_open, props::TFlags64 treenode_flags)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::CollapsingHeader(_label, p_open, (ImGuiTreeNodeFlags)(uint64_t)treenode_flags);
}


void GuiImpl::SetNextItemOpen(bool is_open, int64_t cond)
{
	ImGui::SetNextItemOpen(is_open, (ImGuiCond)cond);
}


bool GuiImpl::Selectable(const TCHAR *label, bool selected, props::TFlags64 selectable_flags, const glm::fvec2 &size)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::Selectable(_label, selected, (ImGuiSelectableFlags)(uint64_t)selectable_flags, size);
}


bool GuiImpl::Selectable(const TCHAR *label, bool *p_selected, props::TFlags64 selectable_flags, const glm::fvec2 &size)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::Selectable(_label, p_selected, (ImGuiSelectableFlags)(uint64_t)selectable_flags, size);
}


bool GuiImpl::ListBox(const TCHAR *label, int *current_item, const TCHAR *const items[], int items_count, int height_in_items)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);

	//return ImGui::ListBox(_label, current_item, );
	return false;
}


bool GuiImpl::ListBox(const TCHAR *label, int *current_item, bool (*items_getter)(void *data, int idx, const char **out_text), void *data, int items_count, int height_in_items)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::ListBox(_label, current_item, items_getter, data, items_count, height_in_items);
}


bool GuiImpl::ListBoxHeader(const TCHAR *label, const glm::fvec2 &size)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	//return ImGui::ListBoxHeader(_label, size);
	return false;
}


bool GuiImpl::ListBoxHeader(const TCHAR *label, int items_count, int height_in_items)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	//return ImGui::ListBoxHeader(_label, items_count, height_in_items);
	return false;

}


void GuiImpl::ListBoxFooter()
{
	//ImGui::ListBoxFooter();
}


void GuiImpl::PlotLines(const TCHAR *label, const float *values, int values_count, int values_offset, const TCHAR *overlay_textptr, float scale_min, float scale_max, glm::fvec2 graph_size, int stride)
{
	char *_label, *_overlay_textptr;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(overlay_textptr, _overlay_textptr);
	return ImGui::PlotLines(_label, values, values_count, values_offset, _overlay_textptr, scale_min, scale_max, graph_size, stride);
}


void GuiImpl::PlotLines(const TCHAR *label, float(*values_getter)(void *data, int idx), void *data, int values_count, int values_offset, const TCHAR *overlay_textptr, float scale_min, float scale_max, glm::fvec2 graph_size)
{
	char *_label, *_overlay_textptr;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(overlay_textptr, _overlay_textptr);
	return ImGui::PlotLines(_label, values_getter, data, values_count, values_offset, _overlay_textptr, scale_min, scale_max, graph_size);
}


void GuiImpl::PlotHistogram(const TCHAR *label, const float *values, int values_count, int values_offset, const TCHAR *overlay_textptr, float scale_min, float scale_max, glm::fvec2 graph_size, int stride)
{
	char *_label, *_overlay_textptr;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(overlay_textptr, _overlay_textptr);
	ImGui::PlotHistogram(_label, values, values_count, values_offset, _overlay_textptr, scale_min, scale_max, graph_size, stride);
}


void GuiImpl::PlotHistogram(const TCHAR *label, float(*values_getter)(void *data, int idx), void *data, int values_count, int values_offset, const TCHAR *overlay_textptr, float scale_min, float scale_max, glm::fvec2 graph_size)
{
	char *_label, *_overlay_textptr;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(overlay_textptr, _overlay_textptr);
	ImGui::PlotHistogram(_label, values_getter, data, values_count, values_offset, _overlay_textptr, scale_min, scale_max, graph_size);
}


void GuiImpl::Value(const TCHAR *prefix, bool b)
{
	char *_prefix;
	CONVERT_TCS2MBCS(prefix, _prefix);
	ImGui::Value(_prefix, b);
}


void GuiImpl::Value(const TCHAR *prefix, int v)
{
	char *_prefix;
	CONVERT_TCS2MBCS(prefix, _prefix);
	ImGui::Value(_prefix, v);
}


void GuiImpl::Value(const TCHAR *prefix, unsigned int v)
{
	char *_prefix;
	CONVERT_TCS2MBCS(prefix, _prefix);
	ImGui::Value(_prefix, v);
}


void GuiImpl::Value(const TCHAR *prefix, float v, const TCHAR *float_formatptr)
{
	char *_prefix, *_float_formatptr;
	CONVERT_TCS2MBCS(prefix, _prefix);
	CONVERT_TCS2MBCS(float_formatptr, _float_formatptr);
	ImGui::Value(_prefix, v, _float_formatptr);
}


bool GuiImpl::BeginMenuBar()
{
	return ImGui::BeginMenuBar();
}


void GuiImpl::EndMenuBar()
{
	ImGui::EndMenuBar();
}


bool GuiImpl::BeginMainMenuBar()
{
	return ImGui::BeginMainMenuBar();
}


void GuiImpl::EndMainMenuBar()
{
	ImGui::EndMainMenuBar();
}


bool GuiImpl::BeginMenu(const TCHAR *label, bool enabled)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::BeginMenu(_label, enabled);
}


void GuiImpl::EndMenu()
{
	ImGui::EndMenu();
}


bool GuiImpl::MenuItem(const TCHAR *label, const TCHAR *shortcutptr, bool selected, bool enabled)
{
	char *_label, *_shortcutptr;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(shortcutptr, _shortcutptr);
	return ImGui::MenuItem(_label, _shortcutptr, selected, enabled);
}


bool GuiImpl::MenuItem(const TCHAR *label, const TCHAR *shortcut, bool *p_selected, bool enabled)
{
	char *_label, *_shortcut;
	CONVERT_TCS2MBCS(label, _label);
	CONVERT_TCS2MBCS(shortcut, _shortcut);
	return ImGui::MenuItem(_label, _shortcut, p_selected, enabled);
}


void GuiImpl::BeginTooltip()
{
	ImGui::BeginTooltip();
}


void GuiImpl::EndTooltip()
{
	ImGui::EndTooltip();
}


void GuiImpl::SetTooltip(const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	SetTooltipV(fmt, args);

	va_end(args);
}


void GuiImpl::SetTooltipV(const TCHAR *fmt, va_list args)
{
	char *_fmt;
	CONVERT_TCS2MBCS(fmt, _fmt);
	ImGui::SetTooltipV(_fmt, args);
}


void GuiImpl::OpenPopup(const TCHAR *str_id)
{
	char *_str_id;
	CONVERT_TCS2MBCS(str_id, _str_id);
	ImGui::OpenPopup(_str_id);
}


bool GuiImpl::BeginPopup(const TCHAR *str_id, props::TFlags64 window_flags)
{
	char *_str_id;
	CONVERT_TCS2MBCS(str_id, _str_id);
	return ImGui::BeginPopup(_str_id, (ImGuiWindowFlags)(uint64_t)window_flags);
}


bool GuiImpl::BeginPopupContextItem(const TCHAR *str_idptr, MouseButton mouse_button)
{
	char *_str_idptr;
	CONVERT_TCS2MBCS(str_idptr, _str_idptr);
	return ImGui::BeginPopupContextItem(_str_idptr, mouse_button);
}


bool GuiImpl::BeginPopupContextWindow(const TCHAR *str_idptr, MouseButton mouse_button, bool also_over_items)
{
	char *_str_idptr;
	CONVERT_TCS2MBCS(str_idptr, _str_idptr);
	//return ImGui::BeginPopupContextWindow(_str_idptr, mouse_button, also_over_items);
	return false;
}


bool GuiImpl::BeginPopupContextVoid(const TCHAR *str_idptr, MouseButton mouse_button)
{
	char *_str_idptr;
	CONVERT_TCS2MBCS(str_idptr, _str_idptr);
	return ImGui::BeginPopupContextVoid(_str_idptr, mouse_button);
}


bool GuiImpl::BeginPopupModal(const TCHAR *name, bool *p_openptr, props::TFlags64 window_flags)
{
	char *_name;
	CONVERT_TCS2MBCS(name, _name);
	return ImGui::BeginPopupModal(_name, p_openptr, (ImGuiWindowFlags)(uint64_t)window_flags);
}


void GuiImpl::EndPopup()
{
	ImGui::EndPopup();
}


void GuiImpl::OpenPopupOnItemClick(const TCHAR *str_idptr, MouseButton mouse_button)
{
	char *_str_idptr;
	CONVERT_TCS2MBCS(str_idptr, _str_idptr);
	//return ImGui::OpenPopupOnItemClick(_str_idptr, mouse_button);
}


bool GuiImpl::IsPopupOpen(const TCHAR *str_id)
{
	char *_str_id;
	CONVERT_TCS2MBCS(str_id, _str_id);
	return ImGui::IsPopupOpen(_str_id);
}


void GuiImpl::CloseCurrentPopup()
{
	ImGui::CloseCurrentPopup();
}


void GuiImpl::Columns(int count, const TCHAR *idptr, bool border)
{
	char *_idptr;
	CONVERT_TCS2MBCS(idptr, _idptr);
	//return ImGui::Columns(count, _idptr, border);
}


void GuiImpl::NextColumn()
{
	//ImGui::NextColumn();
}


int GuiImpl::GetColumnIndex()
{
	return -1; // ImGui::GetColumnIndex();
}


float GuiImpl::GetColumnWidth(int column_index)
{
	return 0.0f; // ImGui::GetColumnWidth(column_index);
}


void GuiImpl::SetColumnWidth(int column_index, float width)
{
	//ImGui::SetColumnWidth(column_index, width);
}


float GuiImpl::GetColumnOffset(int column_index)
{
	return 0.0f; // ImGui::GetColumnOffset(column_index);
}


void GuiImpl::SetColumnOffset(int column_index, float offset_x)
{
	//ImGui::SetColumnOffset(column_index, offset_x);
}


int GuiImpl::GetColumnsCount()
{
	return 0; // ImGui::GetColumnsCount();
}


bool GuiImpl::BeginTabBar(const TCHAR *str_id, props::TFlags64 tabbar_flags)
{
	char *_str_id;
	CONVERT_TCS2MBCS(str_id, _str_id);
	return ImGui::BeginTabBar(_str_id, (ImGuiTabBarFlags)(uint64_t)tabbar_flags);
}


void GuiImpl::EndTabBar()
{
	ImGui::EndTabBar();
}


bool GuiImpl::BeginTabItem(const TCHAR *label, bool *p_openptr, props::TFlags64 tabitem_flags)
{
	char *_label;
	CONVERT_TCS2MBCS(label, _label);
	return ImGui::BeginTabItem(_label, p_openptr, (ImGuiTabItemFlags)(uint64_t)tabitem_flags);
}


void GuiImpl::EndTabItem()
{
	ImGui::EndTabItem();
}


void GuiImpl::SetTabItemClosed(const TCHAR *tab_or_docked_window_label)
{
	char *_tab_or_docked_window_label;
	CONVERT_TCS2MBCS(tab_or_docked_window_label, _tab_or_docked_window_label);
	ImGui::SetTabItemClosed(_tab_or_docked_window_label);
}


void GuiImpl::LogToTTY(int auto_open_depth)
{
	ImGui::LogToTTY(auto_open_depth);
}


void GuiImpl::LogToFile(int auto_open_depth, const TCHAR *filenameptr)
{
	char *_filenameptr;
	CONVERT_TCS2MBCS(filenameptr, _filenameptr);
	return ImGui::LogToFile(auto_open_depth, _filenameptr);
}


void GuiImpl::LogToClipboard(int auto_open_depth)
{
	ImGui::LogToClipboard(auto_open_depth);
}


void GuiImpl::LogFinish()
{
	ImGui::LogFinish();
}


void GuiImpl::LogButtons()
{
	ImGui::LogButtons();
}


void GuiImpl::LogText(const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	// TODO

	va_end(args);
}


#if 0
bool GuiImpl::BeginDragDropSource(props::TFlags64 dragdrop_flags)
{
	return ImGui::BeginDragDropSource(dragdrop_flags);
}


bool GuiImpl::SetDragDropPayload(const TCHAR *type, const void *data, size_t sz, int64_t cond)
{
	char *_type;
	CONVERT_TCS2MBCS(type, _type);
	return ImGui::SetDragDropPayload(_type, data, sz, cond);
}


void GuiImpl::EndDragDropSource()
{
	ImGui::EndDragDropSource();
}


bool GuiImpl::BeginDragDropTarget()
{
	return ImGui::BeginDragDropTarget();
}


const ImGuiPayload *GuiImpl::AcceptDragDropPayload(const TCHAR *type, props::TFlags64 dragdrop_flags)
{
	char *_type;
	CONVERT_TCS2MBCS(type, _type);
	return ImGui::AcceptDragDropPayload(_type, dragdrop_flags);
}


void GuiImpl::EndDragDropTarget()
{
	ImGui::EndDragDropTarget();
}


const ImGuiPayload *GuiImpl::GetDragDropPayload()
{
	return ImGui::GetDragDropPayload();
}
#endif


void GuiImpl::PushClipRect(const glm::fvec2 &clip_rect_min, const glm::fvec2 &clip_rect_max, bool intersect_with_current_clip_rect)
{
	ImGui::PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
}


void GuiImpl::PopClipRect()
{
	ImGui::PopClipRect();
}


void GuiImpl::SetItemDefaultFocus()
{
	ImGui::SetItemDefaultFocus();
}


void GuiImpl::SetKeyboardFocusHere(int offset)
{
	ImGui::SetKeyboardFocusHere(offset);
}


bool GuiImpl::IsItemHovered(props::TFlags64 hover_flags)
{
	return IsItemHovered(hover_flags);
}


bool GuiImpl::IsItemActive()
{
	return ImGui::IsItemActive();
}


bool GuiImpl::IsItemFocused()
{
	return ImGui::IsItemFocused();
}


bool GuiImpl::IsItemClicked(MouseButton mouse_button)
{
	return ImGui::IsItemClicked(mouse_button);
}


bool GuiImpl::IsItemVisible()
{
	return ImGui::IsItemVisible();
}


bool GuiImpl::IsItemEdited()
{
	return ImGui::IsItemEdited();
}


bool GuiImpl::IsItemActivated()
{
	return ImGui::IsItemActivated();
}


bool GuiImpl::IsItemDeactivated()
{
	return ImGui::IsItemDeactivated();
}


bool GuiImpl::IsItemDeactivatedAfterEdit()
{
	return ImGui::IsItemDeactivatedAfterEdit();
}


bool GuiImpl::IsItemToggledOpen()
{
	return ImGui::IsItemToggledOpen();
}


bool GuiImpl::IsAnyItemHovered()
{
	return ImGui::IsAnyItemHovered();
}


bool GuiImpl::IsAnyItemActive()
{
	return ImGui::IsAnyItemActive();
}


bool GuiImpl::IsAnyItemFocused()
{
	return ImGui::IsAnyItemFocused();
}


glm::fvec2 GuiImpl::GetItemRectMin()
{
	return ImGui::GetItemRectMin();
}


glm::fvec2 GuiImpl::GetItemRectMax()
{
	return ImGui::GetItemRectMax();
}


glm::fvec2 GuiImpl::GetItemRectSize()
{
	return ImGui::GetItemRectSize();
}


void GuiImpl::SetItemAllowOverlap()
{
	ImGui::SetItemAllowOverlap();
}


bool GuiImpl::IsRectVisible(const glm::fvec2 &size)
{
	return ImGui::IsRectVisible(size);
}


bool GuiImpl::IsRectVisible(const glm::fvec2 &rect_min, const glm::fvec2 &rect_max)
{
	return ImGui::IsRectVisible(rect_min, rect_max);
}


double GuiImpl::GetTime()
{
	return ImGui::GetTime();
}


int GuiImpl::GetFrameCount()
{
	return ImGui::GetFrameCount();
}


const TCHAR *GuiImpl::GetStyleColorName(ColorType idx)
{
	const char *ret = ImGui::GetStyleColorName(idx);
	TCHAR *_ret;
	CONVERT_MBCS2TCS(ret, _ret);

	return nullptr;
}


#if 0
void GuiImpl::SetStateStorage(ImGuiStorage *storage)
{
	return ImGui::SetStateStorage(storage);
}


ImGuiStorage *GuiImpl::GetStateStorage()
{
	return ImGui::GetStateStorage();
}
#endif


glm::fvec2 GuiImpl::CalcTextSize(const TCHAR *text, const TCHAR *text_endptr, bool hide_text_after_double_hash, float wrap_width)
{
	char *_text;
	CONVERT_TCS2MBCS(text, _text);
	const char *_text_endptr = _text + (text_endptr - text);
	return ImGui::CalcTextSize(_text, _text_endptr, hide_text_after_double_hash, wrap_width);
}


void GuiImpl::CalcListClipping(int items_count, float items_height, int *out_items_display_start, int *out_items_display_end)
{
	//ImGui::CalcListClipping(items_count, items_height, out_items_display_start, out_items_display_end);
}


bool GuiImpl::BeginChildFrame(uint64_t id, const glm::fvec2 &size, props::TFlags64 window_flags)
{
	return ImGui::BeginChildFrame((ImGuiID)id, size, (ImGuiWindowFlags)(uint64_t)window_flags);
}


void GuiImpl::EndChildFrame()
{
	ImGui::EndChildFrame();
}


glm::fvec4 GuiImpl::ColorConvertU32ToFloat4(uint32_t in)
{
	return ImGui::ColorConvertU32ToFloat4(in);
}


uint32_t GuiImpl::ColorConvertFloat4ToU32(const glm::fvec4 &in)
{
	return ImGui::ColorConvertFloat4ToU32(in);
}


void GuiImpl::ColorConvertRGBtoHSV(float r, float g, float b, float &out_h, float &out_s, float &out_v)
{
	ImGui::ColorConvertRGBtoHSV(r, g, b, out_h, out_s, out_v);
}


void GuiImpl::ColorConvertHSVtoRGB(float h, float s, float v, float &out_r, float &out_g, float &out_b)
{
	ImGui::ColorConvertHSVtoRGB(h, s, v, out_r, out_g, out_b);
}


#if 0
int GuiImpl::GetKeyIndex(ImGuiKey imgui_key)
{
	return ImGui::GetKeyIndex(imgui_key);
}
#endif

bool GuiImpl::IsKeyDown(int user_key_index)
{
	return ImGui::IsKeyDown((ImGuiKey)user_key_index);
}


bool GuiImpl::IsKeyPressed(int user_key_index, bool repeat)
{
	return ImGui::IsKeyPressed((ImGuiKey)user_key_index, repeat);
}


bool GuiImpl::IsKeyReleased(int user_key_index)
{
	return ImGui::IsKeyReleased((ImGuiKey)user_key_index);
}


int GuiImpl::GetKeyPressedAmount(int key_index, float repeat_delay, float rate)
{
	return ImGui::GetKeyPressedAmount((ImGuiKey)key_index, repeat_delay, rate);
}


void GuiImpl::CaptureKeyboardFromApp(bool want_capture_keyboard_value)
{
	ImGui::CaptureKeyboardFromApp(want_capture_keyboard_value);
}


bool GuiImpl::IsMouseDown(MouseButton button)
{
	return ImGui::IsMouseDown(button);
}


bool GuiImpl::IsMouseClicked(MouseButton button, bool repeat)
{
	return ImGui::IsMouseClicked(button, repeat);
}


bool GuiImpl::IsMouseReleased(MouseButton button)
{
	return ImGui::IsMouseReleased(button);
}


bool GuiImpl::IsMouseDoubleClicked(MouseButton button)
{
	return ImGui::IsMouseDoubleClicked(button);
}


bool GuiImpl::IsMouseHoveringRect(const glm::fvec2 &r_min, const glm::fvec2 &r_max, bool clip)
{
	return ImGui::IsMouseHoveringRect(r_min, r_max, clip);
}


bool GuiImpl::IsMousePosValid(const glm::fvec2 *mouse_posptr)
{
	return IsMousePosValid(mouse_posptr);
}


bool GuiImpl::IsAnyMouseDown()
{
	return ImGui::IsAnyMouseDown();
}


glm::fvec2 GuiImpl::GetMousePos()
{
	return ImGui::GetMousePos();
}


glm::fvec2 GuiImpl::GetMousePosOnOpeningCurrentPopup()
{
	return ImGui::GetMousePosOnOpeningCurrentPopup();
}


bool GuiImpl::IsMouseDragging(MouseButton button, float lock_threshold)
{
	return ImGui::IsMouseDragging(button, lock_threshold);
}


glm::fvec2 GuiImpl::GetMouseDragDelta(MouseButton button, float lock_threshold)
{
	return ImGui::GetMouseDragDelta(button, lock_threshold);
}


void GuiImpl::ResetMouseDragDelta(MouseButton button)
{
	ImGui::ResetMouseDragDelta(button);
}


Gui::MouseCursor GuiImpl::GetMouseCursor()
{
	return (Gui::MouseCursor)ImGui::GetMouseCursor();
}


void GuiImpl::SetMouseCursor(Gui::MouseCursor cursor_type)
{
	ImGui::SetMouseCursor(cursor_type);
}


void GuiImpl::CaptureMouseFromApp(bool want_capture_mouse_value)
{
	ImGui::CaptureMouseFromApp(want_capture_mouse_value);
}


const TCHAR *GuiImpl::GetClipboardText()
{
	const char *text = ImGui::GetClipboardText();
	TCHAR *_text;
	CONVERT_MBCS2TCS(text, _text);
	m_ClipboardText = _text;
	return m_ClipboardText.c_str();
}


void GuiImpl::SetClipboardText(const TCHAR *text)
{
	m_ClipboardText = text;
	char *_text;
	CONVERT_TCS2MBCS(text, _text);
	ImGui::SetClipboardText(_text);
}
