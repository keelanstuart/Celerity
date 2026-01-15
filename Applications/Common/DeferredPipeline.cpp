// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart

#include <tchar.h>

#include <afxwin.h>

#include <C3Utility.h>
#include "DeferredPipeline.h"
#include <C3Gui.h>



CDeferredPipeline::CDeferredPipeline(c3::System *psys, c3::Configuration *pconfig, CWnd *pwnd, bool editor)
{
	m_OldLogFunc = nullptr;

	m_pWnd = pwnd;
	assert(m_pWnd);

	m_pSys = psys;
	assert(m_pSys);

	m_pConfig = pconfig;
	assert(m_pConfig);

	m_pRenderDoc = nullptr;

	tstring renderdoc_path = m_pConfig->GetString(_T("debug.renderdoc.path"), _T("C:\\Program Files\\RenderDoc\\"));
	renderdoc_path += _T("renderdoc.dll");

	// At init, on windows
	if (HMODULE mod = GetModuleHandle(renderdoc_path.c_str()))
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_2, (void **)&m_pRenderDoc);
		m_pSys->GetLog()->Print(_T("RenderDoc detected; Captures Enabled...\n"));
	}

	if (m_pConfig->GetBool(_T("debug.renderdoc.capture_initialization"), true))
		TriggerCapture();

	m_WindowsUIScale = 1.0f;

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
	m_uBlurTex = -1;
	m_uBlurScale = -1;

	m_ShowDebug = false;

	m_bSurfacesCreated = false;
	m_bSurfacesReady = false;

	m_bInitialized = false;
}


CDeferredPipeline::~CDeferredPipeline()
{
	DestroySurfaces();

	C3_SAFERELEASE(m_SSBuf);
	C3_SAFERELEASE(m_ShadowTarg);

	C3_SAFERELEASE(m_SP_resolve);
	C3_SAFERELEASE(m_SP_blur);
	C3_SAFERELEASE(m_SP_combine);

	C3_SAFERELEASE(m_SP_bounds);
}


void CDeferredPipeline::DestroySurfaces()
{
	C3_SAFERELEASE(m_GBuf);

	C3_SAFERELEASE(m_LCBuf);

	C3_SAFERELEASE(m_AuxBuf);

	C3_SAFERELEASE(m_DepthTarg);

	for (auto p : m_BBuf)
		C3_SAFERELEASE(p);
	m_BBuf.fill(nullptr);

	for (auto p : m_BTex)
		C3_SAFERELEASE(p);
	m_BTex.fill(nullptr);

	m_bSurfacesCreated = false;
	m_bSurfacesReady = false;
}


void CDeferredPipeline::CreateSurfaces()
{
	c3::Renderer *prend = m_pSys->GetRenderer();
	assert(prend);

	CRect r;
	m_pWnd->GetClientRect(r);

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

	m_pSys->GetLog()->Print(_T("Creating G-buffer... "));
	if (!m_GBuf)
		m_GBuf = prend->CreateFrameBuffer(0, _T("GBuffer"));
	if (m_GBuf)
		gbok = m_GBuf->Setup(_countof(GBufTargData), GBufTargData, m_DepthTarg, r) == c3::FrameBuffer::RETURNCODE::RET_OK;
	if (gbok)
	{
		m_GBuf->SetClearColor(c3::Color::fBlackFT, 3);
	}
	m_pSys->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

	for (size_t c = 0; c < BLURTARGS; c++)
	{
		m_BTex[c] = prend->CreateTexture2D(w, h, c3::Renderer::TextureType::U8_3CH, 0, TEXCREATEFLAG_RENDERTARGET);

		m_BBuf[c] = prend->CreateFrameBuffer();
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

	m_pSys->GetLog()->Print(_T("Creating light combine buffer... "));
	if (!m_LCBuf)
		m_LCBuf = prend->CreateFrameBuffer(0, _T("LightCombine"));
	if (m_LCBuf)
		gbok = m_LCBuf->Setup(_countof(LCBufTargData), LCBufTargData, m_DepthTarg, r) == c3::FrameBuffer::RETURNCODE::RET_OK;
	m_pSys->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

	CRect auxr = r;

	c3::FrameBuffer::TargetDesc AuxBufTargData[] =
	{
		{ _T("uSamplerAuxiliary"), c3::Renderer::TextureType::F32_4CH, TEXCREATEFLAG_RENDERTARGET },
	};

	m_pSys->GetLog()->Print(_T("Creating auxiliary buffer... "));
	if (!m_AuxBuf)
		m_AuxBuf = prend->CreateFrameBuffer(0, _T("Aux"));
	if (m_AuxBuf)
		gbok = m_AuxBuf->Setup(_countof(AuxBufTargData), AuxBufTargData, m_DepthTarg, auxr) == c3::FrameBuffer::RETURNCODE::RET_OK;
	if (gbok)
	{
		m_AuxBuf->SetClearColor(c3::Color::fBlackFT, 0);
		m_AuxBuf->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
	}
	m_pSys->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

	if (gbok)
		m_bSurfacesCreated = true;
}

void CDeferredPipeline::UpdateShaderSurfaces()
{
	c3::Renderer *prend = m_pSys->GetRenderer();
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

		ul = m_SP_combine->GetUniformLocation(_T("uSamplerAuxiliary"));
		if (ul >= 0)
			m_SP_combine->SetUniformTexture((c3::Texture*)m_AuxBuf->GetColorTarget(0), ul, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);
	}

	m_bSurfacesReady = true;
}


void CDeferredPipeline::OnDraw(CDC* pDC, c3::Object *root, TObjectArray *selections, c3::Object *camera)
{
	bool capturing = false;
	if (m_CaptureFrame)
	{
		capturing = true;
		m_CaptureFrame = false;
		if (m_pRenderDoc)
			m_pRenderDoc->StartFrameCapture(NULL, NULL);
	}

	c3::Renderer *prend = m_pSys->GetRenderer();
	assert(prend);

	if (!prend->Initialized())
	{
		prend->Initialize(m_pWnd->GetSafeHwnd(), 0);

		prend->FlushErrors(_T(__FILE__) _T(":%lu"), __LINE__);
	}

	if (!m_bInitialized)
	{
		InitializeGraphics();
		m_bInitialized = true;
	}

	if (!m_bSurfacesCreated)
		CreateSurfaces();

	if (m_bSurfacesCreated && !m_bSurfacesReady)
	{
		UpdateShaderSurfaces();

		c3::util::RecursiveObjectAction(root, [](c3::Object *pobj)
		{
			pobj->PropertyChanged(pobj->GetProperties()->GetPropertyById('C3RM'));
		});
	}

	uint32_t renderflags = m_pConfig->GetBool(_T("environment.editordraw"), true) ? RF_EDITORDRAW : 0;

	prend->SetOverrideHwnd(m_pWnd->GetSafeHwnd());

	CRect r;
	m_pWnd->GetClientRect(r);

	c3::Positionable *pcampos = camera ? dynamic_cast<c3::Positionable *>(camera->FindComponent(c3::Positionable::Type())) : nullptr;
	c3::Camera *pcam = camera ? dynamic_cast<c3::Camera *>(camera->FindComponent(c3::Camera::Type())) : nullptr;

	prend->UseFrameBuffer(nullptr);

	pcam->SetOrthoDimensions((float)r.Width(), (float)r.Height());
	prend->SetViewport(r);

	c3::Environment *penv = m_pSys->GetEnvironment();
	assert(penv);

	float farclip = camera->GetProperties()->GetPropertyById('C:FC')->AsFloat();
	float nearclip = camera->GetProperties()->GetPropertyById('C:NC')->AsFloat();

	glm::fvec4 cc = glm::fvec4(*penv->GetBackgroundColor(), 1.0f);
	prend->SetClearColor(&cc);
	m_GBuf->SetClearColor(cc, 0);
	m_GBuf->SetClearColor(glm::fvec4(0, 0, 0, farclip), 2);

	prend->SetClearDepth(1.0f);

	c3::Positionable *pcpos = dynamic_cast<c3::Positionable *>(camera->FindComponent(c3::Positionable::Type()));
	c3::Camera *pccam = dynamic_cast<c3::Camera *>(camera->FindComponent(c3::Camera::Type()));

	if (pccam)
	{
		prend->SetViewMatrix(pccam->GetViewMatrix());
		prend->SetProjectionMatrix(pccam->GetProjectionMatrix());
		prend->SetEyePosition(pccam->GetEyePos());
		prend->SetEyeDirection(pcpos->GetFacingVector());
	}

	m_SP_combine->SetUniform3(m_ulAmbientColor, penv->GetAmbientColor());
	m_SP_combine->SetUniform3(m_ulSunColor, penv->GetSunColor());
	m_SP_combine->SetUniform3(m_ulSunDir, penv->GetSunDirection());

	m_GBuf->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
	if (prend->BeginScene(BSFLAG_SHOWGUI))
	{
		// Solid color pass
		prend->UseFrameBuffer(m_GBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH | UFBFLAG_CLEARSTENCIL | UFBFLAG_UPDATEVIEWPORT);
		prend->SetDepthMode(c3::Renderer::DepthMode::DM_READWRITE);
		prend->SetDepthTest(c3::Renderer::Test::DT_LESSER);
		prend->SetAlphaPassRange(254.9f / 255.0f);
		prend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
		prend->SetCullMode(c3::Renderer::CullMode::CM_BACK);
		prend->SetTextureTransformMatrix(nullptr);
		c3::RenderMethod::ForEachOrderedDrawDo([&](int order)
		{
			root->Render(renderflags, order);
		});

		// after the main pass, clear everything with black...
		prend->SetClearColor(&c3::Color::fBlack);
		m_LCBuf->SetClearColor(c3::Color::fBlack, 0);

		// Lighting pass(es)
		prend->UseFrameBuffer(m_LCBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_UPDATEVIEWPORT); // | UFBFLAG_FINISHLAST);
		prend->SetDepthMode(c3::Renderer::DepthMode::DM_READONLY);
		prend->SetDepthTest(c3::Renderer::Test::DT_ALWAYS);
		prend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
		c3::RenderMethod::ForEachOrderedDrawDo([&](int order)
		{
			root->Render(renderflags | RF_LIGHT, order);
		});

		// Shadow pass
		{
			// use the sun direction to determine its position - in the opposite direction
			static float sunposmult = -800.0f;

			// Set up our shadow transforms

			glm::fmat4x4 depthProjectionMatrix = glm::ortho<float>(-800, 800, -800, 800, nearclip, farclip);
			glm::fvec3 sunpos;
			penv->GetSunDirection(&sunpos);
			sunpos *= sunposmult;

			glm::fvec3 campos;
			pcam->GetEyePos(&campos);
			glm::fmat4x4 depthViewMatrix = glm::lookAt(sunpos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			glm::fmat4x4 depthMVP = depthProjectionMatrix * depthViewMatrix;

			prend->SetSunShadowMatrix(&depthMVP);
			prend->SetViewMatrix(&depthViewMatrix);
			prend->SetProjectionMatrix(&depthProjectionMatrix);

			prend->SetDepthMode(c3::Renderer::DepthMode::DM_READWRITE);
			prend->UseFrameBuffer(m_SSBuf, UFBFLAG_CLEARDEPTH | UFBFLAG_UPDATEVIEWPORT);
			c3::RenderMethod::ForEachOrderedDrawDo([&](int order)
			{
				root->Render(renderflags | RF_SHADOW, order);
			});
		}

		// clear the render method and material
		prend->UseRenderMethod();
		prend->UseMaterial();

		if (pccam)
		{
			prend->SetViewMatrix(pccam->GetViewMatrix());
			prend->SetProjectionMatrix(pccam->GetProjectionMatrix());
			prend->SetEyePosition(pccam->GetEyePos());
			prend->SetEyeDirection(pcpos->GetFacingVector());
		}

		// Selection hilighting
		prend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
		m_AuxBuf->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
		prend->UseFrameBuffer(m_AuxBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_UPDATEVIEWPORT); // | UFBFLAG_FINISHLAST);
		prend->SetDepthMode(c3::Renderer::DepthMode::DM_DISABLED);
		prend->UseProgram(m_SP_bounds);

		if (selections)
		{
			for (auto o : *selections)
			{
				o->Render(RF_FORCE | RF_LOCKSHADER | RF_LOCKMATERIAL | RF_AUXILIARY);
			};
		}

		// Resolve
		prend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
		m_BBuf[0]->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
		prend->UseFrameBuffer(m_BBuf[0], UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH); // | UFBFLAG_FINISHLAST);
		prend->SetDepthMode(c3::Renderer::DepthMode::DM_DISABLED);
		prend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
		prend->SetCullMode(c3::Renderer::CullMode::CM_DISABLED);
		prend->UseProgram(m_SP_combine);
		prend->GetFullScreenPlaneMesh()->Draw();

		float bs = 2.0f;
		for (int b = 0; b < BLURTARGS - 1; b++)
		{
			prend->UseFrameBuffer(m_BBuf[b + 1], 0); // UFBFLAG_FINISHLAST);
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
			prend->UseProgram(m_SP_blur);
			m_SP_blur->SetUniformTexture(m_BTex[b], m_uBlurTex);
			m_SP_blur->SetUniform1(m_uBlurScale, bs);
			m_SP_blur->ApplyUniforms(true);
			prend->GetFullScreenPlaneMesh()->Draw();
			bs *= 2.0f;
		}

		prend->UseFrameBuffer(nullptr, 0); // UFBFLAG_FINISHLAST);
		prend->UseProgram(m_SP_resolve);
		glm::fmat4x4 revmat = glm::scale(glm::fvec3(-1, 1, 1));
		prend->SetTextureTransformMatrix(&revmat);
		m_SP_resolve->ApplyUniforms(true);
		prend->GetFullScreenPlaneMesh()->Draw();

		c3::Gui *pgui = prend->GetGui();

		if (m_ShowDebug)
			pgui->ShowDebugWindow(&m_ShowDebug);

		prend->EndScene(BSFLAG_SHOWGUI);
		prend->Present();
	}

	if (capturing)
	{
		if (m_pRenderDoc)
			m_pRenderDoc->EndFrameCapture(NULL, NULL);

		m_pSys->GetRenderer()->SetLogFunc(m_OldLogFunc);
	}
}

void CDeferredPipeline::InitializeGraphics()
{
	// Compensate for screen scaling in Windows 10+
	m_WindowsUIScale = m_pWnd->GetDC()->GetDeviceCaps(LOGPIXELSX) / 96.0f;
	if ((m_WindowsUIScale > 1.0f) && (m_WindowsUIScale < 1.1f))
		m_WindowsUIScale = 1.0f;

	c3::Renderer* prend = m_pSys->GetRenderer();
	assert(prend);

	// Compensate for screen scaling in Windows 10+
	m_WindowsUIScale = m_pWnd->GetDC()->GetDeviceCaps(LOGPIXELSX) / 96.0f;
	if ((m_WindowsUIScale > 1.0f) && (m_WindowsUIScale < 1.1f))
		m_WindowsUIScale = 1.0f;

	m_pSys->GetLog()->Print(_T("Initializing Renderer... "));

	if (prend->Initialize(m_pWnd->GetSafeHwnd(), 0))
	{
		bool gbok;

		prend->FlushErrors(_T(__FILE__) _T(":%lu"), __LINE__);

		prend->GetFont(_T("Arial"), 16);

		c3::ResourceManager* rm = m_pSys->GetResourceManager();

		prend->FlushErrors(_T(__FILE__) _T(":%lu"), __LINE__);

		m_pSys->GetLog()->Print(_T("Creating shadow buffer... "));

		if (!m_ShadowTarg)
			m_ShadowTarg = prend->CreateDepthBuffer(2048, 2048, c3::Renderer::DepthType::F32_SHADOW);

		prend->FlushErrors(_T(__FILE__) _T(":%lu"), __LINE__);

		if (!m_SSBuf)
			m_SSBuf = prend->CreateFrameBuffer(0, _T("Shadow"));
		if (m_SSBuf)
			m_SSBuf->AttachDepthTarget(m_ShadowTarg);
		gbok = m_SSBuf->Seal() == c3::FrameBuffer::RETURNCODE::RET_OK;
		m_pSys->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

		prend->FlushErrors(_T(__FILE__) _T(":%lu"), __LINE__);

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

		prend->FlushErrors(_T(__FILE__) _T(":%lu"), __LINE__);

		m_VS_resolve = (c3::ShaderComponent*)((rm->GetResource(_T("resolve.vsh"), RESF_DEMANDLOAD))->GetData());
		m_FS_resolve = (c3::ShaderComponent*)((rm->GetResource(_T("resolve.fsh"), RESF_DEMANDLOAD))->GetData());
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

		prend->FlushErrors(_T(__FILE__) _T(":%lu"), __LINE__);

		m_VS_combine = (c3::ShaderComponent*)((rm->GetResource(_T("combine.vsh"), RESF_DEMANDLOAD))->GetData());
		m_FS_combine = (c3::ShaderComponent*)((rm->GetResource(_T("combine-editor.fsh"), RESF_DEMANDLOAD))->GetData());
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
			}
		}

		prend->FlushErrors(_T(__FILE__) _T(":%lu"), __LINE__);

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

		prend->FlushErrors(_T(__FILE__) _T(":%lu"), __LINE__);

		CreateSurfaces();

		m_bInitialized = true;
	}
}

bool CDeferredPipeline::Initialized()
{
	return m_bInitialized;
}


void CDeferredPipeline::TriggerCapture()
{
	static TCHAR filename[64] = {0};
	static size_t evnum = 0;

	m_CaptureFrame = true;
	m_OldLogFunc = m_pSys->GetRenderer()->GetLogFunc();

	size_t framenum = m_pSys->GetRenderer()->GetCurrentFrameNumber();
	_stprintf_s(filename, _T("c3_gfx_log_%zu.txt"), framenum);
	DeleteFile(filename);
	evnum = 0;

	m_pSys->GetRenderer()->SetLogFunc([&](const TCHAR *msg)
	{
		FILE *f;
		if (!_tfopen_s(&f, filename, _T("a")) && f)
		{
			_ftprintf(f, _T("%05zu:\t"), evnum); evnum++;
			_ftprintf(f, msg);
			_ftprintf(f, _T(";\n"));
			fclose(f);
		}
	});
}


bool CDeferredPipeline::CapturesAvailable()
{
	return true;
}


void CDeferredPipeline::ShowDebug(bool show)
{
	m_ShowDebug = show;
}


bool CDeferredPipeline::DebugShowing()
{
	return m_ShowDebug;
}