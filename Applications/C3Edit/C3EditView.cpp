
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
END_MESSAGE_MAP()


c3::FrameBuffer *C3EditView::m_GBuf = nullptr;
c3::FrameBuffer *C3EditView::m_LCBuf = nullptr;
C3EditView::TTextureArray C3EditView::m_ColorTarg;
c3::DepthBuffer *C3EditView::m_DepthTarg = nullptr;

c3::ShaderComponent *C3EditView::m_VS_copyback = nullptr;
c3::ShaderComponent *C3EditView::m_FS_copyback = nullptr;
c3::ShaderProgram *C3EditView::m_SP_copyback = nullptr;

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

		if (m_DepthTarg)
		{
			m_DepthTarg->Release();
			m_DepthTarg = nullptr;
		}

		for (auto ct : m_ColorTarg)
		{
			ct->Release();
		}
		m_ColorTarg.clear();

		if (m_VS_copyback)
		{
			m_VS_copyback->Release();
			m_VS_copyback = nullptr;
		}

		if (m_FS_copyback)
		{
			m_FS_copyback->Release();
			m_FS_copyback = nullptr;
		}

		if (m_SP_copyback)
		{
			m_SP_copyback->Release();
			m_SP_copyback = nullptr;
		}

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

	c3::Renderer *prend = theApp.m_C3->GetRenderer();
	assert(prend);
	c3::ResourceManager *presman = theApp.m_C3->GetResourceManager();
	assert(presman);

	if (!prend->Initialized())
	{
		tstring renderdoc_path = theApp.m_Config->GetString(_T("debug.renderdoc.path"), _T("C:/Program Files/RenderDoc"));
		renderdoc_path += _T("/renderdoc.dll");

		// At init, on windows
		if (HMODULE mod = GetModuleHandle(renderdoc_path.c_str()))
		{
			pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
			int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_2, (void **)&m_pRenderDoc);
			theApp.m_C3->GetLog()->Print(_T("RenderDoc detected; capturing initialization...\n"));
		}

		if (prend->Initialize(GetSafeHwnd(), 0))
		{
			if (m_pRenderDoc)
				m_pRenderDoc->StartFrameCapture(NULL, NULL);

			c3::FrameBuffer::TargetDesc GBufTargData[] =
			{
				{ _T("uSamplerDiffuse"), c3::Renderer::TextureType::F16_4CH, TEXCREATEFLAG_RENDERTARGET },
				{ _T("uSamplerNormal"), c3::Renderer::TextureType::F16_4CH, TEXCREATEFLAG_RENDERTARGET },
				{ _T("uSamplerPosDepth"), c3::Renderer::TextureType::F16_4CH, TEXCREATEFLAG_RENDERTARGET },
				{ _T("uSamplerEmission"), c3::Renderer::TextureType::F16_4CH, TEXCREATEFLAG_RENDERTARGET }
			};

			// It SEEEEMS like in order to get blending to work, the light combine buffer needs to pre-multiply alpha,
			// but then write alpha=1 in the shader... the depth test is still a problem
			c3::FrameBuffer::TargetDesc LCBufTargData[] =
			{
				{ _T("uSamplerLights"), c3::Renderer::TextureType::F16_4CH, TEXCREATEFLAG_RENDERTARGET },
			};

			props::TFlags64 rf = c3::ResourceManager::RESFLAG(c3::ResourceManager::DEMANDLOAD);

			CRect r;
			GetClientRect(r);

			m_DepthTarg = prend->CreateDepthBuffer(r.Width(), r.Height(), c3::Renderer::DepthType::U32_DS);

			bool gbok = false;

			theApp.m_C3->GetLog()->Print(_T("Creating G-buffer... "));
			m_GBuf = prend->CreateFrameBuffer();
			gbok = m_GBuf->Setup(_countof(GBufTargData), GBufTargData, m_DepthTarg, r) == c3::FrameBuffer::RETURNCODE::RET_OK;
			theApp.m_C3->GetLog()->Print(gbok ? _T("ok\n") : _T("\n"));

			theApp.m_C3->GetLog()->Print(_T("Creating light combine buffer... "));
			m_LCBuf = prend->CreateFrameBuffer();
			gbok = m_LCBuf->Setup(_countof(LCBufTargData), LCBufTargData, m_DepthTarg, r) == c3::FrameBuffer::RETURNCODE::RET_OK;
			theApp.m_C3->GetLog()->Print(gbok ? _T("ok\n") : _T("\n"));

			m_SP_copyback = prend->CreateShaderProgram();
			if (m_SP_copyback)
			{
				c3::Resource *tmp;
				tmp = presman->GetResource(_T("resolve.vsh"), rf);
				m_VS_copyback = tmp ? (c3::ShaderComponent *)(tmp->GetData()) : nullptr;
				tmp = presman->GetResource(_T("resolve.fsh"), rf);
				m_FS_copyback = tmp ? (c3::ShaderComponent *)(tmp->GetData()) : nullptr;

				m_SP_copyback->AttachShader(m_VS_copyback);
				m_SP_copyback->AttachShader(m_FS_copyback);
				if (m_SP_copyback->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
				{
				}
			}

			if (m_pRenderDoc)
				m_pRenderDoc->EndFrameCapture(NULL, NULL);
		}
	}

	return 0;
}


void C3EditView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	c3::Renderer *prend = theApp.m_C3->GetRenderer();
	if (!prend || !prend->Initialized())
		return;

	c3::ResourceManager *presman = theApp.m_C3->GetResourceManager();

	C3EditDoc *pDoc = GetDocument();
	if (!pDoc)
		return;

	C3EditFrame *pmf = (C3EditFrame *)theApp.GetMainWnd();

	c3::Object *camobj = pDoc->GetCamera(GetSafeHwnd());

	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type()));
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		pcam->SetFOV(glm::radians(70.0f));
		pcam->SetViewMode(c3::Camera::ViewMode::VM_POLAR);
		pcam->SetPolarDistance(1.0f);
	}

	SetTimer('DRAW', 16, nullptr);
}


void C3EditView::OnDraw(CDC *pDC)
{
	C3EditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	C3EditFrame *pmf = (C3EditFrame *)theApp.GetMainWnd();

	c3::Renderer *prend = theApp.m_C3->GetRenderer();
	if (prend && prend->Initialized())
	{
		if (m_pRenderDoc && m_RenderDocCaptureFrame)
			m_pRenderDoc->StartFrameCapture(NULL, NULL);

		prend->SetOverrideHwnd(GetSafeHwnd());

		CRect r;
		GetClientRect(r);
		prend->SetViewport(r);

		prend->SetClearColor(&(pDoc->m_ClearColor));
		prend->SetClearDepth(1.0f);

		c3::Object *camobj = pDoc->GetCamera(GetSafeHwnd());
		if (camobj)
			camobj->Update();

		pDoc->m_RootObj->Update();

		if (pDoc->m_Brush)
			pDoc->m_Brush->Update();

		c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(camobj->FindComponent(c3::Positionable::Type()));
		c3::Camera *pcam = dynamic_cast<c3::Camera *>(camobj->FindComponent(c3::Camera::Type()));

		theApp.m_C3->UpdateTime();
		theApp.m_C3->SetCurrentFrameNumber(theApp.m_C3->GetCurrentFrameNumber() + 1);

		// BeginScene clears the back buffer when the flags are set with default values
		if (prend->BeginScene())
		{
			if (pcam)
			{
				prend->SetViewMatrix(pcam->GetViewMatrix());
				prend->SetProjectionMatrix(pcam->GetProjectionMatrix());
			}

			// Color pass
			prend->SetDepthMode(c3::Renderer::DepthMode::DM_READWRITE);
			prend->UseFrameBuffer(m_GBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH);
			prend->SetDepthTest(c3::Renderer::Test::DT_LESSER);
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_ALPHA);
			prend->SetCullMode(c3::Renderer::CullMode::CM_BACK);
			if (pDoc->m_RootObj->Prerender(c3::Object::DRAW))
				pDoc->m_RootObj->Render(c3::Object::OBJFLAG(c3::Object::DRAW));

			// Lighting pass(es)
			prend->UseFrameBuffer(m_LCBuf, UFBFLAG_FINISHLAST | UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH);
			prend->SetDepthMode(c3::Renderer::DepthMode::DM_READONLY);
			prend->SetDepthTest(c3::Renderer::Test::DT_LESSEREQUAL);
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			prend->SetCullMode(c3::Renderer::CullMode::CM_BACK);
			m_LCBuf->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			if (pDoc->m_RootObj->Prerender(c3::Object::LIGHT))
				pDoc->m_RootObj->Render(c3::Object::OBJFLAG(c3::Object::LIGHT));

			// Resolve, don't clear -- that was done in BeginScene
			prend->UseFrameBuffer(nullptr, UFBFLAG_FINISHLAST);
			prend->SetDepthMode(c3::Renderer::DepthMode::DM_DISABLED);
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			prend->SetDepthTest(c3::Renderer::Test::DT_ALWAYS);
			prend->UseProgram(m_SP_copyback);
			m_SP_copyback->ApplyUniforms(false);
			prend->UseVertexBuffer(prend->GetFullscreenPlaneVB());
			prend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);

			prend->EndScene();
			prend->Present();
		}

		if (m_pRenderDoc && m_RenderDocCaptureFrame)
		{
			m_pRenderDoc->EndFrameCapture(NULL, NULL);
			m_RenderDocCaptureFrame = false;
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

	int deltax = point.x - m_MousePos.x;
	int deltay = point.y - m_MousePos.y;

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

	c3::Object *camobj = pDoc->GetCamera(GetSafeHwnd());

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

		if (theApp.m_Config->GetBool(_T("environment.camera.lockroll"), true))
		{

			pDoc->m_CamYaw += (float)deltax;
			pDoc->m_CamPitch += (float)-deltay;

			c3::Object *camobj = pDoc->GetCamera(GetSafeHwnd());

			props::IProperty *campitch_min = camobj->GetProperties()->GetPropertyById('PCAN');
			props::IProperty *campitch_max = camobj->GetProperties()->GetPropertyById('PCAX');

			float pitchmin = campitch_min ? campitch_min->AsFloat() : -FLT_MAX;
			float pitchmax = campitch_max ? campitch_max->AsFloat() :  FLT_MAX;

			pDoc->m_CamPitch = std::min(std::max(pitchmin, pDoc->m_CamPitch), pitchmax);

			pcampos->SetYawPitchRoll(0, 0, 0);
			pcampos->Update(0);
			pcampos->AdjustYaw(glm::radians(pDoc->m_CamYaw));
			pcampos->Update(0);
			pcampos->AdjustPitch(glm::radians(pDoc->m_CamPitch));
			pcampos->Update(0);

			RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
		}
		else
		{
			pcampos->AdjustPitch(glm::radians((float)deltay));
			pcampos->AdjustYaw(glm::radians((float)deltax));
		}
	}
	else if (nFlags & MK_MBUTTON)
	{
		// If the user is holding down the middle mouse button, zoom the camera when they move up/down

		pcam->SetPolarDistance(pcam->GetPolarDistance() + (float)deltay);

		RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);

#if 0
		C2Property *dist = cam->Properties().GetPropertyById('DIST');
		if (dist)
		{
			dist->SetFloat(dist->AsFloat() + ((float)deltay / 2.0f));
		}
#endif
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
		theApp.m_C3->UpdateTime();

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

	c3::Object *camobj = pDoc->GetCamera(GetSafeHwnd());

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
	CView::OnSize(nType, cx, cy);

	c3::Renderer *pr = theApp.m_C3->GetRenderer();
	if (pr->Initialized())
		pr->SetViewport();
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
