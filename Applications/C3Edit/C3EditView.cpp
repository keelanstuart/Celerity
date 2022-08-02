
// C3EditView.cpp : implementation of the C3EditView class
//

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
	ON_UPDATE_COMMAND_UI(ID_EDIT_TRIGGERRENDERDOCCAPTURE, &C3EditView::OnUpdateEditTriggerrenderdoccapture)
	ON_COMMAND(ID_EDIT_TRIGGERRENDERDOCCAPTURE, &C3EditView::OnEditTriggerrenderdoccapture)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

c3::FrameBuffer *C3EditView::m_GBuf = nullptr;
c3::FrameBuffer *C3EditView::m_LCBuf = nullptr;
c3::FrameBuffer *C3EditView::m_SSBuf = nullptr;
std::vector<c3::Texture2D *> C3EditView::m_ColorTarg = { };
c3::DepthBuffer *C3EditView::m_DepthTarg = nullptr;
c3::DepthBuffer *C3EditView::m_ShadowTarg = nullptr;
std::vector<c3::Texture2D *> C3EditView::m_BTex = { };
std::vector<c3::FrameBuffer *> C3EditView::m_BBuf = { };
c3::ShaderComponent *C3EditView::m_VS_resolve = nullptr;
c3::ShaderComponent *C3EditView::m_FS_resolve = nullptr;
c3::ShaderProgram *C3EditView::m_SP_resolve = nullptr;
c3::ShaderComponent *C3EditView::m_VS_blur = nullptr;
c3::ShaderComponent *C3EditView::m_FS_blur = nullptr;
c3::ShaderProgram *C3EditView::m_SP_blur = nullptr;
c3::ShaderComponent *C3EditView::m_VS_combine = nullptr;
c3::ShaderComponent *C3EditView::m_FS_combine = nullptr;
c3::ShaderProgram *C3EditView::m_SP_combine = nullptr;

int32_t C3EditView::m_ulSunDir = -1;
int32_t C3EditView::m_ulSunColor = -1;
int32_t C3EditView::m_ulAmbientColor = -1;
glm::fvec3 C3EditView::m_SunDir = { 0, 0, -1 };
glm::fvec3 C3EditView::m_SunColor = { 1.0f, 1.0f, 1.0f };
glm::fvec3 C3EditView::m_AmbientColor = { 0.1f, 0.1f, 0.1f };
int32_t C3EditView::m_uBlurTex = -1;
int32_t C3EditView::m_uBlurScale = -1;

RENDERDOC_API_1_4_0 *C3EditView::m_pRenderDoc = nullptr;

size_t C3EditView::m_ResourcesRefCt = 0;


C3EditView::C3EditView() noexcept
{
	m_RenderDocCaptureFrame = theApp.m_Config->GetBool(_T("debug.renderdoc.capture_initialization"), true);
	m_ResourcesRefCt++;
}


C3EditView::~C3EditView()
{
	m_ResourcesRefCt--;

	if (!m_ResourcesRefCt)
	{
		C3_SAFERELEASE(m_GBuf);
		C3_SAFERELEASE(m_LCBuf);
		C3_SAFERELEASE(m_SSBuf);
		for (auto p : m_ColorTarg)
			C3_SAFERELEASE(p);
		C3_SAFERELEASE(m_DepthTarg);
		C3_SAFERELEASE(m_ShadowTarg);
		for (auto p : m_BBuf)
			C3_SAFERELEASE(p);
		for (auto p : m_BTex)
			C3_SAFERELEASE(p);
		C3_SAFERELEASE(m_SP_resolve);
		C3_SAFERELEASE(m_SP_blur);
		C3_SAFERELEASE(m_SP_combine);

		c3::Renderer *prend = theApp.m_C3->GetRenderer();
		assert(prend);

		prend->Shutdown();
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
		theApp.m_C3->GetLog()->Print(_T("RenderDoc detected; capturing initialization...\n"));
	}

	SetTimer('DRAW', 17, nullptr);
}


void C3EditView::OnDraw(CDC *pDC)
{
	C3EditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (!pDoc->m_RootObj)
		return;

	C3EditFrame *pmf = (C3EditFrame *)theApp.GetMainWnd();

	C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

	c3::Object *camobj = pvi->obj;
	c3::Positionable *pcampos = camobj ? dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type())) : nullptr;
	c3::Camera *pcam = camobj ? dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type())) : nullptr;

	CRect r;
	GetClientRect(r);

	float dt = pDoc->m_Paused ? 0.0f : (pDoc->m_TimeWarp * theApp.m_C3->GetElapsedTime());

	c3::Renderer *prend = theApp.m_C3->GetRenderer();
	assert(prend);

	c3::ResourceManager *presman = theApp.m_C3->GetResourceManager();
	assert(presman);

	if (prend && prend->Initialized())
	{
		prend->SetOverrideHwnd(GetSafeHwnd());

		prend->SetViewport(r);

		prend->SetClearColor(&(pDoc->m_ClearColor));
		prend->SetClearDepth(1.0f);

		theApp.m_C3->UpdateTime();
		float dt = theApp.m_C3->GetElapsedTime();

		C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

//		if (GetFocus() == this)
		{
			glm::vec3 mv(0, 0, 0);

			bool run = theApp.m_C3->GetInputManager()->ButtonPressed(c3::InputDevice::VirtualButton::LSHIFT);

			float spd = theApp.m_Config->GetFloat(_T("environment.movement.speed"), 1.0f);

			float mdf = (theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_W) +
						 theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS1_POSY)) / 2.0f;

			float mdb = (theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_S) +
						 theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS1_NEGY)) / 2.0f;

			mv += *(pcampos->GetFacingVector()) * (mdf - mdb) * spd;

			float mdl = (theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_A) + 
						 theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS1_NEGX)) / 2.0f;

			float mdr = (theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_D) +
						 theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS1_POSX)) / 2.0f;

			mv += *(pcampos->GetLocalLeftVector()) * (mdl - mdr) * spd;

			float mdu = (theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_Q) +
						 theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS1_POSZ)) / 2.0f;
			mv.z += mdu * spd;

			float mdd = (theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::LETTER_Z) + 
						 theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS1_NEGZ)) / 2.0f;
			mv.z -= mdd * spd;

#if 0
			float zoo = pcam->GetPolarDistance();
			zoo += theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::BUTTON5) -
				theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::BUTTON6);
			zoo = std::max(zoo, 0.1f);
			pcam->SetPolarDistance(zoo);
#endif

			pcampos->AdjustPos(mv.x, mv.y, mv.z);

#if 0
			float pau = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS2_NEGY);
			float pad = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS2_POSY);
			float yal = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS2_POSX);
			float yar = theApp.m_C3->GetInputManager()->ButtonPressedProportional(c3::InputDevice::VirtualButton::AXIS2_NEGX);

			pcampos->AdjustPitch((pad - pau) * 0.05f);
			pcampos->AdjustYawFlat((yar - yal) * 0.05f);
#endif
		}

		c3::Object *camobj = pvi->obj;
		if (camobj)
			camobj->Update(dt);

		pDoc->m_RootObj->Update(dt);

		if (pDoc->m_Brush)
			pDoc->m_Brush->Update(dt);

		c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type()));
		c3::Camera *pcam = dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type()));

		if (pcam)
		{
			prend->SetViewMatrix(pcam->GetViewMatrix());
			prend->SetProjectionMatrix(pcam->GetProjectionMatrix());
			prend->SetEyePosition(pcam->GetEyePos());
			glm::fvec3 eyedir = glm::normalize(*pcam->GetTargetPos() - *(pcam->GetEyePos()));
			prend->SetEyeDirection(&eyedir);
		}

		float farclip = camobj->GetProperties()->GetPropertyById('C:FC')->AsFloat();
		float nearclip = camobj->GetProperties()->GetPropertyById('C:NC')->AsFloat();
		glm::fmat4x4 depthProjectionMatrix = glm::ortho<float>(-800, 800, -800, 800, nearclip, farclip);
		glm::fvec3 sunpos = m_SunDir * -700.0f;
		glm::fvec3 campos;
		pcam->GetEyePos(&campos);
		//		sunpos += campos;
		glm::fmat4x4 depthViewMatrix = glm::lookAt(sunpos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::fmat4x4 depthMVP = depthProjectionMatrix * depthViewMatrix;
		prend->SetSunShadowMatrix(&depthMVP);

		glm::mat4 biasmat(
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0
		);

		float sunx = sinf((float)(prend->GetCurrentFrameNumber() / 8) * 3.14159f / 180.0f * 1.0f) * 0.4f;
		float suny = cosf((float)(prend->GetCurrentFrameNumber() / 8) * 3.14159f / 180.0f * 1.0f) * 0.4f;
		m_SunDir = glm::normalize(glm::fvec3(sunx, suny, -1.0f));
		float sunDotUp = glm::dot(m_SunDir, glm::fvec3(0, 0, -1));
		m_SunColor = glm::lerp(c3::Color::DarkYellow, c3::Color::White, sunDotUp * sunDotUp * sunDotUp * sunDotUp * sunDotUp);
		m_AmbientColor = m_SunColor * 0.3f;

		m_SP_combine->SetUniform3(m_ulAmbientColor, &m_AmbientColor);
		m_SP_combine->SetUniform3(m_ulSunColor, &m_SunColor);
		m_SP_combine->SetUniform3(m_ulSunDir, &m_SunDir);

		pDoc->m_RootObj->Update(dt);

		if (prend->BeginScene())
		{
			if (m_pRenderDoc && m_RenderDocCaptureFrame)
			{
				m_pRenderDoc->StartFrameCapture(NULL, NULL);
			}

			// Solid color pass
			prend->SetDepthMode(c3::Renderer::DepthMode::DM_READWRITE);
			prend->UseFrameBuffer(m_GBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH | UFBFLAG_CLEARSTENCIL);
			prend->SetDepthTest(c3::Renderer::Test::DT_LESSER);
			prend->SetAlphaPassRange(3.0f / 255.0f);
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
			prend->SetCullMode(c3::Renderer::CullMode::CM_BACK);
			pDoc->m_RootObj->Render(c3::Object::OBJFLAG(c3::Object::DRAW));

			// Shadow pass
			prend->UseFrameBuffer(m_SSBuf, UFBFLAG_CLEARDEPTH | UFBFLAG_UPDATEVIEWPORT);
			pDoc->m_RootObj->Render(c3::Object::OBJFLAG(c3::Object::CASTSHADOW));

			prend->SetViewport();

			// Lighting pass(es)
			prend->UseFrameBuffer(m_LCBuf, UFBFLAG_FINISHLAST | UFBFLAG_CLEARCOLOR | UFBFLAG_UPDATEVIEWPORT);
			prend->SetDepthMode(c3::Renderer::DepthMode::DM_READONLY);
			prend->SetDepthTest(c3::Renderer::Test::DT_ALWAYS);
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			pDoc->m_RootObj->Render(c3::Object::OBJFLAG(c3::Object::LIGHT));

			// Resolve
			prend->UseFrameBuffer(m_BBuf[0], UFBFLAG_FINISHLAST | UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH);
			prend->SetDepthMode(c3::Renderer::DepthMode::DM_DISABLED);
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			prend->SetCullMode(c3::Renderer::CullMode::CM_DISABLED);
			prend->UseProgram(m_SP_combine);
			m_SP_combine->ApplyUniforms(true);
			prend->UseVertexBuffer(prend->GetFullscreenPlaneVB());
			prend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);

			float bs = 2.0f;
			for (int b = 0; b < BLURTARGS - 1; b++)
			{
				prend->UseFrameBuffer(m_BBuf[b + 1], UFBFLAG_FINISHLAST);
				prend->SetBlendMode(c3::Renderer::BlendMode::BM_REPLACE);
				prend->UseProgram(m_SP_blur);
				m_SP_blur->SetUniformTexture(m_BTex[b], m_uBlurTex);
				m_SP_blur->SetUniform1(m_uBlurScale, bs);
				m_SP_blur->ApplyUniforms(true);
				prend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);
				bs *= 2.0f;
			}

			prend->UseFrameBuffer(nullptr, UFBFLAG_FINISHLAST);
			prend->UseProgram(m_SP_resolve);
			m_SP_resolve->ApplyUniforms(true);
			prend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);
		
			prend->EndScene();
			prend->Present();

			if (m_pRenderDoc && m_RenderDocCaptureFrame)
			{
				m_pRenderDoc->EndFrameCapture(NULL, NULL);
				m_RenderDocCaptureFrame = false;
			}
		}
	}
	else
	{
		theApp.m_C3->GetLog()->Print(_T("Initializing Renderer... "));

		if (prend->Initialize(GetSafeHwnd(), 0))
		{
			if (m_pRenderDoc)
				m_pRenderDoc->StartFrameCapture(NULL, NULL);

			c3::ResourceManager *rm = theApp.m_C3->GetResourceManager();
			props::TFlags64 rf = c3::ResourceManager::RESFLAG(c3::ResourceManager::DEMANDLOAD);

			prend->FlushErrors(_T("%s %d"), __FILEW__, __LINE__);

			size_t w = r.Width();
			size_t h = r.Height();

			m_DepthTarg = prend->CreateDepthBuffer(w, h, c3::Renderer::DepthType::U32_DS);
			m_ShadowTarg = prend->CreateDepthBuffer(2048, 2048, c3::Renderer::DepthType::F32_SHADOW);

			bool gbok = false;

			c3::FrameBuffer::TargetDesc GBufTargData[] =
			{
				{ _T("uSamplerDiffuseMetalness"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET },	// diffuse color (rgb) and metalness (a)
				{ _T("uSamplerNormalAmbOcc"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET },		// fragment normal (rgb) and ambient occlusion (a)
				{ _T("uSamplerPosDepth"), c3::Renderer::TextureType::F32_4CH, TEXCREATEFLAG_RENDERTARGET },			// fragment position in world space (rgb) and dpeth in screen space (a)
				{ _T("uSamplerEmissionRoughness"), c3::Renderer::TextureType::U8_4CH, TEXCREATEFLAG_RENDERTARGET }	// emission color (rgb) and roughness (a)
			};

			theApp.m_C3->GetLog()->Print(_T("Creating G-buffer... "));
			m_GBuf = prend->CreateFrameBuffer();
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


			c3::FrameBuffer::TargetDesc LCBufTargData[] =
			{
				{ _T("uSamplerLights"), c3::Renderer::TextureType::F16_3CH, TEXCREATEFLAG_RENDERTARGET },
			};

			theApp.m_C3->GetLog()->Print(_T("Creating light combine buffer... "));
			m_LCBuf = prend->CreateFrameBuffer();
			gbok = m_LCBuf->Setup(_countof(LCBufTargData), LCBufTargData, m_DepthTarg, r) == c3::FrameBuffer::RETURNCODE::RET_OK;
			theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

			theApp.m_C3->GetLog()->Print(_T("Creating shadow buffer... "));
			m_SSBuf = prend->CreateFrameBuffer();
			m_SSBuf->AttachDepthTarget(m_ShadowTarg);
			gbok = m_SSBuf->Seal() == c3::FrameBuffer::RETURNCODE::RET_OK;
			theApp.m_C3->GetLog()->Print(_T("%s\n"), gbok ? _T("ok") : _T("failed"));

			m_VS_blur = (c3::ShaderComponent *)((rm->GetResource(_T("blur.vsh"), rf))->GetData());
			m_FS_blur = (c3::ShaderComponent *)((rm->GetResource(_T("blur.fsh"), rf))->GetData());
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

			m_VS_resolve = (c3::ShaderComponent *)((rm->GetResource(_T("resolve.vsh"), rf))->GetData());
			m_FS_resolve = (c3::ShaderComponent *)((rm->GetResource(_T("resolve.fsh"), rf))->GetData());
			m_SP_resolve = prend->CreateShaderProgram();
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
			m_SP_combine = prend->CreateShaderProgram();
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
						m_SP_combine->SetUniformTexture((c3::Texture *)m_ShadowTarg, ul, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);

					m_ulSunDir = m_SP_combine->GetUniformLocation(_T("uSunDirection"));
					m_ulSunColor = m_SP_combine->GetUniformLocation(_T("uSunColor"));
					m_ulAmbientColor = m_SP_combine->GetUniformLocation(_T("uAmbientColor"));
				}
			}

			if (m_pRenderDoc)
				m_pRenderDoc->EndFrameCapture(NULL, NULL);
		}
	}
}

void C3EditView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void C3EditView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
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

	CView::OnDestroy();
}


void C3EditView::OnMouseMove(UINT nFlags, CPoint point)
{
	C3EditDoc *pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	int64_t active_tool = theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT);
	int64_t active_axis = theApp.m_Config->GetInt(_T("environment.active.axis"), C3EditApp::AT_X | C3EditApp::AT_Y | C3EditApp::AT_SCREENREL);

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
		//SetAppropriateMouseCursor(nFlags);
	}

	CView::OnMouseMove(nFlags, point);

	int deltax = m_MousePos.x - point.x;
	int deltay = m_MousePos.y - point.y;

	m_MousePos.x = point.x;
	m_MousePos.y = point.y;

	CRect r;
	GetClientRect(&r);
	ClientToScreen(&r);

	float scrpctx = (float)point.x / (float)r.Width();
	float scrpcty = (float)point.y / (float)r.Height();

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

	C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

	c3::Object *camobj = pvi->obj;

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type()));
	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type()));

	c3::ModelRenderer *pbr = pDoc->m_Brush ? dynamic_cast<c3::ModelRenderer *>(pDoc->m_Brush->FindComponent(c3::ModelRenderer::Type())) : nullptr;
	c3::Positionable *pbp = pDoc->m_Brush ? dynamic_cast<c3::Positionable *>(pDoc->m_Brush->FindComponent(c3::Positionable::Type())) : nullptr;

	if ((active_tool == C3EditApp::ToolType::TT_WAND) && pcam && pbr && pbp)
	{
		camobj->Update();

		glm::fvec3 pos3d_near((float)m_MousePos.x, (float)m_MousePos.y, 0.0f);
		glm::fvec3 pos3d_far((float)m_MousePos.x, (float)m_MousePos.y, 1.0f);

		glm::fmat4x4 viewmat, projmat;

		// Get the current projection and view matrices from d3d
		pcam->GetViewMatrix(&viewmat);
		pcam->GetProjectionMatrix(&projmat);

		c3::Renderer *pr = theApp.m_C3->GetRenderer();

		// Construct a viewport that desribes our view metric
		glm::fvec4 viewport(0.0f, 0.0f, (float)(r.right - r.left), (float)(r.bottom - r.top));

		pos3d_near = glm::unProject(pos3d_near, viewmat, projmat, viewport);
		pos3d_far = glm::unProject(pos3d_far, viewmat, projmat, viewport);

		glm::fvec3 rayvec = pos3d_far;
		rayvec -= pos3d_near;

		float shortdist = FLT_MAX;

#if 0
		for (UINT32 i = 0; i < world->GetNumWalkables(); i++)
		{
			TObjectInstance *obj = world->GetWalkable(i);
			if (!obj || !obj->IsFlagSet(OBJFLAG(DRAW)))
				continue;

			float tmp = FLT_MAX;
			obj->CheckCollision(pos3d_near, rayvec, NULL, &tmp, true);
			if (tmp < shortdist)
			{
				shortdist = tmp;
			}
		}
#endif

		rayvec = glm::normalize(rayvec);

		rayvec *= shortdist;
		rayvec += pos3d_near;

		pbp->SetPos(rayvec.x, rayvec.y, rayvec.z);
	}

	//	m_pCam
	//	m_pCamPos

	if ((nFlags & MK_SHIFT) && (deltax || deltay))// && !camori_lock)
	{
		// If the user is holding down the shift key, pan / tilt the camera

		C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

		pvi->pitch -= (float)deltay;
		pvi->yaw += (float)deltax;

		if (theApp.m_Config->GetBool(_T("environment.camera.lockroll"), true))
		{

			c3::Object *camobj = pvi->obj;

			props::IProperty *campitch_min = camobj->GetProperties()->GetPropertyById('PCAN');
			props::IProperty *campitch_max = camobj->GetProperties()->GetPropertyById('PCAX');

			float pitchmin = campitch_min ? campitch_min->AsFloat() : -FLT_MAX;
			float pitchmax = campitch_max ? campitch_max->AsFloat() :  FLT_MAX;

			pvi->pitch = std::min(std::max(pitchmin, pvi->pitch), pitchmax);

			pcampos->SetYawPitchRoll(0, 0, 0);
			pcampos->Update(0);
			pcampos->AdjustYaw(glm::radians(pvi->yaw));
			pcampos->Update(0);
			pcampos->AdjustPitch(glm::radians(pvi->pitch));
		}
		else
		{
			pcampos->AdjustYaw(glm::radians((float)deltax));
			pcampos->Update(0);
			pcampos->AdjustPitch(glm::radians((float)deltay));
		}

		pcam->SetPolarDistance(pcam->GetPolarDistance()); // HACK?!

		RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
	}
	else if (nFlags & MK_MBUTTON)
	{
		// If the user is holding down the middle mouse button, zoom the camera when they move up/down

		float d = pcam->GetPolarDistance();
		d += ((deltay < 0) ? 0.5f : -0.5f);
		d = std::max(d, 0.1f);

		pcam->SetPolarDistance(d);

		RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
	}
	else if (nFlags & MK_LBUTTON)
	{
		float tsens = theApp.m_Config->GetFloat(_T("environment.sensitivity.translation"), 0.1f);
		float rsens = theApp.m_Config->GetFloat(_T("environment.sensitivity.rotation"), 0.1f);
		float ssens = theApp.m_Config->GetFloat(_T("environment.sensitivity.scale"), 0.1f);

#if 0
		for (size_t i = 0; i < pDoc->GetNumSelected(); i++)
		{
			C2BaseObject *obj = pDoc->GetSelection(i);

			switch (active_tool)
			{
				// For movement, use the orientation of the camera!  World coordinates make it too confusing!
			case TT_TRANSLATE:
			{
				if (active_axis & AT_X)
				{
					C2VEC3 fv(1, 0, 0);
					if (active_axis & AT_SCREENREL)
					{
						cam->GetLocalLeftVector(&fv);
					}

					fv *= (float)(-deltax) * tsens;
					obj->AdjustPos(fv.x, fv.y, fv.z);
				}

				if (active_axis & AT_Y)
				{
					C2VEC3 fv(0, 1, 0);
					if (active_axis & AT_SCREENREL)
					{
						cam->GetLocalUpVector(&fv);
					}

					fv *= (float)(-deltay) * tsens;
					obj->AdjustPos(fv.x, fv.y, fv.z);
				}

				if (active_axis & AT_Z)
				{
					C2VEC3 fv(0, 0, 1);
					if (active_axis & AT_SCREENREL)
					{
						cam->GetFacingVector(&fv);
					}

					fv *= (float)(-deltay) * tsens;
					obj->AdjustPos(fv.x, fv.y, fv.z);
				}

				break;
			}

			case TT_ROTATE:
			{
				float pitchval = D3DXToRadian((active_axis & AT_X) ? (deltax * rsens) : 0);
				float rollval = D3DXToRadian((active_axis & AT_Y) ? (deltay * rsens) : 0);
				float yawval = D3DXToRadian((active_axis & AT_Z) ? (deltax * rsens) : 0);

				obj->AdjustPitch(pitchval);
				obj->AdjustRoll(rollval);
				obj->AdjustYaw(yawval);

				break;
			}

			case TT_UNISCALE:
			{
				float sclval = -deltay * ssens;

				obj->AdjustScl(sclval, sclval, sclval);

				break;
			}

			case TT_SCALE:
			{
				float xscl = (active_axis & AT_X) ? (deltax * ssens) : 0.0f;
				float yscl = (active_axis & AT_Y) ? (-deltay * ssens) : 0.0f;
				float zscl = (active_axis & AT_Z) ? (-deltay * ssens) : 0.0f;

				obj->AdjustScl(xscl, yscl, zscl);

				break;
			}
			}
		}
#endif
	}
}


void C3EditView::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
		case 'DRAW':
			RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
			break;

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

	c3::Object *camobj = pvi->obj;

	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type()));
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		float d = pcam->GetPolarDistance();
		d += ((zDelta < 0) ? 0.5f : -0.5f);
		d = std::max(d, 0.1f);

		pcam->SetPolarDistance(d);

		RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
	}

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void C3EditView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	CRect r(0, 0, cx, cy);

	c3::Renderer *pr = theApp.m_C3->GetRenderer();
	if (pr->Initialized())
		pr->SetViewport(&r);
}


void C3EditView::ClearSelection()
{
	m_Selected.clear();
}


void C3EditView::AddToSelection(c3::Object *obj)
{
	if (std::find(m_Selected.cbegin(), m_Selected.cend(), obj) == m_Selected.cend())
		m_Selected.push_back(obj);
}


void C3EditView::RemoveFromSelection(c3::Object *obj)
{
	TObjectArray::iterator it = std::find(m_Selected.begin(), m_Selected.end(), obj);
	if (it != m_Selected.cend())
		m_Selected.erase(it);
}


size_t C3EditView::GetNumSelected()
{
	return m_Selected.size();
}


void C3EditView::OnUpdateEditTriggerrenderdoccapture(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((m_pRenderDoc != nullptr) ? TRUE : FALSE);
}


void C3EditView::OnEditTriggerrenderdoccapture()
{
	m_RenderDocCaptureFrame = true;
}


void C3EditView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnLButtonDown(nFlags, point);
}


void C3EditView::OnLButtonUp(UINT nFlags, CPoint point)
{
	C3EditDoc *pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CRect r;
	GetClientRect(r);

	C3EditDoc::SPerViewInfo *pvi = pDoc->GetPerViewInfo(GetSafeHwnd());

	c3::Object *camobj = pvi->obj;

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type()));
	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type()));

	c3::ModelRenderer *pbr = pDoc->m_Brush ? dynamic_cast<c3::ModelRenderer *>(pDoc->m_Brush->FindComponent(c3::ModelRenderer::Type())) : nullptr;
	c3::Positionable *pbp = pDoc->m_Brush ? dynamic_cast<c3::Positionable *>(pDoc->m_Brush->FindComponent(c3::Positionable::Type())) : nullptr;

	camobj->Update();

	glm::fvec3 pos3d_near((float)m_MousePos.x, (float)m_MousePos.y, 0.0f);
	glm::fvec3 pos3d_far((float)m_MousePos.x, (float)m_MousePos.y, 1.0f);

	glm::fmat4x4 viewmat, projmat;

	// Get the current projection and view matrices from d3d
	pcam->GetViewMatrix(&viewmat);
	pcam->GetProjectionMatrix(&projmat);

	c3::Renderer *pr = theApp.m_C3->GetRenderer();

	// Construct a viewport that desribes our view metric
	glm::fvec4 viewport(0.0f, 0.0f, (float)(r.right - r.left), (float)(r.bottom - r.top));

	pos3d_near = glm::unProject(pos3d_near, viewmat, projmat, viewport);
	pos3d_far = glm::unProject(pos3d_far, viewmat, projmat, viewport);

	glm::fvec3 rayvec = pos3d_far;
	rayvec -= pos3d_near;

	float shortdist = FLT_MAX;

	//pDoc->m_RootObj->CheckCollision(pos3d_near, rayvec, NULL, &tmp, true);

	rayvec = glm::normalize(rayvec);

	rayvec *= shortdist;
	rayvec += pos3d_near;

	CView::OnLButtonUp(nFlags, point);
}


void C3EditView::OnSetFocus(CWnd *pOldWnd)
{
	CView::OnSetFocus(pOldWnd);

	theApp.m_C3->GetInputManager()->AcquireAll();
}
