// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "C3Edit.h"
#include "C3EditFrame.h"

#include "C3EditDoc.h"
#include "C3EditView.h"

#include "GraphEditorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::vector<std::pair<const TCHAR *, uint64_t>> FlagInfo =
{
	{ _T("Update"), OF_UPDATE },
	{ _T("Draw"), OF_DRAW },
	{ _T("Draw in Editor"), OF_DRAWINEDITOR },
	{ _T("Check Collisions"), OF_CHECKCOLLISIONS },
	{ _T("Track Camera X"), OF_TRACKCAMX },
	{ _T("Track Camera Y"), OF_TRACKCAMY },
	{ _T("Track Camera Z"), OF_TRACKCAMZ },
	{ _T("Literal Camera Tracking"), OF_TRACKCAMLITERAL },
	{ _T("Billboard"), OF_BILLBOARD },
	{ _T("Light Source"), OF_LIGHT },
	{ _T("Cast Shadow"), OF_CASTSHADOW },
	{ _T("Locked"), OF_LOCKED },
};

// C3EditApp

BEGIN_MESSAGE_MAP(C3EditApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &C3EditApp::OnAppAbout)
	ON_COMMAND(ID_FILE_NEW_FRAME, &C3EditApp::OnFileNewFrame)
	ON_COMMAND(ID_FILE_NEW, &C3EditApp::OnFileNew)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
    ON_COMMAND(ID_VIEW_GRAPHEDITOR, &C3EditApp::OnViewGrapheditor)
END_MESSAGE_MAP()


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


// C3EditApp construction

C3EditApp::C3EditApp() noexcept
{
	m_C3 = nullptr;
	m_Config = nullptr;

	SetAppID(_T("Celerity.C3Edit"));
}

// The one and only C3EditApp object

C3EditApp theApp;


// C3EditApp initialization

BOOL C3EditApp::InitInstance()
{
	CoInitialize(NULL);

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	AfxInitRichEdit5();

	CWinAppEx::InitInstance();

	m_C3 = c3::System::Create(NULL, 0);
	if (!m_C3)
		return FALSE;

	PWSTR appdata = nullptr;
	if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &appdata) == S_OK)
	{
		TCHAR *appdatat;
		CONVERT_WCS2TCS(appdata, appdatat);
		CoTaskMemFree(appdata);

		m_AppDataRoot = appdatat;
		std::replace(m_AppDataRoot.begin(), m_AppDataRoot.end(), _T('\\'), _T('/'));
		std::transform(m_AppDataRoot.begin(), m_AppDataRoot.end(), m_AppDataRoot.begin(), tolower);
		m_AppDataRoot += _T("/Celerity/C3Edit/");

		CreateDirectories(m_AppDataRoot.c_str());
	}

	tstring cfgpath = m_AppDataRoot;
	cfgpath += _T("C3Edit.config");
	m_Config = m_C3->CreateConfiguration(cfgpath.c_str());

	theApp.m_C3->GetLog()->Print(_T("Mapping file types..."));

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

	respaths = m_Config->GetString(_T("resources.animations.paths"), _T("./;./assets;./assets/animations"));
	resexts = m_Config->GetString(_T("resources.animations.extensions"), _T("c3anim;xaf"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.animstates.paths"), _T("./;./assets;./assets/animations"));
	resexts = m_Config->GetString(_T("resources.animstates.extensions"), _T("c3states"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.scripts.paths"), _T("./;./assets;./assets/scripts"));
	resexts = m_Config->GetString(_T("resources.scripts.extensions"), _T("c3js"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.sounds.paths"), _T("./;./assets;./assets/sounds"));
	resexts = m_Config->GetString(_T("resources.sounds.extensions"), _T("wav;mp3;ogg"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	respaths = m_Config->GetString(_T("resources.packfiles.paths"), _T("./;./assets;./assets/models;./assets/textures;./assets/shaders;./assets/scripts;./assets/animations;./assets/sounds;./assets/levels"));
	resexts = m_Config->GetString(_T("resources.packfiles.extensions"), _T("zip;c3z"));
	pfm->SetMappingsFromDelimitedStrings(resexts.c_str(), respaths.c_str(), _T(';'));

	theApp.m_C3->GetLog()->Print(_T(" done.\n"));

	c3::Factory *pf = m_C3->GetFactory();
	if (!pf)
		return FALSE;

	c3::Material::SetAlternateTextureFilenameFunc(AltTextureName);

	theApp.m_C3->GetLog()->Print(_T("Loading Prototypes..."));

	for (size_t q = 0; q < pfm->GetNumPaths(_T("c3protoa")); q++)
	{
		tstring ppath = pfm->GetPath(_T("c3protoa"), q);
		tstring spath = ppath;
		spath.append(_T("*.c3protoa"));

		WIN32_FIND_DATA fd;
		HANDLE hff = FindFirstFile(spath.c_str(), &fd);
		if (hff != INVALID_HANDLE_VALUE)
		{
			do
			{
				tstring tpath = ppath;
				tpath.append(fd.cFileName);
				pf->LoadPrototypes((tinyxml2::XMLDocument *)nullptr, tpath.c_str());
			}
			while (FindNextFile(hff, &fd));

			FindClose(hff);
		}
	}

	theApp.m_C3->GetLog()->Print(_T(" done.\n"));

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

	theApp.m_C3->GetLog()->Print(_T("Loading Plugins..."));

	c3::PluginManager *ppm = m_C3->GetPluginManager();
	if (!ppm)
		return FALSE;

	ppm->DiscoverPlugins(m_Config->GetString(_T("resources.plugin.path"), _T("./")));	// scan in app data
	ppm->DiscoverPlugins();																// scan locally

	theApp.m_C3->GetLog()->Print(_T(" done.\n"));

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Celerity\\C3Edit"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	InitContextMenuManager();
	InitShellManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(C3EditDoc),
		RUNTIME_CLASS(C3EditFrame),       // main SDI frame window
		RUNTIME_CLASS(C3EditView));
	if (!pDocTemplate)
		return FALSE;
	m_pDocTemplate = pDocTemplate;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_C3->SetOwner(m_pMainWnd->GetSafeHwnd());
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	return TRUE;
}

// C3EditApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void C3EditApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// C3EditApp customization load/save methods

void C3EditApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;

	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);

	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);

	bNameValid = strName.LoadString(IDS_POPUP_GRAPH);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_GRAPH);

	bNameValid = strName.LoadString(IDS_SCRIPTING_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_SCRIPTING);

	bNameValid = strName.LoadString(IDS_RESOURCESPY_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_RESOURCESPY);
}

void C3EditApp::LoadCustomState()
{
}

void C3EditApp::SaveCustomState()
{
}

// C3EditApp message handlers

void C3EditApp::OnFileNewFrame()
{
	ASSERT(m_pDocTemplate != nullptr);

	CDocument* pDoc = nullptr;
	CFrameWnd* pFrame = nullptr;

	// Create a new instance of the document referenced
	// by the m_pDocTemplate member.
	if (m_pDocTemplate != nullptr)
		pDoc = m_pDocTemplate->CreateNewDocument();

	if (pDoc != nullptr)
	{
		// If creation worked, use create a new frame for
		// that document.
		pFrame = m_pDocTemplate->CreateNewFrame(pDoc, nullptr);
		if (pFrame != nullptr)
		{
			// Set the title, and initialize the document.
			// If document initialization fails, clean-up
			// the frame window and document.

			m_pDocTemplate->SetDefaultTitle(pDoc);
			if (!pDoc->OnNewDocument())
			{
				pFrame->DestroyWindow();
				pFrame = nullptr;
			}
			else
			{
				// Otherwise, update the frame
				m_pDocTemplate->InitialUpdateFrame(pFrame, pDoc, TRUE);
			}
		}
	}

	// If we failed, clean up the document and show a
	// message to the user.

	if (pFrame == nullptr || pDoc == nullptr)
	{
		delete pDoc;
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
	}
}

void C3EditApp::OnFileNew()
{
	CDocument* pDoc = nullptr;
	CFrameWnd* pFrame;
	pFrame = DYNAMIC_DOWNCAST(CFrameWnd, CWnd::GetActiveWindow());

	if (pFrame != nullptr)
		pDoc = pFrame->GetActiveDocument();

	if (pFrame == nullptr || pDoc == nullptr)
	{
		// if it's the first document, create as normal
		CWinApp::OnFileNew();
	}
	else
	{
		// Otherwise, see if we have to save modified, then
		// ask the document to reinitialize itself.
		if (!pDoc->SaveModified())
			return;

		CDocTemplate* pTemplate = pDoc->GetDocTemplate();
		ASSERT(pTemplate != nullptr);

		if (pTemplate != nullptr)
			pTemplate->SetDefaultTitle(pDoc);
		pDoc->OnNewDocument();
	}
}




int C3EditApp::ExitInstance()
{
	m_Config->Release();

	m_C3->Release();

	return CWinAppEx::ExitInstance();
}


void C3EditApp::SetActiveProperties(props::IPropertySet *props, bool readonly, const TCHAR *title)
{
	if (m_pMainWnd && m_pMainWnd->GetSafeHwnd())
		((C3EditFrame *)m_pMainWnd)->SetActiveProperties(props, readonly, title);
}

void C3EditApp::SetActivePrototype(c3::Prototype *pproto, bool readonly, const TCHAR *title)
{
	if (m_pMainWnd && m_pMainWnd->GetSafeHwnd())
		((C3EditFrame *)m_pMainWnd)->SetActivePrototype(pproto, readonly, title);
}

void C3EditApp::SetActiveObject(c3::Object *pobj, bool readonly, const TCHAR *title)
{
	if (m_pMainWnd && m_pMainWnd->GetSafeHwnd())
		((C3EditFrame *)m_pMainWnd)->SetActiveObject(pobj, readonly, title);
}

c3::Object *C3EditApp::GetActiveObject() const
{
	if (m_pMainWnd && m_pMainWnd->GetSafeHwnd())
		return ((C3EditFrame *)m_pMainWnd)->GetActiveObject();

	return nullptr;
}

void C3EditApp::RefreshActiveProperties()
{
	if (m_pMainWnd && m_pMainWnd->GetSafeHwnd())
		((C3EditFrame *)m_pMainWnd)->RefreshActiveProperties();
}


void C3EditApp::UpdateObjectList()
{
	C3EditFrame *pef = (C3EditFrame *)m_pMainWnd;
	if (pef->GetSafeHwnd() && pef->m_wndObjects.GetSafeHwnd())
		pef->m_wndObjects.UpdateContents();
}


void C3EditApp::UpdateStatusMessage(const TCHAR *msg)
{
	C3EditFrame *pef = (C3EditFrame *)m_pMainWnd;
	if (pef->GetSafeHwnd() && pef->m_wndStatusBar.GetSafeHwnd())
		pef->m_wndStatusBar.SetPaneText(0, msg);
}


void C3EditApp::OnViewGrapheditor()
{
	CGraphEditorDlg *pged = CGraphEditorDlg::DoModeless(this->GetMainWnd());
}


void C3EditApp::PushUndo(SUndoInfo::UNDOTYPE type, const tstring &data)
{
	m_Undo.emplace_back();
	//m_Undo.back();
}


void C3EditApp::PopUndo()
{
	if (m_Undo.empty())
		return;
#if 0
	switch (m_Undo.back().type)
	{
		case SUndoInfo::UNDOTYPE::ADD_COMPONENT:
			break;
	}
#endif
	m_Redo.push_back(m_Undo.back());
	m_Undo.pop_back();
}


void C3EditApp::ExtractPaths(const TCHAR *allpaths, std::vector<tstring> &paths)
{
	if (!allpaths)
		return;

	tstring p;

	TCHAR c;
	while ((c = *allpaths) != 0)
	{
		while (((c = *allpaths) != 0) && (c != _T(';')))
		{
			p.push_back(c);
			allpaths++;
		}

		if (!p.empty())
		{

			paths.push_back(p);
			p.clear();
		}

		if (*allpaths == _T(';'))
			allpaths++;
	}
}
