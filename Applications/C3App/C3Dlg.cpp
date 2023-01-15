
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
	memset(m_BBuf, 0, sizeof(c3::FrameBuffer *) * BLURTARGS);
	m_FS_combine = m_VS_combine = nullptr;
	m_SP_combine = nullptr;
	m_FS_resolve = m_VS_resolve = nullptr;
	m_SP_resolve = nullptr;
	m_FS_blur = m_VS_blur = nullptr;
	m_SP_blur = nullptr;
	m_DepthTarg = nullptr;
	m_pRDoc = nullptr;
	m_bCapturedFirstFrame = false;
	m_AmbientColor = c3::Color::fVeryDarkGrey;
	m_SunColor = glm::fvec4(0.8f, 0.7f, 0.4f, 1.0f);
	m_SunDir = glm::normalize(glm::fvec3(0.3f, 0.2f, -1.0f));
	m_bFirstDraw = true;
	memset(m_pControllable, 0, sizeof(c3::Object *) * MAX_USERS);
	m_ViewMode = VM_FREE;//FOLLOW_POSDIR;
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
	{ _T("uSamplerNormalAmbOcc"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET },		// fragment normal (rgb) and ambient occlusion (a)
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

	theApp.m_C3->GetLog()->Print(_T("Setting up actions... "));
	c3::ActionMapper *pam = theApp.m_C3->GetActionMapper();

	pam->RegisterAction(_T("Move Forward"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		if (user < MAX_USERS)
			((C3Dlg *)userdata)->m_Controls[user].move.forward = value;
		return true;
	}, this);

	pam->RegisterAction(_T("Move Backward"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		if (user < MAX_USERS)
			((C3Dlg *)userdata)->m_Controls[user].move.backward = value;
		return true;
	}, this);

	pam->RegisterAction(_T("Strafe Left"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		if (user < MAX_USERS)
			((C3Dlg *)userdata)->m_Controls[user].move.left = value;
		return true;
	}, this);

	pam->RegisterAction(_T("Strafe Right"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		if (user < MAX_USERS)
			((C3Dlg *)userdata)->m_Controls[user].move.right = value;
		return true;
	}, this);

	pam->RegisterAction(_T("Look Up"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		if (user < MAX_USERS)
			((C3Dlg *)userdata)->m_Controls[user].look.up = value;
		return true;
	}, this);

	pam->RegisterAction(_T("Look Down"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		if (user < MAX_USERS)
			((C3Dlg *)userdata)->m_Controls[user].look.down = value;
		return true;
	}, this);

	pam->RegisterAction(_T("Look Left"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		if (user < MAX_USERS)
			((C3Dlg *)userdata)->m_Controls[user].look.left = value;
		return true;
	}, this);

	pam->RegisterAction(_T("Look Right"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		if (user < MAX_USERS)
			((C3Dlg *)userdata)->m_Controls[user].look.right = value;
		return true;
	}, this);

	pam->RegisterAction(_T("Ascend"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		if (user < MAX_USERS)
			((C3Dlg *)userdata)->m_Controls[user].move.up = value;
		return true;
	}, this);

	pam->RegisterAction(_T("Descend"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		if (user < MAX_USERS)
			((C3Dlg *)userdata)->m_Controls[user].move.down = value;
		return true;
	}, this);
	theApp.m_C3->GetLog()->Print(_T("ok\n"));

	pam->RegisterAction(_T("Run"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		if (user < MAX_USERS)
			((C3Dlg *)userdata)->m_Controls[user].move.run = value;
		return true;
	}, this);
	theApp.m_C3->GetLog()->Print(_T("ok\n"));

	pam->RegisterAction(_T("Jump"), c3::ActionMapper::ETriggerType::DOWN_CONTINUOUS, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		if (user < MAX_USERS)
			((C3Dlg *)userdata)->m_Controls[user].move.jump = value;
		return true;
	}, this);

	pam->RegisterAction(_T("Cycle View Mode"), c3::ActionMapper::ETriggerType::UP_DELTA, [](c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		((C3Dlg *)userdata)->m_ViewMode++;
		if (((C3Dlg *)userdata)->m_ViewMode >= VM_NUMMODES)
			((C3Dlg *)userdata)->m_ViewMode = VM_FOLLOW_POSDIR;

		return true;
	}, this);

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

	theApp.m_C3->GetLog()->Print(_T("Creating Renderer... "));
	m_Rend = theApp.m_C3->GetRenderer();
	if (!m_Rend)
	{
		theApp.m_C3->GetLog()->Print(_T("failed\n"));
		exit(-1);
	}
	theApp.m_C3->GetLog()->Print(_T("ok\n"));

	theApp.m_C3->GetLog()->Print(_T("Initializing Renderer... "));
	m_Rend->SetClearColor(&c3::Color::fBlack);
	CRect r;
	GetClientRect(r);
	if (!m_Rend->Initialize(GetSafeHwnd(), 0))
	{
		theApp.m_C3->GetLog()->Print(_T("failed\n"));
		exit(-2);
	}
	theApp.m_C3->GetLog()->Print(_T("ok\n"));

	m_Rend->SetAlphaCoverage(0.5f, false);

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
	m_ShadowTarg = m_Rend->CreateDepthBuffer(4096, 4096, c3::Renderer::DepthType::F32_SHADOW);

	bool gbok = false;

	theApp.m_C3->GetLog()->Print(_T("Creating G-buffer... "));
	m_GBuf = m_Rend->CreateFrameBuffer(0, _T("GBuffer"));
	gbok = m_GBuf->Setup(_countof(GBufTargData), GBufTargData, m_DepthTarg, r) == c3::FrameBuffer::RETURNCODE::RET_OK;
	theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

	for (size_t c = 0; c < BLURTARGS; c++)
	{
		m_BTex[c] = m_Rend->CreateTexture2D(w, h, c3::Renderer::TextureType::U8_3CH, 0, TEXCREATEFLAG_RENDERTARGET);
		m_BBuf[c] = m_Rend->CreateFrameBuffer();
		m_BBuf[c]->AttachDepthTarget(m_DepthTarg);
		m_BBuf[c]->AttachColorTarget(m_BTex[c], 0);
		m_BBuf[c]->Seal();
		w /= 2;
		h /= 2;
	}

	theApp.m_C3->GetLog()->Print(_T("Creating light combine buffer... "));
	m_LCBuf = m_Rend->CreateFrameBuffer(0, _T("LightCombine"));
	gbok = m_LCBuf->Setup(_countof(LCBufTargData), LCBufTargData, m_DepthTarg, r) == c3::FrameBuffer::RETURNCODE::RET_OK;
	theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

	theApp.m_C3->GetLog()->Print(_T("Creating shadow buffer... "));
	m_SSBuf = m_Rend->CreateFrameBuffer(0, _T("Shadow"));
	m_SSBuf->AttachDepthTarget(m_ShadowTarg);
	gbok = m_SSBuf->Seal() == c3::FrameBuffer::RETURNCODE::RET_OK;
	theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

	m_VS_blur = (c3::ShaderComponent *)((rm->GetResource(_T("blur.vsh"), rf))->GetData());
	m_FS_blur = (c3::ShaderComponent *)((rm->GetResource(_T("blur.fsh"), rf))->GetData());
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

	m_VS_resolve = (c3::ShaderComponent *)((rm->GetResource(_T("resolve.vsh"), rf))->GetData());
	m_FS_resolve = (c3::ShaderComponent *)((rm->GetResource(_T("resolve.fsh"), rf))->GetData());
	m_SP_resolve = m_Rend->CreateShaderProgram();
	if (m_SP_resolve)
	{
		m_SP_resolve->AttachShader(m_VS_resolve);
		m_SP_resolve->AttachShader(m_FS_resolve);
		if (m_SP_resolve->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
		{
			int32_t ut;

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

			ut = m_SP_resolve->GetUniformLocation(_T("uFocusDist"));
			m_SP_resolve->SetUniform1(ut, 0.1f);

			ut = m_SP_resolve->GetUniformLocation(_T("uFocusFalloff"));
			m_SP_resolve->SetUniform1(ut, 0.0f);
		}
	}

	m_VS_combine = (c3::ShaderComponent *)((rm->GetResource(_T("combine.vsh"), rf))->GetData());
	m_FS_combine = (c3::ShaderComponent *)((rm->GetResource(_T("combine.fsh"), rf))->GetData());
	m_SP_combine = m_Rend->CreateShaderProgram();
	if (m_SP_combine)
	{
		m_SP_combine->AttachShader(m_VS_combine);
		m_SP_combine->AttachShader(m_FS_combine);
		if (m_SP_combine->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
		{
			uint32_t i;
			int32_t ul;
			for (i = 0; i < m_GBuf->GetNumColorTargets(); i++)
			{
				c3::Texture2D *pt = m_GBuf->GetColorTarget(i);
				ul = m_SP_combine->GetUniformLocation(pt->GetName());
				m_SP_combine->SetUniformTexture((ul != c3::ShaderProgram::INVALID_UNIFORM) ? pt : m_Rend->GetBlackTexture());
			}

			for (i = 0; i < m_LCBuf->GetNumColorTargets(); i++)
			{
				c3::Texture2D* pt = m_LCBuf->GetColorTarget(i);
				ul = m_SP_combine->GetUniformLocation(pt->GetName());
				m_SP_combine->SetUniformTexture((ul != c3::ShaderProgram::INVALID_UNIFORM) ? pt : m_Rend->GetBlackTexture());
			}

			ul = m_SP_combine->GetUniformLocation(_T("uSamplerShadow"));
			if (ul >= 0)
				m_SP_combine->SetUniformTexture((c3::Texture *)m_ShadowTarg, ul, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);

			m_ulSunDir = m_SP_combine->GetUniformLocation(_T("uSunDirection"));
			m_ulSunColor = m_SP_combine->GetUniformLocation(_T("uSunColor"));
			m_ulAmbientColor = m_SP_combine->GetUniformLocation(_T("uAmbientColor"));
		}
	}

	m_Camera = m_Factory->Build();
	m_Camera->AddComponent(c3::Positionable::Type());
	m_Camera->AddComponent(c3::Camera::Type());
	m_Camera->SetName(_T("Camera"));
	theApp.m_C3->GetLog()->Print(_T("Camera created\n"));
	m_pControllable[0] = m_Camera;

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		pcam->SetPolarDistance(3.5f);
		pcam->SetFOV(glm::radians(78.0f));
	}
	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(m_Camera->FindComponent(c3::Positionable::Type()));

	m_RootObj = m_Factory->Build((c3::Prototype *)nullptr);
	m_RootObj->AddComponent(c3::Positionable::Type());
	m_RootObj->Flags().Set(OF_LIGHT | OF_CASTSHADOW);

#if 0
	genio::IInputStream *is = genio::IInputStream::Create();
	if (is)
	{
		if (is->Assign(_T("TuneTown.c3o")) && is->Open())
		{
			genio::FOURCHARCODE b = is->NextBlockId();
			if (b == 'CEL0')
			{
				if (is->BeginBlock(b))
				{
					uint16_t len;

					tstring name, description, author, website, copyright;

					is->ReadUINT16(len);
					name.resize(len);
					if (len)
						is->ReadString((TCHAR *)(name.c_str()));

					is->ReadUINT16(len);
					description.resize(len);
					if (len)
						is->ReadString((TCHAR *)(description.c_str()));

					is->ReadUINT16(len);
					author.resize(len);
					if (len)
						is->ReadString((TCHAR *)(author.c_str()));

					is->ReadUINT16(len);
					website.resize(len);
					if (len)
						is->ReadString((TCHAR *)(website.c_str()));

					is->ReadUINT16(len);
					copyright.resize(len);
					if (len)
						is->ReadString((TCHAR *)(copyright.c_str()));

					if (is->BeginBlock('CAM0'))
					{
						is->EndBlock();
					}

					if (is->BeginBlock('ENV0'))
					{
						is->EndBlock();
					}

					if (m_RootObj)
						m_RootObj->Load(is);

					is->EndBlock();
				}
			}
		}

		is->Release();
	}
#endif

	c3::Prototype *pproto;

#if 1
	if (nullptr != (pproto = m_Factory->FindPrototype(_T("Sponza"))))
	{
		c3::Object *pobj = m_Factory->Build(pproto);
		if (pobj)
		{
			c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(pobj->FindComponent(c3::Positionable::Type()));
			ppos->AdjustPos(2.0f, 0, 0);
			m_RootObj->AddChild(pobj);

			theApp.m_C3->GetLog()->Print(_T("Sponza created\n"));
		}
	}
#endif

#if 0
	if (nullptr != (pproto = m_Factory->FindPrototype(_T("Terrain"))))
	{
		c3::Object *pobj = m_Factory->Build(pproto);
		if (pobj)
		{
			c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(pobj->FindComponent(c3::Positionable::Type()));
			if (ppos)
			{
				ppos->Update(0);
			}
			m_RootObj->AddChild(pobj);

			theApp.m_C3->GetLog()->Print(_T("Terrain created\n"));
		}
	}
#endif

#if 0
	if (nullptr != (pproto = m_Factory->FindPrototype(_T("AH64e"))))
	{
		c3::Object *pobj = m_Factory->Build(pproto);
		if (pobj)
		{
			m_pControllable[1] = pobj;

			c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(pobj->FindComponent(c3::Positionable::Type()));
			if (ppos)
			{
				ppos->AdjustPos(0, 0, 40.0f);
				ppos->SetScl(0.1f, 0.1f, 0.1f);
				ppos->Update(0);
			}

			m_RootObj->AddChild(pobj);

			theApp.m_C3->GetLog()->Print(_T("Chopper created\n"));
		}
	}
#endif

#if 0
	if (nullptr != (pproto = m_Factory->FindPrototype(_T("REFCUBE"))))
	{
		c3::Object *pobj = m_Factory->Build(pproto);
		if (pobj)
		{
			m_pControllable[1] = pobj;
			c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(pobj->FindComponent(c3::Positionable::Type()));
			if (ppos)
			{
				ppos->AdjustPos(0, 0, 80.0f);
				ppos->Update(0);
			}
			m_RootObj->AddChild(pobj);

			theApp.m_C3->GetLog()->Print(_T("Reference Cube created\n"));
		}
	}
#endif

#if 0
	for (size_t i = 0, maxi = m_RootObj->GetNumChildren(); i < maxi; i++)
	{
		c3::Object *pco = m_RootObj->GetChild(i);

		if (!_tcsicmp(pco->GetName(), _T("BasicCharacter")))
		{
			m_pControllable[1] = m_RootObj->GetChild(i);
			c3::Positionable *pcp = (c3::Positionable *)m_pControllable[1]->FindComponent(c3::Positionable::Type());
			if (pcp)
			{
				pcp->SetScl(0.6f, 0.6f, 0.6f);
				pcp->SetPosZ(2.1f);
			}
			break;
		}
	}
#endif

#define NUMLIGHTS		20
#if defined(NUMLIGHTS) && (NUMLIGHTS > 0)
	if (nullptr != (pproto = m_Factory->FindPrototype(_T("Light"))))
	{
		for (size_t i = 0; i < NUMLIGHTS; i++)
		{
			float f = float(i);
			c3::Object *temp  = m_Factory->Build(pproto);
			if (temp)
			{
				m_Light.push_back(temp);

				glm::fvec3 mv((i & 1) ? 1.0f : -1.0f, 0.0f, (i & 1) ? 2.0f : -2.0f);
				m_LightMove.push_back(glm::normalize(mv) * 0.025f);

				c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(temp->FindComponent(c3::Positionable::Type()));
				if (ppos)
				{
					float s = 10.0f;//(float)(rand() % 100) / 100.0f + 0.5f;

					ppos->AdjustPos(((i & 1) ? -2.0f : 2.0f) + (float)(i % 4), (float)i * 3.0f - ((float)NUMLIGHTS / 2.0f * 3.0f), 4.0f);

					ppos->SetScl(s, s, s);
					ppos->Update(0);
				}

				c3::OmniLight *plight = dynamic_cast<c3::OmniLight *>(temp->FindComponent(c3::OmniLight::Type()));
				if (plight)
					plight->SetSourceFrameBuffer(m_GBuf);

				props::IPropertySet *pps = temp->GetProperties();
				props::IProperty *pp = pps->CreateProperty(_T("uLightColor"), 'LCLR');
				const props::TVec3F c(((float)(i % 3) * 0.75f) + ((float)(i % 4) * 0.25f), ((float)((i + 1) % 3) * 0.75f) + ((float)((i - 1) % 4) * 0.25f), ((float)((i + 2) % 3) * 0.75f) + ((float)((i - 2) % 4) * 0.25f));
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

		if (!m_bMouseCursorEnabled)
		{
			float mf = std::max<float>(m_Controls[0].move.forward, m_Controls[1].move.forward);
			float mb = std::max<float>(m_Controls[0].move.backward, m_Controls[1].move.backward);
			float mr = std::max<float>(m_Controls[0].move.right, m_Controls[1].move.right);
			float ml = std::max<float>(m_Controls[0].move.left, m_Controls[1].move.left);
			float lr = std::max<float>(m_Controls[0].look.right, m_Controls[1].look.right);
			float ll = std::max<float>(m_Controls[0].look.left, m_Controls[1].look.left);
			float ms = std::max<float>(m_Controls[0].move.run, m_Controls[1].move.run);

			for (size_t user = 0; user < MAX_USERS; user++)
			{
				if (m_pControllable[user])
				{
					c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(m_pControllable[user]->FindComponent(c3::Positionable::Type()));
					if (ppos)
					{
						float spd = 1, utd = 0, urd = 0, uod = 0;

						if (m_ViewMode == VM_FREE)
						{
							spd += ms;
							utd = m_Controls[user].move.forward - m_Controls[user].move.backward;
							uod = m_Controls[user].move.left - m_Controls[user].move.right;
							urd = m_Controls[user].look.right - m_Controls[user].look.left;
						}
						else
						{
							spd += m_Controls[user].move.run;
							utd = mf - mb;
							urd = std::max<float>(mr, lr) - std::max<float>(ml, ll);
						}
						spd *= 0.02f;

						glm::vec3 mv(0, 0, 0);
						mv += *(ppos->GetFacingVector()) * utd * spd;
						if ((m_ViewMode == VM_FREE) && !user)
						{
							glm::vec3 ml = *(ppos->GetLocalRightVector());
							ml *= uod * spd;
							mv += ml;
						}
						mv += *(ppos->GetLocalUpVector()) * (m_Controls[user].move.up - m_Controls[user].move.down) * spd;

						ppos->AdjustYawFlat(urd * 0.04f);
						m_pControllable[user]->Update();

						ppos->AdjustPos(mv.x, mv.y, mv.z);
						if ((m_ViewMode == VM_FREE) && !user)
							ppos->AdjustPitch((m_Controls[user].look.up - m_Controls[user].look.down) * 0.04f);
						m_pControllable[user]->Update();
					}
				}

				memset(&m_Controls[user], 0, sizeof(SControls));
			}
#if 0
			if (pcam)
			{
				float zoo = pcam->GetPolarDistance();
				zoo += theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::BUTTON5) -
					theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::BUTTON6);
				zoo = std::max(zoo, 0.1f);
				pcam->SetPolarDistance(zoo);
			}
#endif

			c3::Positionable *pfollowerpos = nullptr;//dynamic_cast<c3::Positionable *>(m_pControllable[0]->FindComponent(c3::Positionable::Type()));
			c3::Positionable *pleaderpos = nullptr;//dynamic_cast<c3::Positionable *>(m_pControllable[1]->FindComponent(c3::Positionable::Type()));

			if (pfollowerpos && pleaderpos)
			{
				switch (m_ViewMode)
				{
					case VM_FOLLOW_POSDIR:
					{
						glm::fquat q = glm::slerp(*(pfollowerpos->GetOriQuat()), *(pleaderpos->GetOriQuat()), 0.9f);
						pfollowerpos->SetOriQuat(&q);
						pfollowerpos->AdjustPitch(glm::radians(-41.0f));
					}
					case VM_FOLLOW_POS:
					{
						glm::fvec3 adjlp = *(pleaderpos->GetPosVec()) + glm::fvec3(0, 0, 1);
						glm::fvec3 lp = glm::lerp(*(pfollowerpos->GetPosVec()), adjlp, 0.6f);
						pfollowerpos->SetPosVec(&lp);
						break;
					}
				}
			}
		}

		m_Camera->Update(dt);

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

#if 1
		for (size_t i = 0; i < m_Light.size(); i++)
		{
			c3::Positionable *plpos = dynamic_cast<c3::Positionable *>(m_Light[i]->FindComponent(c3::Positionable::Type()));
			float s = sinf((float)(m_Rend->GetCurrentFrameNumber() + i) * 3.14159f / 180.0f * 1.0f) * 0.5f;
			plpos->AdjustPos(m_LightMove[i].x * s, m_LightMove[i].y * s, m_LightMove[i].z * s);
			plpos->Update(m_Light[i]);
		}
#endif

		glm::mat4 biasmat(
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0
		);

		m_AmbientColor = c3::Color::fDarkGrey;

		float farclip = m_Camera->GetProperties()->GetPropertyById('C:FC')->AsFloat();
		float nearclip = m_Camera->GetProperties()->GetPropertyById('C:NC')->AsFloat();
		glm::fmat4x4 depthProjectionMatrix = glm::ortho<float>(-55, 50, -71, 71, nearclip, farclip);
		glm::fvec3 sunpos = m_SunDir * -61.5f;
		glm::fvec3 campos;
		pcam->GetEyePos(&campos);
//		sunpos += campos;
		glm::fmat4x4 depthViewMatrix = glm::lookAt(sunpos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::fmat4x4 depthMVP = depthProjectionMatrix * depthViewMatrix;
		m_Rend->SetSunShadowMatrix(&depthMVP);

		m_SP_combine->SetUniform3(m_ulAmbientColor, &m_AmbientColor);
		m_SP_combine->SetUniform3(m_ulSunColor, &m_SunColor);
		m_SP_combine->SetUniform3(m_ulSunDir, &m_SunDir);

		m_RootObj->Update(dt);

		if (m_Rend->BeginScene(BSFLAG_SHOWGUI))
		{
			// Solid color pass
			m_Rend->SetDepthMode(c3::Renderer::DepthMode::DM_READWRITE);
			m_Rend->UseFrameBuffer(m_GBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH | UFBFLAG_CLEARSTENCIL);
			m_Rend->SetDepthTest(c3::Renderer::Test::DT_LESSER);
			m_Rend->SetAlphaPassRange(3.0f / 255.0f);
			m_Rend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
			m_Rend->SetCullMode(c3::Renderer::CullMode::CM_BACK);
			m_RootObj->Render();

			// Shadow pass
			m_Rend->UseFrameBuffer(m_SSBuf, UFBFLAG_CLEARDEPTH | UFBFLAG_UPDATEVIEWPORT);
			m_RootObj->Render(RF_SHADOW);

			m_Rend->SetViewport();

			// Lighting pass(es)
			m_Rend->UseFrameBuffer(m_LCBuf, UFBFLAG_FINISHLAST | UFBFLAG_CLEARCOLOR | UFBFLAG_UPDATEVIEWPORT);
			m_Rend->SetDepthMode(c3::Renderer::DepthMode::DM_READONLY);
			m_Rend->SetDepthTest(c3::Renderer::Test::DT_ALWAYS);
			m_Rend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			m_RootObj->Render(RF_LIGHT);

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
			for (int b = 0; b < BLURTARGS - 1; b++)
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


BOOL C3Dlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		float m = (m_Controls[0].move.run * 3.5f) + 1.0f;
		float d = pcam->GetPolarDistance();
		d += ((zDelta < 0) ? m : -m);
		d = std::max(d, 0.1f);

		pcam->SetPolarDistance(d);
	}

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}


void C3Dlg::OnSize(UINT nType, int cx, int cy)
{
	RECT r;
	r.left = 0;
	r.right = cx;
	r.top = 0;
	r.bottom = cy;

	theApp.m_Cfg->SetRect(_T("window.rect"), r);

	CDialog::OnSize(nType, r.right, r.bottom);

//	theApp.m_C3->GetRenderer()->SetViewport(&r);

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(m_Camera->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		glm::fvec2 dim;
		dim.x = (float)r.right;
		dim.y = (float)r.bottom;
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

	int deltax = cpc.x - point.x;
	int deltay = cpc.y - point.y;

	SetCursorPos(cp.x, cp.y);
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
//	if (nIDEvent == m_DrawTimerId)
//		RedrawWindow(nullptr, nullptr, 0);

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

	switch (device->GetType())
	{
		case c3::InputDevice::DeviceType::KEYBOARD:
		{
			theApp.m_C3->GetInputManager()->AssignUser(device, 0);

			pam->MakeAssociation(ai_mf, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_POSY);
			pam->MakeAssociation(ai_mb, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_NEGY);
			pam->MakeAssociation(ai_sl, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_NEGX);
			pam->MakeAssociation(ai_sr, device->GetUID(), c3::InputDevice::VirtualButton::AXIS1_POSX);

			pam->MakeAssociation(ai_mf, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_W);
			pam->MakeAssociation(ai_mb, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_S);
			pam->MakeAssociation(ai_sl, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_A);
			pam->MakeAssociation(ai_sr, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_D);

			pam->MakeAssociation(ai_ld, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_NEGY);
			pam->MakeAssociation(ai_lu, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_POSY);
			pam->MakeAssociation(ai_ll, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_NEGX);
			pam->MakeAssociation(ai_lr, device->GetUID(), c3::InputDevice::VirtualButton::AXIS2_POSX);

			pam->MakeAssociation(ai_a, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_Q);
			pam->MakeAssociation(ai_d, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_Z);

			pam->MakeAssociation(ai_r, device->GetUID(), c3::InputDevice::VirtualButton::LSHIFT);
			pam->MakeAssociation(ai_j, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON1);

			pam->MakeAssociation(ai_cvm, device->GetUID(), c3::InputDevice::VirtualButton::LETTER_C);

			break;
		}

		case c3::InputDevice::DeviceType::MOUSE:
		{
			theApp.m_C3->GetInputManager()->AssignUser(device, 0);

			//pam->MakeAssociation(ai_mf, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON1);
			//pam->MakeAssociation(ai_mb, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON2);

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

				pam->MakeAssociation(ai_r, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON2);
				pam->MakeAssociation(ai_j, device->GetUID(), c3::InputDevice::VirtualButton::BUTTON1);
			
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
