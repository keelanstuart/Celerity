// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


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
	m_StartScript = _T("c3demo.c3js");
	m_AlwaysPaint = false;

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


tstring ReplaceFileExtension(const tstring &filename, const tstring &newExtension)
{
	tstring::size_type dotPos = filename.find_last_of('.');

	if (dotPos == std::string::npos)
	{
		// No extension found, just append the new extension
		return filename + newExtension;
	}
	else
	{
		// Replace the extension
		return filename.substr(0, dotPos) + newExtension;
	}
}

// C3App initialization

using SettingCB = std::function<void(const TCHAR *)>;
using SettingsTable = std::map<tstring, SettingCB>;

SettingsTable gSettings =
{
	{ _T("alwayspaint"), [](const TCHAR *opts) { theApp.m_AlwaysPaint = true; } },
	{ _T("resetlog"), [](const TCHAR* opts) { theApp.m_C3->GetLog()->Reset(); }},
};

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

	// Get the user's home path for their configuration
	CString appname;
	appname.LoadStringW(IDS_APPNAME);
	m_AppName = appname;

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

	// make sure the thing we're loading is able to find it's resources
	TCHAR currentDir[MAX_PATH * 2];
	TCHAR targetDir[MAX_PATH * 2];
	TCHAR relativePath[MAX_PATH * 2], relativeParentPath[MAX_PATH * 2];

	// Get the current working directory
	_tgetcwd(currentDir, MAX_PATH);

	// Get the path of the script
	_tcscpy_s(targetDir, m_StartScript.c_str());
	PathRemoveFileSpec(targetDir);

	// Calculate the relative path
	PathRelativePathTo(
		relativePath,
		currentDir,
		FILE_ATTRIBUTE_DIRECTORY,
		targetDir,
		FILE_ATTRIBUTE_DIRECTORY);

	_tcscpy_s(relativeParentPath, relativePath);
	_tcscat_s(relativeParentPath, _T("\\..\\"));

	tstring path_extra = _T(";");
	path_extra += relativePath;
	path_extra += _T(";");
	path_extra += relativeParentPath;

	tstring cfgpath = m_AppDataRoot;
	cfgpath += appname;
	cfgpath += _T(".config");
	m_Config = m_C3->CreateConfiguration(cfgpath.c_str());

	tstring logpath = m_AppDataRoot;
	logpath += appname;
	logpath += _T(".log");
	m_C3->GetLog()->SetLogFile(logpath.c_str());

	const TCHAR *fn = PathFindFileName(theApp.m_AppName.c_str());

	m_StartScript = ReplaceFileExtension(fn, _T(".c3js"));

	// Parse the command line into arguments
	int argc = 0;
	LPTSTR *argv = CommandLineToArgvW(m_lpCmdLine, &argc);
	if (argv)
	{
		for (int a = 0; a < argc; a++)
		{
			// get options
			if (*argv[a] == _T('-'))
			{
				tstring cmd = argv[a] + 1;
				size_t opt_ofs = cmd.find(_T(':'));
				const TCHAR *opt = nullptr;
				if (opt_ofs != tstring::npos)
				{
					opt = cmd.c_str() + opt_ofs + 1;
					cmd[opt_ofs] = _T('\0');
				}
				SettingsTable::const_iterator it = gSettings.find(cmd);
				if (it != gSettings.end())
				{
					it->second(opt);
				}
				else
				{
					m_C3->GetLog()->Print(_T("Unrecognized command-line option: \"%hs\"\n"), cmd);
				}
			}
			else
				m_StartScript = argv[a];
		}

		tstring modpack = ReplaceFileExtension(fn, _T(".c3z"));
		if (theApp.m_C3->GetResourceManager()->RegisterZipArchive(modpack.c_str()))
		{
			if (!PathFileExists(m_StartScript.c_str()))
			{
				m_StartScript.insert(m_StartScript.rfind(_T('.'), 0), _T("/main"));
			}
		}

		// Free memory allocated by CommandLineToArgvW
		LocalFree(argv);
	}

	theApp.m_C3->GetLog()->Print(_T("Celerity3 system created\nC3App starting up...\n"));

	c3::Factory *pfactory = m_C3->GetFactory();

	c3::Material::SetAlternateTextureFilenameFunc(AltTextureName);

	theApp.m_C3->GetLog()->Print(_T("Mapping file types... "));

	c3::FileMapper *pfm = m_C3->GetFileMapper();

	tstring respaths, resexts;

	respaths = m_Config->GetString(_T("resources.textures.paths"), _T("./;./assets;./assets/textures"));
	respaths += path_extra;
	resexts = m_Config->GetString(_T("resources.textures.extensions"), _T("tga;png;jpg;dds;bmp;tif"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.models.paths"), _T("./;./assets;./assets/models"));
	respaths += path_extra;
	resexts = m_Config->GetString(_T("resources.models.extensions"), _T("fbx;gltf;glb;obj;3ds;dae;x;c3terr"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.shaders.paths"), _T("./;./assets;./assets/shaders"));
	respaths += path_extra;
	resexts = m_Config->GetString(_T("resources.shaders.extensions"), _T("vsh;fsh;gsh;esh;tsh"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.rendermethods.paths"), _T("./;./assets;./assets/shaders"));
	respaths += path_extra;
	resexts = m_Config->GetString(_T("resources.rendermethods.extensions"), _T("c3rm"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.prototypes.paths"), _T("./;./assets"));
	respaths += path_extra;
	resexts = m_Config->GetString(_T("resources.prototypes.extensions"), _T("c3protoa"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.scripts.paths"), _T("./;./assets;./assets/scripts"));
	respaths += path_extra;
	resexts = m_Config->GetString(_T("resources.scripts.extensions"), _T("c3js"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.animstates.paths"), _T("./;./assets;./assets/animations"));
	respaths += path_extra;
	resexts = m_Config->GetString(_T("resources.animstates.extensions"), _T("c3states"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.levels.paths"), _T("./;./assets;./assets/levels"));
	respaths += path_extra;
	resexts = m_Config->GetString(_T("resources.levels.extensions"), _T("c3o"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.sound.paths"), _T("./;./assets;./assets/sound"));
	respaths += path_extra;
	resexts = m_Config->GetString(_T("resources.sound.extensions"), _T("wav;mp3;flac"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.packfiles.paths"), _T("./;./assets;./assets/models;./assets/textures;./assets/shaders;./assets/scripts;./assets/animations;./assets/sounds;./assets/levels"));
	respaths += path_extra;
	resexts = m_Config->GetString(_T("resources.packfiles.extensions"), _T("zip;c3z"));
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

	theApp.m_C3->GetLog()->Print(_T("Registering Resource Packages...\n"));
	size_t numrespacks = m_Config->GetNumSubKeys(_T("resources.packfiles.archives"));
	for (size_t rpi = 0; rpi < numrespacks; rpi++)
	{
		TCHAR kn[256];
		_stprintf_s(kn, _T("resources.packfiles.archives.packfile#%zu"), rpi);

		const TCHAR *zp = m_Config->GetString(kn, nullptr);
		if (!zp)
			break;

		theApp.m_C3->GetLog()->Print(_T("\t%s\n"), zp);
		theApp.m_C3->GetResourceManager()->RegisterZipArchive(zp);
	}
	theApp.m_C3->GetLog()->Print(_T("done.\n"));

	// auto-register a c3z file with a name matching the app name ("MyCelerityGame.exe" would try to load "MyCelerityGame.c3z")
	tstring c3zpath = appname;
	c3zpath += _T(".c3z");
	theApp.m_C3->GetResourceManager()->RegisterZipArchive(c3zpath.c_str());

	theApp.m_C3->GetLog()->Print(_T("Loading Plugins..."));

	c3::PluginManager *ppm = m_C3->GetPluginManager();
	if (!ppm)
		return FALSE;

	ppm->DiscoverPlugins(m_Config->GetString(_T("resources.plugin.path"), _T("./")));	// scan in app data
	ppm->DiscoverPlugins();																// scan locally

	theApp.m_C3->GetLog()->Print(_T(" done\n"));

	// Standard initialization
	tstring regkey = _T("CelerityApps\\");
	regkey += appname;
	SetRegistryKey(regkey.c_str());

	// if no start script was given, try to get one with a name matching the app name
	if (m_StartScript.empty())
	{
		m_StartScript = appname;
		m_StartScript += _T(".c3js");
	}

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
