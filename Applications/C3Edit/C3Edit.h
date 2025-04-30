// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// C3EditApp:
// See C3Edit.cpp for the implementation of this class
//

extern std::vector<std::pair<const TCHAR *, uint64_t>> FlagInfo;

class C3EditApp : public CWinAppEx
{
public:

	typedef enum
	{
		TT_SELECT = 0,
		TT_TRANSLATE,
		TT_ROTATE,
		TT_SCALE,
		TT_UNISCALE,
		TT_WAND
	} ToolType;

	typedef enum
	{
		AT_X = 0x1,
		AT_Y = 0x2,
		AT_Z = 0x4,

		AT_SCREENREL = 0x8
	} AxisType;

	typedef enum
	{
		CT_PERSPECTIVE_FREE = 0,
		CT_ORTHO_FREE,
		CT_TOP,
		CT_BOTTOM,
		CT_LEFT,
		CT_RIGHT,
		CT_FRONT,
		CT_BACK,

		CT_NUMCAMERATYPES
	} CameraType;

	struct SUndoInfo
	{
		struct UNDOTYPE
		{
			enum
			{
				ADD_OBJECT,
				DELETE_OBJECT,
				ADD_COMPONENT,
				DELETE_COMPONENT,
				SET_FLAG,
				CLEAR_FLAG,
				CHANGE_PROPERTY,
			};
		};

		UNDOTYPE type;
		tstring cmd;
	};

	using TUndoInfoDeque = std::deque<SUndoInfo>;

	TUndoInfoDeque m_Undo, m_Redo;

	void PushUndo(SUndoInfo::UNDOTYPE type, const tstring &data);
	void PopUndo();

	C3EditApp() noexcept;

protected:
	CSingleDocTemplate* m_pDocTemplate;
public:


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	c3::System *m_C3;

	tstring m_AppDataRoot;

	c3::Configuration *m_Config;

	void SetActiveProperties(props::IPropertySet *props, bool readonly = false, const TCHAR *title = nullptr);
	void SetActivePrototype(c3::Prototype *pproto, bool readonly = false, const TCHAR *title = nullptr);
	void SetActiveObject(c3::Object *pobj, bool readonly = false, const TCHAR *title = nullptr);
	c3::Object *GetActiveObject() const;
	void UpdateObjectList();
	void UpdateStatusMessage(const TCHAR *msg = nullptr);

	void ExtractPaths(const TCHAR *allpaths, std::vector<tstring> &paths);

	void RefreshActiveProperties();

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	afx_msg void OnFileNewFrame();
	afx_msg void OnFileNew();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
    afx_msg void OnViewGrapheditor();

};

extern C3EditApp theApp;
