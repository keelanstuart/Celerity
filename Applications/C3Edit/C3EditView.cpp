// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "C3Edit.h"
#endif

#include "C3EditFrame.h"
#include "C3EditDoc.h"
#include "C3EditView.h"
#include "resource.h"
#include "BrushSettingsDlg.h"
#include "RepathDlg.h"

#include <C3Gui.h>
#include <C3RenderMethod.h>
#include <C3Utility.h>
#include <C3CommonVertexDefs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// C3EditView

IMPLEMENT_DYNCREATE(C3EditView, CView)

BEGIN_MESSAGE_MAP(C3EditView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_EDIT_TRIGGERRENDERDOCCAPTURE, &C3EditView::OnUpdateEditTriggerRenderDocCapture)
	ON_COMMAND(ID_EDIT_TRIGGERRENDERDOCCAPTURE, &C3EditView::OnEditTriggerRenderDocCapture)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETFOCUS()

	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, &C3EditView::OnUpdateEditDelete)
	ON_COMMAND(ID_EDIT_DELETE, &C3EditView::OnEditDelete)

	ON_UPDATE_COMMAND_UI(ID_EDIT_DUPLICATE, &C3EditView::OnUpdateEditDuplicate)
	ON_COMMAND(ID_EDIT_DUPLICATE, &C3EditView::OnEditDuplicate)

	ON_UPDATE_COMMAND_UI(ID_EDIT_ASSIGNROOT, &C3EditView::OnUpdateEditAssignRoot)
	ON_COMMAND(ID_EDIT_ASSIGNROOT, &C3EditView::OnEditAssignRoot)

	ON_UPDATE_COMMAND_UI(ID_EDIT_GROUP, &C3EditView::OnUpdateEditGroup)
	ON_COMMAND(ID_EDIT_GROUP, &C3EditView::OnEditGroup)

	ON_UPDATE_COMMAND_UI(ID_EDIT_UNGROUP, &C3EditView::OnUpdateEditUngroup)
	ON_COMMAND(ID_EDIT_UNGROUP, &C3EditView::OnEditUngroup)

	ON_WM_KEYUP()

	ON_COMMAND(ID_GRAPH_DELETENODE, &C3EditView::OnGraphDeleteNode)
	ON_UPDATE_COMMAND_UI(ID_GRAPH_DELETENODE, &C3EditView::OnUpdateGraphDeleteNode)

	ON_COMMAND(ID_EDIT_CENTERCAMERAON, &C3EditView::OnEditCenterCamera)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CENTERCAMERAON, &C3EditView::OnUpdateEditCenterCamera)

	ON_WM_KEYDOWN()
	ON_COMMAND(ID_EDIT_CAMERASETTINGS, &C3EditView::OnEditCameraSettings)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CAMERASETTINGS, &C3EditView::OnUpdateEditCameraSettings)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HOVERINFORMATION, &C3EditView::OnUpdateViewHoverInformation)
	ON_COMMAND(ID_VIEW_HOVERINFORMATION, &C3EditView::OnViewHoverInformation)

	ON_COMMAND(ID_EDIT_COPY, &C3EditView::OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &C3EditView::OnUpdateEditCopy)

	ON_COMMAND(ID_EDIT_CUT, &C3EditView::OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, &C3EditView::OnUpdateEditCut)

	ON_COMMAND(ID_EDIT_PASTE, &C3EditView::OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &C3EditView::OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_BRUSHSETTINGS, &C3EditView::OnEditBrushSettings)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_REPATH, &C3EditView::OnUpdateToolsRepath)
	ON_COMMAND(ID_TOOLS_REPATH, &C3EditView::OnToolsRepath)
END_MESSAGE_MAP()


RENDERDOC_API_1_4_0* C3EditView::m_pRenderDoc = nullptr;
size_t C3EditView::m_ResourcesRefCt = 0;


C3EditView::C3EditView() noexcept
{
	m_RenderDocCaptureFrame = theApp.m_Config->GetBool(_T("debug.renderdoc.capture_initialization"), true);
	m_ResourcesRefCt++;

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

	m_pHoverObj = nullptr;

	m_bCenter = false;
}


C3EditView::~C3EditView()
{
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


void C3EditView::DestroySurfaces()
{
	C3EditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	c3::util::RecursiveObjectAction(pDoc->m_RootObj, [](c3::Object *pobj)
	{
		pobj->PropertyChanged(pobj->GetProperties()->GetPropertyById('C3RM'));
	});

	C3_SAFERELEASE(m_GBuf);

	C3_SAFERELEASE(m_LCBuf);

	C3_SAFERELEASE(m_AuxBuf);

	C3_SAFERELEASE(m_DepthTarg);

	for (auto p : m_BBuf)
		C3_SAFERELEASE(p);
	m_BBuf.clear();

	for (auto p : m_BTex)
		C3_SAFERELEASE(p);
	m_BTex.clear();
}


void C3EditView::CreateSurfaces()
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

	m_GBuf->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE, 0);

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

void C3EditView::UpdateShaderSurfaces()
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

		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerSceneMip2"));
		m_SP_resolve->SetUniformTexture(m_BTex[2], ut);

		ut = m_SP_resolve->GetUniformLocation(_T("uSamplerSceneMip3"));
		m_SP_resolve->SetUniformTexture(m_BTex[3], ut);

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
}

BOOL C3EditView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}


int C3EditView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!GetSafeHwnd())
		return -2;

	return 0;
}


void C3EditView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	tstring renderdoc_path = theApp.m_Config->GetString(_T("debug.renderdoc.path"), _T("C:/Program Files/RenderDoc"));
	renderdoc_path += _T("/renderdoc.dll");

	// At init, on windows
	if (HMODULE mod = GetModuleHandle(renderdoc_path.c_str()))
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_2, (void **)&m_pRenderDoc);
		theApp.m_C3->GetLog()->Print(_T("RenderDoc detected; Captures Enabled...\n"));
	}

	theApp.m_C3->GetSoundPlayer()->Initialize();
	theApp.m_C3->GetPhysicsManager()->Initialize();

	CRect r;
	GetClientRect(r);

	SetTimer('DRAW', 17, nullptr);
	SetTimer('PICK', 33, nullptr);
	SetTimer('PROP', 1000, nullptr);
}


void C3EditView::OnDraw(CDC *pDC)
{
	C3EditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (!pDoc->m_RootObj)
		return;

	theApp.m_C3->GetGlobalObjectRegistry()->RegisterObject(c3::GlobalObjectRegistry::OD_WORLDROOT, pDoc->m_RootObj);

	C3EditFrame *pmf = (C3EditFrame *)theApp.GetMainWnd();

	C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

	c3::Object *camobj = pvi->m_Camera;
	c3::Positionable *pcampos = camobj ? dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type())) : nullptr;
	c3::Camera *pcam = camobj ? dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type())) : nullptr;
	theApp.m_C3->GetGlobalObjectRegistry()->RegisterObject(c3::GlobalObjectRegistry::OD_CAMERA, camobj);

	CRect r;
	GetClientRect(r);

	pcam->SetOrthoDimensions((float)r.Width(), (float)r.Height());

	uint32_t renderflags = theApp.m_Config->GetBool(_T("environment.editordraw"), true) ? RF_EDITORDRAW : 0;

	bool advtime = theApp.m_Config->GetBool(_T("environment.advancetime"), true);

	bool paused = pDoc->m_Paused || !advtime;

	theApp.m_C3->UpdateTime(paused);
	float dt = paused ? 0.0f : (pDoc->m_TimeWarp * theApp.m_C3->GetElapsedTime());

	c3::Renderer *prend = theApp.m_C3->GetRenderer();
	assert(prend);
	if (!prend->Initialized())
		InitializeGraphics();

	if (prend && prend->Initialized())
	{
		if ((GetFocus() == this) && !theApp.m_C3->GetInputManager()->ButtonPressed(c3::InputDevice::VirtualButton::LCTRL))
			HandleInput(pcampos);

		prend->SetOverrideHwnd(GetSafeHwnd());

		prend->SetViewport(r);

		c3::Environment *penv = theApp.m_C3->GetEnvironment();
		assert(penv);

		C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

		if (camobj)
			camobj->Update(dt);

		int64_t active_tool = theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT);

		float farclip = camobj->GetProperties()->GetPropertyById('C:FC')->AsFloat();
		float nearclip = camobj->GetProperties()->GetPropertyById('C:NC')->AsFloat();

		glm::fvec4 cc = glm::fvec4(*penv->GetBackgroundColor(), 0);
		prend->SetClearColor(&cc);
		m_GBuf->SetClearColor(0, cc);
		m_GBuf->SetClearColor(2, glm::fvec4(0, 0, 0, farclip));

		prend->SetClearDepth(1.0f);

		pDoc->m_RootObj->Update(paused ? 0 : dt);

		if ((active_tool == C3EditApp::ToolType::TT_WAND) && pDoc->m_Brush)
			pDoc->m_Brush->Update(dt);

		if (m_bCenter)
		{
			CenterViewOnSelection();
			m_bCenter = false;
		}

		c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(pvi->m_Camera->FindComponent(c3::Positionable::Type()));
		c3::Camera *pcam = dynamic_cast<c3::Camera *>(pvi->m_Camera->FindComponent(c3::Camera::Type()));

		if (pcam)
		{
			prend->SetViewMatrix(pcam->GetViewMatrix());
			prend->SetProjectionMatrix(pcam->GetProjectionMatrix());
			prend->SetEyePosition(pcam->GetEyePos());
			prend->SetEyeDirection(pcampos->GetFacingVector());
		}

		m_SP_combine->SetUniform3(m_ulAmbientColor, penv->GetAmbientColor());
		m_SP_combine->SetUniform3(m_ulSunColor, penv->GetSunColor());
		m_SP_combine->SetUniform3(m_ulSunDir, penv->GetSunDirection());

		bool capturing = false;
		if (m_pRenderDoc && m_RenderDocCaptureFrame)
		{
			capturing = true;
			m_RenderDocCaptureFrame = false;
			m_pRenderDoc->StartFrameCapture(NULL, NULL);
		}

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
				pDoc->m_RootObj->Render(renderflags, order);
			});

			if ((active_tool == C3EditApp::ToolType::TT_WAND) && pDoc->m_Brush)
			{
				c3::RenderMethod::ForEachOrderedDrawDo([&](int order)
				{
					pDoc->m_Brush->Render(renderflags, order);
				});
			}

			// after the main pass, clear everything with black...
			prend->SetClearColor(&c3::Color::fBlack);
			m_LCBuf->SetClearColor(0, c3::Color::fBlack);

			// Lighting pass(es)
			prend->UseFrameBuffer(m_LCBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_UPDATEVIEWPORT); // | UFBFLAG_FINISHLAST);
			prend->SetDepthMode(c3::Renderer::DepthMode::DM_READONLY);
			prend->SetDepthTest(c3::Renderer::Test::DT_ALWAYS);
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			c3::RenderMethod::ForEachOrderedDrawDo([&](int order)
			{
				pDoc->m_RootObj->Render(RF_LIGHT, order);
			});

			if ((active_tool == C3EditApp::ToolType::TT_WAND) && pDoc->m_Brush)
				pDoc->m_Brush->Render(RF_LIGHT);

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
					pDoc->m_RootObj->Render(RF_SHADOW, order);
				});

				if ((active_tool == C3EditApp::ToolType::TT_WAND) && pDoc->m_Brush)
					pDoc->m_Brush->Render(RF_SHADOW);
			}

			// clear the render method and material
			prend->UseRenderMethod();
			prend->UseMaterial();

			if (pcam)
			{
				prend->SetViewMatrix(pcam->GetViewMatrix());
				prend->SetProjectionMatrix(pcam->GetProjectionMatrix());
				prend->SetEyePosition(pcam->GetEyePos());
				prend->SetEyeDirection(pcampos->GetFacingVector());
			}

			// Selection hilighting
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
			m_AuxBuf->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
			prend->UseFrameBuffer(m_AuxBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_UPDATEVIEWPORT); // | UFBFLAG_FINISHLAST);
			prend->SetDepthMode(c3::Renderer::DepthMode::DM_DISABLED);
			prend->UseProgram(m_SP_bounds);

			pDoc->DoForAllSelected([&](c3::Object *pobj)
			{
				pobj->Render(RF_FORCE | RF_LOCKSHADER | RF_LOCKMATERIAL | RF_AUXILIARY);
			});

			// Resolve
			prend->UseMaterial();
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
			m_BBuf[0]->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
			prend->UseFrameBuffer(m_BBuf[0], UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH); // | UFBFLAG_FINISHLAST);
			prend->SetDepthMode(c3::Renderer::DepthMode::DM_DISABLED);
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			prend->SetCullMode(c3::Renderer::CullMode::CM_DISABLED);
			prend->UseProgram(m_SP_combine);
			prend->UseVertexBuffer(prend->GetFullscreenPlaneVB());
			prend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);

			float bs = 2.0f;
			for (int b = 0; b < BLURTARGS - 1; b++)
			{
				m_BBuf[b + 1]->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
				prend->UseFrameBuffer(m_BBuf[b + 1], 0); // UFBFLAG_FINISHLAST);
				prend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
				prend->UseProgram(m_SP_blur);
				m_SP_blur->SetUniformTexture(m_BTex[b], m_uBlurTex);
				m_SP_blur->SetUniform1(m_uBlurScale, bs);
				m_SP_blur->ApplyUniforms(true);
				prend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);
				bs *= 2.0f;
			}

			prend->UseFrameBuffer(nullptr, 0); // UFBFLAG_FINISHLAST);
			prend->UseProgram(m_SP_resolve);
			glm::fmat4x4 revmat = glm::scale(glm::fvec3(-1, 1, 1));
			prend->SetTextureTransformMatrix(&revmat);
			m_SP_resolve->ApplyUniforms(true);
			prend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);

			c3::Gui *pgui = prend->GetGui();

			if (m_ShowDebug)
				pgui->ShowDebugWindow(&m_ShowDebug);

			prend->EndScene(BSFLAG_SHOWGUI);
			prend->Present();
		}

		if (capturing)
		{
			m_pRenderDoc->EndFrameCapture(NULL, NULL);
		}
	}
}

void C3EditView::InitializeGraphics()
{
	if (m_pRenderDoc && m_RenderDocCaptureFrame)
	{
		m_pRenderDoc->StartFrameCapture(NULL, NULL);
	}

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

		if (m_pRenderDoc)
			m_pRenderDoc->StartFrameCapture(NULL, NULL);

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

		if (m_pRenderDoc && m_RenderDocCaptureFrame)
		{
			m_RenderDocCaptureFrame = false;
			m_pRenderDoc->EndFrameCapture(NULL, NULL);
		}

		theApp.UpdateStatusMessage(_T(""));
	}
}


void C3EditView::HandleInput(c3::Positionable *pcampos)
{
	glm::vec3 mv(0, 0, 0);

	bool run = false;

	float spd = theApp.m_Config->GetFloat(_T("environment.movement.speed"), 1.0f) + (run * 2.0f);

	float mdf = (theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_W) +
		theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS1_POSY)) / 2.0f;

	float mdb = (theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_S) +
		theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS1_NEGY)) / 2.0f;

	mv += *(pcampos->GetFacingVector()) * (mdf - mdb) * spd;

	float mdl = (theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_A) +
		theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS1_NEGX)) / 2.0f;

	float mdr = (theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_D) +
		theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS1_POSX)) / 2.0f;

	mv += *(pcampos->GetLocalRightVector()) * (mdr - mdl) * spd;

	float mdu = (theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_Q) +
		theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS1_POSZ)) / 2.0f;
	mv.z += mdu * spd;

	float mdd = (theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_Z) +
		theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS1_NEGZ)) / 2.0f;
	mv.z -= mdd * spd;

	float ldu = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS2_NEGY, 1);
	float ldd = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS2_POSY, 1);
	float ldl = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS2_NEGX, 1);
	float ldr = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS2_POSX, 1);
	if (ldu != 0 || ldd != 0 || ldl != 0 || ldr != 0)
		AdjustYawPitch((ldl - ldr) * 4, (ldu - ldd) * 4, false);

	m_bCenter = theApp.m_C3->GetInputManager()->ButtonPressed(c3::InputDevice::VirtualButton::LETTER_C);

	if (theApp.m_C3->GetInputManager()->ButtonReleased(c3::InputDevice::VirtualButton::DEBUGBUTTON))
		m_ShowDebug ^= true;

	pcampos->AdjustPos(mv.x, mv.y, mv.z);
}


void C3EditView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void C3EditView::OnContextMenu(CWnd *pWnd, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// C3EditView diagnostics

#ifdef _DEBUG
void C3EditView::AssertValid() const
{
	CView::AssertValid();
}

void C3EditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

C3EditDoc* C3EditView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(C3EditDoc)));
	return (C3EditDoc*)m_pDocument;
}
#endif //_DEBUG


void C3EditView::OnDestroy()
{
	KillTimer('DRAW');
	KillTimer('SIZE');
	KillTimer('PICK');
	KillTimer('PROP');

	CView::OnDestroy();
}


void C3EditView::ComputePickRay(POINT screenpos, glm::fvec3 &pickpos, glm::fvec3 &pickvec) const
{
	C3EditDoc *pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

	c3::Object *camobj = pvi->m_Camera;
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type()));
	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type()));

	camobj->Update();

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


c3::Object *C3EditView::Pick(POINT p, glm::fvec3 *picked_point, bool allow_root) const
{
	c3::Object *ret = nullptr;

	C3EditDoc *pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	glm::fvec3 pickpos, pickray;

	ComputePickRay(p, pickpos, pickray);

	props::TFlags64 flagmask = OF_DRAW;
	if (uint32_t renderflags = theApp.m_Config->GetBool(_T("environment.editordraw"), true))
		flagmask |= OF_DRAWINEDITOR;

	float dist = FLT_MAX;
	pDoc->m_RootObj->Intersect(&pickpos, &pickray, nullptr, &dist, &ret, flagmask, -1, false);

	c3::Object *stop_obj = pDoc->m_OperationalRootObj;
	if (allow_root)
		stop_obj = stop_obj->GetParent();

	while (ret && (ret->GetParent() != stop_obj))
	{
		ret = ret->GetParent();
	}

	if (ret && picked_point)
		*picked_point = (pickray * dist) + pickpos;

	return ret;
}


void C3EditView::AdjustYawPitch(float yawadj, float pitchadj, bool redraw)
{
	C3EditDoc *pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

	c3::Object *camobj = pvi->m_Camera;
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type()));
	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type()));

	if (theApp.m_Config->GetBool(_T("environment.camera.lockroll"), true))
	{
		float &pitch = pvi->pitch;//glm::degrees(pcampos->GetPitch());
		float &yaw = pvi->yaw;//glm::degrees(pcampos->GetYaw());

		pitch += pitchadj;
		yaw += yawadj;

		props::IProperty *campitch_min = camobj->GetProperties()->GetPropertyById('PCAN');
		props::IProperty *campitch_max = camobj->GetProperties()->GetPropertyById('PCAX');

		float pitchmin = campitch_min ? campitch_min->AsFloat() : -FLT_MAX;
		float pitchmax = campitch_max ? campitch_max->AsFloat() :  FLT_MAX;

		pitch = std::min(std::max(pitchmin, pitch), pitchmax);

		pcampos->SetYawPitchRoll(0, 0, 0);
		pcampos->Update(0);
		pcampos->AdjustYaw(glm::radians(yaw));
		pcampos->Update(0);
		pcampos->AdjustPitch(glm::radians(pitch));
	}
	else
	{
		pcampos->AdjustYaw(glm::radians(yawadj));
		pcampos->Update(0);
		pcampos->AdjustPitch(glm::radians(pitchadj));
	}
}


void C3EditView::OnMouseMove(UINT nFlags, CPoint point)
{
	C3EditDoc *pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	theApp.m_C3->GetInputManager()->SetMousePos(point.x, point.y);

	int64_t active_tool = theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT);
	int64_t active_axis_t = theApp.m_Config->GetInt(_T("environment.active.axis.translation"), C3EditApp::AT_X | C3EditApp::AT_Y);
	int64_t active_axis_r = theApp.m_Config->GetInt(_T("environment.active.axis.rotation"), C3EditApp::AT_Z);
	int64_t active_axis_s = theApp.m_Config->GetInt(_T("environment.active.axis.scale"), C3EditApp::AT_X | C3EditApp::AT_Y);

	if (nFlags & MK_SHIFT)
	{
		SetCursor(theApp.LoadCursor(IDC_CAMERA_PANTILT));
	}
	else if (nFlags & MK_MBUTTON)
	{
		SetCursor(theApp.LoadCursor(IDC_ZOOM));
	}
	else
	{
		SetAppropriateMouseCursor(nFlags);
	}

	CView::OnMouseMove(nFlags, point);

	int deltax = point.x - m_MousePos.x;
	int deltay = m_MousePos.y - point.y;

	m_MousePos.x = point.x;
	m_MousePos.y = point.y;

	C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

	CRect r;
	GetClientRect(&r);
	ClientToScreen(&r);

	float scrpctx = (float)point.x / (float)r.Width();
	float scrpcty = (float)point.y / (float)r.Height();

	c3::Object *camobj = pvi->m_Camera;
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type()));
	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type()));

	glm::fvec3 pickpos, pickvec;
	ComputePickRay(m_MousePos, pickpos, pickvec);
	theApp.m_C3->GetInputManager()->SetPickRay(pickpos, pickvec);

	// Wrap the cursor around to the other side if we've Captured the mouse and it's gone beyond the client rect...
	if (this == GetCapture())
	{
		ClientToScreen(&point);

		CPoint p = point;

		// If we're out the client area, the get the amount we're out and move us off the other side by that much
		if (p.x < r.left)
			p.x = r.right - (r.left - p.x);
		else if (p.x > r.right)
			p.x = r.left + (p.x - r.right);

		// If we're out the client area, the get the amount we're out and move us off the other side by that much
		if (p.y < r.top)
			p.y = r.bottom - (r.top - p.y);
		else if (p.y > r.bottom)
			p.y = r.top + (p.y - r.bottom);

		// If the point we have now isn't the same as when we started (ie., we wrapped), then reset the cursor position
		if (p != point)
		{
			SetCursorPos(p.x, p.y);

			ScreenToClient(&p);
			m_MousePos.x = p.x;
			m_MousePos.y = p.y;
		}

		// If the user isn't holding down the shift key (for mouse look) or using a tool, release the mouse...
		if (!(nFlags & (MK_LBUTTON | MK_MBUTTON | MK_SHIFT)))
			ReleaseCapture();
	}

	// It the mouse isn't currently captured, but it should be because we're mouse-looking (shift key), then capture it...
	if ((nFlags & (MK_SHIFT | MK_MBUTTON | MK_LBUTTON)) && (this != GetCapture()))
		SetCapture();

	if ((active_tool == C3EditApp::ToolType::TT_WAND) && pDoc->m_Brush)
	{
		glm::fvec3 pp, pd, pf;
		ComputePickRay(m_MousePos, pp, pd);

		int64_t placemode = (int)theApp.m_Config->GetInt(_T("brush.placemode"), CBrushSettingsDlg::PlacementMode::PLACEMENT_RAYCAST);
		if (placemode == CBrushSettingsDlg::PlacementMode::PLACEMENT_RAYCAST)
		{
			c3::Object *pickobj = Pick(m_MousePos, &pf, true);

			if (!pickobj)
				pf = pp + (glm::normalize(pd) * 20.0f);
		}
		else
		{
			glm::fvec3 orig(0, 0, 0), pdir(0, 0, 0);
			if (placemode < CBrushSettingsDlg::PlacementMode::PLACEMENT_XYORIG)
				pcampos->GetPosVec(&orig);
			float d;
			switch (placemode)
			{
				case CBrushSettingsDlg::PlacementMode::PLACEMENT_XYORIG:
				case CBrushSettingsDlg::PlacementMode::PLACEMENT_XYCAM:
					pdir.z = 1;
					break;

				case CBrushSettingsDlg::PlacementMode::PLACEMENT_YZORIG:
				case CBrushSettingsDlg::PlacementMode::PLACEMENT_YZCAM:
					pdir.x = 1;
					break;

				case CBrushSettingsDlg::PlacementMode::PLACEMENT_XZORIG:
				case CBrushSettingsDlg::PlacementMode::PLACEMENT_XZCAM:
					pdir.y = 1;
					break;
			}

			glm::intersectRayPlane(pp, pd, orig, pdir, d);
			pf = pp + (glm::normalize(pd) * d);
		}

		c3::Positionable *pbp = pDoc->m_Brush ? dynamic_cast<c3::Positionable *>(pDoc->m_Brush->FindComponent(c3::Positionable::Type())) : nullptr;

		if (theApp.m_Config->GetBool(_T("brush.snap.x.apply"), false))
		{
			float snapx = theApp.m_Config->GetFloat(_T("brush.snap.x.val"), 1.0f);
			pf.x = roundf(pf.x / snapx) * snapx;
		}

		if (theApp.m_Config->GetBool(_T("brush.snap.y.apply"), false))
		{
			float snapy = theApp.m_Config->GetFloat(_T("brush.snap.y.val"), 1.0f);
			pf.y = roundf(pf.y / snapy) * snapy;
		}

		if (theApp.m_Config->GetBool(_T("brush.snap.z.apply"), false))
		{
			float snapz = theApp.m_Config->GetFloat(_T("brush.snap.z.apply"), 1.0f);
			pf.z = roundf(pf.z / snapz) * snapz;
		}

		pbp->SetPos(pf.x, pf.y, pf.z);
	}

	if ((nFlags & MK_SHIFT) && (deltax || deltay))// && !camori_lock)
	{
		// If the user is holding down the shift key, pan / tilt the camera

		C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

		AdjustYawPitch((float)-deltax, (float)deltay);
	}
	else if (nFlags & MK_MBUTTON)
	{
		// If the user is holding down the middle mouse button, zoom the camera when they move up/down

		float d = pcam->GetPolarDistance();
		d += ((deltay < 0) ? 0.5f : -0.5f);
		d = std::max(d, 0.1f);

		pcam->SetPolarDistance(d);
	}
	else if (nFlags & MK_LBUTTON)
	{
		float tsens = theApp.m_Config->GetFloat(_T("environment.sensitivity.translation"), 0.1f);
		float rsens = theApp.m_Config->GetFloat(_T("environment.sensitivity.rotation"), 0.1f);
		float ssens = theApp.m_Config->GetFloat(_T("environment.sensitivity.scale"), 0.1f);

		static glm::fvec3 xaxis(1, 0, 0), yaxis(0, 1, 0), zaxis(0, 0, 1);

		for (size_t i = 0, maxi = pDoc->GetNumSelected(); i < maxi; i++)
		{
			c3::Object *obj = pDoc->GetSelection(i);
			if (!obj || obj->Flags().IsSet(OF_LOCKED))
				continue;

			c3::Positionable *pos = dynamic_cast<c3::Positionable *>(obj->FindComponent(c3::Positionable::Type()));
			if (!pos)
				continue;

			switch (active_tool)
			{
				// For movement, use the orientation of the camera!  World coordinates make it too confusing!
				case C3EditApp::TT_TRANSLATE:
				{
					glm::fvec3 fv, rv, uv;
					pcampos->GetFacingVector(&fv);
					pcampos->GetLocalRightVector(&rv);
					pcampos->GetLocalUpVector(&uv);

					glm::fvec3 mv;

					if (active_axis_t & C3EditApp::AT_X)
					{
						if (active_axis_t & C3EditApp::AT_SCREENREL)
						{
							mv += (rv * (float)(deltax));
						}
						else
						{
							mv += xaxis * ((glm::dot(fv, xaxis) * (float)deltay) + (glm::dot(rv, xaxis) * (float)deltax));
						}
					}

					if (active_axis_t & C3EditApp::AT_Y)
					{
						if (active_axis_t & C3EditApp::AT_SCREENREL)
						{
							mv += (uv * (float)(deltay));
						}
						else
						{
							mv += yaxis * ((glm::dot(fv, yaxis) * (float)deltay) + (glm::dot(rv, yaxis) * (float)deltax));
						}
					}

					if (active_axis_t & C3EditApp::AT_Z)
					{
						if (active_axis_t & C3EditApp::AT_SCREENREL)
						{
							mv += (fv * (float)(deltay));
						}
						else
						{
							mv += zaxis * (float)deltay;
						}
					}

					mv *= tsens;

					pos->AdjustPos(mv.x, mv.y, mv.z);

					pDoc->SetModifiedFlag();

					break;
				}

				case C3EditApp::TT_ROTATE:
				{
					float pitchval = glm::radians((active_axis_r & C3EditApp::AT_X) ? (deltax * rsens) : 0);
					float rollval = glm::radians((active_axis_r & C3EditApp::AT_Y) ? (deltay * rsens) : 0);
					float yawval = glm::radians((active_axis_r & C3EditApp::AT_Z) ? (deltax * rsens) : 0);

					pos->AdjustPitch(pitchval);
					pos->AdjustRoll(rollval);
					pos->AdjustYaw(yawval);

					pDoc->SetModifiedFlag();

					break;
				}

				case C3EditApp::TT_UNISCALE:
				{
					float sclval = deltay * ssens;

					pos->AdjustScl(sclval, sclval, sclval);

					pDoc->SetModifiedFlag();

					break;
				}

				case C3EditApp::TT_SCALE:
				{
					float xscl = (active_axis_s & C3EditApp::AT_X) ? (deltax * ssens) : 0.0f;
					float yscl = (active_axis_s & C3EditApp::AT_Y) ? (deltay * ssens) : 0.0f;
					float zscl = (active_axis_s & C3EditApp::AT_Z) ? (deltay * ssens) : 0.0f;

					pos->AdjustScl(xscl, yscl, zscl);

					pDoc->SetModifiedFlag();

					break;
				}
			}
		}
	}
}


void C3EditView::OnTimer(UINT_PTR nIDEvent)
{
	C3EditDoc *pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	switch (nIDEvent)
	{
		case 'DRAW':
			RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
			break;

		case 'PICK':
			if (theApp.m_Config->GetBool(_T("environment.hoverinfo"), false))
			{
				m_pHoverObj = Pick(m_MousePos);
				pDoc->UpdateStatusMessage(m_pHoverObj);
			}
			break;

		case 'PROP':
			theApp.RefreshActiveProperties();
			break;

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

				AdjustYawPitch(0, 0, false);
			}

			c3::util::RecursiveObjectAction(pDoc->m_RootObj, [](c3::Object *pobj)
			{
				pobj->PropertyChanged(pobj->GetProperties()->GetPropertyById('C3RM'));
			});

			KillTimer('SIZE');
			SetTimer('DRAW', 17, nullptr);
			break;
		}

		default:
			break;
	}

	CView::OnTimer(nIDEvent);
}


BOOL C3EditView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	C3EditDoc *pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

	c3::Object *camobj = pvi->m_Camera;

	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type()));
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		float d = pcam->GetPolarDistance();
		d += ((zDelta < 0) ? 0.5f : -0.5f);
		d = std::max(d, 0.1f);

		pcam->SetPolarDistance(d);
	}

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void C3EditView::OnSize(UINT nType, int cx, int cy)
{
	// Reset the timers
	KillTimer('DRAW');
	KillTimer('SIZE');

	CView::OnSize(nType, cx, cy);

	SetTimer('SIZE', 500, NULL);
}


void C3EditView::OnUpdateEditTriggerRenderDocCapture(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((m_pRenderDoc != nullptr) ? TRUE : FALSE);
}


void C3EditView::OnEditTriggerRenderDocCapture()
{
	m_RenderDocCaptureFrame = true;
}


void C3EditView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CView::OnLButtonDown(nFlags, point);

	C3EditDoc *pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT) != C3EditApp::TT_WAND)
	{
		c3::Object *pickobj = Pick(point);

		if (pickobj)
		{
			if (nFlags & MK_CONTROL)
			{
				if (pickobj && pDoc->IsSelected(pickobj))
					pDoc->RemoveFromSelection(pickobj);
				else
					pDoc->AddToSelection(pickobj);
			}
			else if (!pDoc->IsSelected(pickobj))
			{
				pDoc->ClearSelection();

				if (pickobj)
					pDoc->AddToSelection(pickobj);
			}
		}
		else
		{
			if (!(nFlags & MK_CONTROL))
				pDoc->ClearSelection();
		}

		theApp.SetActiveObject(pickobj);
	}
	else
	{
		c3::Object *root = pDoc->m_OperationalRootObj ? pDoc->m_OperationalRootObj : pDoc->m_RootObj;

		c3::Object *pobj = theApp.m_C3->GetFactory()->Build(pDoc->m_Brush, nullptr, root, true);
		pobj->Flags().Set(OF_EXPANDED);

		pDoc->SetModifiedFlag();
		((C3EditFrame *)(theApp.GetMainWnd()))->UpdateObjectList();

		RandomizeBrush();
	}

	SetAppropriateMouseCursor(nFlags);

	theApp.m_C3->GetRenderer()->GetGui()->AddMouseButtonEvent(c3::Gui::MouseButton::MBUT_LEFT, true);
}


void C3EditView::OnLButtonUp(UINT nFlags, CPoint point)
{
	C3EditDoc *pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CView::OnLButtonUp(nFlags, point);

	if (GetCapture() == this)
		ReleaseCapture();

	SetAppropriateMouseCursor(nFlags);

	theApp.m_C3->GetRenderer()->GetGui()->AddMouseButtonEvent(c3::Gui::MouseButton::MBUT_LEFT, false);
}


void C3EditView::OnSetFocus(CWnd *pOldWnd)
{
	CView::OnSetFocus(pOldWnd);

	theApp.m_C3->GetInputManager()->AcquireAll();
}


void C3EditView::SetAppropriateMouseCursor(UINT32 nFlags)
{
	int64_t active_tool = theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT);

	HCURSOR hcur = NULL;

	if (GetCapture() != this)
	{
		switch (active_tool)
		{
			case C3EditApp::TT_SELECT:
				if (nFlags & MK_CONTROL)
				{
					hcur = theApp.LoadCursor(IDC_SELECT_PLUS);
				}
				else
				{
					hcur = theApp.LoadCursor(IDC_SELECT);
				}
				break;

			case C3EditApp::TT_TRANSLATE:
				hcur = theApp.LoadCursor(IDC_TRANSLATE);
				break;

			case C3EditApp::TT_ROTATE:
				hcur = theApp.LoadCursor(IDC_ROTATE);
				break;

			case C3EditApp::TT_UNISCALE:
				hcur = theApp.LoadCursor(IDC_UNISCALE);
				break;

			case C3EditApp::TT_SCALE:
				hcur = theApp.LoadCursor(IDC_SCALE);
				break;

			case C3EditApp::TT_WAND:
				hcur = theApp.LoadCursor(IDC_WAND);
				break;
		}
	}

	SetCursor(hcur);
}


void C3EditView::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
	C3EditDoc *pDoc = GetDocument();

	pCmdUI->Enable(pDoc->GetNumSelected() ? TRUE : FALSE);
}


void C3EditView::OnEditDelete()
{
	if (GetFocus() != this)
		return;

	C3EditDoc *pDoc = GetDocument();

	if (!pDoc->IsSelected(pDoc->m_RootObj))
	{
		pDoc->SortSelectionsByDescendingDepth();

		pDoc->DoForAllSelected([&](c3::Object *pobj)
		{
			pobj->Release();
		});
	}
	else
	{
		while (size_t i = pDoc->m_RootObj->GetNumChildren())
		{
			i--;
			pDoc->m_RootObj->RemoveChild(pDoc->m_RootObj->GetChild(i), true);
		}
	}

	((C3EditFrame *)(theApp.GetMainWnd()))->UpdateObjectList();

	pDoc->ClearSelection();
}


bool ValidSelection_SameParent(C3EditDoc *pd)
{
	// To duplicate Objects, they all have to have the same parent

	size_t maxi = pd ? pd->GetNumSelected() : 0;
	bool b = maxi > 0;

	if (maxi)
	{
		c3::Object *o = pd->GetSelection(0)->GetParent();
		if (!o)
		{
			b = false;
		}
		else
		{
			pd->DoForAllSelectedBreakable([&](c3::Object *pobj)
			{
				if (pobj->GetParent() != o)
				{
					b = false;
					return false;
				}

				return true;
			});
		}
	}

	return b;
}


bool ValidSelection_OnlyOne(C3EditDoc *pd)
{
	// Only allow 1 selection to be ungrouped -- and it can't be the root

	size_t c = pd ? pd->GetNumSelected() : 0;
	if (c != 1)
		return false;

	c3::Object *o = pd->GetSelection(0);
	if (!o->GetParent())
		return false;
	
	if (!o->GetNumChildren())
		return false;

	return true;
}


void C3EditView::OnUpdateEditDuplicate(CCmdUI *pCmdUI)
{
	C3EditDoc* pDoc = GetDocument();

	pCmdUI->Enable(ValidSelection_SameParent(pDoc));
}


void C3EditView::OnEditDuplicate()
{
	C3EditDoc* pDoc = GetDocument();

	pDoc->DoForAllSelected([&](c3::Object *pobj)
	{
		c3::Object *pdo = theApp.m_C3->GetFactory()->Build(pobj, nullptr, pobj->GetParent(), true);
		pdo->SetParent(pobj->GetParent());
	});

	((C3EditFrame *)(theApp.GetMainWnd()))->UpdateObjectList();
}


void C3EditView::OnUpdateEditGroup(CCmdUI *pCmdUI)
{
	C3EditDoc* pDoc = GetDocument();

	pCmdUI->Enable(ValidSelection_SameParent(pDoc));
}


void C3EditView::OnEditGroup()
{
	C3EditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	c3::Object *pg = theApp.m_C3->GetFactory()->Build();
	if (pg)
	{
		pg->SetName(_T("Group"));
		pg->Flags().Set(OF_UPDATE | OF_DRAW | OF_LIGHT | OF_CASTSHADOW | OF_CHECKCOLLISIONS | OF_EXPANDED);
		pg->SetParent(pDoc->GetSelection(0)->GetParent());

		pDoc->DoForAllSelected([&](c3::Object *pobj)
		{
			pobj->SetParent(pg);
		});

		pDoc->ClearSelection();
		pDoc->AddToSelection(pg);

		((C3EditFrame *)(theApp.GetMainWnd()))->UpdateObjectList();
	}
}


void C3EditView::OnUpdateEditUngroup(CCmdUI *pCmdUI)
{
	C3EditDoc* pDoc = GetDocument();

	pCmdUI->Enable(ValidSelection_OnlyOne(pDoc));
}


void C3EditView::OnEditUngroup()
{
	C3EditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	c3::Object *pg = pDoc->GetSelection(0);
	if (!pg->GetParent())
		return;

	pDoc->ClearSelection();
	while (size_t i = pg->GetNumChildren())
	{
		c3::Object *o = pg->GetChild(i - 1);
		o->SetParent(pg->GetParent());
		pDoc->AddToSelection(o);
	}

	pg->Release();

	((C3EditFrame *)(theApp.GetMainWnd()))->UpdateObjectList();
}


void C3EditView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	C3EditFrame *pmf = (C3EditFrame *)theApp.GetMainWnd();
	switch (nChar)
	{
		case VK_OEM_3:	// '~'
		{
			props::TFlags64 a = pmf->GetAxes();
			a.Toggle(C3EditApp::AT_X);
			a.Toggle(C3EditApp::AT_Y);
			a.Toggle(C3EditApp::AT_Z);
			pmf->SetAxes(a);
			break;
		}

		case _T('1'):
			pmf->SetActiveTool(C3EditApp::TT_SELECT);
			break;

		case _T('2'):
			pmf->SetActiveTool(C3EditApp::TT_TRANSLATE);
			break;

		case _T('3'):
			pmf->SetActiveTool(C3EditApp::TT_ROTATE);
			break;

		case _T('4'):
			pmf->SetActiveTool(C3EditApp::TT_UNISCALE);
			break;

		case _T('5'):
			pmf->SetActiveTool(C3EditApp::TT_SCALE);
			break;

		case _T('6'):
			if (theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT) == C3EditApp::TT_WAND)
			{
				// if the tool is already the brush, then advance the yaw or re-randomize if no detents are set
				if (!AdvanceBrushYaw())
					RandomizeBrush();
			}
			else
			{
				pmf->SetActiveTool(C3EditApp::TT_WAND);
				RandomizeBrush();
			}
			break;

		case VK_DELETE:
			OnEditDelete();
			break;
	}

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}


void C3EditView::OnGraphDeleteNode()
{
	// TODO: Add your command handler code here
}


void C3EditView::OnUpdateGraphDeleteNode(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}


void C3EditView::CenterViewOnSelection()
{
	C3EditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (!pDoc->m_RootObj)
		return;

	C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

	c3::Object *camobj = pvi->m_Camera;
	c3::Positionable *pcampos = camobj ? dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type())) : nullptr;

	glm::fvec3 cpos;
	size_t ct = 0;
	pDoc->DoForAllSelected([&](c3::Object *pobj)
	{
		glm::fmat4x4 root_mat;
		c3::util::ComputeFinalTransform(pobj, &root_mat);
		glm::fvec3 pt = root_mat * glm::fvec4(0, 0, 0, 1);
		cpos += pt;
		ct++;
	});

	if (ct)
	{
		cpos /= (float)ct;
		pcampos->SetPosVec(&cpos);
		camobj->Update();
	}
}

void C3EditView::OnEditCenterCamera()
{
	m_bCenter = true;
}


void C3EditView::OnUpdateEditCenterCamera(CCmdUI *pCmdUI)
{
	C3EditDoc* pDoc = GetDocument();

	pCmdUI->Enable(pDoc && pDoc->GetNumSelected());
}


void C3EditView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	C3EditFrame *pmf = (C3EditFrame *)theApp.GetMainWnd();
	switch (nChar)
	{
		case VK_OEM_MINUS:
		case VK_SUBTRACT:
		{
			float spd = theApp.m_Config->GetFloat(_T("environment.movement.speed"), 1.0f);
			theApp.m_Config->SetFloat(_T("environment.movement.speed"), std::max<float>(spd - 1.0f, 1.0f));
			break;
		}

		case VK_OEM_PLUS:
		case VK_ADD:
		{
			float spd = theApp.m_Config->GetFloat(_T("environment.movement.speed"), 1.0f);
			theApp.m_Config->SetFloat(_T("environment.movement.speed"), spd + 1.0f);
			break;
		}
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void C3EditView::OnEditCameraSettings()
{
	C3EditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

	c3::Object *camobj = pvi->m_Camera;

	theApp.SetActiveObject(camobj, false, _T("Camera"));
}


void C3EditView::OnUpdateEditCameraSettings(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}


void C3EditView::OnUpdateViewHoverInformation(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();

	bool b = theApp.m_Config->GetBool(_T("environment.hoverinfo"), false);
	pCmdUI->SetCheck(b);
}


void C3EditView::OnViewHoverInformation()
{
	bool b = !theApp.m_Config->GetBool(_T("environment.hoverinfo"), false);
	theApp.m_Config->SetBool(_T("environment.hoverinfo"), b);
}


void C3EditView::OnEditCopy()
{
	if (GetFocus() != this)
		return;

	if (theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT) != C3EditApp::TT_WAND)
	{
		C3EditDoc* pDoc = GetDocument();

		c3::Object *br = nullptr;

		if (pDoc->GetNumSelected() > 1)
		{
			br = theApp.m_C3->GetFactory()->Build();
			br->SetName(_T("[COPIED OBJECTS]"));
			br->Flags().Set(OF_UPDATE | OF_DRAW | OF_LIGHT | OF_CASTSHADOW | OF_CHECKCOLLISIONS | OF_EXPANDED);
			br->AddComponent(c3::Positionable::Type());

			pDoc->DoForAllSelected([&](c3::Object *pobj)
			{
				c3::Object *brc = theApp.m_C3->GetFactory()->Build(pobj, nullptr, br, true);
			});
		}
		else if (pDoc->GetNumSelected() == 1)
		{
			 br = theApp.m_C3->GetFactory()->Build(pDoc->GetSelection(0), nullptr, nullptr, true);
		}

		pDoc->SetBrush(br);
	}
}


void C3EditView::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	C3EditDoc* pDoc = GetDocument();

	pCmdUI->Enable(ValidSelection_SameParent(pDoc));
}


void C3EditView::OnEditCut()
{
	if (GetFocus() != this)
		return;

	if (theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT) != C3EditApp::TT_WAND)
	{
		C3EditDoc* pDoc = GetDocument();

		c3::Object *br = nullptr;

		if (pDoc->GetNumSelected() > 1)
		{
			br = theApp.m_C3->GetFactory()->Build();
			br->SetName(_T("[COPIED OBJECTS]"));
			br->Flags().Set(OF_UPDATE | OF_DRAW | OF_LIGHT | OF_CASTSHADOW | OF_CHECKCOLLISIONS | OF_EXPANDED);
			br->AddComponent(c3::Positionable::Type());

			pDoc->DoForAllSelected([&](c3::Object *pobj)
			{
				pobj->SetParent(br);
			});
		}
		else if (pDoc->GetNumSelected() == 1)
		{
			br = pDoc->GetSelection(0);
			br->SetParent(nullptr);
		}

		pDoc->SetBrush(br);
		pDoc->ClearSelection();
		pDoc->SetModifiedFlag();

		((C3EditFrame *)(theApp.GetMainWnd()))->UpdateObjectList();
	}
}


void C3EditView::OnUpdateEditCut(CCmdUI *pCmdUI)
{
	C3EditDoc* pDoc = GetDocument();

	pCmdUI->Enable(ValidSelection_SameParent(pDoc));
}


void C3EditView::OnEditPaste()
{
	C3EditDoc* pDoc = GetDocument();

	// get the combined transform and then use on the camera position to derived an offset from the root

	c3::Object *root = pDoc->m_OperationalRootObj ? pDoc->m_OperationalRootObj : pDoc->m_RootObj;

	c3::Object *pobj = theApp.m_C3->GetFactory()->Build(pDoc->m_Brush, nullptr, root, true);
	pobj->Flags().Set(OF_EXPANDED);

	c3::Positionable *pobjpos = (c3::Positionable *)(pobj->FindComponent(c3::Positionable::Type()));
	if (pobjpos)
	{
		c3::Object *pcam = pDoc->GetPerViewInfo(GetSafeHwnd())->m_Camera;
		c3::Camera *pcampos = (c3::Camera *)(pcam->FindComponent(c3::Camera::Type()));
		pcam->Update();

		glm::fvec3 ct;
		pcampos->GetTargetPos(&ct);

		glm::fmat4x4 root_mat;
		c3::util::ComputeFinalTransform(root, &root_mat);
		glm::fvec3 pt = root_mat * glm::fvec4(0, 0, 0, 1);
		ct -= pt;

		pobjpos->SetPosVec(&ct);
	}

	pDoc->SetModifiedFlag();
	((C3EditFrame *)(theApp.GetMainWnd()))->UpdateObjectList();
}


void C3EditView::OnUpdateEditPaste(CCmdUI *pCmdUI)
{
	C3EditDoc* pDoc = GetDocument();

	pCmdUI->Enable(pDoc->m_Brush != nullptr);
}


void C3EditView::OnUpdateEditAssignRoot(CCmdUI *pCmdUI)
{
	C3EditDoc* pDoc = GetDocument();

	pCmdUI->Enable(pDoc->GetNumSelected() == 1);
}


void C3EditView::OnEditAssignRoot()
{
	C3EditDoc* pDoc = GetDocument();

	pDoc->m_OperationalRootObj = pDoc->GetSelection(0);

	((C3EditFrame *)(theApp.GetMainWnd()))->UpdateObjectList();
}


void C3EditView::OnEditBrushSettings()
{
	CBrushSettingsDlg dlg;
	dlg.DoModal();

	RandomizeBrush();
}


bool C3EditView::AdvanceBrushYaw()
{
	C3EditDoc* pDoc = GetDocument();
	if (!pDoc->m_Brush)
		return false;

	c3::Positionable *ppos = (c3::Positionable *)pDoc->m_Brush->FindComponent(c3::Positionable::Type());
	if (!ppos)
		return false;

	if (!theApp.m_Config->GetBool(_T("brush.yaw.apply"), true) || !theApp.m_Config->GetBool(_T("brush.yaw.detents.apply"), true))
		return false;

	float minyaw = theApp.m_Config->GetFloat(_T("brush.yaw.min"), 0.0f);
	float maxyaw = theApp.m_Config->GetFloat(_T("brush.yaw.max"), 360.0f);
	if (minyaw > maxyaw)
		std::swap(minyaw, maxyaw);

	int64_t dv = theApp.m_Config->GetInt(_T("brush.yaw.detents.val"), 4);
	float dr = (maxyaw - minyaw) / (float)dv;

	ppos->AdjustYaw(glm::radians(dr));

	return true;
}

void C3EditView::RandomizeBrush()
{
	C3EditDoc* pDoc = GetDocument();
	if (!pDoc->m_Brush)
		return;

	c3::Positionable *ppos = (c3::Positionable *)pDoc->m_Brush->FindComponent(c3::Positionable::Type());
	if (!ppos)
		return;

	bool yaw = theApp.m_Config->GetBool(_T("brush.yaw.apply"), true);
	float minyaw = theApp.m_Config->GetFloat(_T("brush.yaw.min"), 0.0f);
	float maxyaw = theApp.m_Config->GetFloat(_T("brush.yaw.max"), 360.0f);
	if (minyaw > maxyaw)
		std::swap(minyaw, maxyaw);

	bool pitch = theApp.m_Config->GetBool(_T("brush.pitch.apply"), false);
	bool minpitch = theApp.m_Config->GetFloat(_T("brush.pitch.min"), 0.0f);
	bool maxpitch = theApp.m_Config->GetFloat(_T("brush.pitch.max"), 0.0f);
	if (minpitch > maxpitch)
		std::swap(minpitch, maxpitch);

	bool roll = theApp.m_Config->GetBool(_T("brush.roll.apply"), false);
	float minroll = theApp.m_Config->GetFloat(_T("brush.roll.min"), 0.0f);
	float maxroll = theApp.m_Config->GetFloat(_T("brush.roll.max"), 0.0f);
	if (minroll > maxroll)
		std::swap(minroll, maxroll);

	bool sclx = theApp.m_Config->GetBool(_T("brush.scale.x.apply"), true);
	float minsclx = theApp.m_Config->GetFloat(_T("brush.scale.x.min"), 1.0f);
	float maxsclx = theApp.m_Config->GetFloat(_T("brush.scale.x.max"), 1.0f);

	bool scly = theApp.m_Config->GetBool(_T("brush.scale.y.apply"), true);
	float minscly = theApp.m_Config->GetFloat(_T("brush.scale.y.min"), 1.0f);
	float maxscly = theApp.m_Config->GetFloat(_T("brush.scale.y.max"), 1.0f);

	bool sclz = theApp.m_Config->GetBool(_T("brush.scale.z.apply"), true);
	float minsclz = theApp.m_Config->GetFloat(_T("brush.scale.z.min"), 1.0f);
	float maxsclz = theApp.m_Config->GetFloat(_T("brush.scale.z.max"), 1.0f);

	bool lockxy = theApp.m_Config->GetBool(_T("brush.scale.lockxy"), true);
	bool lockyz = theApp.m_Config->GetBool(_T("brush.scale.lockyz"), false);

	float y = 0, p = 0, r = 0;
	float xs = 1, ys = 1, zs = 1;

	if (yaw)
	{
		y = c3::math::RandomRange(minyaw, maxyaw);

		if (theApp.m_Config->GetBool(_T("brush.yaw.detents.apply"), true))
		{
			int64_t dv = theApp.m_Config->GetInt(_T("brush.yaw.detents.val"), 4);
			float dr = (maxyaw - minyaw) / (float)dv;
			float d = floor((y - minyaw) / (float)dr);
			y = d * dr + minyaw;
		}
	}

	if (pitch)
	{
		p = c3::math::RandomRange(minpitch, maxpitch);

		if (theApp.m_Config->GetBool(_T("brush.pitch.detents.apply"), true))
		{
			int64_t dv = theApp.m_Config->GetInt(_T("brush.pitch.detents.val"), 4);
			float dr = (maxpitch - minpitch) / (float)dv;
			float d = floor((p - minpitch) / (float)dr);
			p = d * dr + minpitch;
		}
	}

	if (roll)
	{
		r = c3::math::RandomRange(minroll, maxroll);

		if (theApp.m_Config->GetBool(_T("brush.roll.detents.apply"), true))
		{
			int64_t dv = theApp.m_Config->GetInt(_T("brush.roll.detents.val"), 4);
			float dr = (maxroll - minroll) / (float)dv;
			float d = floor((r - minroll) / (float)dr);
			r = d * dr + minroll;
		}
	}

	if (sclz)
		zs = c3::math::RandomRange(minsclz, maxsclz);

	if (scly)
		ys = c3::math::RandomRange(minscly, maxscly);

	if (sclx)
		xs = c3::math::RandomRange(minsclx, maxsclx);

	if (lockyz)
		ys = zs;

	if (lockxy)
		xs = ys;

	ppos->SetYawPitchRoll(glm::radians(y), glm::radians(p), glm::radians(r));
	ppos->SetScl(xs, ys, zs);
}



void C3EditView::OnUpdateToolsRepath(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(true);
}


void C3EditView::OnToolsRepath()
{
	CRepathDlg dlg;
	dlg.DoModal();

	((C3EditFrame *)(theApp.GetMainWnd()))->RefreshActiveProperties();
}
