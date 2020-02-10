
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
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	ON_WM_SIZING()
END_MESSAGE_MAP()


// C3Dlg message handlers

BOOL C3Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	DWM_BLURBEHIND bb;
	bb.dwFlags = DWM_BB_ENABLE | DWM_BB_TRANSITIONONMAXIMIZED | DWM_BB_BLURREGION;
	bb.fEnable = TRUE;
	bb.hRgnBlur = CreateRectRgn(0, 0, -1, -1);
	bb.fTransitionOnMaximized = TRUE;
	DwmEnableBlurBehindWindow(GetSafeHwnd(), &bb);


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
	if (!m_Rend->Initialize(GetSafeHwnd(), 0))
		exit(-2);

#if 0
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

	c3::Prototype *pproto;
	
	pproto = m_Factory->FindPrototype(_T("Camera"));
	m_Camera = pproto ? m_Factory->Build(pproto) : nullptr;

	pproto = m_Factory->FindPrototype(_T("GenericControl"));
	m_RootObj = pproto ? m_Factory->Build(pproto) : nullptr;

	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(m_Camera->FindComportment(c3::Positionable::Type()));
	if (pcampos)
	{
		// look up a little
		pcampos->AdjustPitch(glm::radians(-60.0f));
		pcampos->AdjustPos(0.0f, 0.0f, -8.0f);
	}

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindComportment(c3::Camera::Type()));
	if (pcam)
	{
		pcam->SetPolarDistance(8.0f);
		pcam->SetFOV(glm::radians(70.0f));
	}

	c3::Positionable *pdomepos = dynamic_cast<c3::Positionable *>(m_RootObj->FindComportment(c3::Positionable::Type()));
	if (pdomepos)
	{
		pdomepos->SetScl(40.0f, 40.0f, 40.0f);
	}


	SetTimer('DRAW', 16, NULL);

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
		c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(m_Camera->FindComportment(c3::Positionable::Type()));
		if (pcampos)
			pcampos->AdjustYaw(glm::radians(0.5f));

		m_Camera->Update();

		c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindComportment(c3::Camera::Type()));
		if (pcam)
		{
			m_Rend->SetViewMatrix(pcam->GetViewMatrix());
			m_Rend->SetProjectionMatrix(pcam->GetProjectionMatrix());
		}

		theApp.m_C3->UpdateTime();

		float dt = theApp.m_C3->GetElapsedTime();

		glm::fvec4 c = glm::fvec4(0.0f, 0.0f, 0.0f, 0.0f);
		m_Rend->SetClearColor(&c);
		if (m_Rend->BeginScene())
		{
			c3::Positionable *ppos = (c3::Positionable *)(m_RootObj->FindComportment(c3::Positionable::Type()));

			//ppos->AdjustPitch(0.02f);
			m_RootObj->Update(dt);

			m_Rend->SetWorldMatrix(ppos->GetTransformMatrix());

			if (m_RootObj->Prerender())
				if (m_RootObj->Render())
					m_RootObj->Postrender();

			m_Rend->EndScene();
			m_Rend->Present();
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
		RedrawWindow(nullptr, nullptr, RDW_NOERASE | RDW_UPDATENOW | RDW_INVALIDATE);

	CDialog::OnTimer(nIDEvent);
}


BOOL C3Dlg::DestroyWindow()
{
	Cleanup();

	return CDialog::DestroyWindow();
}


BOOL C3Dlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindComportment(c3::Camera::Type()));
	if (pcam)
	{
		float d = pcam->GetPolarDistance();
		d += ((zDelta < 0) ? 0.5f : -0.5f);
		d = std::max(d, 0.1f);

		pcam->SetPolarDistance(d);
	}

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}


void C3Dlg::OnSize(UINT nType, int cx, int cy)
{
	cx = std::max(cx, 384);
	cy = std::max(cy, 256);

	CDialog::OnSize(nType, cx, cy);

	theApp.m_C3->GetRenderer()->SetViewport();

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindComportment(c3::Camera::Type()));
	if (pcam)
	{
		glm::fvec2 dim;
		dim.x = (float)cx;
		dim.y = (float)cy;
		pcam->SetOrthoDimensions(&dim);
	}
}


void C3Dlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);
}
