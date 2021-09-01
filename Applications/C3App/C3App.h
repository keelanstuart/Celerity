
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

class UIControl : public c3::Feature
{

protected:
	c3::Object *m_Owner;
	c3::ShaderComponent *m_VS;
	c3::ShaderComponent *m_FS;
	c3::ShaderProgram *m_SP;
	c3::Material *m_Mtl;
	c3::Mesh *m_M;
	c3::Texture2D *m_Tex;
	c3::Resource *m_NormRes;
	c3::Resource *m_TexRes;
	int64_t m_iMatMVP, m_iMatN;
	c3::Model *m_Mod;
	c3::Resource *m_ModRes;


public:
	static const c3::FeatureType *Type();

	UIControl();
	virtual ~UIControl();
	virtual void Release();
	virtual c3::FeatureType *GetType();
	virtual props::TFlags64 Flags();
	virtual void Update(c3::Object *powner, float elapsed_time = 0.0f);
	virtual bool Initialize(c3::Object *powner);
	virtual bool Prerender(c3::Object *powner, props::TFlags64 rendflags);
	virtual void Render(c3::Object *powner, props::TFlags64 rendflags);
	virtual void PropertyChanged(const props::IProperty *pprop) { }

};

DEFINE_FEATURETYPE(UIControl, UIControl, GUID({0x8e92d07e, 0x3169, 0x41e8, { 0x81, 0x2a, 0x4, 0x96, 0x4e, 0x3c, 0xd5, 0x7b }}), "UIControl", "");

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
