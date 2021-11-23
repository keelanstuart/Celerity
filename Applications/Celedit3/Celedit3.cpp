// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface
// (the "Fluent UI") and is provided only as referential material to supplement the
// Microsoft Foundation Classes Reference and related electronic documentation
// included with the MFC C++ library software.
// License terms to copy, use or distribute the Fluent UI are available separately.
// To learn more about our Fluent UI licensing program, please visit
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// Celedit3.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "Celedit3.h"
#include "MainFrm.h"

#include "Celedit3Doc.h"
#include "Celedit3View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCeledit3App

BEGIN_MESSAGE_MAP(CCeledit3App, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CCeledit3App::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()


// CCeledit3App construction

CCeledit3App::CCeledit3App() noexcept
{
	m_C3 = nullptr;
	m_Config = nullptr;

	m_bHiColorIcons = TRUE;

	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Celedit3.AppID.NoVersion"));
}

// The one and only CCeledit3App object

CCeledit3App theApp;


// CCeledit3App initialization

BOOL CCeledit3App::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	m_C3 = c3::System::Create(0);
	if (!m_C3)
		return FALSE;

	PWSTR appdata = nullptr;
	if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &appdata) == S_OK)
	{
		TCHAR *appdatat;
		CONVERT_WCS2TCS(appdata, appdatat);
		m_AppDataRoot = appdatat;
		m_AppDataRoot += _T("\\Celerity\\Celedit3\\");
		CoTaskMemFree(appdata);
	}

	TCHAR *adr;
	CONVERT_WCS2TCS(m_AppDataRoot.c_str(), adr);

	theApp.m_C3->GetLog()->Print(_T("Mapping file types... "));

	c3::FileMapper *pfm = m_C3->GetFileMapper();

	tstring resroot;

	resroot = _T("assets\\textures");
	pfm->AddMapping(_T("tga"), resroot.c_str());
	pfm->AddMapping(_T("png"), resroot.c_str());
	pfm->AddMapping(_T("jpg"), resroot.c_str());
	resroot.insert(0, m_AppDataRoot.c_str());
	pfm->AddMapping(_T("tga"), resroot.c_str());
	pfm->AddMapping(_T("png"), resroot.c_str());
	pfm->AddMapping(_T("jpg"), resroot.c_str());

	resroot = _T("assets\\models");
	pfm->AddMapping(_T("fbx"), resroot.c_str());
	pfm->AddMapping(_T("gltf"), resroot.c_str());
	pfm->AddMapping(_T("obj"), resroot.c_str());
	pfm->AddMapping(_T("3ds"), resroot.c_str());
	resroot.insert(0, m_AppDataRoot.c_str());
	pfm->AddMapping(_T("fbx"), resroot.c_str());
	pfm->AddMapping(_T("gltf"), resroot.c_str());
	pfm->AddMapping(_T("obj"), resroot.c_str());
	pfm->AddMapping(_T("3ds"), resroot.c_str());

	resroot = _T("assets\\shaders");
	pfm->AddMapping(_T("vsh"), resroot.c_str());
	pfm->AddMapping(_T("fsh"), resroot.c_str());
	pfm->AddMapping(_T("gsh"), resroot.c_str());
	resroot.insert(0, m_AppDataRoot.c_str());
	pfm->AddMapping(_T("vsh"), resroot.c_str());
	pfm->AddMapping(_T("fsh"), resroot.c_str());
	pfm->AddMapping(_T("gsh"), resroot.c_str());

	theApp.m_C3->GetLog()->Print(_T("done\n"));

	m_Config = m_C3->CreateConfiguration(_T("Celedit3.cfg"));

	c3::Factory *pf = m_C3->GetFactory();
	if (!pf)
		return FALSE;

	m_ProtoFilename = m_AppDataRoot;
	m_ProtoFilename += _T("Prototypes.xml");

	if (PathFileExists(m_ProtoFilename.c_str()))
	{
		FILE *protof;
		if (!_tfopen_s(&protof, m_ProtoFilename.c_str(), _T("r, css=UTF-8")))
		{
			int f = _fileno(protof);

#pragma warning(disable: 4312)
			HANDLE h = (f > 0) ? (HANDLE)f : INVALID_HANDLE_VALUE;
#pragma warning(default: 4312)

			genio::IInputStream *pis = genio::IInputStream::Create(h);
			pf->LoadPrototypes(pis);
			pis->Release();
		}
	}

	c3::PluginManager *ppm = m_C3->GetPluginManager();
	if (!ppm)
		return FALSE;

	ppm->DiscoverPlugins();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	SetRegistryKey(_T("Celerity3\\Celedit3"));
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
		RUNTIME_CLASS(CCeledit3Doc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CCeledit3View));
	if (!pDocTemplate)
		return FALSE;
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
	m_pMainWnd->UpdateWindow();

	m_pMainWnd->DragAcceptFiles();

	return TRUE;
}

int CCeledit3App::ExitInstance()
{
	if (m_Config)
	{
		m_Config->Release();
		m_Config = nullptr;
	}

	if (m_C3)
	{
		c3::Renderer *pr = m_C3->GetRenderer();
		if (pr)
			pr->Shutdown();

		m_C3->Release();
		m_C3 = nullptr;
	}

	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CCeledit3App message handlers


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
void CCeledit3App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CCeledit3App customization load/save methods

void CCeledit3App::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CCeledit3App::LoadCustomState()
{
}

void CCeledit3App::SaveCustomState()
{
}

// CCeledit3App message handlers



