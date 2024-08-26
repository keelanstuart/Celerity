
// C3Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"

#include "C3App.h"
#include "C3Dlg.h"
#include "afxdialogex.h"
#include <C3Gui.h>
#include <Pool.h>

#include <C3CommonVertexDefs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



C3Dlg::C3Dlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_C3APP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_Rend = nullptr;
	m_GBuf = nullptr;
	m_LCBuf = nullptr;
	m_FS_combine = m_VS_combine = nullptr;
	m_SP_combine = nullptr;
	m_FS_resolve = m_VS_resolve = nullptr;
	m_SP_resolve = nullptr;
	m_FS_blur = m_VS_blur = nullptr;
	m_SP_blur = nullptr;
	m_DepthTarg = nullptr;
	m_pRDoc = nullptr;
	m_bCapturedFirstFrame = false;
	m_bFirstDraw = true;
	m_DebugEnabled = false;
	m_Camera = m_CameraRoot = m_CameraArm = m_GUICamera = nullptr;
	m_WorldRoot = m_GUIRoot = nullptr;
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
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

c3::FrameBuffer::TargetDesc GBufTargData[] =
{
	{ _T("uSamplerDiffuseMetalness"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET },	// diffuse color (rgb) and metalness (a)
	{ _T("uSamplerNormalAmbOcc"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET },		// fragment normal (rgb) and ambient occlusion (a)
	{ _T("uSamplerPosDepth"), c3::Renderer::TextureType::F32_4CH, TEXCREATEFLAG_RENDERTARGET },			// fragment position in world space (rgb) and dpeth in screen space (a)
	{ _T("uSamplerEmissionRoughness"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET }	// emission color (rgb) and roughness (a)
};

c3::FrameBuffer::TargetDesc LCBufTargData[] =
{
	{ _T("uSamplerLights"), c3::Renderer::TextureType::F16_3CH, TEXCREATEFLAG_RENDERTARGET },
};


void C3Dlg::RegisterAction(const TCHAR *name, c3::ActionMapper::ETriggerType tt, float delay)
{
	c3::ActionMapper *pam = theApp.m_C3->GetActionMapper();

	pam->RegisterAction(name, tt, delay, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		if (theApp.GetMainWnd() != GetCapture())
			return false;

		const TCHAR *name = (const TCHAR *)userdata;

		c3::Object *pplayer = theApp.m_C3->GetGlobalObjectRegistry()->GetRegisteredObject(c3::GlobalObjectRegistry::OD_PLAYER);
		if (pplayer)
		{
			c3::Scriptable *pscr = dynamic_cast<c3::Scriptable *>(pplayer->FindComponent(c3::Scriptable::Type()));
			if (pscr)
			{
				value *= theApp.m_C3->GetElapsedTime();
				pscr->Execute(_T("handle_input(\"%s\", %0.5f);"), name, value);
				return true;
			}
		}
		return false;
	}, (void *)name);
};


void C3Dlg::DestroySurfaces()
{
	C3_SAFERELEASE(m_GBuf);

	C3_SAFERELEASE(m_LCBuf);

	for (auto p : m_ColorTarg)
		C3_SAFERELEASE(p);
	m_ColorTarg.clear();

	C3_SAFERELEASE(m_DepthTarg);

	for (auto p : m_BBuf)
		C3_SAFERELEASE(p);
	m_BBuf.clear();

	for (auto p : m_BTex)
		C3_SAFERELEASE(p);
	m_BTex.clear();
}


void C3Dlg::CreateSurfaces()
{
	c3::Renderer *prend = theApp.m_C3->GetRenderer();
	assert(prend);

	CRect r;
	GetClientRect(r);

	size_t w = (size_t)((float)r.Width() / m_WindowsUIScale);
	size_t h = (size_t)((float)r.Height() / m_WindowsUIScale);

	if (!m_DepthTarg)
		m_DepthTarg = prend->CreateDepthBuffer(w, h, c3::Renderer::DepthType::U32_DS);

	bool gbok;

	c3::FrameBuffer::TargetDesc GBufTargData[] =
	{
		{ _T("uSamplerDiffuseMetalness"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET },	// diffuse color (rgb) and metalness (a)
		{ _T("uSamplerNormalAmbOcc"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET },		// fragment normal (rgb) and ambient occlusion (a)
		{ _T("uSamplerPosDepth"), c3::Renderer::TextureType::F32_4CH, TEXCREATEFLAG_RENDERTARGET },			// fragment position in world space (rgb) and dpeth in screen space (a)
		{ _T("uSamplerEmissionRoughness"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET }	// emission color (rgb) and roughness (a)
	};

	theApp.m_C3->GetLog()->Print(_T("Creating G-buffer... "));
	if (!m_GBuf)
		m_GBuf = prend->CreateFrameBuffer(0, _T("GBuffer"));
	if (m_GBuf)
		gbok = m_GBuf->Setup(_countof(GBufTargData), GBufTargData, m_DepthTarg, r) == c3::FrameBuffer::RETURNCODE::RET_OK;
	theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

	size_t maxc = theApp.m_Config->GetInt(_T("graphics.blurcount"), 4);
	for (size_t c = 0; c < maxc; c++)
	{
		m_BTex.push_back(prend->CreateTexture2D(w, h, c3::Renderer::TextureType::U8_3CH, 0, TEXCREATEFLAG_RENDERTARGET));
		m_BBuf.push_back(prend->CreateFrameBuffer());
		m_BBuf[c]->AttachDepthTarget(m_DepthTarg);
		m_BBuf[c]->AttachColorTarget(m_BTex[c], 0);
		m_BBuf[c]->Seal();
		w /= 2;
		h /= 2;
	}

	c3::FrameBuffer::TargetDesc LCBufTargData[] =
	{
		{ _T("uSamplerLights"), c3::Renderer::TextureType::F16_3CH, TEXCREATEFLAG_RENDERTARGET },
	};

	theApp.m_C3->GetLog()->Print(_T("Creating light combine buffer... "));
	if (!m_LCBuf)
		m_LCBuf = prend->CreateFrameBuffer(0, _T("LightCombine"));
	if (m_LCBuf)
		gbok = m_LCBuf->Setup(_countof(LCBufTargData), LCBufTargData, m_DepthTarg, r) == c3::FrameBuffer::RETURNCODE::RET_OK;
	theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

	CRect auxr = r;

	c3::FrameBuffer::TargetDesc AuxBufTargData[] =
	{
		{ _T("uSamplerAuxiliary"), c3::Renderer::TextureType::U8_3CH, TEXCREATEFLAG_RENDERTARGET },
	};

	UpdateShaderSurfaces();
}


void C3Dlg::UpdateShaderSurfaces()
{
	c3::Renderer *prend = theApp.m_C3->GetRenderer();
	assert(prend);

	int32_t ut;

	if (m_SP_resolve)
	{
		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerSceneMip0"));
		m_SP_resolve->SetUniformTexture(m_BTex[0], ut);

		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerSceneMip1"));
		m_SP_resolve->SetUniformTexture(m_BTex[1], ut);

#if (BLURTARGS > 2)
		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerSceneMip2"));
		m_SP_resolve->SetUniformTexture(m_BTex[2], ut);
#endif

#if (BLURTARGS > 3)
		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerSceneMip3"));
		m_SP_resolve->SetUniformTexture(m_BTex[3], ut);
#endif

		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerPosDepth"));
		m_SP_resolve->SetUniformTexture(m_GBuf->GetColorTargetByName(_T("uSamplerPosDepth")), ut);
	}

	if (m_SP_combine)
	{
		uint32_t i;
		int32_t ul;
		for (i = 0; i < m_GBuf->GetNumColorTargets(); i++)
		{
			c3::Texture2D* pt = m_GBuf->GetColorTarget(i);
			ul = m_SP_combine->GetUniformLocation(pt->GetName());
			m_SP_combine->SetUniformTexture((ul != c3::ShaderProgram::INVALID_UNIFORM) ? pt : prend->GetBlackTexture());
		}

		for (i = 0; i < m_LCBuf->GetNumColorTargets(); i++)
		{
			c3::Texture2D* pt = m_LCBuf->GetColorTarget(i);
			ul = m_SP_combine->GetUniformLocation(pt->GetName());
			m_SP_combine->SetUniformTexture((ul != c3::ShaderProgram::INVALID_UNIFORM) ? pt : prend->GetBlackTexture());
		}

		ul = m_SP_combine->GetUniformLocation(_T("uSamplerShadow"));
		if (ul >= 0)
			m_SP_combine->SetUniformTexture((c3::Texture*)m_ShadowTarg, ul, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);
	}
}


void C3Dlg::InitializeGraphics()
{
	m_Rend = theApp.m_C3->GetRenderer();
	assert(m_Rend);

		// Compensate for screen scaling in Windows 10+
	m_WindowsUIScale = GetDC()->GetDeviceCaps(LOGPIXELSX) / 96.0f;
	if ((m_WindowsUIScale > 1.0f) && (m_WindowsUIScale < 1.1f))
		m_WindowsUIScale = 1.0f;

	theApp.m_C3->GetLog()->Print(_T("Initializing Renderer... "));

	if (m_Rend->Initialize(GetSafeHwnd(), 0))
	{
		bool gbok;

		CreateSurfaces();

		m_Rend->GetFont(_T("Arial"), 16);

		c3::ResourceManager* rm = theApp.m_C3->GetResourceManager();

		m_Rend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);

		theApp.m_C3->GetLog()->Print(_T("Creating shadow buffer... "));

		if (!m_ShadowTarg)
			m_ShadowTarg = m_Rend->CreateDepthBuffer(2048, 2048, c3::Renderer::DepthType::F32_SHADOW);

		if (!m_SSBuf)
			m_SSBuf = m_Rend->CreateFrameBuffer(0, _T("Shadow"));
		if (m_SSBuf)
			m_SSBuf->AttachDepthTarget(m_ShadowTarg);
		gbok = m_SSBuf->Seal() == c3::FrameBuffer::RETURNCODE::RET_OK;
		theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

		m_VS_blur = (c3::ShaderComponent*)((rm->GetResource(_T("blur.vsh"), RESF_DEMANDLOAD))->GetData());
		m_FS_blur = (c3::ShaderComponent*)((rm->GetResource(_T("blur.fsh"), RESF_DEMANDLOAD))->GetData());
		if (!m_SP_blur)
			m_SP_blur = m_Rend->CreateShaderProgram();
		if (m_SP_blur)
		{
			m_SP_blur->AttachShader(m_VS_blur);
			m_SP_blur->AttachShader(m_FS_blur);
			if (m_SP_blur->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
			{
				m_uBlurTex = m_SP_blur->GetUniformLocation(_T("uSamplerUpRes"));
				m_uBlurScale = m_SP_blur->GetUniformLocation(_T("uBlurScale"));
			}
		}

		m_VS_resolve = (c3::ShaderComponent*)((rm->GetResource(_T("resolve.vsh"), RESF_DEMANDLOAD))->GetData());
		m_FS_resolve = (c3::ShaderComponent*)((rm->GetResource(_T("resolve.fsh"), RESF_DEMANDLOAD))->GetData());
		if (!m_SP_resolve)
			m_SP_resolve = m_Rend->CreateShaderProgram();
		if (m_SP_resolve)
		{
			m_SP_resolve->AttachShader(m_VS_resolve);
			m_SP_resolve->AttachShader(m_FS_resolve);
			if (m_SP_resolve->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
			{
				int32_t ut;

				ut = m_SP_resolve->GetUniformLocation(_T("uFocusDist"));
				m_SP_resolve->SetUniform1(ut, 0.1f);

				ut = m_SP_resolve->GetUniformLocation(_T("uFocusFalloff"));
				m_SP_resolve->SetUniform1(ut, 0.0f);
			}
		}

		m_VS_combine = (c3::ShaderComponent*)((rm->GetResource(_T("combine.vsh"), RESF_DEMANDLOAD))->GetData());
		m_FS_combine = (c3::ShaderComponent*)((rm->GetResource(_T("combine.fsh"), RESF_DEMANDLOAD))->GetData());
		if (!m_SP_combine)
			m_SP_combine = m_Rend->CreateShaderProgram();
		if (m_SP_combine)
		{
			m_SP_combine->AttachShader(m_VS_combine);
			m_SP_combine->AttachShader(m_FS_combine);
		}

		UpdateShaderSurfaces();
	}
}


BOOL C3Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	theApp.m_C3->SetOwner(GetSafeHwnd());

	theApp.m_C3->GetSoundPlayer()->Initialize();

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

	theApp.m_C3->GetLog()->Print(_T("Setting up actions... "));

	RegisterAction(_T("Move Forward"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Move Backward"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Strafe Left"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Strafe Right"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Look Up"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Look Down"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Look Left"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Look Right"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Ascend"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Descend"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Run"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Jump"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0.1f);
	RegisterAction(_T("Fire 1"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Fire 2"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Increase Velocity"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Decrease Velocity"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, 0);
	RegisterAction(_T("Cycle View Mode"), c3::ActionMapper::ETriggerType::UP_DELTA, 0);

	theApp.m_C3->GetActionMapper()->RegisterAction(_T("Toggle Debug"), c3::ActionMapper::ETriggerType::UP_DELTA, 0,
		[](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
		{
			*((bool *)userdata) ^= true;
			return true;
		},
		&m_DebugEnabled);

	theApp.m_C3->GetLog()->Print(_T("ok\n"));

	theApp.m_C3->GetLog()->Print(_T("Creating InputManager... "));
	c3::InputManager::SetDeviceConnectionCallback(DeviceConnected, this);
	m_Input = theApp.m_C3->GetInputManager();
	if (!m_Input)
	{
		theApp.m_C3->GetLog()->Print(_T("failed\n"));
		exit(-1);
	}
	theApp.m_C3->GetLog()->Print(_T("ok\n"));

	InitializeGraphics();

	m_Rend->SetAlphaCoverage(0.5f, false);

	c3::ResourceManager *rm = theApp.m_C3->GetResourceManager();

	m_Rend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);

	m_WorldRoot = m_Factory->Build();
	m_WorldRoot->AddComponent(c3::Positionable::Type());
	m_WorldRoot->AddComponent(c3::Scriptable::Type());
	m_WorldRoot->Flags().Set(OF_LIGHT | OF_CASTSHADOW);
	theApp.m_C3->GetGlobalObjectRegistry()->RegisterObject(c3::GlobalObjectRegistry::OD_WORLDROOT, m_WorldRoot);
	theApp.m_C3->GetLog()->Print(_T("World root created and registered\n"));

	m_GUIRoot = m_Factory->Build();
	m_GUIRoot->AddComponent(c3::Positionable::Type());
	m_GUIRoot->AddComponent(c3::Scriptable::Type());
	theApp.m_C3->GetGlobalObjectRegistry()->RegisterObject(c3::GlobalObjectRegistry::OD_GUI_ROOT, m_GUIRoot);
	theApp.m_C3->GetLog()->Print(_T("GUI root created and registered\n"));

	m_CameraRoot = m_Factory->Build();
	m_CameraRoot->SetName(_T("CameraRoot"));
	m_CameraRoot->AddComponent(c3::Scriptable::Type());
	m_CameraRoot->AddComponent(c3::Positionable::Type());
	m_CameraRoot->GetProperties()->GetPropertyById('SUDR')->SetFloat(0.0f);

	m_CameraArm = m_Factory->Build();
	m_CameraArm->SetName(_T("CameraArm"));
	m_CameraRoot->AddChild(m_CameraArm);
	m_CameraArm->AddComponent(c3::Positionable::Type());
	c3::Positionable *parmpos = dynamic_cast<c3::Positionable *>(m_CameraArm->FindComponent(c3::Positionable::Type()));
	if (parmpos)
	{
		parmpos->AdjustPitch(glm::radians(-24.0f));
	}

	m_Camera = m_Factory->Build();
	m_Camera->SetName(_T("Camera"));
	m_CameraArm->AddChild(m_Camera);
	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(m_Camera->AddComponent(c3::Positionable::Type()));
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->AddComponent(c3::Camera::Type()));

	if (pcam)
	{
		pcam->SetFOV(glm::radians(78.0f));
		pcam->SetPolarDistance(0.01f);
	}

	if (pcampos)
	{
		pcampos->AdjustPos(0, -10.0f, 0);
		pcampos->AdjustPitch(glm::radians(24.0f));
	}

	m_GUICamera = m_Factory->Build();
	m_GUICamera->SetName(_T("GUI Camera"));
	c3::Positionable *puicampos = dynamic_cast<c3::Positionable *>(m_GUICamera->AddComponent(c3::Positionable::Type()));
	c3::Camera *puicam = dynamic_cast<c3::Camera *>(m_GUICamera->AddComponent(c3::Camera::Type()));

	if (puicam)
	{
		puicam->SetProjectionMode(c3::Camera::EProjectionMode::PM_ORTHOGRAPHIC);
		puicam->SetPolarDistance(10.0f);
	}

	if (puicampos)
	{
		puicampos->AdjustPos(0, 0, 10.0f);
		puicampos->SetYawPitchRoll(0, glm::radians(-90.0f), 0);
	}

	theApp.m_C3->GetGlobalObjectRegistry()->RegisterObject(c3::GlobalObjectRegistry::OD_CAMERA_ROOT, m_CameraRoot);
	theApp.m_C3->GetGlobalObjectRegistry()->RegisterObject(c3::GlobalObjectRegistry::OD_CAMERA_ARM, m_CameraArm);
	theApp.m_C3->GetGlobalObjectRegistry()->RegisterObject(c3::GlobalObjectRegistry::OD_CAMERA, m_Camera);
	theApp.m_C3->GetGlobalObjectRegistry()->RegisterObject(c3::GlobalObjectRegistry::OD_GUI_CAMERA, m_GUICamera);
	theApp.m_C3->GetLog()->Print(_T("Camera hierarchy created and registered\n"));

	props::IProperty *psp = m_WorldRoot->GetProperties()->GetPropertyById('SRCF');
	if (psp)
	{
		psp->SetString(theApp.m_StartScript.c_str());
	}

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

		CRect rect;
		GetClientRect(&rect);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);

		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		theApp.m_C3->UpdateTime();
		float dt = theApp.m_C3->GetElapsedTime();

		m_CameraRoot->Update(dt);
		m_GUICamera->Update(dt);

		c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(m_Camera->FindComponent(c3::Positionable::Type()));
		c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindComponent(c3::Camera::Type()));
		if (pcam)
		{
			m_Rend->SetViewMatrix(pcam->GetViewMatrix());
			m_Rend->SetProjectionMatrix(pcam->GetProjectionMatrix());
			m_Rend->SetEyePosition(pcam->GetEyePos());
			glm::fvec3 eyedir = glm::normalize(*pcam->GetTargetPos() - *(pcam->GetEyePos()));
			m_Rend->SetEyeDirection(&eyedir);
		}

		glm::mat4 biasmat(
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0
		);

		float farclip = m_Camera->GetProperties()->GetPropertyById('C:FC')->AsFloat();
		float nearclip = m_Camera->GetProperties()->GetPropertyById('C:NC')->AsFloat();
		glm::fmat4x4 depthProjectionMatrix = glm::ortho<float>(-55, 50, -71, 71, nearclip, farclip);
		glm::fvec3 sunpos = *(theApp.m_C3->GetEnvironment()->GetSunDirection()) * -61.5f;
		glm::fvec3 campos;
		pcam->GetEyePos(&campos);
//		sunpos += campos;
		glm::fmat4x4 depthViewMatrix = glm::lookAt(sunpos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::fmat4x4 depthMVP = depthProjectionMatrix * depthViewMatrix;
		m_Rend->SetSunShadowMatrix(&depthMVP);

		c3::Object *skybox_root = theApp.m_C3->GetGlobalObjectRegistry()->GetRegisteredObject(c3::GlobalObjectRegistry::OD_SKYBOXROOT);
		if (skybox_root)
			skybox_root->Update(dt);
		m_WorldRoot->Update(dt);
		m_GUIRoot->Update(dt);

		if (m_Rend->BeginScene(BSFLAG_SHOWGUI))
		{
			// Solid color pass
			m_Rend->SetDepthMode(c3::Renderer::DepthMode::DM_READWRITE);
			m_Rend->UseFrameBuffer(m_GBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH | UFBFLAG_CLEARSTENCIL | UFBFLAG_UPDATEVIEWPORT);
			m_Rend->SetDepthTest(c3::Renderer::Test::DT_LESSER);
			m_Rend->SetAlphaPassRange(254.9f / 255.0f);
			m_Rend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
			m_Rend->SetCullMode(c3::Renderer::CullMode::CM_BACK);
			if (skybox_root)
				skybox_root->Render();
			m_WorldRoot->Render();

			// Shadow pass
			m_Rend->SetDepthMode(c3::Renderer::DepthMode::DM_READWRITE);
			m_Rend->UseFrameBuffer(m_SSBuf, UFBFLAG_CLEARDEPTH | UFBFLAG_UPDATEVIEWPORT);
			m_WorldRoot->Render(RF_SHADOW);

			m_Rend->SetViewport();

			// Lighting pass(es)
			m_Rend->UseFrameBuffer(m_LCBuf, UFBFLAG_FINISHLAST | UFBFLAG_CLEARCOLOR | UFBFLAG_UPDATEVIEWPORT);
			m_Rend->SetDepthMode(c3::Renderer::DepthMode::DM_READONLY);
			m_Rend->SetDepthTest(c3::Renderer::Test::DT_ALWAYS);
			m_Rend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			m_WorldRoot->Render(RF_LIGHT);

			m_Rend->UseRenderMethod();
			m_Rend->UseMaterial();

			// Resolve
			m_Rend->UseFrameBuffer(m_BBuf[0], UFBFLAG_FINISHLAST | UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH);
			m_Rend->SetDepthMode(c3::Renderer::DepthMode::DM_DISABLED);
			m_Rend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			m_Rend->SetCullMode(c3::Renderer::CullMode::CM_DISABLED);
			m_Rend->UseProgram(m_SP_combine);
			m_Rend->UseVertexBuffer(m_Rend->GetFullscreenPlaneVB());
			m_Rend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);

			float bs = 2.0f;
			int maxb = (int)m_BBuf.size() - 1;
			for (int b = 0; b < maxb; b++)
			{
				m_Rend->UseFrameBuffer(m_BBuf[b + 1], UFBFLAG_FINISHLAST);
				m_Rend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
				m_Rend->UseProgram(m_SP_blur);
				m_SP_blur->SetUniformTexture(m_BTex[b], m_uBlurTex);
				m_SP_blur->SetUniform1(m_uBlurScale, bs);
				m_Rend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);
				bs *= 2.0f;
			}

			m_Rend->UseFrameBuffer(nullptr, UFBFLAG_FINISHLAST);
			m_Rend->UseProgram(m_SP_resolve);
			m_Rend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);

			c3::Positionable *puicampos = dynamic_cast<c3::Positionable *>(m_GUICamera->FindComponent(c3::Positionable::Type()));
			c3::Camera *puicam = dynamic_cast<c3::Camera *>(m_GUICamera->FindComponent(c3::Camera::Type()));
			if (puicam)
			{
				m_Rend->SetViewMatrix(puicam->GetViewMatrix());
				m_Rend->SetProjectionMatrix(puicam->GetProjectionMatrix());
				m_Rend->SetEyePosition(puicam->GetEyePos());
				glm::fvec3 eyedir = glm::normalize(*puicam->GetTargetPos() - *(puicam->GetEyePos()));
				m_Rend->SetEyeDirection(&eyedir);
			}
			m_GUIRoot->Render();

			if (m_DebugEnabled)
				theApp.m_C3->GetRenderer()->GetGui()->ShowDebugWindow(&m_DebugEnabled);

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

		RedrawWindow(nullptr, nullptr, RDW_NOERASE);
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
	DestroySurfaces();

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

	if (m_WorldRoot)
	{
		m_WorldRoot->Release();
		m_WorldRoot = nullptr;
	}

	if (m_CameraRoot)
	{
		m_CameraRoot->Release();
	}
	m_CameraRoot = m_CameraArm = m_Camera = nullptr;

	if (m_SP_combine)
	{
		m_SP_combine->Release();
		m_SP_combine = nullptr;
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



void C3Dlg::OnSize(UINT nType, int cx, int cy)
{
	RECT r;
	r.left = 0;
	r.right = cx;
	r.top = 0;
	r.bottom = cy;

	theApp.m_Config->SetRect(_T("window.rect"), r);

	CDialog::OnSize(nType, r.right, r.bottom);

//	theApp.m_C3->GetRenderer()->SetViewport(&r);

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		pcam->SetOrthoDimensions((float)r.right, (float)r.bottom);
		m_Camera->Update();
		m_Rend->SetProjectionMatrix(pcam->GetProjectionMatrix());
	}
}


void C3Dlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);
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
	if (!bActive)
	{
		SetMouseEnabled(true);
	}
}


void C3Dlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);
	if (nState != WA_INACTIVE)
	{
		theApp.m_C3->GetInputManager()->AcquireAll();
	}
	else if (!theApp.m_C3->IsSplashWnd(pWndOther->GetSafeHwnd()))
	{
		SetMouseEnabled(true);
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
	switch (nIDEvent)
	{
		case 'SIZE':
			break;
	}

	CDialog::OnTimer(nIDEvent);
}

bool __cdecl C3Dlg::DeviceConnected(c3::InputDevice *device, bool conn, void *userdata)
{
	assert(device);

	if (!conn)
		return false;

	c3::ActionMapper *pam = theApp.m_C3->GetActionMapper();

	size_t ai_mf = pam->FindActionIndex(_T("Move Forward"));
	size_t ai_mb = pam->FindActionIndex(_T("Move Backward"));
	size_t ai_sl = pam->FindActionIndex(_T("Strafe Left"));
	size_t ai_sr = pam->FindActionIndex(_T("Strafe Right"));
	size_t ai_lu = pam->FindActionIndex(_T("Look Up"));
	size_t ai_ld = pam->FindActionIndex(_T("Look Down"));
	size_t ai_ll = pam->FindActionIndex(_T("Look Left"));
	size_t ai_lr = pam->FindActionIndex(_T("Look Right"));
	size_t ai_a = pam->FindActionIndex(_T("Ascend"));
	size_t ai_d = pam->FindActionIndex(_T("Descend"));
	size_t ai_r = pam->FindActionIndex(_T("Run"));
	size_t ai_j = pam->FindActionIndex(_T("Jump"));
	size_t ai_cvm = pam->FindActionIndex(_T("Cycle View Mode"));
	size_t ai_f1 = pam->FindActionIndex(_T("Fire 1"));
	size_t ai_f2 = pam->FindActionIndex(_T("Fire 2"));
	size_t ai_td = pam->FindActionIndex(_T("Toggle Debug"));
	size_t ai_iv = pam->FindActionIndex(_T("Increase Velocity"));
	size_t ai_dv = pam->FindActionIndex(_T("Decrease Velocity"));

	switch (device->GetType())
	{
		case c3::InputDevice::DeviceType::KEYBOARD:
		{
			theApp.m_C3->GetInputManager()->AssignUser(device, 0);

			pam->MakeAssociation(ai_mf, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_POSY);
			pam->MakeAssociation(ai_mb, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_NEGY);
			pam->MakeAssociation(ai_sl, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_NEGX);
			pam->MakeAssociation(ai_sr, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_POSX);

			pam->MakeAssociation(ai_iv, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_W);
			pam->MakeAssociation(ai_dv, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_S);
			pam->MakeAssociation(ai_sl, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_A);
			pam->MakeAssociation(ai_sr, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_D);

			pam->MakeAssociation(ai_ld, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_NEGY);
			pam->MakeAssociation(ai_lu, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_POSY);
			pam->MakeAssociation(ai_ll, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_NEGX);
			pam->MakeAssociation(ai_lr, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_POSX);

			pam->MakeAssociation(ai_a, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_Q);
			pam->MakeAssociation(ai_d, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_Z);

			pam->MakeAssociation(ai_r, device->GetUID(), c3::InputDevice::VirtualButton::LSHIFT);
			pam->MakeAssociation(ai_f1, device->GetUID(), c3::InputDevice::VirtualButton::SELECT);
			pam->MakeAssociation(ai_f2, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_X);

			pam->MakeAssociation(ai_cvm, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_C);

			pam->MakeAssociation(ai_td, device->GetUID(), c3::InputDevice::VirtualButton::DEBUGBUTTON);

			break;
		}

		case c3::InputDevice::DeviceType::MOUSE:
		{
			theApp.m_C3->GetInputManager()->AssignUser(device, 0);

			pam->MakeAssociation(ai_f1, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON1);
			pam->MakeAssociation(ai_f2, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON2);

			pam->MakeAssociation(ai_iv, device->GetUID(), c3::InputDevice::VirtualButton::THROTTLE1);
			pam->MakeAssociation(ai_dv, device->GetUID(), c3::InputDevice::VirtualButton::THROTTLE2);
			pam->MakeAssociation(ai_iv, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_POSZ);
			pam->MakeAssociation(ai_dv, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_NEGZ);

			pam->MakeAssociation(ai_lu, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_NEGY);
			pam->MakeAssociation(ai_ld, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_POSY);
			pam->MakeAssociation(ai_ll, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_NEGX);
			pam->MakeAssociation(ai_lr, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_POSX);

			break;
		}

		case c3::InputDevice::DeviceType::JOYSTICK:
		{
			theApp.m_C3->GetInputManager()->AssignUser(device, 1);

			if (device->GetNumAxes() > 1)
			{
				pam->MakeAssociation(ai_mf, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_POSY);
				pam->MakeAssociation(ai_mb, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_NEGY);
				pam->MakeAssociation(ai_sl, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_NEGX);
				pam->MakeAssociation(ai_sr, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_POSX);

				pam->MakeAssociation(ai_ld, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_NEGY);
				pam->MakeAssociation(ai_lu, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_POSY);
				pam->MakeAssociation(ai_ll, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_NEGX);
				pam->MakeAssociation(ai_lr, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_POSX);

				pam->MakeAssociation(ai_iv, device->GetUID(), c3::InputDevice::VirtualButton::THROTTLE1);
				pam->MakeAssociation(ai_dv, device->GetUID(), c3::InputDevice::VirtualButton::THROTTLE2);
				pam->MakeAssociation(ai_iv, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_POSZ);
				pam->MakeAssociation(ai_dv, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_NEGZ);

				pam->MakeAssociation(ai_r, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON2);
				pam->MakeAssociation(ai_j, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON1);
				pam->MakeAssociation(ai_f1, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON3);
				pam->MakeAssociation(ai_f2, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON4);

				pam->MakeAssociation(ai_a, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON6);
				pam->MakeAssociation(ai_d, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON5);

				pam->MakeAssociation(ai_cvm, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON3);
			}
			else
			{
				pam->MakeAssociation(ai_mf, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON1);
				pam->MakeAssociation(ai_mb, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON2);
				pam->MakeAssociation(ai_sl, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON3);
				pam->MakeAssociation(ai_sr, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON4);

				pam->MakeAssociation(ai_ld, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_NEGY);
				pam->MakeAssociation(ai_lu, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_POSY);
				pam->MakeAssociation(ai_ll, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_NEGX);
				pam->MakeAssociation(ai_lr, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_POSX);
			}

			break;
		}
	}

	return true;
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

	int deltax = cpc.x - point.x;
	int deltay = cpc.y - point.y;

	SetCursorPos(cp.x, cp.y);
}


BOOL C3Dlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	theApp.m_C3->GetRenderer()->GetGui()->AddMouseWheelEvent(0, (float)zDelta);

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}


void C3Dlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	theApp.m_C3->GetRenderer()->GetGui()->AddMouseButtonEvent(c3::Gui::MouseButton::MBUT_LEFT, true);

	CDialog::OnLButtonDown(nFlags, point);
}


void C3Dlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	theApp.m_C3->GetRenderer()->GetGui()->AddMouseButtonEvent(c3::Gui::MouseButton::MBUT_LEFT, false);

	CDialog::OnLButtonUp(nFlags, point);
}


void C3Dlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	theApp.m_C3->GetRenderer()->GetGui()->AddMouseButtonEvent(c3::Gui::MouseButton::MBUT_MIDDLE, true);

	CDialog::OnMButtonDown(nFlags, point);
}


void C3Dlg::OnMButtonUp(UINT nFlags, CPoint point)
{
	theApp.m_C3->GetRenderer()->GetGui()->AddMouseButtonEvent(c3::Gui::MouseButton::MBUT_MIDDLE, false);

	CDialog::OnMButtonUp(nFlags, point);
}


void C3Dlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	theApp.m_C3->GetRenderer()->GetGui()->AddMouseButtonEvent(c3::Gui::MouseButton::MBUT_RIGHT, true);

	CDialog::OnRButtonDown(nFlags, point);
}


void C3Dlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	theApp.m_C3->GetRenderer()->GetGui()->AddMouseButtonEvent(c3::Gui::MouseButton::MBUT_MIDDLE, false);

	CDialog::OnRButtonUp(nFlags, point);
}
