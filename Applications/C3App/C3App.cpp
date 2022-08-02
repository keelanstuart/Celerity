
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
	m_Cfg = nullptr;
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

	m_C3 = c3::System::Create(NULL, 0);
	if (!m_C3)
		return FALSE;

	CString appname;
	appname.LoadStringW(IDS_APPNAME);

	CString cfgname = appname + _T(".cfg");
	m_Cfg = m_C3->CreateConfiguration(cfgname);

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
	pfm->AddMapping(_T("3ds"), _T("assets\\models"));

	pfm->AddMapping(_T("vsh"), _T("assets\\shaders"));
	pfm->AddMapping(_T("fsh"), _T("assets\\shaders"));
	pfm->AddMapping(_T("gsh"), _T("assets\\shaders"));

	pfm->AddMapping(_T("c3protoa"), _T("assets"));

	theApp.m_C3->GetLog()->Print(_T("done\n"));
	
#if 1
	theApp.m_C3->GetLog()->Print(_T("Loading prototypes... "));

	for (size_t q = 0; q < pfm->GetNumPaths(_T("c3protoa")); q++)
	{
		tstring ppath = pfm->GetPath(_T("c3protoa"), q);
		tstring spath = ppath + _T("*.c3protoa");

		WIN32_FIND_DATA fd;
		HANDLE hff = FindFirstFile(spath.c_str(), &fd);
		if (hff != INVALID_HANDLE_VALUE)
		{
			do
			{
				tstring fpath = ppath + fd.cFileName;
				char *fp;
				CONVERT_TCS2MBCS(fpath.c_str(), fp);

				tinyxml2::XMLDocument protodoc;
				protodoc.LoadFile(fp);
				pfactory->LoadPrototypes(protodoc.RootElement());
			}
			while (FindNextFile(hff, &fd));

			FindClose(hff);
		}
	}
#else
	c3::Prototype *pproto;

	theApp.m_C3->GetLog()->Print(_T("Creating prototypes... "));

	{
		pproto = pfactory->CreatePrototype();
		pproto->SetName(_T("Sponza"));
		pproto->AddComponent(c3::Positionable::Type());
		pproto->AddComponent(c3::ModelRenderer::Type());
		pproto->GetProperties()->CreateProperty(_T("VertexShader"), 'VSHF')->SetString(_T("def-obj.vsh"));
		pproto->GetProperties()->CreateProperty(_T("FragmentShader"), 'FSHF')->SetString(_T("def-obj.fsh"));
		pproto->GetProperties()->CreateProperty(_T("ShadowVertexShader"), 'VSSF')->SetString(_T("def-obj-shadow.vsh"));
		pproto->GetProperties()->CreateProperty(_T("ShadowFragmentShader"), 'FSSF')->SetString(_T("def-obj-shadow.fsh"));
		pproto->GetProperties()->CreateProperty(_T("Model"), 'MODF')->SetString(_T("sponza\\sponza.gltf"));
		glm::fquat qy = glm::angleAxis(glm::radians(0.0f), glm::fvec3(0, 0, 1));
		glm::fquat qp = glm::angleAxis(glm::radians(90.0f), glm::fvec3(1, 0, 0));
		glm::fquat qr = glm::angleAxis(glm::radians(0.0f), glm::fvec3(0, 1, 0));
		glm::fquat ori = (qr * qp) * qy;
		pproto->GetProperties()->CreateProperty(_T("Orientation"), 'ORI')->SetVec4F(props::TVec4F(ori.x, ori.y, ori.z, ori.w));
		pproto->GetProperties()->CreateProperty(_T("Scale"), 'SCL')->SetVec3F(props::TVec3F(50.0f, 50.0f, 50.0f));
		pproto->Flags().Set(c3::Object::OBJFLAG(c3::Object::CASTSHADOW) | c3::Object::OBJFLAG(c3::Object::DRAW));
	}

	{
		pproto = pfactory->CreatePrototype();
		pproto->SetName(_T("AH64e"));
		pproto->AddComponent(c3::Positionable::Type());
		pproto->AddComponent(c3::ModelRenderer::Type());
		pproto->GetProperties()->CreateProperty(_T("VertexShader"), 'VSHF')->SetString(_T("def-obj.vsh"));
		pproto->GetProperties()->CreateProperty(_T("FragmentShader"), 'FSHF')->SetString(_T("def-obj.fsh"));
		pproto->GetProperties()->CreateProperty(_T("ShadowVertexShader"), 'VSSF')->SetString(_T("def-obj-shadow.vsh"));
		pproto->GetProperties()->CreateProperty(_T("ShadowFragmentShader"), 'FSSF')->SetString(_T("def-obj-shadow.fsh"));
		pproto->GetProperties()->CreateProperty(_T("Model"), 'MODF')->SetString(_T("ah64e\\ah64e.fbx"));
		glm::fquat qy = glm::angleAxis(glm::radians(90.0f), glm::fvec3(0, 0, 1));
		glm::fquat qp = glm::angleAxis(glm::radians(0.0f), glm::fvec3(1, 0, 0));
		glm::fquat qr = glm::angleAxis(glm::radians(0.0f), glm::fvec3(0, 1, 0));
		glm::fquat ori = (qr * qp) * qy;
		pproto->GetProperties()->CreateProperty(_T("ModelOrientation"), 'MORI')->SetVec4F(props::TVec4F(ori.x, ori.y, ori.z, ori.w));
		pproto->GetProperties()->CreateProperty(_T("ModelScale"), 'MSCL')->SetVec3F(props::TVec3F(0.08f, 0.08f, 0.08f));
		pproto->GetProperties()->CreateProperty(_T("Position"), 'POS')->SetVec3F(props::TVec3F(0.0f, 0.0f, 80.0f));
		pproto->Flags().Set(c3::Object::OBJFLAG(c3::Object::CASTSHADOW) | c3::Object::OBJFLAG(c3::Object::UPDATE) | c3::Object::OBJFLAG(c3::Object::DRAW));
	}

	{
		pproto = pfactory->CreatePrototype();
		pproto->SetName(_T("Racecar A"));
		pproto->AddComponent(c3::Positionable::Type());
		pproto->AddComponent(c3::ModelRenderer::Type());
		pproto->GetProperties()->CreateProperty(_T("VertexShader"), 'VSHF')->SetString(_T("def-obj.vsh"));
		pproto->GetProperties()->CreateProperty(_T("FragmentShader"), 'FSHF')->SetString(_T("def-obj.fsh"));
		pproto->GetProperties()->CreateProperty(_T("ShadowVertexShader"), 'VSSF')->SetString(_T("def-obj-shadow.vsh"));
		pproto->GetProperties()->CreateProperty(_T("ShadowFragmentShader"), 'FSSF')->SetString(_T("def-obj-shadow.fsh"));
		pproto->GetProperties()->CreateProperty(_T("Model"), 'MODF')->SetString(_T("race.fbx"));
		glm::fquat qy = glm::angleAxis(glm::radians(0.0f), glm::fvec3(0, 0, 1));
		glm::fquat qp = glm::angleAxis(glm::radians(90.0f), glm::fvec3(1, 0, 0));
		glm::fquat qr = glm::angleAxis(glm::radians(0.0f), glm::fvec3(0, 1, 0));
		glm::fquat ori = (qr * qp) * qy;
		pproto->GetProperties()->CreateProperty(_T("ModelOrientation"), 'MORI')->SetVec4F(props::TVec4F(ori.x, ori.y, ori.z, ori.w));
		pproto->GetProperties()->CreateProperty(_T("ModelScale"), 'MSCL')->SetVec3F(props::TVec3F(1.0f, 1.0f, 1.0f));
		pproto->GetProperties()->CreateProperty(_T("Position"), 'POS')->SetVec3F(props::TVec3F(0.0f, 0.0f, 80.0f));
		pproto->Flags().Set(c3::Object::OBJFLAG(c3::Object::CASTSHADOW) | c3::Object::OBJFLAG(c3::Object::UPDATE) | c3::Object::OBJFLAG(c3::Object::DRAW));
	}

	WIN32_FIND_DATA ffd;
	HANDLE ffh = FindFirstFile(_T("D:\\temp\\Celerity\\bin\\assets\\Models\\Hexagons\\*.fbx"), &ffd);
	if (ffh != INVALID_HANDLE_VALUE)
	{
		do
		{
			tstring fname = _T("hexagons/");
			fname += ffd.cFileName;
			tstring pname = ffd.cFileName;
			tstring::iterator cit;
			for (cit = pname.begin(); cit != pname.end(); cit++)
			{
				if (*cit == _T('.'))
					break;
				if (!isalnum(*cit))
					*cit = _T(' ');
				if ((cit == pname.begin()) || (*(cit - 1) == _T(' ')))
					*cit = toupper(*cit);
			}
			if ((cit != pname.begin()) && (cit != pname.end()))
				pname.erase(cit, pname.end());

			pproto = pfactory->CreatePrototype();
			pproto->SetName(pname.c_str());
			pproto->AddComponent(c3::Positionable::Type());
			pproto->AddComponent(c3::ModelRenderer::Type());
			pproto->GetProperties()->CreateProperty(_T("VertexShader"), 'VSHF')->SetString(_T("def-obj.vsh"));
			pproto->GetProperties()->CreateProperty(_T("FragmentShader"), 'FSHF')->SetString(_T("def-obj.fsh"));
			pproto->GetProperties()->CreateProperty(_T("ShadowVertexShader"), 'VSSF')->SetString(_T("def-obj-shadow.vsh"));
			pproto->GetProperties()->CreateProperty(_T("ShadowFragmentShader"), 'FSSF')->SetString(_T("def-obj-shadow.fsh"));
			pproto->GetProperties()->CreateProperty(_T("Model"), 'MODF')->SetString(fname.c_str());
			glm::fquat qy = glm::angleAxis(glm::radians(0.0f), glm::fvec3(0, 0, 1));
			glm::fquat qp = glm::angleAxis(glm::radians(90.0f), glm::fvec3(1, 0, 0));
			glm::fquat qr = glm::angleAxis(glm::radians(0.0f), glm::fvec3(0, 1, 0));
			glm::fquat ori = (qr * qp) * qy;
			pproto->GetProperties()->CreateProperty(_T("ModelOrientation"), 'MORI')->SetVec4F(props::TVec4F(ori.x, ori.y, ori.z, ori.w));
			pproto->GetProperties()->CreateProperty(_T("ModelScale"), 'MSCL')->SetVec3F(props::TVec3F(1.0f, 1.0f, 1.0f));
			pproto->GetProperties()->CreateProperty(_T("ModelPosition"), 'MPOS')->SetVec3F(props::TVec3F(0.0f, 0.0f, 0.0f));
			pproto->Flags().Set(c3::Object::OBJFLAG(c3::Object::CASTSHADOW) | c3::Object::OBJFLAG(c3::Object::UPDATE) | c3::Object::OBJFLAG(c3::Object::DRAW));
		} while (FindNextFile(ffh, &ffd));

		FindClose(ffh);
	}

	pproto = pfactory->CreatePrototype();
	pproto->SetName(_T("TestBox"));
	pproto->AddComponent(c3::Positionable::Type());
	pproto->AddComponent(c3::ModelRenderer::Type());
	pproto->GetProperties()->CreateProperty(_T("VertexShader"), 'VSHF')->SetString(_T("def-obj.vsh"));
	pproto->GetProperties()->CreateProperty(_T("FragmentShader"), 'FSHF')->SetString(_T("def-obj.fsh"));
	pproto->GetProperties()->CreateProperty(_T("ShadowVertexShader"), 'VSSF')->SetString(_T("def-obj-shadow.vsh"));
	pproto->GetProperties()->CreateProperty(_T("ShadowFragmentShader"), 'FSSF')->SetString(_T("def-obj-shadow.fsh"));
	pproto->GetProperties()->CreateProperty(_T("Model"), 'MODF')->SetString(_T("TestBox\\testbox.3ds"));
	pproto->GetProperties()->CreateProperty(_T("Position"), 'POS')->SetVec3F(props::TVec3F(0.0f, 0.0f, 80.0f));
	pproto->Flags().Set(c3::Object::OBJFLAG(c3::Object::CASTSHADOW) | c3::Object::OBJFLAG(c3::Object::DRAW));

	pproto = pfactory->CreatePrototype();
	pproto->SetName(_T("Light"));
	pproto->AddComponent(c3::Positionable::Type());
	pproto->AddComponent(c3::OmniLight::Type());
	pproto->GetProperties()->CreateProperty(_T("VertexShader"), 'VSHF')->SetString(_T("def-omnilight.vsh"));
	pproto->GetProperties()->CreateProperty(_T("FragmentShader"), 'FSHF')->SetString(_T("def-omnilight.fsh"));
	pproto->GetProperties()->CreateProperty(_T("AttenGradientTexture"), 'GRAD')->SetString(_T("LightAtten.png"));
	pproto->Flags().SetAll(c3::Object::OBJFLAG(c3::Object::UPDATE) | c3::Object::OBJFLAG(c3::Object::DRAWINEDITOR) | c3::Object::OBJFLAG(c3::Object::LIGHT));

	pproto = pfactory->CreatePrototype();
	pproto->SetName(_T("Terrain"));
	pproto->AddComponent(c3::Positionable::Type());
	pproto->AddComponent(c3::QuadTerrain::Type());
	pproto->GetProperties()->CreateProperty(_T("VertexShader"), 'VSHF')->SetString(_T("def-terrain.vsh"));
	pproto->GetProperties()->CreateProperty(_T("FragmentShader"), 'FSHF')->SetString(_T("def-terrain.fsh"));
	pproto->GetProperties()->CreateProperty(_T("ShadowVertexShader"), 'VSSF')->SetString(_T("def-terrain-shadow.vsh"));
	pproto->GetProperties()->CreateProperty(_T("ShadowFragmentShader"), 'FSSF')->SetString(_T("def-obj-shadow.fsh"));
	pproto->GetProperties()->CreateProperty(_T("ModelScale"), 'MSCL')->SetVec3F(props::TVec3F(50.0f, 50.0f, 50.0f));
	pproto->Flags().SetAll(c3::Object::OBJFLAG(c3::Object::CASTSHADOW) | c3::Object::OBJFLAG(c3::Object::UPDATE) | c3::Object::OBJFLAG(c3::Object::DRAW));

	tinyxml2::XMLDocument protodoc;
	tinyxml2::XMLElement *protoroot = protodoc.NewElement("prototypes");
	protodoc.InsertEndChild(protoroot);
	pfactory->SavePrototypes(protoroot);
	protodoc.SaveFile("assets/sample_protos.c3protoa");

#endif

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
	if (m_Cfg)
	{
		m_Cfg->Release();
		m_Cfg = nullptr;
	}

	if (m_C3)
	{
		m_C3->Release();
		m_C3 = nullptr;
	}

	return CWinApp::ExitInstance();
}
