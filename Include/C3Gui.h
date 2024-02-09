// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class Gui
	{

	public:

		typedef enum
		{
			GS_NORMAL = 0,
			GS_LIGHT,
			GS_DARK,

			GS_NUMSTYLES
		} ColorScheme;

		typedef enum
		{
			SV_ALPHA = 0,
			SV_WINDOW_PADDING,
			SV_WINDOW_ROUNDING,
			SV_WINDOW_BORDERSIZE,
			SV_WINDOW_MINSIZE,
			SV_WINDOW_TITLEALIGN,
			SV_CHILD_ROUNDING,
			SV_CHILD_BORDERSIZE,
			SV_POPUP_ROUNDING,
			SV_POPUP_BORDERSIZE,
			SV_FRAME_PADDING,
			SV_FRAME_ROUNDING,
			SV_FRAME_BORDERSIZE,
			SV_ITEM_SPACING,
			SV_ITEM_INNERSPACING,
			SV_INDENT_SPACING,
			SV_SCROLLBAR_SIZE,
			SV_SCROLLBAR_ROUNDING,
			SV_GRAB_MINSIZE,
			SV_GRAB_ROUNDING,
			SV_TAB_ROUNDING,
			SV_BUTTON_TEXTALIGN,
			SV_SELECTABLE_TEXTALIGN,

			SV_NUMSTYLEVARS
		} StyleVar;

		typedef enum
		{
			CT_TEXT = 0,
			CT_TEXTDISABLED,
			CT_WINDOWBG,
			CT_CHILDBG,
			CT_POPUPBG,
			CT_BORDER,
			CT_BORDERSHADOW,
			CT_FRAMEBG,
			CT_FRAMEBGHOVERED,
			CT_FRAMEBGACTIVE,
			CT_TITLEBG,
			CT_TITLEBGACTIVE,
			CT_TITLEBGCOLLAPSED,
			CT_MENUBARBG,
			CT_SCROLLBARBG,
			CT_SCROLLBARGRAB,
			CT_SCROLLBARGRABHOVERED,
			CT_SCROLLBARGRABACTIVE,
			CT_CHECKMARK,
			CT_SLIDERGRAB,
			CT_SLIDERGRABACTIVE,
			CT_BUTTON,
			CT_BUTTONHOVERED,
			CT_BUTTONACTIVE,
			CT_HEADER,
			CT_HEADERHOVERED,
			CT_HEADERACTIVE,
			CT_SEPARATOR,
			CT_SEPARATORHOVERED,
			CT_SEPARATORACTIVE,
			CT_RESIZEGRIP,
			CT_RESIZEGRIPHOVERED,
			CT_RESIZEGRIPACTIVE,
			CT_TAB,
			CT_TABHOVERED,
			CT_TABACTIVE,
			CT_TABUNFOCUSED,
			CT_TABUNFOCUSEDACTIVE,
			CT_PLOTLINES,
			CT_PLOTLINESHOVERED,
			CT_PLOTHISTOGRAM,
			CT_PLOTHISTOGRAMHOVERED,
			CT_TEXTSELECTEDBG,
			CT_DRAGDROPTARGET,
			CT_NAVHIGHLIGHT,
			CT_NAVWINDOWINGHIGHLIGHT,
			CT_NAVWINDOWINGDIMBG,
			CT_MODALWINDOWDIMBG,

			CT_NUMCOLORTYPES
		} ColorType;

		using Direction = enum
		{
			DIR_NONE = -1,

			DIR_LEFT,
			DIR_RIGHT,
			DIR_UP,
			DIR_DOWN,

			DIR_NUMDIRS
		};

		using MouseButton = enum
		{
			MBUT_LEFT = 0,
			MBUT_RIGHT,
			MBUT_MIDDLE
		};

		using MouseCursor = enum
		{
			MCUR_NONE = -1,

			MCUR_ARROW,
			MCUR_TEXTINPUT,
			MCUR_RESIZEALL,
			MCUR_RESIZENS,
			MCUR_RESIZEEW,
			MCUR_RESIZENESW,
			MCUR_RESIZENWSE,
			MCUR_HAND,
			MCUR_NOTALLOWED,

			MCUR_NUMCURSORS
		};

		using DataType = enum
		{
			DT_S8 = 0,
			DT_U8,
			DT_S16,
			DT_U16,
			DT_S32,
			DT_U32,
			DT_S64,
			DT_U64,
			DT_FLOAT,
			DT_DOUBLE,

			DT_NUMTYPES
		};

		virtual void AddMouseButtonEvent(MouseButton but, bool down) = NULL;

		virtual void AddMouseWheelEvent(float wheelx, float wheely) = NULL;

		virtual bool Begin(const TCHAR *name, bool *p_open = nullptr, props::TFlags64 flags = 0) = NULL;

		virtual void End() = NULL;

		virtual bool BeginChild(const TCHAR *str_id, const glm::fvec2 &size = glm::fvec2(0, 0), bool border = false, props::TFlags64 window_flags = 0) = NULL;

		virtual bool BeginChild(uint64_t id, const glm::fvec2 &size = glm::fvec2(0, 0), bool border = false, props::TFlags64 window_flags = 0) = NULL;

		virtual void EndChild() = NULL;

		virtual bool IsWindowAppearing() = NULL;

		virtual bool IsWindowCollapsed() = NULL;

		virtual bool IsWindowFocused(props::TFlags64 focus_flags = 0) = NULL;

		virtual bool IsWindowHovered(props::TFlags64 hover_flags = 0) = NULL;

		virtual glm::fvec2 GetWindowPos() = NULL;

		virtual glm::fvec2 GetWindowSize() = NULL;

		virtual float GetWindowWidth() = NULL;

		virtual float GetWindowHeight() = NULL;

		virtual void SetNextWindowPos(const glm::fvec2 &pos, int64_t cond = 0, const glm::fvec2 &pivot = glm::fvec2(0, 0)) = NULL;

		virtual void SetNextWindowSize(const glm::fvec2 &size, int64_t cond = 0) = NULL;

		virtual void SetNextWindowSizeConstraints(const glm::fvec2 &size_min, const glm::fvec2 &size_max) = NULL;

		virtual void SetNextWindowContentSize(const glm::fvec2 &size) = NULL;

		virtual void SetNextWindowCollapsed(bool collapsed, int64_t cond = 0) = NULL;

		virtual void SetNextWindowFocus() = NULL;

		virtual void SetNextWindowBgAlpha(float alpha) = NULL;

		virtual void SetWindowPos(const glm::fvec2 &pos, int64_t cond = 0) = NULL;

		virtual void SetWindowSize(const glm::fvec2 &size, int64_t cond = 0) = NULL;

		virtual void SetWindowCollapsed(bool collapsed, int64_t cond = 0) = NULL;

		virtual void SetWindowFocus() = NULL;

		virtual void SetWindowFontScale(float scale) = NULL;

		virtual void SetWindowPos(const TCHAR *name, const glm::fvec2 &pos, int64_t cond = 0) = NULL;

		virtual void SetWindowSize(const TCHAR *name, const glm::fvec2 &size, int64_t cond = 0) = NULL;

		virtual void SetWindowCollapsed(const TCHAR *name, bool collapsed, int64_t cond = 0) = NULL;

		virtual void SetWindowFocus(const TCHAR *name) = NULL;

		virtual glm::fvec2 GetContentRegionMax() = NULL;

		virtual glm::fvec2 GetContentRegionAvail() = NULL;

		virtual glm::fvec2 GetWindowContentRegionMin() = NULL;

		virtual glm::fvec2 GetWindowContentRegionMax() = NULL;

		virtual float GetWindowContentRegionWidth() = NULL;

		virtual float GetScrollX() = NULL;

		virtual float GetScrollY() = NULL;

		virtual float GetScrollMaxX() = NULL;

		virtual float GetScrollMaxY() = NULL;

		virtual void SetScrollX(float scroll_x) = NULL;

		virtual void SetScrollY(float scroll_y) = NULL;

		virtual void SetScrollHereX(float center_x_ratio = 0.5f) = NULL;

		virtual void SetScrollHereY(float center_y_ratio = 0.5f) = NULL;

		virtual void SetScrollFromPosX(float local_x, float center_x_ratio = 0.5f) = NULL;

		virtual void SetScrollFromPosY(float local_y, float center_y_ratio = 0.5f) = NULL;

#if 0
		virtual void PushFont(ImFont *font) = NULL;

		virtual void PopFont() = NULL;
#endif

		virtual void SetColorScheme(ColorScheme scheme) = NULL;

		virtual ColorScheme GetColorScheme() = NULL;

		virtual void PushStyleColor(ColorType idx, uint32_t col) = NULL;

		virtual void PushStyleColor(ColorType idx, const glm::fvec4 &col) = NULL;

		virtual void PopStyleColor(int count = 1) = NULL;

		virtual void PushStyleVar(StyleVar idx, float val) = NULL;

		virtual void PushStyleVar(StyleVar idx, const glm::fvec2 &val) = NULL;

		virtual void PopStyleVar(int count = 1) = NULL;

		virtual const glm::fvec4 GetStyleColorVec4(ColorType idx) = NULL;

#if 0
		virtual ImFont *GetFont() = NULL;

		virtual float GetFontSize() = NULL;

		virtual glm::fvec2 GetFontTexUvWhitePixel() = NULL;
#endif

		virtual uint32_t GetColorU32(ColorType idx, float alpha_mul = 1.0f) = NULL;

		virtual uint32_t GetColorU32(const glm::fvec4 &col) = NULL;

		virtual uint32_t GetColorU32(uint32_t col) = NULL;

		virtual void PushItemWidth(float item_width) = NULL;

		virtual void PopItemWidth() = NULL;

		virtual void SetNextItemWidth(float item_width) = NULL;

		virtual float CalcItemWidth() = NULL;

		virtual void PushTextWrapPos(float wrap_local_pos_x = 0.0f) = NULL;

		virtual void PopTextWrapPos() = NULL;

		virtual void PushAllowKeyboardFocus(bool allow_keyboard_focus) = NULL;

		virtual void PopAllowKeyboardFocus() = NULL;

		virtual void PushButtonRepeat(bool repeat) = NULL;

		virtual void PopButtonRepeat() = NULL;

		virtual void Separator() = NULL;

		virtual void SameLine(float offset_from_start_x=0.0f, float spacing=-1.0f) = NULL;

		virtual void NewLine() = NULL;

		virtual void Spacing() = NULL;

		virtual void Dummy(const glm::fvec2 &size) = NULL;

		virtual void Indent(float indent_w = 0.0f) = NULL;

		virtual void Unindent(float indent_w = 0.0f) = NULL;

		virtual void BeginGroup() = NULL;

		virtual void EndGroup() = NULL;

		virtual glm::fvec2 GetCursorPos() = NULL;

		virtual float GetCursorPosX() = NULL;

		virtual float GetCursorPosY() = NULL;

		virtual void SetCursorPos(const glm::fvec2 &local_pos) = NULL;

		virtual void SetCursorPosX(float local_x) = NULL;

		virtual void SetCursorPosY(float local_y) = NULL;

		virtual glm::fvec2 GetCursorStartPos() = NULL;

		virtual glm::fvec2 GetCursorScreenPos() = NULL;

		virtual void SetCursorScreenPos(const glm::fvec2 &pos) = NULL;

		virtual void AlignTextToFramePadding() = NULL;

		virtual float GetTextLineHeight() = NULL;

		virtual float GetTextLineHeightWithSpacing() = NULL;

		virtual float GetFrameHeight() = NULL;

		virtual float GetFrameHeightWithSpacing() = NULL;

		virtual void PushID(const TCHAR *str_id) = NULL;

		virtual void PushID(const TCHAR *str_id_begin, const TCHAR *str_id_end) = NULL;

		virtual void PushID(const void *ptr_id) = NULL;

		virtual void PushID(int int_id) = NULL;

		virtual void PopID() = NULL;

		virtual uint64_t GetID(const TCHAR *str_id) = NULL;

		virtual uint64_t GetID(const TCHAR *str_id_begin, const TCHAR *str_id_end) = NULL;

		virtual uint64_t GetID(const void *ptr_id) = NULL;

		virtual void TextUnformatted(const TCHAR *text, const TCHAR *text_end = nullptr) = NULL;

		virtual void Text(const TCHAR *fmt, ...) = NULL;

		virtual void TextV(const TCHAR *fmt, va_list args) = NULL;

		virtual void TextColored(const glm::fvec4 &col, const TCHAR *fmt, ...) = NULL;

		virtual void TextColoredV(const glm::fvec4 &col, const TCHAR *fmt, va_list args) = NULL;

		virtual void TextDisabled(const TCHAR *fmt, ...) = NULL;

		virtual void TextDisabledV(const TCHAR *fmt, va_list args) = NULL;

		virtual void TextWrapped(const TCHAR *fmt, ...) = NULL;

		virtual void TextWrappedV(const TCHAR *fmt, va_list args) = NULL;

		virtual void LabelText(const TCHAR *label, const TCHAR *fmt, ...) = NULL;

		virtual void LabelTextV(const TCHAR *label, const TCHAR *fmt, va_list args) = NULL;

		virtual void BulletText(const TCHAR *fmt, ...) = NULL;

		virtual void BulletTextV(const TCHAR *fmt, va_list args) = NULL;

		virtual bool Button(const TCHAR *label, const glm::fvec2 &size = glm::fvec2(0, 0)) = NULL;

		virtual bool SmallButton(const TCHAR *label) = NULL;                              

		virtual bool InvisibleButton(const TCHAR *str_id, const glm::fvec2 &size) = NULL;     

		virtual bool ArrowButton(const TCHAR *str_id, Direction dir) = NULL;               

		virtual void Image(Texture2D *user_texture, const glm::fvec2 &size, const glm::fvec2 &uv0 = glm::fvec2(0, 0), const glm::fvec2 &uv1 = glm::fvec2(1, 1), const glm::fvec4 &tint_col = glm::fvec4(1, 1, 1, 1), const glm::fvec4 &border_col = glm::fvec4(0, 0, 0, 0)) = NULL;

		virtual bool ImageButton(Texture2D *user_texture, const glm::fvec2 &size, const glm::fvec2 &uv0 = glm::fvec2(0, 0), const glm::fvec2 &uv1 = glm::fvec2(1, 1), int frame_padding = -1, const glm::fvec4 &bg_col = glm::fvec4(0, 0, 0, 0), const glm::fvec4 &tint_col = glm::fvec4(1, 1, 1, 1)) = NULL;

		virtual bool Checkbox(const TCHAR *label, bool *v) = NULL;

		virtual bool CheckboxFlags(const TCHAR *label, unsigned int *flags, unsigned int flags_value) = NULL;

		virtual bool RadioButton(const TCHAR *label, bool active) = NULL;                   

		virtual bool RadioButton(const TCHAR *label, int *v, int v_button) = NULL;          

		virtual void ProgressBar(float fraction, const glm::fvec2 &size_arg = glm::fvec2(-1, 0), const TCHAR *overlay = nullptr) = NULL;

		virtual void Bullet() = NULL;

		virtual bool BeginCombo(const TCHAR *label, const TCHAR *preview_value, props::TFlags64 combo_flags = 0) = NULL;

		virtual void EndCombo() = NULL;

		virtual bool Combo(const TCHAR *label, int *current_item, const TCHAR *const items[], int items_count, int popup_max_height_in_items = -1) = NULL;

		virtual bool Combo(const TCHAR *label, int *current_item, const TCHAR *items_separated_by_zeros, int popup_max_height_in_items = -1) = NULL;

		virtual bool Combo(const TCHAR *label, int *current_item, bool(*items_getter)(void *data, int idx, const char **out_text), void *data, int items_count, int popup_max_height_in_items = -1) = NULL;

#if 0
		virtual bool DragFloat(const TCHAR *label, float *v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const TCHAR *format = _T("%.3f"), float power = 1.0f) = NULL;

		virtual bool DragFloat2(const TCHAR *label, float v[2], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const TCHAR *format = _T("%.3f"), float power = 1.0f) = NULL;

		virtual bool DragFloat3(const TCHAR *label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const TCHAR *format = _T("%.3f"), float power = 1.0f) = NULL;

		virtual bool DragFloat4(const TCHAR *label, float v[4], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const TCHAR *format = _T("%.3f"), float power = 1.0f) = NULL;

		virtual bool DragFloatRange2(const TCHAR *label, float *v_current_min, float *v_current_max, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const TCHAR *format = _T("%.3f"), const TCHAR *format_max = nullptr, float power = 1.0f) = NULL;

		virtual bool DragInt(const TCHAR *label, int *v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const TCHAR *format = _T("%d")) = NULL;

		virtual bool DragInt2(const TCHAR *label, int v[2], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const TCHAR *format = _T("%d")) = NULL;

		virtual bool DragInt3(const TCHAR *label, int v[3], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const TCHAR *format = _T("%d")) = NULL;

		virtual bool DragInt4(const TCHAR *label, int v[4], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const TCHAR *format = _T("%d")) = NULL;

		virtual bool DragIntRange2(const TCHAR *label, int *v_current_min, int *v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const TCHAR *format = _T("%d"), const TCHAR *format_max = nullptr) = NULL;

		virtual bool DragScalar(const TCHAR *label, DataType data_type, void *p_data, float v_speed, const void *p_min = nullptr, const void *p_max = nullptr, const TCHAR *format = nullptr, float power = 1.0f) = NULL;

		virtual bool DragScalarN(const TCHAR *label, DataType data_type, void *p_data, int components, float v_speed, const void *p_min = nullptr, const void *p_max = nullptr, const TCHAR *format = nullptr, float power = 1.0f) = NULL;
#endif

		virtual bool SliderFloat(const TCHAR *label, float *v, float v_min, float v_max, const TCHAR *format = _T("%.3f"), float power = 1.0f) = NULL;

		virtual bool SliderFloat2(const TCHAR *label, float v[2], float v_min, float v_max, const TCHAR *format = _T("%.3f"), float power = 1.0f) = NULL;

		virtual bool SliderFloat3(const TCHAR *label, float v[3], float v_min, float v_max, const TCHAR *format = _T("%.3f"), float power = 1.0f) = NULL;

		virtual bool SliderFloat4(const TCHAR *label, float v[4], float v_min, float v_max, const TCHAR *format = _T("%.3f"), float power = 1.0f) = NULL;

		virtual bool SliderAngle(const TCHAR *label, float *v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const TCHAR *format = _T("%.0f deg")) = NULL;

		virtual bool SliderInt(const TCHAR *label, int *v, int v_min, int v_max, const TCHAR *format = _T("%d")) = NULL;

		virtual bool SliderInt2(const TCHAR *label, int v[2], int v_min, int v_max, const TCHAR *format = _T("%d")) = NULL;

		virtual bool SliderInt3(const TCHAR *label, int v[3], int v_min, int v_max, const TCHAR *format = _T("%d")) = NULL;

		virtual bool SliderInt4(const TCHAR *label, int v[4], int v_min, int v_max, const TCHAR *format = _T("%d")) = NULL;

		virtual bool SliderScalar(const TCHAR *label, DataType data_type, void *p_data, const void *p_min, const void *p_max, const TCHAR *format = nullptr, float power = 1.0f) = NULL;

		virtual bool SliderScalarN(const TCHAR *label, DataType data_type, void *p_data, int components, const void *p_min, const void *p_max, const TCHAR *format = nullptr, float power = 1.0f) = NULL;

		virtual bool VSliderFloat(const TCHAR *label, const glm::fvec2 &size, float *v, float v_min, float v_max, const TCHAR *format = _T("%.3f"), props::TFlags64 flags = 0) = NULL;

		virtual bool VSliderInt(const TCHAR *label, const glm::fvec2 &size, int *v, int v_min, int v_max, const TCHAR *format = _T("%d")) = NULL;

		virtual bool VSliderScalar(const TCHAR *label, const glm::fvec2 &size, DataType data_type, void *p_data, const void *p_min, const void *p_max, const TCHAR *format = nullptr, props::TFlags64 flags = 0) = NULL;

#if 0
		virtual bool InputText(const TCHAR *label, TCHAR *buf, size_t buf_size, props::TFlags64 inputtext_flags = 0, ImGuiInputTextCallback callback = nullptr, void *user_data = nullptr) = NULL;

		virtual bool InputTextMultiline(const TCHAR *label, TCHAR *buf, size_t buf_size, const glm::fvec2 &size = glm::fvec2(0, 0), props::TFlags64 inputtext_flags = 0, ImGuiInputTextCallback callback = nullptr, void *user_data = nullptr) = NULL;

		virtual bool InputTextWithHint(const TCHAR *label, const TCHAR *hint, TCHAR *buf, size_t buf_size, props::TFlags64 inputtext_flags = 0, ImGuiInputTextCallback callback = nullptr, void *user_data = nullptr) = NULL;

		virtual bool InputFloat(const TCHAR *label, float *v, float step = 0.0f, float step_fast = 0.0f, const TCHAR *format = _T("%.3f"), props::TFlags64 inputtext_flags = 0) = NULL;

		virtual bool InputFloat2(const TCHAR *label, float v[2], const TCHAR *format = _T("%.3f"), props::TFlags64 inputtext_flags = 0) = NULL;

		virtual bool InputFloat3(const TCHAR *label, float v[3], const TCHAR *format = _T("%.3f"), props::TFlags64 inputtext_flags = 0) = NULL;

		virtual bool InputFloat4(const TCHAR *label, float v[4], const TCHAR *format = _T("%.3f"), props::TFlags64 inputtext_flags = 0) = NULL;

		virtual bool InputInt(const TCHAR *label, int *v, int step = 1, int step_fast = 100, props::TFlags64 inputtext_flags = 0) = NULL;

		virtual bool InputInt2(const TCHAR *label, int v[2], props::TFlags64 inputtext_flags = 0) = NULL;

		virtual bool InputInt3(const TCHAR *label, int v[3], props::TFlags64 inputtext_flags = 0) = NULL;

		virtual bool InputInt4(const TCHAR *label, int v[4], props::TFlags64 inputtext_flags = 0) = NULL;

		virtual bool InputDouble(const TCHAR *label, double *v, double step = 0.0, double step_fast = 0.0, const TCHAR *format = _T("%.6f"), props::TFlags64 inputtext_flags = 0) = NULL;

		virtual bool InputScalar(const TCHAR *label, DataType data_type, void *p_data, const void *p_step = nullptr, const void *p_step_fast = nullptr, const TCHAR *format = nullptr, props::TFlags64 inputtext_flags = 0) = NULL;

		virtual bool InputScalarN(const TCHAR *label, DataType data_type, void *p_data, int components, const void *p_step = nullptr, const void *p_step_fast = nullptr, const TCHAR *format = nullptr, props::TFlags64 inputtext_flags = 0) = NULL;
#endif

		virtual bool ColorEdit3(const TCHAR *label, float col[3], props::TFlags64 coloredit_flags = 0) = NULL;

		virtual bool ColorEdit4(const TCHAR *label, float col[4], props::TFlags64 coloredit_flags = 0) = NULL;

		virtual bool ColorPicker3(const TCHAR *label, float col[3], props::TFlags64 coloredit_flags = 0) = NULL;

		virtual bool ColorPicker4(const TCHAR *label, float col[4], props::TFlags64 coloredit_flags = 0, const float *ref_col = nullptr) = NULL;

		virtual bool ColorButton(const TCHAR *desc_id, const glm::fvec4 &col, props::TFlags64 coloredit_flags = 0, glm::fvec2 size = glm::fvec2(0, 0)) = NULL;

		virtual void SetColorEditOptions(props::TFlags64 coloredit_flags) = NULL;

		virtual bool TreeNode(const TCHAR *label) = NULL;

		virtual bool TreeNode(const TCHAR *str_id, const TCHAR *fmt, ...) = NULL;

		virtual bool TreeNode(const void *ptr_id, const TCHAR *fmt, ...) = NULL;

		virtual bool TreeNodeV(const TCHAR *str_id, const TCHAR *fmt, va_list args) = NULL;

		virtual bool TreeNodeV(const void *ptr_id, const TCHAR *fmt, va_list args) = NULL;

		virtual bool TreeNodeEx(const TCHAR *label, props::TFlags64 treenode_flags = 0) = NULL;

		virtual bool TreeNodeEx(const TCHAR *str_id, props::TFlags64 treenode_flags, const TCHAR *fmt, ...) = NULL;

		virtual bool TreeNodeEx(const void *ptr_id, props::TFlags64 treenode_flags, const TCHAR *fmt, ...) = NULL;

		virtual bool TreeNodeExV(const TCHAR *str_id, props::TFlags64 treenode_flags, const TCHAR *fmt, va_list args) = NULL;

		virtual bool TreeNodeExV(const void *ptr_id, props::TFlags64 treenode_flags, const TCHAR *fmt, va_list args) = NULL;

		virtual void TreePush(const TCHAR *str_id) = NULL;

		virtual void TreePush(const void *ptr_id = nullptr) = NULL;

		virtual void TreePop() = NULL;

		virtual float GetTreeNodeToLabelSpacing() = NULL;

		virtual bool CollapsingHeader(const TCHAR *label, props::TFlags64 treenode_flags = 0) = NULL;

		virtual bool CollapsingHeader(const TCHAR *label, bool *p_open, props::TFlags64 treenode_flags = 0) = NULL;

		virtual void SetNextItemOpen(bool is_open, int64_t cond = 0) = NULL;

		virtual bool Selectable(const TCHAR *label, bool selected = false, props::TFlags64 selectable_flags = 0, const glm::fvec2 &size = glm::fvec2(0, 0)) = NULL;

		virtual bool Selectable(const TCHAR *label, bool *p_selected, props::TFlags64 selectable_flags = 0, const glm::fvec2 &size = glm::fvec2(0, 0)) = NULL;

		virtual bool ListBox(const TCHAR *label, int *current_item, const TCHAR *const items[], int items_count, int height_in_items = -1) = NULL;

		virtual bool ListBox(const TCHAR *label, int *current_item, bool (*items_getter)(void *data, int idx, const char **out_text), void *data, int items_count, int height_in_items = -1) = NULL;

		virtual bool ListBoxHeader(const TCHAR *label, const glm::fvec2 &size = glm::fvec2(0, 0)) = NULL;

		virtual bool ListBoxHeader(const TCHAR *label, int items_count, int height_in_items = -1) = NULL;

		virtual void ListBoxFooter() = NULL;

		virtual void PlotLines(const TCHAR *label, const float *values, int values_count, int values_offset = 0, const TCHAR *overlay_text = nullptr, float scale_min = FLT_MAX, float scale_max = FLT_MAX, glm::fvec2 graph_size = glm::fvec2(0, 0), int stride = sizeof(float)) = NULL;

		virtual void PlotLines(const TCHAR *label, float(*values_getter)(void *data, int idx), void *data, int values_count, int values_offset = 0, const TCHAR *overlay_text = nullptr, float scale_min = FLT_MAX, float scale_max = FLT_MAX, glm::fvec2 graph_size = glm::fvec2(0, 0)) = NULL;

		virtual void PlotHistogram(const TCHAR *label, const float *values, int values_count, int values_offset = 0, const TCHAR *overlay_text = nullptr, float scale_min = FLT_MAX, float scale_max = FLT_MAX, glm::fvec2 graph_size = glm::fvec2(0, 0), int stride = sizeof(float)) = NULL;

		virtual void PlotHistogram(const TCHAR *label, float(*values_getter)(void *data, int idx), void *data, int values_count, int values_offset = 0, const TCHAR *overlay_text = nullptr, float scale_min = FLT_MAX, float scale_max = FLT_MAX, glm::fvec2 graph_size = glm::fvec2(0, 0)) = NULL;

		virtual void Value(const TCHAR *prefix, bool b) = NULL;

		virtual void Value(const TCHAR *prefix, int v) = NULL;

		virtual void Value(const TCHAR *prefix, unsigned int v) = NULL;

		virtual void Value(const TCHAR *prefix, float v, const TCHAR *float_format = nullptr) = NULL;

		virtual bool BeginMenuBar() = NULL;

		virtual void EndMenuBar() = NULL;

		virtual bool BeginMainMenuBar() = NULL;

		virtual void EndMainMenuBar() = NULL;

		virtual bool BeginMenu(const TCHAR *label, bool enabled = true) = NULL;

		virtual void EndMenu() = NULL;

		virtual bool MenuItem(const TCHAR *label, const TCHAR *shortcut = nullptr, bool selected = false, bool enabled = true) = NULL;

		virtual bool MenuItem(const TCHAR *label, const TCHAR *shortcut, bool *p_selected, bool enabled = true) = NULL;

		virtual void BeginTooltip() = NULL;

		virtual void EndTooltip() = NULL;

		virtual void SetTooltip(const TCHAR *fmt, ...) = NULL;

		virtual void SetTooltipV(const TCHAR *fmt, va_list args) = NULL;

		virtual void OpenPopup(const TCHAR *str_id) = NULL;

		virtual bool BeginPopup(const TCHAR *str_id, props::TFlags64 window_flags = 0) = NULL;

		virtual bool BeginPopupContextItem(const TCHAR *str_id = nullptr, MouseButton mouse_button = MouseButton::MBUT_LEFT) = NULL;

		virtual bool BeginPopupContextWindow(const TCHAR *str_id = nullptr, MouseButton mouse_button = MouseButton::MBUT_LEFT, bool also_over_items = true) = NULL;

		virtual bool BeginPopupContextVoid(const TCHAR *str_id = nullptr, MouseButton mouse_button = MouseButton::MBUT_LEFT) = NULL;

		virtual bool BeginPopupModal(const TCHAR *name, bool *p_open = nullptr, props::TFlags64 window_flags = 0) = NULL;

		virtual void EndPopup() = NULL;

		virtual void OpenPopupOnItemClick(const TCHAR *str_id = nullptr, MouseButton mouse_button = MouseButton::MBUT_LEFT) = NULL;

		virtual bool IsPopupOpen(const TCHAR *str_id) = NULL;

		virtual void CloseCurrentPopup() = NULL;

		virtual void Columns(int count = 1, const TCHAR *id = nullptr, bool border = true) = NULL;

		virtual void NextColumn() = NULL;

		virtual int GetColumnIndex() = NULL;

		virtual float GetColumnWidth(int column_index = -1) = NULL;

		virtual void SetColumnWidth(int column_index, float width) = NULL;

		virtual float GetColumnOffset(int column_index = -1) = NULL;

		virtual void SetColumnOffset(int column_index, float offset_x) = NULL;

		virtual int GetColumnsCount() = NULL;

		virtual bool BeginTabBar(const TCHAR *str_id, props::TFlags64 tabbar_flags = 0) = NULL;

		virtual void EndTabBar() = NULL;

		virtual bool BeginTabItem(const TCHAR *label, bool *p_open = nullptr, props::TFlags64 tabitem_flags = 0) = NULL;

		virtual void EndTabItem() = NULL;

		virtual void SetTabItemClosed(const TCHAR *tab_or_docked_window_label) = NULL;

		virtual void LogToTTY(int auto_open_depth = -1) = NULL;

		virtual void LogToFile(int auto_open_depth = -1, const TCHAR *filename = nullptr) = NULL;

		virtual void LogToClipboard(int auto_open_depth = -1) = NULL;

		virtual void LogFinish() = NULL;

		virtual void LogButtons() = NULL;

		virtual void LogText(const TCHAR *fmt, ...) = NULL;

#if 0
		virtual bool BeginDragDropSource(props::TFlags64 dragdrop_flags = 0) = NULL;

		virtual bool SetDragDropPayload(const TCHAR *type, const void *data, size_t sz, int64_t cond = 0) = NULL;

		virtual void EndDragDropSource() = NULL;

		virtual bool BeginDragDropTarget() = NULL;

		virtual const ImGuiPayload *AcceptDragDropPayload(const TCHAR *type, props::TFlags64 dragdrop_flags = 0) = NULL;

		virtual void EndDragDropTarget() = NULL;

		virtual const ImGuiPayload *GetDragDropPayload() = NULL;
#endif

		virtual void PushClipRect(const glm::fvec2 &clip_rect_min, const glm::fvec2 &clip_rect_max, bool intersect_with_current_clip_rect) = NULL;

		virtual void PopClipRect() = NULL;

		virtual void SetItemDefaultFocus() = NULL;

		virtual void SetKeyboardFocusHere(int offset = 0) = NULL;

		virtual bool IsItemHovered(props::TFlags64 hover_flags = 0) = NULL;

		virtual bool IsItemActive() = NULL;

		virtual bool IsItemFocused() = NULL;

		virtual bool IsItemClicked(MouseButton mouse_button = MouseButton::MBUT_LEFT) = NULL;

		virtual bool IsItemVisible() = NULL;

		virtual bool IsItemEdited() = NULL;

		virtual bool IsItemActivated() = NULL;

		virtual bool IsItemDeactivated() = NULL;

		virtual bool IsItemDeactivatedAfterEdit() = NULL;

		virtual bool IsItemToggledOpen() = NULL;

		virtual bool IsAnyItemHovered() = NULL;

		virtual bool IsAnyItemActive() = NULL;

		virtual bool IsAnyItemFocused() = NULL;

		virtual glm::fvec2 GetItemRectMin() = NULL;

		virtual glm::fvec2 GetItemRectMax() = NULL;

		virtual glm::fvec2 GetItemRectSize() = NULL;

		virtual void SetItemAllowOverlap() = NULL;

		virtual bool IsRectVisible(const glm::fvec2 &size) = NULL;

		virtual bool IsRectVisible(const glm::fvec2 &rect_min, const glm::fvec2 &rect_max) = NULL;

		virtual double GetTime() = NULL;

		virtual int GetFrameCount() = NULL;

		virtual const TCHAR *GetStyleColorName(ColorType idx) = NULL;

#if 0
		virtual void SetStateStorage(ImGuiStorage *storage) = NULL;

		virtual ImGuiStorage *GetStateStorage() = NULL;
#endif

		virtual glm::fvec2 CalcTextSize(const TCHAR *text, const TCHAR *text_end = nullptr, bool hide_text_after_double_hash = false, float wrap_width = -1.0f) = NULL;

		virtual void CalcListClipping(int items_count, float items_height, int *out_items_display_start, int *out_items_display_end) = NULL;

		virtual bool BeginChildFrame(uint64_t id, const glm::fvec2 &size, props::TFlags64 window_flags = 0) = NULL;

		virtual void EndChildFrame() = NULL;

		virtual glm::fvec4 ColorConvertU32ToFloat4(uint32_t in) = NULL;

		virtual uint32_t ColorConvertFloat4ToU32(const glm::fvec4 &in) = NULL;

		virtual void ColorConvertRGBtoHSV(float r, float g, float b, float &out_h, float &out_s, float &out_v) = NULL;

		virtual void ColorConvertHSVtoRGB(float h, float s, float v, float &out_r, float &out_g, float &out_b) = NULL;

#if 0
		virtual int GetKeyIndex(ImGuiKey imgui_key) = NULL;
#endif

		virtual bool IsKeyDown(int user_key_index) = NULL;

		virtual bool IsKeyPressed(int user_key_index, bool repeat = true) = NULL;

		virtual bool IsKeyReleased(int user_key_index) = NULL;

		virtual int GetKeyPressedAmount(int key_index, float repeat_delay, float rate) = NULL;

		virtual void CaptureKeyboardFromApp(bool want_capture_keyboard_value = true) = NULL;

		virtual bool IsMouseDown(MouseButton button) = NULL;

		virtual bool IsMouseClicked(MouseButton button, bool repeat = false) = NULL;

		virtual bool IsMouseReleased(MouseButton button) = NULL;

		virtual bool IsMouseDoubleClicked(MouseButton button) = NULL;

		virtual bool IsMouseHoveringRect(const glm::fvec2 &r_min, const glm::fvec2 &r_max, bool clip = true) = NULL;

		virtual bool IsMousePosValid(const glm::fvec2 *mouse_pos = nullptr) = NULL;

		virtual bool IsAnyMouseDown() = NULL;

		virtual glm::fvec2 GetMousePos() = NULL;

		virtual glm::fvec2 GetMousePosOnOpeningCurrentPopup() = NULL;

		virtual bool IsMouseDragging(MouseButton button, float lock_threshold = -1.0f) = NULL;

		virtual glm::fvec2 GetMouseDragDelta(MouseButton button = MBUT_LEFT, float lock_threshold = -1.0f) = NULL;

		virtual void ResetMouseDragDelta(MouseButton button = MBUT_LEFT) = NULL;

		virtual MouseCursor GetMouseCursor() = NULL;

		virtual void SetMouseCursor(MouseCursor cursor_type) = NULL;

		virtual void CaptureMouseFromApp(bool want_capture_mouse_value = true) = NULL;

		virtual const TCHAR *GetClipboardText() = NULL;

		virtual void SetClipboardText(const TCHAR *text) = NULL;

		virtual void ShowDebugWindow(bool* p_open) = NULL;

	};

};