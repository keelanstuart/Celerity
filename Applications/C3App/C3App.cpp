// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


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
	m_Config = nullptr;
}


// The one and only C3App object

C3App theApp;


bool CreateDirectories(const TCHAR *dir)
{
	if (!dir || !*dir)
		return false;

	if (PathFileExists(dir))
		return true;

	if (PathIsRoot(dir))
		return false;

	bool ret = true;

	TCHAR _dir[MAX_PATH], *d = _dir;
	_tcscpy_s(_dir, dir);
	while (d && *(d++)) { if (*d == _T('/')) *d = _T('\\'); }
	PathRemoveFileSpec(_dir);

	// it's a network path and this is the network device... don't try to create it and don't try to go any further
	if (!_tcscmp(_dir, _T("\\\\")) || !_tcscmp(_dir, _T("//")))
		return true;

	ret &= CreateDirectories(_dir);

	ret &= (CreateDirectory(dir, NULL) ? true : false);

	return ret;
}

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

	ofs = s.find(tstring(_T("_diffuse.")), 0);
	if (ofs != std::string::npos)
	{
		switch (typeneeded)
		{
			case c3::Material::TCT_NORMAL:
				s.replace(ofs, 6, _T("_normal."));
				_tcsncpy_s(needed_texnamebuf, texnamebuf_len, s.c_str(), texnamebuf_len);
				return true;

			case c3::Material::TCT_EMISSIVE:
				s.replace(ofs, 6, _T("_emissive."));
				_tcsncpy_s(needed_texnamebuf, texnamebuf_len, s.c_str(), texnamebuf_len);
				return true;

			case c3::Material::TCT_SURFACEDESC:
				s.replace(ofs, 6, _T("_mtlpack."));
				_tcsncpy_s(needed_texnamebuf, texnamebuf_len, s.c_str(), texnamebuf_len);
				return true;
		}
	}

	// if we aren't look for a diffuse texture and we didn't find a diffuse-identifying name, then just
	// insert a special name before the dot
	if (typeneeded != c3::Material::TCT_DIFFUSE)
	{
		ofs = s.find(tstring(_T(".")), 0);
		if (ofs != std::string::npos)
		{
			switch (typeneeded)
			{
				case c3::Material::TCT_NORMAL:
					s.replace(ofs, 1, _T("_norm."));
					_tcsncpy_s(needed_texnamebuf, texnamebuf_len, s.c_str(), texnamebuf_len);
					return true;

				case c3::Material::TCT_EMISSIVE:
					s.replace(ofs, 1, _T("_emis."));
					_tcsncpy_s(needed_texnamebuf, texnamebuf_len, s.c_str(), texnamebuf_len);
					return true;

				case c3::Material::TCT_SURFACEDESC:
					s.replace(ofs, 1, _T("_ref."));
					_tcsncpy_s(needed_texnamebuf, texnamebuf_len, s.c_str(), texnamebuf_len);
					return true;
			}
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

	srand(GetTickCount());

	m_C3 = c3::System::Create(NULL, 0);
	if (!m_C3)
		return FALSE;

	CString appname;
	appname.LoadStringW(IDS_APPNAME);

	PWSTR appdata = nullptr;
	if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &appdata) == S_OK)
	{
		TCHAR *appdatat;
		CONVERT_WCS2TCS(appdata, appdatat);
		CoTaskMemFree(appdata);

		m_AppDataRoot = appdatat;
		std::replace(m_AppDataRoot.begin(), m_AppDataRoot.end(), _T('\\'), _T('/'));
		std::transform(m_AppDataRoot.begin(), m_AppDataRoot.end(), m_AppDataRoot.begin(), tolower);
		m_AppDataRoot += _T("/Celerity/C3App/");

		CreateDirectories(m_AppDataRoot.c_str());
	}

	tstring cfgpath = m_AppDataRoot;
	cfgpath += appname;
	cfgpath += _T(".config");
	m_Config = m_C3->CreateConfiguration(cfgpath.c_str());

	m_C3->GetLog()->SetLogFile(_T("C3App.log"));
	theApp.m_C3->GetLog()->Print(_T("Celerity3 system created\nC3App starting up...\n"));

	c3::Factory *pfactory = m_C3->GetFactory();

	c3::Material::SetAlternateTextureFilenameFunc(AltTextureName);

	theApp.m_C3->GetLog()->Print(_T("Mapping file types... "));

	c3::FileMapper *pfm = m_C3->GetFileMapper();

	tstring respaths, resexts;

	respaths = m_Config->GetString(_T("resources.textures.paths"), _T("./;./assets;./assets/textures"));
	resexts = m_Config->GetString(_T("resources.textures.extensions"), _T("tga;png;jpg;dds;bmp;tif"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.models.paths"), _T("./;./assets;./assets/models"));
	resexts = m_Config->GetString(_T("resources.models.extensions"), _T("fbx;gltf;glb;obj;3ds;dae;x"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.shaders.paths"), _T("./;./assets;./assets/shaders"));
	resexts = m_Config->GetString(_T("resources.shaders.extensions"), _T("vsh;fsh;gsh;esh;tsh"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.rendermethods.paths"), _T("./;./assets;./assets/shaders"));
	resexts = m_Config->GetString(_T("resources.rendermethods.extensions"), _T("c3rm"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.prototypes.paths"), _T("./;./assets"));
	resexts = m_Config->GetString(_T("resources.prototypes.extensions"), _T("c3protoa"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.scripts.paths"), _T("./;./assets;./assets/scripts"));
	resexts = m_Config->GetString(_T("resources.scripts.extensions"), _T("c3js"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.levels.paths"), _T("./;./assets;./assets/levels"));
	resexts = m_Config->GetString(_T("resources.levels.extensions"), _T("c3o"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.sound.paths"), _T("./;./assets;./assets/sound"));
	resexts = m_Config->GetString(_T("resources.sound.extensions"), _T("wav;mp3;flac"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	theApp.m_C3->GetLog()->Print(_T("done\n"));

	theApp.m_C3->GetLog()->Print(_T("Loading prototypes..."));

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

	theApp.m_C3->GetLog()->Print(_T(" done\n"));

	theApp.m_C3->GetResourceManager()->RegisterZipArchive(_T("./assets/models/hexagons.zip"));

	theApp.m_C3->GetLog()->Print(_T("Loading Plugins..."));

	c3::PluginManager *ppm = m_C3->GetPluginManager();
	if (!ppm)
		return FALSE;

	ppm->DiscoverPlugins(m_Config->GetString(_T("resources.plugin.path"), _T("./")));	// scan in app data
	ppm->DiscoverPlugins();															// scan locally

	theApp.m_C3->GetLog()->Print(_T(" done\n"));

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
	if (m_Config)
	{
		m_Config->Release();
		m_Config = nullptr;
	}

	if (m_C3)
	{
		m_C3->Release();
		m_C3 = nullptr;
	}

	return CWinApp::ExitInstance();
}
