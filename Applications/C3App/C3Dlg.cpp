
// C3Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "C3App.h"
#include "C3Dlg.h"
#include "afxdialogex.h"
#include <C3Gui.h>
#include <Pool.h>

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
	m_pRDoc = nullptr;
	m_bCapturedFirstFrame = false;
	m_AmbientColor = c3::Color::DarkGrey;
	m_SunColor = c3::Color::White;
	m_SunDir = glm::normalize(glm::fvec3(0.2f, 0.5f, -1.0f));
}

void C3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(C3Dlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_CAPTURECHANGED()
	ON_WM_ACTIVATEAPP()
	ON_WM_ACTIVATE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

c3::FrameBuffer::TargetDesc GBufTargData[] =
{
	{ _T("uSamplerDiffuseMetalness"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET },	// diffuse color (rgb) and metalness (a)
	{ _T("uSamplerNormalAmbOcc"), c3::Renderer::TextureType::S8_4CH, TEXCREATEFLAG_RENDERTARGET },		// fragment normal (rgb) and ambient occlusion (a)
	{ _T("uSamplerPosDepth"), c3::Renderer::TextureType::F32_4CH, TEXCREATEFLAG_RENDERTARGET },			// fragment position in world space (rgb) and dpeth in screen space (a)
	{ _T("uSamplerEmissionRoughness"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET }	// emission color (rgb) and roughness (a)
};

c3::FrameBuffer::TargetDesc LCBufTargData[] =
{
	{ _T("uSamplerLights"), c3::Renderer::TextureType::F16_3CH, TEXCREATEFLAG_RENDERTARGET },
};


// C3Dlg message handlers

BOOL C3Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	theApp.m_C3->SetOwner(GetSafeHwnd());
	theApp.m_C3->GetInputManager()->AcquireAll();

	// At init, on windows
	if (HMODULE mod = GetModuleHandle(_T("C:/Program Files/RenderDoc/renderdoc.dll")))
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_2, (void **)&m_pRDoc);
		//MessageBox(_T("RenderDoc detected -- click OK to capture frame 0"), _T("C3App"), MB_OK);
	}

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

	theApp.m_C3->GetLog()->Print(_T("Creating InputManager... "));
	m_Input = theApp.m_C3->GetInputManager();
	if (!m_Input)
	{
		theApp.m_C3->GetLog()->Print(_T("failed\n"));
		exit(-1);
	}
	theApp.m_C3->GetLog()->Print(_T("ok\n"));

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

	m_Rend->SetAlphaCoverage(0.5f, false);
	//m_Rend->GetGui()->SetWindowFocus();

	// To start a frame capture, call StartFrameCapture.
	// You can specify NULL, NULL for the device to capture on if you have only one device and
	// either no windows at all or only one window, and it will capture from that device.
	// See the documentation below for a longer explanation
	if (m_pRDoc)
		m_pRDoc->StartFrameCapture(NULL, NULL);

	c3::ResourceManager *rm = theApp.m_C3->GetResourceManager();
	props::TFlags64 rf = c3::ResourceManager::RESFLAG(c3::ResourceManager::DEMANDLOAD);

	m_Rend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);

	size_t w = r.Width();
	size_t h = r.Height();

	m_DepthTarg = m_Rend->CreateDepthBuffer(w, h, c3::Renderer::DepthType::U32_DS);
	m_ShadowTarg = m_Rend->CreateDepthBuffer(2048, 2048, c3::Renderer::DepthType::F32_SHADOW);

	bool gbok = false;

	theApp.m_C3->GetLog()->Print(_T("Creating G-buffer... "));
	m_GBuf = m_Rend->CreateFrameBuffer();
	gbok = m_GBuf->Setup(_countof(GBufTargData), GBufTargData, m_DepthTarg, r) == c3::FrameBuffer::RETURNCODE::RET_OK;
	theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

	theApp.m_C3->GetLog()->Print(_T("Creating light combine buffer... "));
	m_LCBuf = m_Rend->CreateFrameBuffer();
	gbok = m_LCBuf->Setup(_countof(LCBufTargData), LCBufTargData, m_DepthTarg, r) == c3::FrameBuffer::RETURNCODE::RET_OK;
	theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

	theApp.m_C3->GetLog()->Print(_T("Creating shadow buffer... "));
	m_SSBuf = m_Rend->CreateFrameBuffer();
	m_SSBuf->AttachDepthTarget(m_ShadowTarg);
	gbok = m_SSBuf->Seal() == c3::FrameBuffer::RETURNCODE::RET_OK;
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
			for (i = 0; i < m_GBuf->GetNumColorTargets(); i++)
			{
				c3::Texture2D *pt = m_GBuf->GetColorTarget(i);
				int32_t ul = m_SP_copyback->GetUniformLocation(pt->GetName());
				m_SP_copyback->SetUniformTexture((ul != c3::ShaderProgram::INVALID_UNIFORM) ? pt : m_Rend->GetBlackTexture());
			}

			for (i = 0; i < m_LCBuf->GetNumColorTargets(); i++)
			{
				c3::Texture2D* pt = m_LCBuf->GetColorTarget(i);
				int32_t ul = m_SP_copyback->GetUniformLocation(pt->GetName());
				m_SP_copyback->SetUniformTexture((ul != c3::ShaderProgram::INVALID_UNIFORM) ? pt : m_Rend->GetBlackTexture());
			}

			m_ulSunDir = m_SP_copyback->GetUniformLocation(_T("uSunDirection"));
			m_ulSunColor = m_SP_copyback->GetUniformLocation(_T("uSunColor"));
			m_ulAmbientColor = m_SP_copyback->GetUniformLocation(_T("uAmbientColor"));
		}
	}

	m_Camera = m_Factory->Build();
	m_Camera->AddComponent(c3::Positionable::Type());
	m_Camera->AddComponent(c3::Camera::Type());
	m_Camera->SetName(_T("Camera"));
	theApp.m_C3->GetLog()->Print(_T("Camera created\n"));

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		pcam->SetPolarDistance(10.0f);
		pcam->SetFOV(glm::radians(70.0f));
	}

	m_RootObj = m_Factory->Build((c3::Prototype *)nullptr);
	m_RootObj->AddComponent(c3::Positionable::Type());
	m_RootObj->Flags().Set(c3::Object::OBJFLAG(c3::Object::LIGHT) | c3::Object::OBJFLAG(c3::Object::CASTSHADOW));

	c3::Prototype *pproto;

#if 1
	if (nullptr != (pproto = m_Factory->FindPrototype(_T("Sponza"))))
	{
		c3::Object *pobj = m_Factory->Build(pproto);
		if (pobj)
		{
			c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(pobj->FindComponent(c3::Positionable::Type()));
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
#endif

#if 0
	if (nullptr != (pproto = m_Factory->FindPrototype(_T("AH64e"))))
	{
		c3::Object *pobj = m_Factory->Build(pproto);
		if (pobj)
		{
			c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(pobj->FindComponent(c3::Positionable::Type()));
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

#if 0
	if (nullptr != (pproto = m_Factory->FindPrototype(_T("TestBox"))))
	{
		c3::Object *pobj = m_Factory->Build(pproto);
		if (pobj)
		{
			c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(pobj->FindComponent(c3::Positionable::Type()));
			m_RootObj->AddChild(pobj);

			theApp.m_C3->GetLog()->Print(_T("TestBox created\n"));
		}
	}
#endif

#if 1
#define NUMLIGHTS		300
	if (nullptr != (pproto = m_Factory->FindPrototype(_T("Light"))))
	{
		for (size_t i = 0; i < NUMLIGHTS; i++)
		{
			float f = float(i);
			c3::Object *temp  = m_Factory->Build(pproto);
			if (temp)
			{
				m_Light.push_back(temp);

				glm::fvec3 mv(0.0f, 0.0f, 20.0f);

				if (i > 0)
				{
					mv.x = (float)(RAND_MAX / 2 - rand());
					mv.y = (float)(RAND_MAX / 2 - rand());
					mv.z = (float)(RAND_MAX / 2 - rand());
				}
				m_LightMove.push_back(glm::normalize(mv) * (float)(rand() % 10));

				c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(temp->FindComponent(c3::Positionable::Type()));
				if (ppos)
				{
					float s = 150;
					if (i > 0)
					{
						ppos->SetPos((float)(rand() % 1000) - 500.0f, (float)(rand() % 500) - 250.0f, (float)(rand() % 400) + 5.0f);
						s = (float)(rand() % 100) + 50.0f;
					}
					else
						ppos->AdjustPos(0, 0, 10.0f);

					ppos->SetScl(s, s, s);
					ppos->Update(0);
				}

				c3::OmniLight *plight = dynamic_cast<c3::OmniLight *>(temp->FindComponent(c3::OmniLight::Type()));
				if (plight)
					plight->SetSourceFrameBuffer(m_GBuf);

				props::IPropertySet *pps = temp->GetProperties();
				props::IProperty *pp = pps->CreateProperty(_T("uLightColor"), 'LCLR');
				const props::TVec3F c((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
				pp->SetVec3F(c);

				m_RootObj->AddChild(temp);

				//theApp.m_C3->GetLog()->Print(_T("Light %d created\n"), i);
			}
		}
	}
#endif

	m_DrawTimerId = SetTimer('DRAW', 33, nullptr);

	m_bMouseCursorEnabled = false;
	SetCapture();
	ShowCursor(FALSE);

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

		c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(m_Camera->FindComponent(c3::Positionable::Type()));
		if (pcampos)
		{
			glm::vec3 mv(0, 0, 0);

			float spd = (theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::BUTTON1) * 2.0f) + 1.5f;

			float mdf = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_W);
			float mdb = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_S);
			mv += *(pcampos->GetFacingVector()) * (mdf - mdb) * spd;

			float mdl = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_A);
			float mdr = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_D);
			mv += *(pcampos->GetLocalLeftVector()) * (mdl - mdr) * spd;

			float mdu = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_Q);
			mv.z += mdu * spd;

			float mdd = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_Z);
			mv.z -= mdd * spd;

			pcampos->AdjustPos(mv.x, mv.y, mv.z);
		}
		m_Camera->Update(dt);

		c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindComponent(c3::Camera::Type()));
		if (pcam)
		{
			m_Rend->SetViewMatrix(pcam->GetViewMatrix());
			m_Rend->SetProjectionMatrix(pcam->GetProjectionMatrix());
			m_Rend->SetEyePosition(pcam->GetEyePos());
			glm::fvec3 eyedir = glm::normalize(*pcam->GetTargetPos() - *(pcam->GetEyePos()));
			m_Rend->SetEyeDirection(&eyedir);
		}

#if 1
		for (size_t i = 0; i < m_Light.size(); i++)
		{
			c3::Positionable *plpos = dynamic_cast<c3::Positionable *>(m_Light[i]->FindComponent(c3::Positionable::Type()));
			float s = sinf((float)(m_Rend->GetCurrentFrameNumber() + i) * 3.14159f / 180.0f * 1.0f) * 0.5f;
			plpos->AdjustPos(m_LightMove[i].x * s, m_LightMove[i].x * s, m_LightMove[i].x * s);
			plpos->Update(m_Light[i]);
		}
#endif

		glm::fmat4x4 depthProjectionMatrix = glm::ortho<float>(-50, 50, -50, 50, -50, 100);
		glm::fmat4x4 depthViewMatrix = glm::lookAt(m_SunDir, glm::vec3(0,0,0), glm::vec3(0, 0, 1));
		glm::fmat4x4 depthMVP = depthProjectionMatrix * depthViewMatrix;
		m_Rend->SetSunShadowMatrix(&depthMVP);

		m_SP_copyback->SetUniform3(m_ulAmbientColor, &m_AmbientColor);
		m_SP_copyback->SetUniform3(m_ulSunColor, &m_SunColor);
		m_SP_copyback->SetUniform3(m_ulSunDir, &m_SunDir);

		m_RootObj->Update(dt);

		if (m_Rend->BeginScene(0))
		{
			// Solid color pass
			m_Rend->SetDepthMode(c3::Renderer::DepthMode::DM_READWRITE);
			m_Rend->UseFrameBuffer(m_GBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH | UFBFLAG_CLEARSTENCIL);
			m_Rend->SetDepthTest(c3::Renderer::Test::DT_LESSER);
			m_Rend->SetAlphaPassRange(3.0f / 255.0f);
			m_Rend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
			m_Rend->SetCullMode(c3::Renderer::CullMode::CM_BACK);
			m_RootObj->Render(c3::Object::OBJFLAG(c3::Object::DRAW));

			// Shadow pass
			m_Rend->UseFrameBuffer(m_SSBuf, UFBFLAG_CLEARDEPTH);
			m_RootObj->Render(c3::Object::OBJFLAG(c3::Object::CASTSHADOW));

			// Lighting pass(es)
			m_Rend->UseFrameBuffer(m_LCBuf, UFBFLAG_FINISHLAST | UFBFLAG_CLEARCOLOR);
			m_Rend->SetDepthMode(c3::Renderer::DepthMode::DM_READONLY);
			m_Rend->SetDepthTest(c3::Renderer::Test::DT_ALWAYS);
			m_Rend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			m_RootObj->Render(c3::Object::OBJFLAG(c3::Object::LIGHT));

			// Resolve
			m_Rend->UseFrameBuffer(nullptr, UFBFLAG_FINISHLAST | UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH);
			m_Rend->SetDepthMode(c3::Renderer::DepthMode::DM_DISABLED);
			m_Rend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			m_Rend->SetCullMode(c3::Renderer::CullMode::CM_DISABLED);
			m_Rend->UseProgram(m_SP_copyback);
			m_SP_copyback->ApplyUniforms(false);
			m_Rend->UseVertexBuffer(m_Rend->GetFullscreenPlaneVB());
			m_Rend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);

			m_Rend->EndScene();
			m_Rend->Present();
			if (!m_bCapturedFirstFrame)
			{
				m_bCapturedFirstFrame = true;
				// stop the capture

				if (m_pRDoc)
					m_pRDoc->EndFrameCapture(NULL, NULL);
			}
		}
		//CDialog::OnPaint();
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


BOOL C3Dlg::DestroyWindow()
{
	Cleanup();

	return CDialog::DestroyWindow();
}


BOOL C3Dlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		float m = m_Run ? 3.5f : 1.0f;
		float d = pcam->GetPolarDistance();
		d += ((zDelta < 0) ? m : -m);
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

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindComponent(c3::Camera::Type()));
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

	theApp.m_C3->GetInputManager()->SetMousePos(point.x, point.y);

	if ((m_bMouseCursorEnabled) || (this != GetCapture()))
		return;

	CRect r;
	GetClientRect(&r);

	CPoint cp = r.CenterPoint(), cpc = cp;
	ClientToScreen(&cp);

	static float campitch = 0;
	static float camyaw = 0;

	int deltax = cpc.x - point.x;
	int deltay = cpc.y - point.y;

	SetCursorPos(cp.x, cp.y);

	c3::Positionable *pos = dynamic_cast<c3::Positionable *>(m_Camera->FindComponent(c3::Positionable::Type()));
	c3::Camera *cam = dynamic_cast<c3::Camera *>(m_Camera->FindComponent(c3::Camera::Type()));
	if (cam && pos)
	{
		campitch -= deltay;
		camyaw += deltax;

		campitch = std::min(std::max(-88.0f, campitch), 88.0f);

		glm::quat q;
		glm::vec3 posx(1, 0, 0);
		q = glm::angleAxis(0.0f, posx);
		pos->SetOriQuat(&q);
		pos->Update(m_Camera);
		pos->AdjustYaw(glm::radians(camyaw));
		pos->Update(m_Camera);
		pos->AdjustPitch(glm::radians(campitch));
		pos->Update(m_Camera);
		cam->Update(m_Camera);
	}
}


void C3Dlg::OnOK()
{
	//__super::OnOK();
}


void C3Dlg::OnCancel()
{
	__super::OnCancel();
}


BOOL C3Dlg::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_ESCAPE))
	{
		SetMouseEnabled(!m_bMouseCursorEnabled);

		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}


void C3Dlg::OnCaptureChanged(CWnd* pWnd)
{
	CDialog::OnCaptureChanged(pWnd);
}


void C3Dlg::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	CDialog::OnActivateApp(bActive, dwThreadID);
}


void C3Dlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);
	if (nState != WA_INACTIVE)
	{
		theApp.m_C3->GetInputManager()->AcquireAll();
		//SetMouseEnabled(m_bMouseCursorEnabled);
	}
	else
	{
		theApp.m_C3->GetInputManager()->UnacquireAll();
	}
}

void C3Dlg::SetMouseEnabled(bool b)
{
	m_bMouseCursorEnabled = b;

	if (this != GetFocus())
		return;

	if (m_bMouseCursorEnabled && (this == GetCapture()))
		ReleaseCapture();
	else if (this != GetCapture())
		SetCapture();

	CURSORINFO ci = { 0 };
	ci.cbSize = sizeof(CURSORINFO);
	GetCursorInfo(&ci);
	if (m_bMouseCursorEnabled != ((ci.flags & CURSOR_SHOWING) ? true : false))
		ShowCursor(m_bMouseCursorEnabled ? TRUE : FALSE);
}


void C3Dlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == m_DrawTimerId)
		RedrawWindow(nullptr, nullptr, 0);

	CDialog::OnTimer(nIDEvent);
}
