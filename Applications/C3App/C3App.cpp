
// C3App.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "C3App.h"
#include "C3Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// C3App

BEGIN_MESSAGE_MAP(C3App, CWinApp)
END_MESSAGE_MAP()

// C3App construction

C3App::C3App()
{
	m_C3 = nullptr;
}


// The one and only C3App object

C3App theApp;


// use this as a callback for loading surface descriptor textures when you only know the diffuse map filename
bool __cdecl AltTextureName(const TCHAR *diffuse_texname, c3::Material::TextureComponentType typeneeded, TCHAR *needed_texnamebuf, size_t texnamebuf_len)
{
	tstring s = diffuse_texname;
	std::transform(s.begin(), s.end(), s.begin(), std::tolower);
	size_t ofs = s.find(tstring(_T("_diff.")), 0);
	if (ofs != std::string::npos)
	{
		switch (typeneeded)
		{
			case c3::Material::TCT_NORMAL:
				s.replace(ofs, 6, _T("_norm."));
				_tcsncpy_s(needed_texnamebuf, texnamebuf_len, s.c_str(), texnamebuf_len);
				return true;

			case c3::Material::TCT_EMISSIVE:
				s.replace(ofs, 6, _T("_emis."));
				_tcsncpy_s(needed_texnamebuf, texnamebuf_len, s.c_str(), texnamebuf_len);
				return true;

			case c3::Material::TCT_SURFACEDESC:
				s.replace(ofs, 6, _T("_ref."));
				_tcsncpy_s(needed_texnamebuf, texnamebuf_len, s.c_str(), texnamebuf_len);
				return true;
		}
	}

	return false;
}

// C3App initialization

BOOL C3App::InitInstance()
{
#if 0
	MessageBox(NULL, L"attach graphics debugger", L"debug", MB_OK);
#endif

	CWinApp::InitInstance();

	m_C3 = c3::System::Create(0);
	if (!m_C3)
		return FALSE;

	m_C3->GetLog()->SetLogFile(_T("C3App.log"));
	theApp.m_C3->GetLog()->Print(_T("Celerity3 system created\nC3App starting up...\n"));

	c3::Factory *pfactory = m_C3->GetFactory();

	c3::Material::SetAlternateTextureFilenameFunc(AltTextureName);

	theApp.m_C3->GetLog()->Print(_T("Mapping file types... "));

	c3::FileMapper *pfm = m_C3->GetFileMapper();
	pfm->AddMapping(_T("tga"), _T("assets\\textures"));
	pfm->AddMapping(_T("png"), _T("assets\\textures"));
	pfm->AddMapping(_T("jpg"), _T("assets\\textures"));

	pfm->AddMapping(_T("fbx"), _T("assets\\models"));
	pfm->AddMapping(_T("gltf"), _T("assets\\models"));
	pfm->AddMapping(_T("obj"), _T("assets\\models"));

	pfm->AddMapping(_T("vsh"), _T("assets\\shaders"));
	pfm->AddMapping(_T("fsh"), _T("assets\\shaders"));
	pfm->AddMapping(_T("gsh"), _T("assets\\shaders"));

	theApp.m_C3->GetLog()->Print(_T("done\n"));

	c3::Prototype *pproto;

	theApp.m_C3->GetLog()->Print(_T("Creating prototypes... "));

	pproto = pfactory->CreatePrototype();
	pproto->SetName(_T("Sponza"));
	pproto->AddFeature(c3::Positionable::Type());
	pproto->AddFeature(c3::ModelRenderer::Type());
	pproto->GetProperties()->CreateProperty(_T("VertexShader"), 'VSHF')->SetString(_T("def-obj.vsh"));
	pproto->GetProperties()->CreateProperty(_T("FragmentShader"), 'FSHF')->SetString(_T("def-obj.fsh"));
	pproto->GetProperties()->CreateProperty(_T("Model"), 'MODF')->SetString(_T("sponza\\sponza.gltf"));

	pproto = pfactory->CreatePrototype();
	pproto->SetName(_T("AH64e"));
	pproto->AddFeature(c3::Positionable::Type());
	pproto->AddFeature(c3::ModelRenderer::Type());
	pproto->GetProperties()->CreateProperty(_T("VertexShader"), 'VSHF')->SetString(_T("def-obj.vsh"));
	pproto->GetProperties()->CreateProperty(_T("FragmentShader"), 'FSHF')->SetString(_T("def-obj.fsh"));
	pproto->GetProperties()->CreateProperty(_T("Model"), 'MODF')->SetString(_T("ah64e\\ah64e.fbx"));

	pproto = pfactory->CreatePrototype();
	pproto->SetName(_T("Light"));
	pproto->AddFeature(c3::Positionable::Type());
	pproto->AddFeature(c3::OmniLight::Type());
	pproto->GetProperties()->CreateProperty(_T("VertexShader"), 'VSHF')->SetString(_T("def-omnilight.vsh"));
	pproto->GetProperties()->CreateProperty(_T("FragmentShader"), 'FSHF')->SetString(_T("def-omnilight.fsh"));
	pproto->Flags().SetAll(c3::Object::OBJFLAG(c3::Object::UPDATE) | c3::Object::OBJFLAG(c3::Object::DRAWINEDITOR) | c3::Object::OBJFLAG(c3::Object::LIGHT));

	theApp.m_C3->GetLog()->Print(_T("done\n"));

	// Standard initialization

	SetRegistryKey(_T("Celerity"));

	C3Dlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}



int C3App::ExitInstance()
{
	if (m_C3)
	{
		m_C3->Release();
		m_C3 = nullptr;
	}

	return CWinApp::ExitInstance();
}
