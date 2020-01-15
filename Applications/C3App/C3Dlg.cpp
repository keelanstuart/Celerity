
// C3Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "C3App.h"
#include "C3Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// C3Dlg dialog



C3Dlg::C3Dlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_C3APP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_Rend = nullptr;
	m_Tex = nullptr;
	m_VB = nullptr;
	m_IB = nullptr;
	m_FB = nullptr;
	memset(m_ColorTarg, 0, sizeof(c3::Texture2D *) * 2);;
	m_DepthTarg = nullptr;
}

void C3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(C3Dlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// C3Dlg message handlers

BOOL C3Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_Factory = theApp.m_C3->GetFactory();

	m_Rend = theApp.m_C3->GetRenderer();
	if (!m_Rend)
		exit(-1);

	CRect r;
	GetClientRect(r);
	if (!m_Rend->Initialize(r.Width(), r.Height(), GetSafeHwnd(), 0))
		exit(-2);

#if 1
	m_FB = m_Rend->CreateFrameBuffer();
	if (m_FB)
	{
		size_t w = r.Width() * 2;
		size_t h = r.Height() * 2;

		m_ColorTarg[0] = m_Rend->CreateTexture2D(w, h, c3::Renderer::TextureType::F32_4CH, 1, TEXCREATEFLAG_RENDERTARGET);
		m_ColorTarg[1] = m_Rend->CreateTexture2D(w, h, c3::Renderer::TextureType::F32_4CH, 1, TEXCREATEFLAG_RENDERTARGET);
		m_DepthTarg = m_Rend->CreateDepthBuffer(w, h, c3::Renderer::DepthType::F32_D);

		if (m_ColorTarg[0] && m_ColorTarg[1] && m_DepthTarg)
		{
			m_FB->AttachColorTarget(m_ColorTarg[0], 0);
			m_FB->AttachColorTarget(m_ColorTarg[1], 1);
			m_FB->AttachDepthTarget(m_DepthTarg);

			m_FB->Seal();
		}

//		m_Rend->UseFrameBuffer(m_FB);
	}
#endif

#if 1
	m_Tex = m_Rend->CreateTexture2D(64, 64, c3::Renderer::ETextureType::U8_4CH, 0);
	if (m_Tex)
	{
		c3::Texture2D::SLockInfo li;
		BYTE *buf;
		if (m_Tex->Lock((void **)&buf, li, 0, TEXLOCKFLAG_WRITE) == c3::Texture::RETURNCODE::RET_OK)
		{
			for (size_t y = 0, maxy = li.height; y < maxy; y++)
			{
				for (size_t x = 0, maxx = li.width; x < maxx; x++)
				{
					((uint32_t *)buf)[x] = 0x8000FF00;
				}

				buf += li.stride;
			}

			m_Tex->Unlock();
		}
	}
#endif

	c3::Prototype *pproto = m_Factory->CreatePrototype();
	pproto->AddComportment(m_Factory->FindComportmentType(_T("UIControl")));
	pproto->AddComportment(m_Factory->FindComportmentType(_T("Positionable")));
	pproto->SetName(_T("GenericControl"));

	props::IProperty *pprop = pproto->GetProperties()->CreateProperty(_T("Image"), 'IMG');
	pprop->SetString(_T("default.tga"));
	pprop->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FILENAME);

	m_RootObj = m_Factory->Build(pproto);


	SetTimer('DRAW', 33, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void C3Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		if (m_Rend->BeginScene(0))
		{
			m_RootObj->Update(0);
			if (m_RootObj->Prerender(0))
				if (m_RootObj->Render(0))
					m_RootObj->Postrender(0);

			m_Rend->EndScene(0);
		}

		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR C3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void C3Dlg::Cleanup()
{
	KillTimer('DRAW');

	if (m_Tex)
	{
		m_Tex->Release();
		m_Tex = nullptr;
	}

	if (m_VB)
	{
		m_VB->Release();
		m_VB = nullptr;
	}

	if (m_IB)
	{
		m_IB->Release();
		m_IB = nullptr;
	}

	if (m_ColorTarg[0])
	{
		m_ColorTarg[0]->Release();
		m_ColorTarg[0] = nullptr;
	}

	if (m_ColorTarg[1])
	{
		m_ColorTarg[1]->Release();
		m_ColorTarg[1] = nullptr;
	}

	if (m_DepthTarg)
	{
		m_DepthTarg->Release();
		m_DepthTarg = nullptr;
	}

	if (m_FB)
	{
		m_FB->Release();
		m_FB = nullptr;
	}

	if (m_Rend)
	{
		m_Rend->Shutdown();
		m_Rend = nullptr;
	}

	if (m_RootObj)
	{
		m_RootObj->Release();
		m_RootObj = nullptr;
	}
}

void C3Dlg::OnFinalRelease()
{
	Cleanup();

	CDialog::OnFinalRelease();
}


BOOL C3Dlg::PreCreateWindow(CREATESTRUCT &cs)
{
	return CDialog::PreCreateWindow(cs);
}


BOOL C3Dlg::OnEraseBkgnd(CDC *pDC)
{
	return FALSE;
}


void C3Dlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 'DRAW')
		RedrawWindow(nullptr, nullptr, RDW_NOERASE | RDW_UPDATENOW);

	CDialog::OnTimer(nIDEvent);
}


BOOL C3Dlg::DestroyWindow()
{
	Cleanup();

	return CDialog::DestroyWindow();
}
