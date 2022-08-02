
// C3App.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// C3App:
// See C3App.cpp for the implementation of this class
//

class UIControl : public c3::Component
{

protected:
	c3::Object *m_Owner;
	c3::ShaderComponent *m_VS_defobj;
	c3::ShaderComponent *m_FS_defobj;
	c3::ShaderProgram *m_SP_defobj;
	c3::Resource *m_ModRes;


public:
	static const c3::ComponentType *Type();

	UIControl();
	virtual ~UIControl();
	virtual void Release();
	virtual c3::ComponentType *GetType();
	virtual props::TFlags64 Flags();
	virtual void Update(c3::Object *powner, float elapsed_time = 0.0f);
	virtual bool Initialize(c3::Object *powner);
	virtual bool Prerender(c3::Object *powner, props::TFlags64 rendflags);
	virtual void Render(c3::Object *powner, props::TFlags64 rendflags);
	virtual void PropertyChanged(const props::IProperty *pprop) { }

};

#if 0
DEFINE_COMPONENTTYPE(UIControl, UIControl, GUID({0x8e92d07e, 0x3169, 0x41e8, { 0x81, 0x2a, 0x4, 0x96, 0x4e, 0x3c, 0xd5, 0x7b }}), "UIControl", "");
#endif

class C3App : public CWinApp
{
public:
	c3::System *m_C3;
	c3::Configuration *m_Cfg;

public:
	C3App();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern C3App theApp;
