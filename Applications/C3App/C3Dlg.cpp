
// C3Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"

#include "C3App.h"
#include "C3Dlg.h"
#include "afxdialogex.h"
#include <C3Gui.h>
#include <Pool.h>
#include <C3Utility.h>
#include <C3InputDevice.h>

#include <C3CommonVertexDefs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



C3Dlg::C3Dlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_C3APP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_GBuf = nullptr;
	m_LCBuf = nullptr;
	m_AuxBuf = nullptr;
	m_SSBuf = nullptr;
	m_DepthTarg = nullptr;
	m_ShadowTarg = nullptr;
	m_BTex = { };
	m_BBuf = { };
	m_VS_resolve = nullptr;
	m_FS_resolve = nullptr;
	m_SP_resolve = nullptr;
	m_VS_blur = nullptr;
	m_FS_blur = nullptr;
	m_SP_blur = nullptr;
	m_VS_combine = nullptr;
	m_FS_combine = nullptr;
	m_SP_combine = nullptr;
	m_VS_bounds = nullptr;
	m_FS_bounds = nullptr;
	m_SP_bounds = nullptr;

	m_ulSunDir = -1;
	m_ulSunColor = -1;
	m_ulAmbientColor = -1;
	m_ulFogColor = -1;
	m_ulFogDensity = -1;
	m_ulFogStart = -1;
	m_ulFogEnd = -1;

	m_uBlurTex = -1;
	m_uBlurScale = -1;

	m_ShowDebug = false;

	m_bSurfacesCreated = false;
	m_bSurfacesReady = false;

	m_Camera = m_CameraRoot = m_CameraArm = m_GUICamera = nullptr;
	m_WorldRoot = m_GUIRoot = nullptr;

	m_MotionBlur = true;
	m_MotionUpdateIdx = 0;

	c3::InputManager::SetDeviceConnectionCallback(DeviceConnected, this);
}


C3Dlg::~C3Dlg()
{
	C3_SAFERELEASE(m_WorldRoot);
	C3_SAFERELEASE(m_CameraRoot);
	C3_SAFERELEASE(m_GUIRoot);
	C3_SAFERELEASE(m_GUICamera);

	DestroySurfaces();

	C3_SAFERELEASE(m_SSBuf);
	C3_SAFERELEASE(m_ShadowTarg);

	C3_SAFERELEASE(m_SP_resolve);
	C3_SAFERELEASE(m_SP_blur);
	C3_SAFERELEASE(m_SP_combine);

	C3_SAFERELEASE(m_SP_bounds);

	c3::Renderer *prend = theApp.m_C3->GetRenderer();
	assert(prend);

	prend->Shutdown();


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
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_TIMER()
	ON_WM_MOVE()
END_MESSAGE_MAP()


void C3Dlg::DestroySurfaces()
{
	c3::util::RecursiveObjectAction(m_WorldRoot, [](c3::Object *pobj)
	{
		pobj->PropertyChanged(pobj->GetProperties()->GetPropertyById('C3RM'));
	});

	c3::util::RecursiveObjectAction(m_GUIRoot, [](c3::Object *pobj)
	{
		pobj->PropertyChanged(pobj->GetProperties()->GetPropertyById('C3RM'));
	});

	C3_SAFERELEASE(m_GBuf);

	C3_SAFERELEASE(m_LCBuf);

	C3_SAFERELEASE(m_EffectsBuf);

	C3_SAFERELEASE(m_AuxBuf);

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

	c3::FrameBuffer::TargetDesc EffectsTargData[] =
	{
		{ _T("uSamplerEffectsColor"), c3::Renderer::TextureType::U8_3CH, TEXCREATEFLAG_RENDERTARGET },	// diffuse color (rgb) and metalness (a)
	};

	theApp.m_C3->GetLog()->Print(_T("Creating Effects Buffer... "));
	if (!m_EffectsBuf)
		m_EffectsBuf = prend->CreateFrameBuffer(0, _T("EffectsBuffer"));
	if (m_EffectsBuf)
	{
		gbok = m_EffectsBuf->Setup(_countof(EffectsTargData), EffectsTargData, m_DepthTarg, r) == c3::FrameBuffer::RETURNCODE::RET_OK;
		m_EffectsBuf->SetClearColor(c3::Color::fBlackFT, 0);
		m_EffectsBuf->SetBlendEquation(c3::Renderer::BlendEquation::BE_ADD, 0);
		m_EffectsBuf->SetAlphaBlendEquation(c3::Renderer::BlendEquation::BE_ADD, 0);
	}
	theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

	for (size_t c = 0; c < BLURTARGS; c++)
	{
		m_BTex.push_back(prend->CreateTexture2D(w, h, c3::Renderer::TextureType::U8_3CH, 0, TEXCREATEFLAG_RENDERTARGET));
		m_BBuf.push_back(prend->CreateFrameBuffer());
		m_BBuf[c]->AttachDepthTarget(m_DepthTarg);
		m_BBuf[c]->AttachColorTarget(m_BTex[c], 0);
		m_BBuf[c]->Seal();
		w /= 2;
		h /= 2;
	}

	if (m_MotionBlur)
	{
		// These are my pseudo-motion-blur targets
		w = m_BTex[BLURTARGS / 2]->Width();
		h = m_BTex[BLURTARGS / 2]->Height();
		for (size_t c = 0; c < BLURTARGS; c++)
		{
			m_BTex.push_back(prend->CreateTexture2D(w, h, c3::Renderer::TextureType::U8_3CH, 0, TEXCREATEFLAG_RENDERTARGET));
			m_BBuf.push_back(prend->CreateFrameBuffer());
			m_BBuf.back()->AttachDepthTarget(m_DepthTarg);
			m_BBuf.back()->AttachColorTarget(m_BTex.back(), 0);
			m_BBuf.back()->Seal();
		}
	}

	c3::FrameBuffer::TargetDesc LCBufTargData[] =
	{
		{ _T("uSamplerLights"), c3::Renderer::TextureType::F16_3CH, TEXCREATEFLAG_RENDERTARGET },
	};

	theApp.m_C3->GetLog()->Print(_T("Creating light combine buffer... "));
	if (!m_LCBuf)
		m_LCBuf = prend->CreateFrameBuffer(0, _T("LightCombine"));
	if (m_LCBuf)
	{
		gbok = m_LCBuf->Setup(_countof(LCBufTargData), LCBufTargData, m_DepthTarg, r) == c3::FrameBuffer::RETURNCODE::RET_OK;
		m_LCBuf->SetClearColor(c3::Color::fBlack, 0);
	}
	theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

	CRect auxr = r;

	c3::FrameBuffer::TargetDesc AuxBufTargData[] =
	{
		{ _T("uSamplerAuxiliary"), c3::Renderer::TextureType::F32_4CH, TEXCREATEFLAG_RENDERTARGET },
	};

	theApp.m_C3->GetLog()->Print(_T("Creating auxiliary buffer... "));
	if (!m_AuxBuf)
		m_AuxBuf = prend->CreateFrameBuffer(0, _T("Aux"));
	if (m_AuxBuf)
		gbok = m_AuxBuf->Setup(_countof(AuxBufTargData), AuxBufTargData, m_DepthTarg, auxr) == c3::FrameBuffer::RETURNCODE::RET_OK;
	theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));
	m_AuxBuf->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);

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
		m_SP_resolve->SetUniformTexture(m_BTex[0], ut, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);

		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerSceneMip1"));
		m_SP_resolve->SetUniformTexture(m_BTex[1], ut, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);

		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerSceneMip2"));
		m_SP_resolve->SetUniformTexture(m_BTex[2], ut, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);

		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerSceneMip2_1"));
		m_SP_resolve->SetUniformTexture(m_BTex[4], ut, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);

		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerSceneMip2_2"));
		m_SP_resolve->SetUniformTexture(m_BTex[5], ut, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);

		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerSceneMip2_3"));
		m_SP_resolve->SetUniformTexture(m_BTex[6], ut, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);

		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerSceneMip2_4"));
		m_SP_resolve->SetUniformTexture(m_BTex[7], ut, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);

		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerSceneMip3"));
		m_SP_resolve->SetUniformTexture(m_BTex[3], ut, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);

		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerPosDepth"));
		m_SP_resolve->SetUniformTexture(m_GBuf->GetColorTargetByName(_T("uSamplerPosDepth")), ut, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);
	}

	if (m_SP_combine)
	{
		uint32_t i;
		int32_t ul;
		for (i = 0; i < m_GBuf->GetNumColorTargets(); i++)
		{
			c3::Texture2D* pt = m_GBuf->GetColorTarget(i);
			ul = m_SP_combine->GetUniformLocation(pt->GetName());
			m_SP_combine->SetUniformTexture((ul != c3::ShaderProgram::INVALID_UNIFORM) ? pt : prend->GetBlackTexture(), ul, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);
		}

		for (i = 0; i < m_LCBuf->GetNumColorTargets(); i++)
		{
			c3::Texture2D* pt = m_LCBuf->GetColorTarget(i);
			ul = m_SP_combine->GetUniformLocation(pt->GetName());
			m_SP_combine->SetUniformTexture((ul != c3::ShaderProgram::INVALID_UNIFORM) ? pt : prend->GetBlackTexture(), ul, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);
		}

		ul = m_SP_combine->GetUniformLocation(_T("uSamplerShadow"));
		if (ul >= 0)
			m_SP_combine->SetUniformTexture((c3::Texture *)m_ShadowTarg, ul, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);

		ul = m_SP_combine->GetUniformLocation(_T("uSamplerEffectsColor"));
		if (ul >= 0)
		{
			c3::Texture *pt = m_EffectsBuf ? (c3::Texture*)m_EffectsBuf->GetColorTarget(0) : prend->GetBlackTexture();
			m_SP_combine->SetUniformTexture(pt, ul, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);
		}

		ul = m_SP_combine->GetUniformLocation(_T("uSamplerAuxiliary"));
		if (ul >= 0)
		{
			c3::Texture *pt = m_AuxBuf ? (c3::Texture*)m_AuxBuf->GetColorTarget(0) : prend->GetBlackTexture();
			m_SP_combine->SetUniformTexture(pt, ul, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);
		}
	}
}


void C3Dlg::InitializeGraphics()
{
	c3::Renderer* prend = theApp.m_C3->GetRenderer();
	assert(prend);

#if 0
	// Compensate for screen scaling in Windows 10+
	m_WindowsUIScale = GetDC()->GetDeviceCaps(LOGPIXELSX) / 96.0f;
	if ((m_WindowsUIScale > 1.0f) && (m_WindowsUIScale < 1.1f))
#endif
		m_WindowsUIScale = 1.0f;

	theApp.m_C3->GetLog()->Print(_T("Initializing Renderer... "));

	if (prend->Initialize(GetSafeHwnd(), 0))
	{
		bool gbok;

		prend->GetFont(_T("Arial"), 16);

		c3::ResourceManager* rm = theApp.m_C3->GetResourceManager();

		prend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);

		theApp.m_C3->GetLog()->Print(_T("Creating shadow buffer... "));

		if (!m_ShadowTarg)
			m_ShadowTarg = prend->CreateDepthBuffer(2048, 2048, c3::Renderer::DepthType::F32_SHADOW);

		if (!m_SSBuf)
			m_SSBuf = prend->CreateFrameBuffer(0, _T("Shadow"));
		if (m_SSBuf)
			m_SSBuf->AttachDepthTarget(m_ShadowTarg);
		gbok = m_SSBuf->Seal() == c3::FrameBuffer::RETURNCODE::RET_OK;
		theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

		m_VS_blur = (c3::ShaderComponent*)((rm->GetResource(_T("blur.vsh"), RESF_DEMANDLOAD))->GetData());
		m_FS_blur = (c3::ShaderComponent*)((rm->GetResource(_T("blur.fsh"), RESF_DEMANDLOAD))->GetData());
		if (!m_SP_blur)
			m_SP_blur = prend->CreateShaderProgram();
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
		m_FS_resolve = (c3::ShaderComponent*)((rm->GetResource(m_MotionBlur ? _T("resolve_mb.fsh") : _T("resolve.fsh"), RESF_DEMANDLOAD))->GetData());
		if (!m_SP_resolve)
			m_SP_resolve = prend->CreateShaderProgram();
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
			m_SP_combine = prend->CreateShaderProgram();
		if (m_SP_combine)
		{
			m_SP_combine->AttachShader(m_VS_combine);
			m_SP_combine->AttachShader(m_FS_combine);
			if (m_SP_combine->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
			{
				m_ulSunDir = m_SP_combine->GetUniformLocation(_T("uSunDirection"));
				m_ulSunColor = m_SP_combine->GetUniformLocation(_T("uSunColor"));
				m_ulAmbientColor = m_SP_combine->GetUniformLocation(_T("uAmbientColor"));
				m_ulFogColor = m_SP_combine->GetUniformLocation(_T("uFogColor"));
				m_ulFogDensity = m_SP_combine->GetUniformLocation(_T("uFogDensity"));
				m_ulFogStart = m_SP_combine->GetUniformLocation(_T("uFogStart"));
				m_ulFogEnd = m_SP_combine->GetUniformLocation(_T("uFogEnd"));
			}
		}

		m_VS_bounds = (c3::ShaderComponent*)((rm->GetResource(_T("def-obj.vsh"), RESF_DEMANDLOAD))->GetData());
		m_FS_bounds = (c3::ShaderComponent*)((rm->GetResource(_T("editor-select.fsh"), RESF_DEMANDLOAD))->GetData());
		if (!m_SP_bounds)
			m_SP_bounds = prend->CreateShaderProgram();
		if (m_SP_bounds)
		{
			m_SP_bounds->AttachShader(m_VS_bounds);
			m_SP_bounds->AttachShader(m_FS_bounds);
			if (m_SP_bounds->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
			{
			}
		}

		CreateSurfaces();
	}
}

void C3Dlg::ComputePickRay(c3::Object *cam, POINT screenpos, glm::fvec3 &pickpos, glm::fvec3 &pickvec) const
{
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(cam->FindComponent(c3::Camera::Type()));
	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(cam->FindComponent(c3::Positionable::Type()));

	glm::fmat4x4 viewmat, projmat;

	// Get the current projection and view matrices from d3d
	pcam->GetViewMatrix(&viewmat);
	pcam->GetProjectionMatrix(&projmat);

	CRect r;
	GetClientRect(&r);
	ClientToScreen(&r);
	float pctx = 1.0f - ((float)screenpos.x / (float)r.Width());
	float pcty = 1.0f - ((float)screenpos.y / (float)r.Height());

	size_t w = (size_t)((float)r.Width() / m_WindowsUIScale);
	size_t h = (size_t)((float)r.Height() / m_WindowsUIScale);

	float rposx = pctx * w;
	float rposy = pcty * h;

	// Construct a viewport that desribes our view metric
	glm::fvec4 viewport(0, 0, w, h);

	glm::fvec3 pos3d_near(rposx, rposy, 0.0f);
	glm::fvec3 pos3d_far(rposx, rposy, 1.0f);

	pickpos = glm::unProject(pos3d_near, viewmat, projmat, viewport);
	pos3d_far = glm::unProject(pos3d_far, viewmat, projmat, viewport);

	pickvec = glm::normalize(pos3d_far - pickpos);
}


BOOL C3Dlg::OnInitDialog()
{
	CString title;
	title.Format(_T("%s%s%s"),
		theApp.m_AppName.c_str(),
		theApp.m_StartScript.empty() ? _T("") : _T("  -  "),
		theApp.m_StartScript.empty() ? _T("") : theApp.m_StartScript.c_str());
	SetWindowText(title);

	CRect dr;
	::GetWindowRect(GetDesktopWindow()->GetSafeHwnd(), dr);

	CRect qr = dr;
	qr.DeflateRect(dr.Width() / 5, dr.Height() / 5);

	CRect r;
	theApp.m_Config->GetRect(_T("window.rect"), r, &qr);

	if (r.left < dr.left)
		r.OffsetRect(dr.left - r.left, 0);

	if (r.top < dr.top)
		r.OffsetRect(0, dr.top - r.top);

	if (r.right > dr.right)
		r.OffsetRect(dr.right - r.right, 0);

	if (r.bottom > dr.bottom)
		r.OffsetRect(0, dr.bottom - r.bottom);

	if ((r.Width() > dr.Width()) || (r.Height() > dr.Height()))
		r = dr;

	if (!CDialog::OnInitDialog())
		return FALSE;

	MoveWindow(r);

	GetClientRect(r);

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

	c3::Factory *pfac = theApp.m_C3->GetFactory();

	theApp.m_C3->GetLog()->Print(_T("Creating InputManager... "));
	c3::InputManager *inputman = theApp.m_C3->GetInputManager();
	if (!inputman)
	{
		theApp.m_C3->GetLog()->Print(_T("failed\n"));
		exit(-1);
	}
	theApp.m_C3->GetLog()->Print(_T("ok\n"));

	theApp.m_C3->GetLog()->Print(_T("Setting up actions... "));

	theApp.m_C3->GetActionMapper()->RegisterAction(_T("Toggle Debug"), c3::ActionMapper::ETriggerType::UP_DELTA, 0,
		[](const TCHAR *action_name, c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
		{
			*((bool *)userdata) ^= true;
			return true;
		},
		&m_ShowDebug);

	theApp.m_C3->GetLog()->Print(_T("ok\n"));

	InitializeGraphics();

	m_WorldRoot = pfac->Build();
	m_WorldRoot->SetName(_T("WORLD ROOT"));
	m_WorldRoot->AddComponent(c3::Positionable::Type());
	m_WorldRoot->AddComponent(c3::Scriptable::Type());
	m_WorldRoot->Flags().Set(OF_LIGHT | OF_CASTSHADOW);
	theApp.m_C3->GetGlobalObjectRegistry()->RegisterObject(c3::GlobalObjectRegistry::OD_WORLDROOT, m_WorldRoot);
	theApp.m_C3->GetLog()->Print(_T("World root created and registered\n"));

	m_GUIRoot = pfac->Build();
	m_GUIRoot->SetName(_T("GUI ROOT"));
	m_GUIRoot->AddComponent(c3::Positionable::Type());
	m_GUIRoot->AddComponent(c3::Scriptable::Type());
	theApp.m_C3->GetGlobalObjectRegistry()->RegisterObject(c3::GlobalObjectRegistry::OD_GUI_ROOT, m_GUIRoot);
	theApp.m_C3->GetLog()->Print(_T("GUI root created and registered\n"));

	m_CameraRoot = pfac->Build();
	m_CameraRoot->SetName(_T("CameraRoot"));
	m_CameraRoot->AddComponent(c3::Positionable::Type());

	m_CameraArm = pfac->Build();
	m_CameraArm->SetName(_T("CameraArm"));
	m_CameraRoot->AddChild(m_CameraArm);
	m_CameraArm->AddComponent(c3::Positionable::Type());
	c3::Positionable *parmpos = dynamic_cast<c3::Positionable *>(m_CameraArm->FindComponent(c3::Positionable::Type()));
	if (parmpos)
	{
		parmpos->AdjustPitch(glm::radians(0.01f));
	}

	m_Camera = pfac->Build();
	m_Camera->SetName(_T("Camera"));
	m_CameraArm->AddChild(m_Camera);
	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(m_Camera->AddComponent(c3::Positionable::Type()));
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->AddComponent(c3::Camera::Type()));
	//m_Camera->AddComponent(c3::Physical::Type());

	if (pcam)
	{
		pcam->SetFOV(65.0f);
		pcam->SetPolarDistance(0.01f);
	}

	m_GUICamera = pfac->Build();
	m_GUICamera->SetName(_T("GUI Camera"));

	c3::Camera *puicam = dynamic_cast<c3::Camera *>(m_GUICamera->AddComponent(c3::Camera::Type()));
	if (puicam)
	{
		puicam->SetProjectionMode(c3::Camera::EProjectionMode::PM_ORTHOGRAPHIC);
		puicam->SetOrthoDimensions((float)r.Width(), (float)r.Height());
		puicam->SetPolarDistance(10.0f);
	}

	c3::Positionable *puicampos = dynamic_cast<c3::Positionable *>(m_GUICamera->AddComponent(c3::Positionable::Type()));
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

	// maybe we're supposed to start a script...
	const TCHAR *ext = PathFindExtension(theApp.m_StartScript.c_str());
	if (!_tcsicmp(ext, _T(".c3js")))
	{
		props::IProperty *psp = m_WorldRoot->GetProperties()->GetPropertyById('SRCF');
		if (psp)
		{
			psp->SetString(theApp.m_StartScript.c_str());
		}
	}
	// ...but maybe we're supposed to load a level.
	else if (!_tcsicmp(ext, _T(".c3o")))
	{
		genio::IInputStream *is = genio::IInputStream::Create();
		if (is)
		{
			is->Assign(theApp.m_StartScript.c_str());
			if (is->Open())
			{
				// use the camera from the editor to start
				c3::Object::CameraLoadFunc loadcam = [&](c3::Object *camera, float yaw, float pitch)
				{
					// release the old camera
					if (m_Camera)
						m_Camera->Release();

					// use the one we loaded
					m_Camera = camera;
					m_CameraArm->AddChild(m_Camera);
					theApp.m_C3->GetGlobalObjectRegistry()->RegisterObject(c3::GlobalObjectRegistry::OD_CAMERA, m_Camera);
				};

				m_WorldRoot->Load(is, nullptr, nullptr);
				is->Close();
			}

			is->Release();
		}
	}

	inputman->Reset();

	UINT_PTR timerid = SetTimer('DRAW', 16, nullptr);
	assert(timerid);

	theApp.m_pActiveWnd = this;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void C3Dlg::OnPaint()
{
	CRect r;
	GetClientRect(&r);

	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);

		int x = (r.Width() - cxIcon + 1) / 2;
		int y = (r.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		c3::Renderer *prend = theApp.m_C3->GetRenderer();
		assert(prend);

		if (!prend->Initialized())
			InitializeGraphics();

		if (prend && prend->Initialized())
		{

#ifndef TIMERS_WORK
			// see if we need to resize the drawing surface
			size_t w = (size_t)((float)r.Width() / m_WindowsUIScale);
			size_t h = (size_t)((float)r.Height() / m_WindowsUIScale);
			if (m_GBuf && (m_GBuf->GetDepthTarget()->Width() != w) || (m_GBuf->GetDepthTarget()->Height() != h))
			{
				static ULONGLONG resize_time = 0;
				// if enough time has passed, then see if we need to resize
				if ((GetTickCount64() - resize_time) > 2000)
				{
					resize_time = GetTickCount64();
					OnTimer('SIZE');
				}
			}
#endif

			theApp.m_C3->UpdateTime();
			float dt = theApp.m_C3->GetElapsedTime();

			prend->SetOverrideHwnd(GetSafeHwnd());
			prend->SetViewport(r);

			c3::Environment *penv = theApp.m_C3->GetEnvironment();
			assert(penv);

			c3::Object *camrootobj = theApp.m_C3->GetGlobalObjectRegistry()->GetRegisteredObject(c3::GlobalObjectRegistry::OD_CAMERA_ROOT);
			if (camrootobj)
				camrootobj->Update(dt);
			c3::Object *camobj = theApp.m_C3->GetGlobalObjectRegistry()->GetRegisteredObject(c3::GlobalObjectRegistry::OD_CAMERA);
			c3::Positionable *campos = m_Camera ? dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type())) : nullptr;
			c3::Camera *cam = camobj ? dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type())) : nullptr;

			c3::Object *guicamobj = theApp.m_C3->GetGlobalObjectRegistry()->GetRegisteredObject(c3::GlobalObjectRegistry::OD_GUI_CAMERA);
			if (guicamobj)
				guicamobj->Update(dt);
			c3::Positionable *guicampos = m_Camera ? dynamic_cast<c3::Positionable *>(guicamobj->FindComponent(c3::Positionable::Type())) : nullptr;
			c3::Camera *guicam = camobj ? dynamic_cast<c3::Camera *>(guicamobj->FindComponent(c3::Camera::Type())) : nullptr;

			theApp.m_C3->GetSoundPlayer()->SetListenerPos(campos->GetPosVec());
			theApp.m_C3->GetSoundPlayer()->SetListenerDir(campos->GetFacingVector());
			theApp.m_C3->GetSoundPlayer()->SetListenerRadius(1000.0f, 10000.0f);

			cam->SetOrthoDimensions((float)r.Width(), (float)r.Height());
			float farclip = cam->GetFarClipDistance();
			float nearclip = cam->GetNearClipDistance();

			glm::fvec3 pickpos, pickdir;

			c3::Object *world = theApp.m_C3->GetGlobalObjectRegistry()->GetRegisteredObject(c3::GlobalObjectRegistry::OD_WORLDROOT);
			c3::Object *skybox = theApp.m_C3->GetGlobalObjectRegistry()->GetRegisteredObject(c3::GlobalObjectRegistry::OD_SKYBOXROOT);
			c3::Object *gui = theApp.m_C3->GetGlobalObjectRegistry()->GetRegisteredObject(c3::GlobalObjectRegistry::OD_GUI_ROOT);

			c3::InputManager *inputman = theApp.m_C3->GetInputManager();

			if (!inputman->MouseEnabled())
			{
				if (skybox)
					skybox->Update(dt);

				ComputePickRay(camobj, m_MousePos, pickpos, pickdir);
				theApp.m_C3->GetInputManager()->SetPickRay(pickpos, pickdir);
				if (world)
					world->Update(dt);
			}
			else
			{
				ComputePickRay(guicamobj, m_MousePos, pickpos, pickdir);
				theApp.m_C3->GetInputManager()->SetPickRay(pickpos, pickdir);
				if (gui)
					gui->Update(dt);
			}

			glm::fvec4 cc = glm::fvec4(*penv->GetBackgroundColor(), 0);
			prend->SetClearColor(&cc);
			m_GBuf->SetClearColor(cc, 0);
			m_GBuf->SetClearColor(c3::Color::fBlack, 1);
			m_GBuf->SetClearColor(glm::fvec4(0, 0, 0, farclip), 2);
			m_GBuf->SetClearColor(c3::Color::fBlack, 3);
			m_GBuf->SetClearDepth(1.0f);

			m_GBuf->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
			m_GBuf->SetAlphaBlendMode(c3::Renderer::BlendMode::BM_REPLACE);

			prend->SetClearDepth(1.0f);

			if (cam)
			{
				prend->SetViewMatrix(cam->GetViewMatrix());
				prend->SetProjectionMatrix(cam->GetProjectionMatrix());
				prend->SetNearClipDistance(cam->GetNearClipDistance());
				prend->SetFarClipDistance(cam->GetFarClipDistance());
				prend->SetEyePosition(cam->GetEyePos());
				prend->SetEyeDirection(campos->GetFacingVector());
			}

			m_SP_combine->SetUniform3(m_ulAmbientColor, penv->GetAmbientColor());
			m_SP_combine->SetUniform3(m_ulSunColor, penv->GetSunColor());
			m_SP_combine->SetUniform3(m_ulSunDir, penv->GetSunDirection());
			m_SP_combine->SetUniform4(m_ulFogColor, penv->GetFogColor());
			m_SP_combine->SetUniform1(m_ulFogDensity, penv->GetFogDensity());
			m_SP_combine->SetUniform1(m_ulFogStart, penv->GetFogStart());
			m_SP_combine->SetUniform1(m_ulFogEnd, penv->GetFogEnd());

			if (prend->BeginScene(BSFLAG_SHOWGUI))
			{
				prend->SetClearColor(&c3::Color::fBlackFT);
				prend->SetTextureTransformMatrix(nullptr);
				prend->UseFrameBuffer(m_EffectsBuf, UFBFLAG_CLEARCOLOR);

				// selection: 0.0, near fog: 1.0, far fog: 0.0, cos fog norm: 0.0
				float fogstart = penv->GetFogStart();
				float fogend = penv->GetFogEnd();
				if (fogstart == fogend)
				{
					fogstart = prend->GetNearClipDistance();
					fogend = prend->GetFarClipDistance();
				}
				glm::fvec4 auxcc(0, fogstart, fogend, 0);
				prend->SetClearColor(&auxcc);
				prend->SetChannelWriteMask(CM_RED | CM_GREEN | CM_BLUE | CM_ALPHA);
				prend->UseFrameBuffer(m_AuxBuf, UFBFLAG_CLEARCOLOR);

				prend->SetClearColor(&cc);

				// Solid color pass
				prend->UseFrameBuffer(m_GBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH | UFBFLAG_CLEARSTENCIL | UFBFLAG_UPDATEVIEWPORT);
				prend->SetDepthMode(c3::Renderer::DepthMode::DM_READWRITE);
				prend->SetDepthTest(c3::Renderer::Test::DT_LESSER);
				prend->SetAlphaPassRange(254.0f / 255.0f);
				prend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
				prend->SetCullMode(c3::Renderer::CullMode::CM_BACK);
				props::TFlags64 renderflags = 0;

				if (skybox)
				{
					c3::RenderMethod::ForEachOrderedDrawDo([&](int order)
					{
						skybox->Render(renderflags, order);
					});

					// draw the skybox objects and then clear the depth buffer
					m_GBuf->Clear(UFBFLAG_CLEARDEPTH);
				}

				if (world)
				{
					c3::RenderMethod::ForEachOrderedDrawDo([&](int order)
					{
						world->Render(renderflags, order);
					});
				}

				// after the main pass, clear everything with black...
				prend->SetClearColor(&c3::Color::fBlack);
				m_LCBuf->SetClearColor(c3::Color::fBlack, 0);

				// Lighting pass(es)
				prend->UseFrameBuffer(m_LCBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_UPDATEVIEWPORT); // | UFBFLAG_FINISHLAST);
				prend->SetDepthMode(c3::Renderer::DepthMode::DM_READONLY);
				prend->SetDepthTest(c3::Renderer::Test::DT_ALWAYS);
				prend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
				if (world)
				{
					c3::RenderMethod::ForEachOrderedDrawDo([&](int order)
					{
						world->Render((uint64_t)renderflags | RF_LIGHT, order);
					});

					// Shadow pass
					
					// use the sun direction to determine its position - in the opposite direction
					static float sunposmult = -800.0f;

					// Set up our shadow transforms
					glm::fmat4x4 depthProjectionMatrix = glm::ortho<float>(-800, 800, -800, 800, nearclip, farclip);
					glm::fvec3 sunpos;
					penv->GetSunDirection(&sunpos);
					sunpos *= sunposmult;

					glm::fvec3 eyepos;
					cam->GetEyePos(&eyepos);
					glm::fmat4x4 depthViewMatrix = glm::lookAt(eyepos + sunpos, eyepos, glm::vec3(0, 1, 0));
					glm::fmat4x4 depthMVP = depthProjectionMatrix * depthViewMatrix;

					prend->SetSunShadowMatrix(&depthMVP);
					prend->SetViewMatrix(&depthViewMatrix);
					prend->SetProjectionMatrix(&depthProjectionMatrix);
					prend->SetNearClipDistance(cam->GetNearClipDistance());
					prend->SetFarClipDistance(cam->GetFarClipDistance());

					prend->SetDepthMode(c3::Renderer::DepthMode::DM_READWRITE);
					prend->UseFrameBuffer(m_SSBuf, UFBFLAG_CLEARDEPTH | UFBFLAG_UPDATEVIEWPORT);

					c3::RenderMethod::ForEachOrderedDrawDo([&](int order)
					{
						world->Render((uint64_t)renderflags | RF_SHADOW, order);
					});
				}

				// clear the render method and material
				prend->UseRenderMethod();
				prend->UseMaterial();

				if (cam)
				{
					prend->SetViewMatrix(cam->GetViewMatrix());
					prend->SetProjectionMatrix(cam->GetProjectionMatrix());
					prend->SetNearClipDistance(cam->GetNearClipDistance());
					prend->SetFarClipDistance(cam->GetFarClipDistance());
					prend->SetEyePosition(cam->GetEyePos());
					prend->SetEyeDirection(campos->GetFacingVector());
				}

				// Resolve
				prend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
				m_BBuf[0]->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
				prend->UseFrameBuffer(m_BBuf[0], UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH | UFBFLAG_UPDATEVIEWPORT);
				prend->SetWindingOrder(c3::Renderer::WindingOrder::WO_CW);
				prend->SetDepthMode(c3::Renderer::DepthMode::DM_DISABLED);
				prend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
				prend->SetCullMode(c3::Renderer::CullMode::CM_DISABLED);
				prend->UseProgram(m_SP_combine);
				prend->UseVertexBuffer(prend->GetFullscreenPlaneVB());
				prend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);

				c3::FrameBuffer *mbfb[] =
				{
					m_BBuf[BLURTARGS / 2],
					m_BBuf[BLURTARGS],
					m_BBuf[BLURTARGS + 1],
					m_BBuf[BLURTARGS + 2],
					m_BBuf[BLURTARGS + 3]
				};

				c3::Texture2D *mbtx[] =
				{
					m_BTex[BLURTARGS / 2],
					m_BTex[BLURTARGS],
					m_BTex[BLURTARGS + 1],
					m_BTex[BLURTARGS + 2],
					m_BTex[BLURTARGS + 3]
				};

				c3::Texture2D *curtex;
				c3::FrameBuffer *curfb;

				float bs = 2.0f;
				for (int b = 0; b < BLURTARGS - 1; b++)
				{
					if (m_MotionBlur)
					{
						curfb = ((b + 1) != (BLURTARGS / 2)) ? m_BBuf[b + 1] : mbfb[m_MotionUpdateIdx];
						curtex = (b != (BLURTARGS / 2)) ? m_BTex[b] : mbtx[m_MotionUpdateIdx];
					}
					else
					{
						curfb = m_BBuf[b];
						curtex = m_BTex[b + 1];
					}

					curfb->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
					prend->UseFrameBuffer(curfb);
					prend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
					prend->UseProgram(m_SP_blur);
					m_SP_blur->SetUniformTexture(curtex, m_uBlurTex, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);
					m_SP_blur->SetUniform1(m_uBlurScale, bs);
					m_SP_blur->ApplyUniforms(true);
					prend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);
					bs *= 2.0f;
				}

				m_MotionUpdateIdx = (m_MotionUpdateIdx + 1) % (BLURTARGS + 1);

				prend->UseFrameBuffer(nullptr, 0);
				prend->UseRenderMethod();	// no method now
				prend->UseMaterial();

				prend->UseProgram(m_SP_resolve);
				glm::fmat4x4 revmat = glm::scale(glm::fvec3(-1, 1, 1));
				prend->SetTextureTransformMatrix(&revmat);
				m_SP_resolve->ApplyUniforms(true);

				prend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);

				if (gui && guicampos && guicam)
				{
					prend->SetViewMatrix(guicam->GetViewMatrix());
					glm::fmat4x4 guiproj = *guicam->GetProjectionMatrix() * glm::scale(glm::vec3(-1, 1, 1));
					prend->SetProjectionMatrix(&guiproj);
					prend->SetNearClipDistance(guicam->GetNearClipDistance());
					prend->SetFarClipDistance(guicam->GetFarClipDistance());
					prend->SetEyePosition(guicam->GetEyePos());
					glm::fvec3 eyedir = glm::normalize(*guicam->GetTargetPos() - *(guicam->GetEyePos()));
					prend->SetEyeDirection(&eyedir);

					prend->SetAlphaPassRange(0.0f);

					c3::RenderMethod::ForEachOrderedDrawDo([&](int order)
					{
						gui->Render((uint64_t)renderflags, order);
					});

					theApp.m_C3->GetInputManager()->DrawMouseCursor(prend);
				}

				if (m_ShowDebug)
					prend->GetGui()->ShowDebugWindow(&m_ShowDebug);

				prend->EndScene(BSFLAG_SHOWGUI);
				prend->Present();
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


BOOL C3Dlg::OnEraseBkgnd(CDC *pDC)
{
	return FALSE;
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
	}
	else if (!theApp.m_C3->IsSplashWnd(pWndOther->GetSafeHwnd()))
	{
		theApp.m_C3->GetInputManager()->UnacquireAll();
	}
}

bool __cdecl C3Dlg::DeviceConnected(c3::InputDevice *device, bool conn, void *userdata)
{
	assert(device);

	if (!conn)
		return false;

	static const TCHAR *devtypename[c3::InputDevice::DeviceType::UNKNOWN] = {_T("none"), _T("joystick"), _T("mouse"), _T("keyboard")};

	c3::Scriptable *pscr = nullptr;
	c3::Object *inputobj;

	auto ConfirmInputObject = [&](c3::Object *obj) -> c3::Object *
	{
		if (obj)
		{
			pscr = dynamic_cast<c3::Scriptable *>(obj->FindComponent(c3::Scriptable::Type()));
			if (pscr && pscr->FunctionExists(_T("device_connected")))
				return obj;
		}

		return nullptr;
	};

	inputobj = ConfirmInputObject(theApp.m_C3->GetGlobalObjectRegistry()->GetRegisteredObject(c3::GlobalObjectRegistry::OD_PLAYER));
	if (!inputobj)
		inputobj = ConfirmInputObject(theApp.m_C3->GetGlobalObjectRegistry()->GetRegisteredObject(c3::GlobalObjectRegistry::OD_WORLDROOT));

	if (pscr)
	{
		pscr->Execute(_T("device_connected(\"%s\", \"%s\", %lld, %lld);"),
			devtypename[device->GetType()], device->GetName(), device->GetNumAxes(), device->GetNumButtons());

		return true;
	}

	return false;
}


void C3Dlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialog::OnMouseMove(nFlags, point);

	m_MousePos.x = point.x;
	m_MousePos.y = point.y;
	theApp.m_C3->GetInputManager()->SetMousePos(point.x, point.y);

	CRect r;
	GetClientRect(&r);

	CPoint cp = r.CenterPoint(), cpc = cp;
	ClientToScreen(&cp);

	int deltax = cpc.x - point.x;
	int deltay = cpc.y - point.y;

	c3::InputManager *inputman = theApp.m_C3->GetInputManager();
	if (inputman->MouseCaptured())
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


void C3Dlg::OnDestroy()
{
	KillTimer('DRAW');
	KillTimer('SIZE');

	CDialog::OnDestroy();
}


void C3Dlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
		case 'DRAW':
		{
			RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
			break;
		}

		case 'SIZE':
		{
			CRect r;
			GetClientRect(r);

			size_t w = (size_t)((float)r.Width() / m_WindowsUIScale);
			size_t h = (size_t)((float)r.Height() / m_WindowsUIScale);

			if (m_GBuf && (m_GBuf->GetDepthTarget()->Width() != w) || (m_GBuf->GetDepthTarget()->Height() != h))
			{
				DestroySurfaces();
				CreateSurfaces();

				c3::GlobalObjectRegistry::ObjectDesignation ot[3] =
				{
					c3::GlobalObjectRegistry::OD_WORLDROOT,
					c3::GlobalObjectRegistry::OD_SKYBOXROOT,
					c3::GlobalObjectRegistry::OD_GUI_ROOT
				};

				for (size_t i = 0; i < 3; i++)
				{
					if (c3::Object *root = theApp.m_C3->GetGlobalObjectRegistry()->GetRegisteredObject(ot[i]))
					{
						c3::util::RecursiveObjectAction(root, [](c3::Object *pobj)
						{
							pobj->PropertyChanged(pobj->GetProperties()->GetPropertyById('C3RM'));
						});
					}
				}
			}

			KillTimer('SIZE');
			UINT_PTR timerid = SetTimer('DRAW', 17, nullptr);
			assert(timerid);
			break;
		}
	}

	CDialog::OnTimer(nIDEvent);
}


void C3Dlg::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);

	RECT r;
	GetWindowRect(&r);
	theApp.m_Config->SetRect(_T("window.rect"), r);
}


void C3Dlg::OnSize(UINT nType, int cx, int cy)
{
	KillTimer('DRAW');
	KillTimer('SIZE');

	CDialog::OnSize(nType, cx, cy);

	CRect r;
	GetClientRect(&r);
	theApp.m_Config->SetRect(_T("window.rect"), r);

	UINT_PTR timerid = SetTimer('SIZE', 500, nullptr);
	assert(timerid);

	c3::Object *camobj = theApp.m_C3->GetGlobalObjectRegistry()->GetRegisteredObject(c3::GlobalObjectRegistry::OD_CAMERA);
	c3::Camera *cam = camobj ? dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type())) : nullptr;
	if (cam)
		cam->SetOrthoDimensions((float)r.Width(), (float)r.Height());
}
