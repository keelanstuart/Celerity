
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
	m_GBuf = nullptr;
	m_LCBuf = nullptr;
	m_FS_copyback = m_VS_copyback = nullptr;
	m_SP_copyback = nullptr;
	m_DepthTarg = nullptr;
	m_MoveF = false;
	m_MoveL = false;
	m_MoveR = false;
	m_MoveB = false;
	m_Run = false;
	m_MoveU = false;
	m_MoveD = false;
	m_Light[0] = nullptr;
	m_Light[1] = nullptr;
	m_Light[2] = nullptr;
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

typedef struct sTargData
{
	const TCHAR *name;
	c3::Renderer::ETextureType type;
	uint64_t flags;
} STargData;

STargData GBufTargData[] =
{
	{ _T("uSamplerDiffuse"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET },
	{ _T("uSamplerNormal"), c3::Renderer::TextureType::F16_4CH, TEXCREATEFLAG_RENDERTARGET },
	{ _T("uSamplerPosDepth"), c3::Renderer::TextureType::F32_4CH, TEXCREATEFLAG_RENDERTARGET },
	{ _T("uSamplerEmission"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET }
};

// It SEEEEMS like in order to get blending to work, the light combine buffer needs to pre-multiply alpha,
// but then write alpha=1 in the shader... the depth test is still a problem
STargData LCBufTargData[] =
{
	{ _T("uSamplerLights"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET },
};

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

	theApp.m_C3->GetLog()->Print(_T("Creating Renderer... "));
	m_Rend = theApp.m_C3->GetRenderer();
	if (!m_Rend)
	{
		theApp.m_C3->GetLog()->Print(_T("failed\n"));
		exit(-1);
	}
	theApp.m_C3->GetLog()->Print(_T("ok\n"));

	theApp.m_C3->GetLog()->Print(_T("Initializing Renderer... "));
	m_Rend->SetClearColor(&c3::Color::Black);
	CRect r;
	GetClientRect(r);
	if (!m_Rend->Initialize(GetSafeHwnd(), 0))
	{
		theApp.m_C3->GetLog()->Print(_T("failed\n"));
		exit(-2);
	}
	theApp.m_C3->GetLog()->Print(_T("ok\n"));

	c3::ResourceManager *rm = theApp.m_C3->GetResourceManager();
	props::TFlags64 rf = c3::ResourceManager::RESFLAG(c3::ResourceManager::DEMANDLOAD);

	m_Rend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);

	theApp.m_C3->GetLog()->Print(_T("Creating G-buffer... "));
	bool gbok = true;
	m_GBuf = m_Rend->CreateFrameBuffer();
	if (m_GBuf)
	{
		size_t w = r.Width();
		size_t h = r.Height();

		m_DepthTarg = m_Rend->CreateDepthBuffer(w, h, c3::Renderer::DepthType::U32_DS);
		m_GBuf->AttachDepthTarget(m_DepthTarg);

		for (size_t i = 0; i < _countof(GBufTargData); i++)
		{
			c3::Texture2D *pt = m_Rend->CreateTexture2D(w, h, GBufTargData[i].type, 1, GBufTargData[i].flags);
			gbok &= (pt != nullptr);
			if (!gbok)
				break;

			m_GBuf->AttachColorTarget(pt, i);
			pt->SetName(GBufTargData[i].name);
			m_ColorTarg.push_back(pt);
		}

		if (gbok)
			gbok &= (c3::FrameBuffer::RETURNCODE::RET_OK == m_GBuf->Seal());
	}
	theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

	theApp.m_C3->GetLog()->Print(_T("Creating light combine buffer... "));
	gbok = true;
	m_LCBuf = m_Rend->CreateFrameBuffer();
	if (m_LCBuf)
	{
		size_t w = r.Width();
		size_t h = r.Height();

		m_LCBuf->AttachDepthTarget(m_DepthTarg);

		for (size_t i = 0; i < _countof(LCBufTargData); i++)
		{
			c3::Texture2D *pt = m_Rend->CreateTexture2D(w, h, LCBufTargData[i].type, 1, LCBufTargData[i].flags);
			gbok &= (pt != nullptr);
			if (!gbok)
				break;

			m_LCBuf->AttachColorTarget(pt, i);
			pt->SetName(LCBufTargData[i].name);
		}

		if (gbok)
			gbok &= (c3::FrameBuffer::RETURNCODE::RET_OK == m_LCBuf->Seal());
	}
	theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));


	m_VS_copyback = (c3::ShaderComponent *)((rm->GetResource(_T("resolve.vsh"), rf))->GetData());
	m_FS_copyback = (c3::ShaderComponent *)((rm->GetResource(_T("resolve.fsh"), rf))->GetData());
	m_SP_copyback = m_Rend->CreateShaderProgram();
	if (m_SP_copyback)
	{
		m_SP_copyback->AttachShader(m_VS_copyback);
		m_SP_copyback->AttachShader(m_FS_copyback);
		if (m_SP_copyback->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
		{
			uint32_t i;
			for (i = 0; i < m_ColorTarg.size(); i++)
			{
					m_SP_copyback->SetUniformTexture(m_ColorTarg[i]);
			}

			m_SP_copyback->SetUniformTexture(m_LCBuf->GetColorTarget(0));
		}
	}

	m_Camera = m_Factory->Build();
	m_Camera->AddFeature(c3::Positionable::Type());
	m_Camera->AddFeature(c3::Camera::Type());
	m_Camera->SetName(_T("Camera"));
	theApp.m_C3->GetLog()->Print(_T("Camera created\n"));

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindFeature(c3::Camera::Type()));
	if (pcam)
	{
		pcam->SetPolarDistance(10.0f);
		pcam->SetFOV(glm::radians(70.0f));
	}

	m_RootObj = m_Factory->Build((c3::Prototype *)nullptr);
	m_RootObj->AddFeature(c3::Positionable::Type());
	m_RootObj->Flags().Set(c3::Object::OBJFLAG(c3::Object::LIGHT));

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

			theApp.m_C3->GetLog()->Print(_T("Sponza created\n"));
		}
	}

#if 1
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

			theApp.m_C3->GetLog()->Print(_T("Chopper created\n"));
		}
	}
#endif

#if 1
	if (nullptr != (pproto = m_Factory->FindPrototype(_T("Light"))))
	{
		for (size_t i = 0; i < _countof(m_Light); i++)
		{
			float f = float(i);
			m_Light[i] = m_Factory->Build(pproto);
			if (m_Light[i])
			{
				c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(m_Light[i]->FindFeature(c3::Positionable::Type()));
				if (ppos)
				{
					ppos->SetPos((f - 1) * 30.0f, 0.0f, 1.0f);
					ppos->SetScl(130.0f, 130.0f, 130.0f);
					ppos->Update(0);
				}

				c3::OmniLight *plight = dynamic_cast<c3::OmniLight *>(m_Light[i]->FindFeature(c3::OmniLight::Type()));
				if (plight)
					plight->SetSourceFrameBuffer(m_GBuf);

				props::IPropertySet *pps = m_Light[i]->GetProperties();
				props::IProperty *pp = pps->CreateProperty(_T("uLightColor"), 'LCLR');
				const props::TVec3F c[3] = {{0, 1, 0}, {0, 0, 1}, {1, 0, 0}};
				pp->SetVec3F(c[i]);

				m_RootObj->AddChild(m_Light[i]);

				theApp.m_C3->GetLog()->Print(_T("Light %d created\n"), i);
			}
		}
	}
#endif

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

#if 0
		for (size_t i = 0; i < 3; i++)
		{
			if (!m_Light[i])
				continue;

			c3::Positionable *plpos = dynamic_cast<c3::Positionable *>(m_Light[i]->FindFeature(c3::Positionable::Type()));
			float adjx = sinf((float)theApp.m_C3->GetCurrentFrameNumber() * 0.01f) * 30.0f;
			float adjy = 0.0f;
			float adjz = cosf((float)theApp.m_C3->GetCurrentFrameNumber() * 0.01f) * 5.0f + 5.0f;
			plpos->AdjustPos(adjx, adjy, adjz);
		}
#endif

		m_RootObj->Update(dt);

		if (m_Rend->BeginScene(0))
		{
			// Color pass
			m_Rend->SetDepthMode(c3::Renderer::DepthMode::DM_READWRITE);
			m_Rend->UseFrameBuffer(m_GBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH);
			m_Rend->SetDepthTest(c3::Renderer::Test::DT_LESSER);
			m_Rend->SetBlendMode(c3::Renderer::BlendMode::BM_ALPHA);
			m_Rend->SetCullMode(c3::Renderer::CullMode::CM_BACK);
			m_RootObj->Render(c3::Object::OBJFLAG(c3::Object::DRAW));

			// Lighting pass(es)
			m_Rend->UseFrameBuffer(m_LCBuf, UFBFLAG_FINISHLAST | UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH);
			m_Rend->SetDepthMode(c3::Renderer::DepthMode::DM_READONLY);
			m_Rend->SetDepthTest(c3::Renderer::Test::DT_LESSEREQUAL);
			m_Rend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			m_Rend->SetCullMode(c3::Renderer::CullMode::CM_BACK);
			m_LCBuf->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			m_RootObj->Render(c3::Object::OBJFLAG(c3::Object::LIGHT));

			// Resolve
			m_Rend->UseFrameBuffer(nullptr, UFBFLAG_FINISHLAST | UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH);
			m_Rend->SetDepthMode(c3::Renderer::DepthMode::DM_DISABLED);
			m_Rend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			m_Rend->SetDepthTest(c3::Renderer::Test::DT_ALWAYS);
			m_Rend->UseProgram(m_SP_copyback);
			m_SP_copyback->ApplyUniforms(false);
			m_Rend->UseVertexBuffer(m_Rend->GetFullscreenPlaneVB());
			m_Rend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);

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

	for (auto targ : m_ColorTarg)
	{
		targ->Release();
	}

	if (m_DepthTarg)
	{
		m_DepthTarg->Release();
		m_DepthTarg = nullptr;
	}

	if (m_GBuf)
	{
		m_GBuf->Release();
		m_GBuf = nullptr;
	}

	if (m_LCBuf)
	{
		m_LCBuf->Release();
		m_LCBuf = nullptr;
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

	RECT r;
	r.left = 0;
	r.right = cx;
	r.top = 0;
	r.bottom = cy;
//	theApp.m_C3->GetRenderer()->SetViewport(&r);

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
