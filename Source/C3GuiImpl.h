// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3Gui.h>

#include <imgui.h>

namespace c3
{

	class GuiImpl : public Gui
	{

	protected:
		Renderer *m_pRend;
		ImGuiContext *m_ImGui;
		Texture2D *m_FontTex;// g_imgui_texptr;
		ShaderProgram *m_Prog;// g_imgui_spptr;
		ShaderComponent *m_VS;// g_imgui_vsptr;
		ShaderComponent *m_FS;// g_imgui_fsptr;
		VertexBuffer *m_VB;// g_imgui_vbptr;
		IndexBuffer *m_IB;// g_imgui_ibptr;

		tstring m_ClipboardText;

		void SetupRenderState(ImDrawData *draw_data, int fb_width, int fb_height);

	public:

		GuiImpl(Renderer *prend);

		virtual ~GuiImpl();

		virtual void BeginFrame();

		virtual void Render();

		virtual void EndFrame();

		virtual void SetDisplaySize(float w, float h);

		// USER CALLS BELOW

		virtual void AddMouseButtonEvent(MouseButton but, bool down);

		virtual void AddMouseWheelEvent(float wheelx, float wheely);

		virtual bool Begin(const TCHAR *name, bool *p_openptr, props::TFlags64 flags = 0);

		virtual void End();

		virtual bool BeginChild(const TCHAR *str_id, const glm::fvec2 &size = glm::fvec2(0, 0), bool border = false, props::TFlags64 window_flags = 0);

		virtual bool BeginChild(uint64_t id, const glm::fvec2 &size = glm::fvec2(0, 0), bool border = false, props::TFlags64 window_flags = 0);

		virtual void EndChild();

		virtual bool IsWindowAppearing();

		virtual bool IsWindowCollapsed();

		virtual bool IsWindowFocused(props::TFlags64 focus_flags = 0);

		virtual bool IsWindowHovered(props::TFlags64 hover_flags = 0);

		virtual glm::fvec2 GetWindowPos();

		virtual glm::fvec2 GetWindowSize();

		virtual float GetWindowWidth();

		virtual float GetWindowHeight();

		virtual void SetNextWindowPos(const glm::fvec2 &pos, int64_t cond = 0, const glm::fvec2 &pivot = glm::fvec2(0, 0));

		virtual void SetNextWindowSize(const glm::fvec2 &size, int64_t cond = 0);

		virtual void SetNextWindowSizeConstraints(const glm::fvec2 &size_min, const glm::fvec2 &size_max);

		virtual void SetNextWindowContentSize(const glm::fvec2 &size);

		virtual void SetNextWindowCollapsed(bool collapsed, int64_t cond = 0);

		virtual void SetNextWindowFocus();

		virtual void SetNextWindowBgAlpha(float alpha);

		virtual void SetWindowPos(const glm::fvec2 &pos, int64_t cond = 0);

		virtual void SetWindowSize(const glm::fvec2 &size, int64_t cond = 0);

		virtual void SetWindowCollapsed(bool collapsed, int64_t cond = 0);

		virtual void SetWindowFocus();

		virtual void SetWindowFontScale(float scale);

		virtual void SetWindowPos(const TCHAR *name, const glm::fvec2 &pos, int64_t cond = 0);

		virtual void SetWindowSize(const TCHAR *name, const glm::fvec2 &size, int64_t cond = 0);

		virtual void SetWindowCollapsed(const TCHAR *name, bool collapsed, int64_t cond = 0);

		virtual void SetWindowFocus(const TCHAR *name);

		virtual glm::fvec2 GetContentRegionMax();

		virtual glm::fvec2 GetContentRegionAvail();

		virtual glm::fvec2 GetWindowContentRegionMin();

		virtual glm::fvec2 GetWindowContentRegionMax();

		virtual float GetWindowContentRegionWidth();

		virtual float GetScrollX();

		virtual float GetScrollY();

		virtual float GetScrollMaxX();

		virtual float GetScrollMaxY();

		virtual void SetScrollX(float scroll_x);

		virtual void SetScrollY(float scroll_y);

		virtual void SetScrollHereX(float center_x_ratio = 0.5f);

		virtual void SetScrollHereY(float center_y_ratio = 0.5f);

		virtual void SetScrollFromPosX(float local_x, float center_x_ratio = 0.5f);

		virtual void SetScrollFromPosY(float local_y, float center_y_ratio = 0.5f);

#if 1
		virtual void PushFont(ImFont *font);

		virtual void PopFont();
#endif

		virtual void SetColorScheme(ColorScheme scheme);

		virtual ColorScheme GetColorScheme();

		virtual void PushStyleColor(ColorType idx, uint32_t col);

		virtual void PushStyleColor(ColorType idx, const glm::fvec4 &col);

		virtual void PopStyleColor(int count = 1);

		virtual void PushStyleVar(StyleVar idx, float val);

		virtual void PushStyleVar(StyleVar idx, const glm::fvec2 &val);

		virtual void PopStyleVar(int count = 1);

		virtual const glm::fvec4 GetStyleColorVec4(ColorType idx);

#if 1
		virtual ImFont *GetFont();

		virtual float GetFontSize();

		virtual glm::fvec2 GetFontTexUvWhitePixel();
#endif

		virtual uint32_t GetColorU32(ColorType idx, float alpha_mul = 1.0f);

		virtual uint32_t GetColorU32(const glm::fvec4 &col);

		virtual uint32_t GetColorU32(uint32_t col);

		virtual void PushItemWidth(float item_width);

		virtual void PopItemWidth();

		virtual void SetNextItemWidth(float item_width);

		virtual float CalcItemWidth();

		virtual void PushTextWrapPos(float wrap_local_pos_x = 0.0f);

		virtual void PopTextWrapPos();

		virtual void PushAllowKeyboardFocus(bool allow_keyboard_focus);

		virtual void PopAllowKeyboardFocus();

		virtual void PushButtonRepeat(bool repeat);

		virtual void PopButtonRepeat();

		virtual void Separator();

		virtual void SameLine(float offset_from_start_x=0.0f, float spacing=-1.0f);

		virtual void NewLine();

		virtual void Spacing();

		virtual void Dummy(const glm::fvec2 &size);

		virtual void Indent(float indent_w = 0.0f);

		virtual void Unindent(float indent_w = 0.0f);

		virtual void BeginGroup();

		virtual void EndGroup();

		virtual glm::fvec2 GetCursorPos();

		virtual float GetCursorPosX();

		virtual float GetCursorPosY();

		virtual void SetCursorPos(const glm::fvec2 &local_pos);

		virtual void SetCursorPosX(float local_x);

		virtual void SetCursorPosY(float local_y);

		virtual glm::fvec2 GetCursorStartPos();

		virtual glm::fvec2 GetCursorScreenPos();

		virtual void SetCursorScreenPos(const glm::fvec2 &pos);

		virtual void AlignTextToFramePadding();

		virtual float GetTextLineHeight();

		virtual float GetTextLineHeightWithSpacing();

		virtual float GetFrameHeight();

		virtual float GetFrameHeightWithSpacing();

		virtual void PushID(const TCHAR *str_id);

		virtual void PushID(const TCHAR *str_id_begin, const TCHAR *str_id_end);

		virtual void PushID(const void *ptr_id);

		virtual void PushID(int int_id);

		virtual void PopID();

		virtual uint64_t GetID(const TCHAR *str_id);

		virtual uint64_t GetID(const TCHAR *str_id_begin, const TCHAR *str_id_end);

		virtual uint64_t GetID(const void *ptr_id);

		virtual void TextUnformatted(const TCHAR *text, const TCHAR *text_endptr);

		virtual void Text(const TCHAR *fmt, ...);

		virtual void TextV(const TCHAR *fmt, va_list args);

		virtual void TextColored(const glm::fvec4 &col, const TCHAR *fmt, ...);

		virtual void TextColoredV(const glm::fvec4 &col, const TCHAR *fmt, va_list args);

		virtual void TextDisabled(const TCHAR *fmt, ...);

		virtual void TextDisabledV(const TCHAR *fmt, va_list args);

		virtual void TextWrapped(const TCHAR *fmt, ...);

		virtual void TextWrappedV(const TCHAR *fmt, va_list args);

		virtual void LabelText(const TCHAR *label, const TCHAR *fmt, ...);

		virtual void LabelTextV(const TCHAR *label, const TCHAR *fmt, va_list args);

		virtual void BulletText(const TCHAR *fmt, ...);

		virtual void BulletTextV(const TCHAR *fmt, va_list args);

		virtual bool Button(const TCHAR *label, const glm::fvec2 &size = glm::fvec2(0, 0));

		virtual bool SmallButton(const TCHAR *label);

		virtual bool InvisibleButton(const TCHAR *str_id, const glm::fvec2 &size);

		virtual bool ArrowButton(const TCHAR *str_id, Direction dir);

		virtual void Image(Texture2D *user_texture, const glm::fvec2 &size, const glm::fvec2 &uv0, const glm::fvec2 &uv1, const glm::fvec4 &tint_col, const glm::fvec4 &border_col);

		virtual bool ImageButton(Texture2D *user_texture, const glm::fvec2 &size, const glm::fvec2 &uv0, const glm::fvec2 &uv1, int frame_padding, const glm::fvec4 &bg_col, const glm::fvec4 &tint_col);

		virtual bool Checkbox(const TCHAR *label, bool *v);

		virtual bool CheckboxFlags(const TCHAR *label, unsigned int *flags, unsigned int flags_value);

		virtual bool RadioButton(const TCHAR *label, bool active);

		virtual bool RadioButton(const TCHAR *label, int *v, int v_button);

		virtual void ProgressBar(float fraction, const glm::fvec2 &size_arg, const TCHAR *overlayptr);

		virtual void Bullet();

		virtual bool BeginCombo(const TCHAR *label, const TCHAR *preview_value, props::TFlags64 combo_flags);

		virtual void EndCombo();

		virtual bool Combo(const TCHAR *label, int *current_item, const TCHAR *const items[], int items_count, int popup_max_height_in_items);

		virtual bool Combo(const TCHAR *label, int *current_item, const TCHAR *items_separated_by_zeros, int popup_max_height_in_items);

		virtual bool Combo(const TCHAR *label, int *current_item, bool(*items_getter)(void *data, int idx, const char **out_text), void *data, int items_count, int popup_max_height_in_items);

#if 0
		virtual bool DragFloat(const TCHAR *label, float *v, float v_speed, float v_min, float v_max, const TCHAR *format, float power);

		virtual bool DragFloat2(const TCHAR *label, float v[2], float v_speed, float v_min, float v_max, const TCHAR *format, float power);

		virtual bool DragFloat3(const TCHAR *label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const TCHAR *format = _T("%.3f"), float power = 1.0f);

		virtual bool DragFloat4(const TCHAR *label, float v[4], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const TCHAR *format = _T("%.3f"), float power = 1.0f);

		virtual bool DragFloatRange2(const TCHAR *label, float *v_current_min, float *v_current_max, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const TCHAR *format = _T("%.3f"), const TCHAR *format_maxptr, float power = 1.0f);

		virtual bool DragInt(const TCHAR *label, int *v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const TCHAR *format = _T("%d"));

		virtual bool DragInt2(const TCHAR *label, int v[2], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const TCHAR *format = _T("%d"));

		virtual bool DragInt3(const TCHAR *label, int v[3], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const TCHAR *format = _T("%d"));

		virtual bool DragInt4(const TCHAR *label, int v[4], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const TCHAR *format = _T("%d"));

		virtual bool DragIntRange2(const TCHAR *label, int *v_current_min, int *v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const TCHAR *format = _T("%d"), const TCHAR *format_maxptr);

		virtual bool DragScalar(const TCHAR *label, DataType data_type, void *p_data, float v_speed, const void *p_minptr, const void *p_maxptr, const TCHAR *formatptr, float power = 1.0f);

		virtual bool DragScalarN(const TCHAR *label, DataType data_type, void *p_data, int components, float v_speed, const void *p_minptr, const void *p_maxptr, const TCHAR *formatptr, float power = 1.0f);
#endif

		virtual bool SliderFloat(const TCHAR *label, float *v, float v_min, float v_max, const TCHAR *format, float power);

		virtual bool SliderFloat2(const TCHAR *label, float v[2], float v_min, float v_max, const TCHAR *format, float power);

		virtual bool SliderFloat3(const TCHAR *label, float v[3], float v_min, float v_max, const TCHAR *format, float power);

		virtual bool SliderFloat4(const TCHAR *label, float v[4], float v_min, float v_max, const TCHAR *format, float power);

		virtual bool SliderAngle(const TCHAR *label, float *v_rad, float v_degrees_min, float v_degrees_max, const TCHAR *format);

		virtual bool SliderInt(const TCHAR *label, int *v, int v_min, int v_max, const TCHAR *format);

		virtual bool SliderInt2(const TCHAR *label, int v[2], int v_min, int v_max, const TCHAR *format);

		virtual bool SliderInt3(const TCHAR *label, int v[3], int v_min, int v_max, const TCHAR *format);

		virtual bool SliderInt4(const TCHAR *label, int v[4], int v_min, int v_max, const TCHAR *format);

		virtual bool SliderScalar(const TCHAR *label, DataType data_type, void *p_data, const void *p_min, const void *p_max, const TCHAR *formatptr, float power);

		virtual bool SliderScalarN(const TCHAR *label, DataType data_type, void *p_data, int components, const void *p_min, const void *p_max, const TCHAR *formatptr, float power);

		virtual bool VSliderFloat(const TCHAR *label, const glm::fvec2 &size, float *v, float v_min, float v_max, const TCHAR *format, props::TFlags64 flags);

		virtual bool VSliderInt(const TCHAR *label, const glm::fvec2 &size, int *v, int v_min, int v_max, const TCHAR *format);

		virtual bool VSliderScalar(const TCHAR *label, const glm::fvec2 &size, DataType data_type, void *p_data, const void *p_min, const void *p_max, const TCHAR *formatptr, props::TFlags64 flags);

#if 0
		virtual bool InputText(const TCHAR *label, TCHAR *buf, size_t buf_size, props::TFlags64 inputtext_flags = 0, ImGuiInputTextCallback callbackptr, void *user_dataptr);

		virtual bool InputTextMultiline(const TCHAR *label, TCHAR *buf, size_t buf_size, const glm::fvec2 &size = glm::fvec2(0, 0), props::TFlags64 inputtext_flags = 0, ImGuiInputTextCallback callbackptr, void *user_dataptr);

		virtual bool InputTextWithHint(const TCHAR *label, const TCHAR *hint, TCHAR *buf, size_t buf_size, props::TFlags64 inputtext_flags = 0, ImGuiInputTextCallback callbackptr, void *user_dataptr);

		virtual bool InputFloat(const TCHAR *label, float *v, float step = 0.0f, float step_fast = 0.0f, const TCHAR *format = _T("%.3f"), props::TFlags64 inputtext_flags = 0);

		virtual bool InputFloat2(const TCHAR *label, float v[2], const TCHAR *format = _T("%.3f"), props::TFlags64 inputtext_flags = 0);

		virtual bool InputFloat3(const TCHAR *label, float v[3], const TCHAR *format = _T("%.3f"), props::TFlags64 inputtext_flags = 0);

		virtual bool InputFloat4(const TCHAR *label, float v[4], const TCHAR *format = _T("%.3f"), props::TFlags64 inputtext_flags = 0);

		virtual bool InputInt(const TCHAR *label, int *v, int step = 1, int step_fast = 100, props::TFlags64 inputtext_flags = 0);

		virtual bool InputInt2(const TCHAR *label, int v[2], props::TFlags64 inputtext_flags = 0);

		virtual bool InputInt3(const TCHAR *label, int v[3], props::TFlags64 inputtext_flags = 0);

		virtual bool InputInt4(const TCHAR *label, int v[4], props::TFlags64 inputtext_flags = 0);

		virtual bool InputDouble(const TCHAR *label, double *v, double step = 0.0, double step_fast = 0.0, const TCHAR *format = _T("%.6f"), props::TFlags64 inputtext_flags = 0);

		virtual bool InputScalar(const TCHAR *label, DataType data_type, void *p_data, const void *p_stepptr, const void *p_step_fastptr, const TCHAR *formatptr, props::TFlags64 inputtext_flags = 0);

		virtual bool InputScalarN(const TCHAR *label, DataType data_type, void *p_data, int components, const void *p_stepptr, const void *p_step_fastptr, const TCHAR *formatptr, props::TFlags64 inputtext_flags = 0);
#endif

		virtual bool ColorEdit3(const TCHAR *label, float col[3], props::TFlags64 coloredit_flags);

		virtual bool ColorEdit4(const TCHAR *label, float col[4], props::TFlags64 coloredit_flags);

		virtual bool ColorPicker3(const TCHAR *label, float col[3], props::TFlags64 coloredit_flags);

		virtual bool ColorPicker4(const TCHAR *label, float col[4], props::TFlags64 coloredit_flags, const float *ref_colptr);

		virtual bool ColorButton(const TCHAR *desc_id, const glm::fvec4 &col, props::TFlags64 coloredit_flags, glm::fvec2 size);

		virtual void SetColorEditOptions(props::TFlags64 coloredit_flags);

		virtual bool TreeNode(const TCHAR *label);

		virtual bool TreeNode(const TCHAR *str_id, const TCHAR *fmt, ...);

		virtual bool TreeNode(const void *ptr_id, const TCHAR *fmt, ...);

		virtual bool TreeNodeV(const TCHAR *str_id, const TCHAR *fmt, va_list args);

		virtual bool TreeNodeV(const void *ptr_id, const TCHAR *fmt, va_list args);

		virtual bool TreeNodeEx(const TCHAR *label, props::TFlags64 treenode_flags);

		virtual bool TreeNodeEx(const TCHAR *str_id, props::TFlags64 treenode_flags, const TCHAR *fmt, ...);

		virtual bool TreeNodeEx(const void *ptr_id, props::TFlags64 treenode_flags, const TCHAR *fmt, ...);

		virtual bool TreeNodeExV(const TCHAR *str_id, props::TFlags64 treenode_flags, const TCHAR *fmt, va_list args);

		virtual bool TreeNodeExV(const void *ptr_id, props::TFlags64 treenode_flags, const TCHAR *fmt, va_list args);

		virtual void TreePush(const TCHAR *str_id);

		virtual void TreePush(const void *ptr_idptr);

		virtual void TreePop();

		virtual float GetTreeNodeToLabelSpacing();

		virtual bool CollapsingHeader(const TCHAR *label, props::TFlags64 treenode_flags);

		virtual bool CollapsingHeader(const TCHAR *label, bool *p_open, props::TFlags64 treenode_flags);

		virtual void SetNextItemOpen(bool is_open, int64_t cond);

		virtual bool Selectable(const TCHAR *label, bool selected, props::TFlags64 selectable_flags, const glm::fvec2 &size);

		virtual bool Selectable(const TCHAR *label, bool *p_selected, props::TFlags64 selectable_flags, const glm::fvec2 &size);

		virtual bool ListBox(const TCHAR *label, int *current_item, const TCHAR *const items[], int items_count, int height_in_items);

		virtual bool ListBox(const TCHAR *label, int *current_item, bool (*items_getter)(void *data, int idx, const char **out_text), void *data, int items_count, int height_in_items);

		virtual bool ListBoxHeader(const TCHAR *label, const glm::fvec2 &size);

		virtual bool ListBoxHeader(const TCHAR *label, int items_count, int height_in_items);

		virtual void ListBoxFooter();

		virtual void PlotLines(const TCHAR *label, const float *values, int values_count, int values_offset, const TCHAR *overlay_textptr, float scale_min, float scale_max, glm::fvec2 graph_size, int stride);

		virtual void PlotLines(const TCHAR *label, float(*values_getter)(void *data, int idx), void *data, int values_count, int values_offset, const TCHAR *overlay_textptr, float scale_min, float scale_max, glm::fvec2 graph_size);

		virtual void PlotHistogram(const TCHAR *label, const float *values, int values_count, int values_offset, const TCHAR *overlay_textptr, float scale_min, float scale_max, glm::fvec2 graph_size, int stride);

		virtual void PlotHistogram(const TCHAR *label, float(*values_getter)(void *data, int idx), void *data, int values_count, int values_offset, const TCHAR *overlay_textptr, float scale_min, float scale_max, glm::fvec2 graph_size);

		virtual void Value(const TCHAR *prefix, bool b);

		virtual void Value(const TCHAR *prefix, int v);

		virtual void Value(const TCHAR *prefix, unsigned int v);

		virtual void Value(const TCHAR *prefix, float v, const TCHAR *float_formatptr);

		virtual bool BeginMenuBar();

		virtual void EndMenuBar();

		virtual bool BeginMainMenuBar();

		virtual void EndMainMenuBar();

		virtual bool BeginMenu(const TCHAR *label, bool enabled);

		virtual void EndMenu();

		virtual bool MenuItem(const TCHAR *label, const TCHAR *shortcutptr, bool selected, bool enabled);

		virtual bool MenuItem(const TCHAR *label, const TCHAR *shortcut, bool *p_selected, bool enabled);

		virtual void BeginTooltip();

		virtual void EndTooltip();

		virtual void SetTooltip(const TCHAR *fmt, ...);

		virtual void SetTooltipV(const TCHAR *fmt, va_list args);

		virtual void OpenPopup(const TCHAR *str_id);

		virtual bool BeginPopup(const TCHAR *str_id, props::TFlags64 window_flags);

		virtual bool BeginPopupContextItem(const TCHAR *str_idptr, MouseButton mouse_button);

		virtual bool BeginPopupContextWindow(const TCHAR *str_idptr, MouseButton mouse_button, bool also_over_items);

		virtual bool BeginPopupContextVoid(const TCHAR *str_idptr, MouseButton mouse_button);

		virtual bool BeginPopupModal(const TCHAR *name, bool *p_openptr, props::TFlags64 window_flags);

		virtual void EndPopup();

		virtual void OpenPopupOnItemClick(const TCHAR *str_idptr, MouseButton mouse_button);

		virtual bool IsPopupOpen(const TCHAR *str_id);

		virtual void CloseCurrentPopup();

		virtual void Columns(int count, const TCHAR *idptr, bool border);

		virtual void NextColumn();

		virtual int GetColumnIndex();

		virtual float GetColumnWidth(int column_index);

		virtual void SetColumnWidth(int column_index, float width);

		virtual float GetColumnOffset(int column_index);

		virtual void SetColumnOffset(int column_index, float offset_x);

		virtual int GetColumnsCount();

		virtual bool BeginTabBar(const TCHAR *str_id, props::TFlags64 tabbar_flags);

		virtual void EndTabBar();

		virtual bool BeginTabItem(const TCHAR *label, bool *p_openptr, props::TFlags64 tabitem_flags);

		virtual void EndTabItem();

		virtual void SetTabItemClosed(const TCHAR *tab_or_docked_window_label);

		virtual void LogToTTY(int auto_open_depth);

		virtual void LogToFile(int auto_open_depth, const TCHAR *filenameptr);

		virtual void LogToClipboard(int auto_open_depth);

		virtual void LogFinish();

		virtual void LogButtons();

		virtual void LogText(const TCHAR *fmt, ...);

#if 0
		virtual bool BeginDragDropSource(props::TFlags64 dragdrop_flags);

		virtual bool SetDragDropPayload(const TCHAR *type, const void *data, size_t sz, int64_t cond);

		virtual void EndDragDropSource();

		virtual bool BeginDragDropTarget();

		virtual const ImGuiPayload *AcceptDragDropPayload(const TCHAR *type, props::TFlags64 dragdrop_flags);

		virtual void EndDragDropTarget();

		virtual const ImGuiPayload *GetDragDropPayload();
#endif

		virtual void PushClipRect(const glm::fvec2 &clip_rect_min, const glm::fvec2 &clip_rect_max, bool intersect_with_current_clip_rect);

		virtual void PopClipRect();

		virtual void SetItemDefaultFocus();

		virtual void SetKeyboardFocusHere(int offset);

		virtual bool IsItemHovered(props::TFlags64 hover_flags);

		virtual bool IsItemActive();

		virtual bool IsItemFocused();

		virtual bool IsItemClicked(MouseButton mouse_button);

		virtual bool IsItemVisible();

		virtual bool IsItemEdited();

		virtual bool IsItemActivated();

		virtual bool IsItemDeactivated();

		virtual bool IsItemDeactivatedAfterEdit();

		virtual bool IsItemToggledOpen();

		virtual bool IsAnyItemHovered();

		virtual bool IsAnyItemActive();

		virtual bool IsAnyItemFocused();

		virtual glm::fvec2 GetItemRectMin();

		virtual glm::fvec2 GetItemRectMax();

		virtual glm::fvec2 GetItemRectSize();

		virtual void SetItemAllowOverlap();

		virtual bool IsRectVisible(const glm::fvec2 &size);

		virtual bool IsRectVisible(const glm::fvec2 &rect_min, const glm::fvec2 &rect_max);

		virtual double GetTime();

		virtual int GetFrameCount();

		virtual const TCHAR *GetStyleColorName(ColorType idx);

#if 0
		virtual void SetStateStorage(ImGuiStorage *storage);

		virtual ImGuiStorage *GetStateStorage();
#endif

		virtual glm::fvec2 CalcTextSize(const TCHAR *text, const TCHAR *text_endptr, bool hide_text_after_double_hash, float wrap_width);

		virtual void CalcListClipping(int items_count, float items_height, int *out_items_display_start, int *out_items_display_end);

		virtual bool BeginChildFrame(uint64_t id, const glm::fvec2 &size, props::TFlags64 window_flags);

		virtual void EndChildFrame();

		virtual glm::fvec4 ColorConvertU32ToFloat4(uint32_t in);

		virtual uint32_t ColorConvertFloat4ToU32(const glm::fvec4 &in);

		virtual void ColorConvertRGBtoHSV(float r, float g, float b, float &out_h, float &out_s, float &out_v);

		virtual void ColorConvertHSVtoRGB(float h, float s, float v, float &out_r, float &out_g, float &out_b);

#if 0
		virtual int GetKeyIndex(ImGuiKey imgui_key);
#endif

		virtual bool IsKeyDown(int user_key_index);

		virtual bool IsKeyPressed(int user_key_index, bool repeat);

		virtual bool IsKeyReleased(int user_key_index);

		virtual int GetKeyPressedAmount(int key_index, float repeat_delay, float rate);

		virtual void CaptureKeyboardFromApp(bool want_capture_keyboard_value);

		virtual bool IsMouseDown(MouseButton button);

		virtual bool IsMouseClicked(MouseButton button, bool repeat);

		virtual bool IsMouseReleased(MouseButton button);

		virtual bool IsMouseDoubleClicked(MouseButton button);

		virtual bool IsMouseHoveringRect(const glm::fvec2 &r_min, const glm::fvec2 &r_max, bool clip);

		virtual bool IsMousePosValid(const glm::fvec2 *mouse_posptr);

		virtual bool IsAnyMouseDown();

		virtual glm::fvec2 GetMousePos();

		virtual glm::fvec2 GetMousePosOnOpeningCurrentPopup();

		virtual bool IsMouseDragging(MouseButton button, float lock_threshold);

		virtual glm::fvec2 GetMouseDragDelta(MouseButton button, float lock_threshold);

		virtual void ResetMouseDragDelta(MouseButton button);

		virtual MouseCursor GetMouseCursor();

		virtual void SetMouseCursor(MouseCursor cursor_type);

		virtual void CaptureMouseFromApp(bool want_capture_mouse_value);

		virtual const TCHAR *GetClipboardText();

		virtual void SetClipboardText(const TCHAR *text);

		virtual void ShowDebugWindow(bool* p_open);

	};

};