
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
	m_FS_copyback = m_VS_copyback = nullptr;
	m_SP_copyback = nullptr;
	memset(m_ColorTarg, 0, sizeof(c3::Texture2D *) * 2);;
	m_DepthTarg = nullptr;
	m_MoveF = false;
	m_MoveL = false;
	m_MoveR = false;
	m_MoveB = false;
	m_Run = false;
	m_MoveU = false;
	m_MoveD = false;
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
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
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
	//DwmEnableBlurBehindWindow(GetSafeHwnd(), &bb);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_Factory = theApp.m_C3->GetFactory();

	m_Rend = theApp.m_C3->GetRenderer();
	if (!m_Rend)
		exit(-1);

	m_Rend->SetClearColor(&c3::Color::Black);
	CRect r;
	GetClientRect(r);
	if (!m_Rend->Initialize(GetSafeHwnd(), 0))
		exit(-2);

	c3::ResourceManager *rm = theApp.m_C3->GetResourceManager();
	props::TFlags64 rf = c3::ResourceManager::RESFLAG(c3::ResourceManager::DEMANDLOAD);

	m_VS_copyback = (c3::ShaderComponent *)((rm->GetResource(_T("copyback.vsh"), rf))->GetData());
	m_FS_copyback = (c3::ShaderComponent *)((rm->GetResource(_T("copyback.fsh"), rf))->GetData());
	m_SP_copyback = m_Rend->CreateShaderProgram();
	if (m_SP_copyback)
	{
		m_SP_copyback->AttachShader(m_VS_copyback);
		m_SP_copyback->AttachShader(m_FS_copyback);
		m_SP_copyback->Link();
	}

	m_FB = m_Rend->CreateFrameBuffer();
	if (m_FB)
	{
		size_t w = r.Width();
		size_t h = r.Height();

		m_ColorTarg[0] = m_Rend->CreateTexture2D(w, h, c3::Renderer::TextureType::U8_4CH, 1, TEXCREATEFLAG_RENDERTARGET);
		m_ColorTarg[1] = m_Rend->CreateTexture2D(w, h, c3::Renderer::TextureType::F16_4CH, 1, TEXCREATEFLAG_RENDERTARGET);
		m_ColorTarg[2] = m_Rend->CreateTexture2D(w, h, c3::Renderer::TextureType::F32_4CH, 1, TEXCREATEFLAG_RENDERTARGET);
		m_ColorTarg[3] = m_Rend->CreateTexture2D(w, h, c3::Renderer::TextureType::U8_4CH, 1, TEXCREATEFLAG_RENDERTARGETSMOOTH);
		m_DepthTarg = m_Rend->CreateDepthBuffer(w, h, c3::Renderer::DepthType::U32_DS);

		if (m_ColorTarg[0] && m_ColorTarg[1] && m_ColorTarg[2] && m_DepthTarg)
		{
			m_FB->AttachColorTarget(m_ColorTarg[0], 0);
			m_FB->AttachColorTarget(m_ColorTarg[1], 1);
			m_FB->AttachColorTarget(m_ColorTarg[2], 2);
			m_FB->AttachColorTarget(m_ColorTarg[3], 3);
			m_FB->AttachDepthTarget(m_DepthTarg);

			m_FB->Seal();
		}
	}

	m_Camera = m_Factory->Build();
	m_Camera->AddFeature(c3::Positionable::Type());
	m_Camera->AddFeature(c3::Camera::Type());
	m_Camera->SetName(_T("Camera"));

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindFeature(c3::Camera::Type()));
	if (pcam)
	{
		pcam->SetPolarDistance(10.0f);
		pcam->SetFOV(glm::radians(70.0f));
	}

	m_RootObj = m_Factory->Build((c3::Prototype *)nullptr);
	m_RootObj->AddFeature(c3::Positionable::Type());

	c3::Prototype *pproto;
	if (nullptr != (pproto = m_Factory->FindPrototype(_T("Sponza"))))
	{
		c3::Object *pobj = m_Factory->Build(pproto);
		if (pobj)
		{
			c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(pobj->FindFeature(c3::Positionable::Type()));
			if (ppos)
			{
				ppos->SetScl(50.0f, 50.0f, 50.0f);
				ppos->SetYawPitchRoll(glm::radians(0.0f), glm::radians(90.0f), glm::radians(0.0f));
				ppos->Update(0);
			}
			m_RootObj->AddChild(pobj);
		}
	}

	if (nullptr != (pproto = m_Factory->FindPrototype(_T("AH64e"))))
	{
		c3::Object *pobj = m_Factory->Build(pproto);
		if (pobj)
		{
			c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(pobj->FindFeature(c3::Positionable::Type()));
			if (ppos)
			{
				ppos->SetScl(0.1f, 0.1f, 0.1f);
				ppos->Update(0);
			}
			m_RootObj->AddChild(pobj);
		}
	}

	SetTimer('DRAW', 5, NULL);

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
		theApp.m_C3->UpdateTime();
		float dt = theApp.m_C3->GetElapsedTime();

		m_Camera->Update(dt);
		c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(m_Camera->FindFeature(c3::Positionable::Type()));
		c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindFeature(c3::Camera::Type()));
		if (pcam)
		{
			m_Rend->SetViewMatrix(pcam->GetViewMatrix());
			m_Rend->SetProjectionMatrix(pcam->GetProjectionMatrix());
		}

		if (m_Rend->BeginScene())
		{
			m_RootObj->Update(dt);
			c3::Positionable *ppos = (c3::Positionable *)(m_RootObj->FindFeature(c3::Positionable::Type()));
			m_Rend->SetWorldMatrix(ppos->GetTransformMatrix());

			m_Rend->UseFrameBuffer(m_FB, UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH);

			if (m_RootObj->Prerender())
				if (m_RootObj->Render())
					m_RootObj->Postrender();

			m_Rend->UseFrameBuffer(nullptr);
			m_Rend->UseProgram(m_SP_copyback);
			int32_t ul;

			if (c3::ShaderProgram::INVALID_UNIFORM != (ul = m_SP_copyback->GetUniformLocation(_T("uSamplerDiffuse"))))
				m_SP_copyback->SetUniformTexture(ul, 0, m_ColorTarg[0]);
			if (c3::ShaderProgram::INVALID_UNIFORM != (ul = m_SP_copyback->GetUniformLocation(_T("uSamplerNormal"))))
				m_SP_copyback->SetUniformTexture(ul, 1, m_ColorTarg[1]);
			if (c3::ShaderProgram::INVALID_UNIFORM != (ul = m_SP_copyback->GetUniformLocation(_T("uSamplerPosDepth"))))
				m_SP_copyback->SetUniformTexture(ul, 2, m_ColorTarg[2]);
			if (c3::ShaderProgram::INVALID_UNIFORM != (ul = m_SP_copyback->GetUniformLocation(_T("uSamplerEmission"))))
				m_SP_copyback->SetUniformTexture(ul, 3, m_ColorTarg[3]);

			m_SP_copyback->ApplyUniforms(false);
			m_Rend->UseVertexBuffer(m_Rend->GetFullscreenPlaneVB());
			m_Rend->SetCullMode(c3::Renderer::ECullMode::CM_DISABLED);
			m_Rend->DrawPrimitives(c3::Renderer::EPrimType::TRISTRIP, 4);

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

	if (m_ColorTarg[2])
	{
		m_ColorTarg[2]->Release();
		m_ColorTarg[2] = nullptr;
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

	if (m_SP_copyback)
	{
		m_SP_copyback->Release();
		m_SP_copyback = nullptr;
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
	{
		c3::Positionable *cam = dynamic_cast<c3::Positionable *>(m_Camera->FindFeature(c3::Positionable::Type()));
		if (cam)
		{
			glm::vec3 mv(0, 0, 0);

#define MOVE_SPEED		(m_Run ? 2.5f : 0.5f)

			if (m_MoveF)
				mv += *(cam->GetFacingVector()) * MOVE_SPEED;

			if (m_MoveB)
				mv -= *(cam->GetFacingVector()) * MOVE_SPEED;

			if (m_MoveL)
				mv += *(cam->GetLocalLeftVector()) * MOVE_SPEED;

			if (m_MoveR)
				mv -= *(cam->GetLocalLeftVector()) * MOVE_SPEED;

			if (m_MoveU)
				mv.z += MOVE_SPEED;

			if (m_MoveD)
				mv.z -= MOVE_SPEED;

			cam->AdjustPos(mv.x, mv.y, mv.z);
		}
			
		RedrawWindow(nullptr, nullptr, RDW_NOERASE | RDW_UPDATENOW | RDW_INVALIDATE);
	}

	CDialog::OnTimer(nIDEvent);
}


BOOL C3Dlg::DestroyWindow()
{
	Cleanup();

	return CDialog::DestroyWindow();
}


BOOL C3Dlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindFeature(c3::Camera::Type()));
	if (pcam)
	{
		float d = pcam->GetPolarDistance();
		d += ((zDelta < 0) ? 3.5f : -3.5f);
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

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindFeature(c3::Camera::Type()));
	if (pcam)
	{
		glm::fvec2 dim;
		dim.x = (float)cx;
		dim.y = (float)cy;
		pcam->SetOrthoDimensions(&dim);
		m_Camera->Update();
		m_Rend->SetProjectionMatrix(pcam->GetProjectionMatrix());
	}
}


void C3Dlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);
}


void C3Dlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialog::OnMouseMove(nFlags, point);

	static CPoint lastpos(-1, -1);
	static float campitch = 0;
	static float camyaw = 0;

	int deltax = 0;
	int deltay = 0;

	if (lastpos.x > 0)
		deltax = lastpos.x - point.x;
	if (lastpos.y > 0)
		deltay = lastpos.y - point.y;

	lastpos = point;

	c3::Positionable *cam = dynamic_cast<c3::Positionable *>(m_Camera->FindFeature(c3::Positionable::Type()));
	if (cam)
	{
		campitch -= deltay;
		camyaw += deltax;

		//campitch = std::min(std::max(-88.0f, campitch), 88.0f);

		glm::quat q;
		glm::vec3 posx(1, 0, 0);
		q = glm::angleAxis(0.0f, posx);
		cam->SetOriQuat(&q);
		cam->Update(0);
		cam->AdjustYaw(glm::radians(camyaw));
		cam->Update(0);
		cam->AdjustPitch(glm::radians(campitch));
	}

}


void C3Dlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
		case VK_UP:		case 'W':	m_MoveF = true;	break;
		case VK_LEFT:	case 'A':	m_MoveL = true;	break;
		case VK_DOWN:	case 'S':	m_MoveB = true;	break;
		case VK_RIGHT:	case 'D':	m_MoveR = true;	break;
		case VK_SHIFT:				m_Run = true;	break;
		case 'Q':					m_MoveU = true;	break;
		case 'Z':					m_MoveD = true;	break;
	}

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}


void C3Dlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
		case VK_UP:		case 'W':	m_MoveF = false; break;
		case VK_LEFT:	case 'A':	m_MoveL = false; break;
		case VK_DOWN:	case 'S':	m_MoveB = false; break;
		case VK_RIGHT:	case 'D':	m_MoveR = false; break;
		case VK_SHIFT:				m_Run = false;	break;
		case 'Q':					m_MoveU = false;	break;
		case 'Z':					m_MoveD = false;	break;
	}

	CDialog::OnKeyUp(nChar, nRepCnt, nFlags);
}
