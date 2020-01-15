
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

class UIControl : public c3::Comportment
{

protected:
	c3::Object *m_Owner;
	c3::VertexBuffer *m_VB;
	c3::IndexBuffer *m_IB;
	c3::ShaderComponent *m_VS;
	c3::ShaderComponent *m_FS;
	c3::ShaderProgram *m_SP;
	c3::Mesh *m_M;

public:

	UIControl();
	virtual ~UIControl();
	virtual void Release();
	virtual c3::ComportmentType *GetType();
	virtual props::TFlags64 Flags();
	virtual void Update(c3::Object *powner, float elapsed_time = 0.0f);
	virtual bool Initialize(c3::Object *powner);
	virtual bool Prerender(c3::Object *powner, props::TFlags64 rendflags);
	virtual void Render(c3::Object *powner, props::TFlags64 rendflags);

};

DEFINE_COMPORTMENTTYPE(UIControl, UIControl, GUID({0x8e92d07e, 0x3169, 0x41e8, { 0x81, 0x2a, 0x4, 0x96, 0x4e, 0x3c, 0xd5, 0x7b }}), _T("UIControl"), _T(""));

class C3App : public CWinApp
{
public:
	c3::System *m_C3;

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
