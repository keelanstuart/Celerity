// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface
// (the "Fluent UI") and is provided only as referential material to supplement the
// Microsoft Foundation Classes Reference and related electronic documentation
// included with the MFC C++ library software.
// License terms to copy, use or distribute the Fluent UI are available separately.
// To learn more about our Fluent UI licensing program, please visit
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// Celedit3View.cpp : implementation of the CCeledit3View class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Celedit3.h"
#endif

#include "Celedit3Doc.h"
#include "Celedit3View.h"
#include "MainFrm.h"

#include <C3ModelRenderer.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCeledit3View

IMPLEMENT_DYNCREATE(CCeledit3View, CView)

BEGIN_MESSAGE_MAP(CCeledit3View, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CCeledit3View construction/destruction

CCeledit3View::CCeledit3View() noexcept
{
	m_RDCaptureFrame = true;
	m_ClearColor = glm::fvec4(0, 0, 0, 1);
}

CCeledit3View::~CCeledit3View()
{
}

BOOL CCeledit3View::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// CCeledit3View drawing

void CCeledit3View::OnDraw(CDC *pDC)
{
	CCeledit3Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CMainFrame *pmf = (CMainFrame *)theApp.GetMainWnd();

	c3::Renderer *prend = theApp.m_C3->GetRenderer();
	if (prend && prend->Initialized())
	{
		if (pmf->m_pRDoc && m_RDCaptureFrame)
			pmf->m_pRDoc->StartFrameCapture(NULL, NULL);

		prend->SetOverrideHwnd(GetSafeHwnd());

		prend->SetClearColor(&m_ClearColor);
		prend->SetClearDepth(1.0f);

		for (size_t i = 0; i < CCeledit3Doc::CAMTYPE::CAM_NUMCAMS; i++)
			if (pDoc->m_Camera[i])
				pDoc->m_Camera[i]->Update();

		pDoc->m_RootObj->Update();

		if (pDoc->m_Brush)
			pDoc->m_Brush->Update();

		CCeledit3Doc::CAMTYPE t = CCeledit3Doc::CAMTYPE::CAM_FREE;

		c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(pDoc->m_Camera[t]->FindComponent(c3::Positionable::Type()));
		c3::Camera *pcam = dynamic_cast<c3::Camera *>(pDoc->m_Camera[t]->FindComponent(c3::Camera::Type()));

		theApp.m_C3->UpdateTime();
		theApp.m_C3->SetCurrentFrameNumber(theApp.m_C3->GetCurrentFrameNumber() + 1);

		if (prend->BeginScene())
		{
			if (pcam)
			{
				prend->SetViewMatrix(pcam->GetViewMatrix());
				prend->SetProjectionMatrix(pcam->GetProjectionMatrix());
			}

			// Color pass
			prend->SetDepthMode(c3::Renderer::DEPTHMODE::DM_READWRITE);
			prend->UseFrameBuffer(pmf->m_GBuf, UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH);
			prend->SetDepthTest(c3::Renderer::Test::DT_LESSER);
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_ALPHA);
			prend->SetCullMode(c3::Renderer::CullMode::CM_BACK);
			if (pDoc->m_RootObj->Prerender(c3::Object::DRAW))
				pDoc->m_RootObj->Render(c3::Object::OBJFLAG(c3::Object::DRAW));

			// Lighting pass(es)
			prend->UseFrameBuffer(pmf->m_LCBuf, UFBFLAG_FINISHLAST | UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH);
			prend->SetDepthMode(c3::Renderer::DEPTHMODE::DM_READONLY);
			prend->SetDepthTest(c3::Renderer::Test::DT_LESSEREQUAL);
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			prend->SetCullMode(c3::Renderer::CullMode::CM_BACK);
			pmf->m_LCBuf->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			if (pDoc->m_RootObj->Prerender(c3::Object::LIGHT))
				pDoc->m_RootObj->Render(c3::Object::OBJFLAG(c3::Object::LIGHT));

			// Resolve
			prend->UseFrameBuffer(nullptr, UFBFLAG_FINISHLAST | UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH);
			prend->SetDepthMode(c3::Renderer::DEPTHMODE::DM_DISABLED);
			prend->SetBlendMode(c3::Renderer::BlendMode::BM_ADD);
			prend->SetDepthTest(c3::Renderer::Test::DT_ALWAYS);
			prend->UseProgram(pmf->m_SP_copyback);
			pmf->m_SP_copyback->ApplyUniforms(false);
			prend->UseVertexBuffer(prend->GetFullscreenPlaneVB());
			prend->DrawPrimitives(c3::Renderer::PrimType::TRISTRIP, 4);

			prend->EndScene();
			prend->Present();
		}

		if (pmf->m_pRDoc && m_RDCaptureFrame)
		{
			pmf->m_pRDoc->EndFrameCapture(NULL, NULL);
			m_RDCaptureFrame = false;
		}
	}
}

void CCeledit3View::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CCeledit3View::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CCeledit3View diagnostics

#ifdef _DEBUG
void CCeledit3View::AssertValid() const
{
	CView::AssertValid();
}

void CCeledit3View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCeledit3Doc* CCeledit3View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCeledit3Doc)));
	return (CCeledit3Doc*)m_pDocument;
}
#endif //_DEBUG


// CCeledit3View message handlers


void CCeledit3View::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	c3::Renderer *prend = theApp.m_C3->GetRenderer();
	if (!prend || !prend->Initialized())
		return;

	c3::ResourceManager *presman = theApp.m_C3->GetResourceManager();

	CCeledit3Doc *pdoc = GetDocument();
	if (!pdoc)
		return;

	CMainFrame *pmf = (CMainFrame *)theApp.GetMainWnd();

	CCeledit3Doc::CAMTYPE t = CCeledit3Doc::CAMTYPE::CAM_FREE;

	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(pdoc->m_Camera[t]->FindComponent(c3::Positionable::Type()));
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(pdoc->m_Camera[t]->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		pcam->SetFOV(glm::radians(70.0f));
		pcam->SetViewMode(c3::Camera::ViewMode::VM_POLAR);
		pcam->SetPolarDistance(1.0f);
	}

	SetTimer('DRAW', 16, nullptr);
}


void CCeledit3View::OnDestroy()
{
	KillTimer('DRAW');

	CView::OnDestroy();
}


void CCeledit3View::OnMouseMove(UINT nFlags, CPoint point)
{
	CCeledit3Doc *pdoc = GetDocument();
	ASSERT_VALID(pdoc);

	int64_t active_tool = theApp.m_Config->GetInt(_T("environment.active.tool"), CCeledit3App::TT_SELECT);
	int64_t active_axis = theApp.m_Config->GetInt(_T("environment.active.axis"), CCeledit3App::AT_X | CCeledit3App::AT_Y | CCeledit3App::AT_SCREENREL);

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

	CCeledit3Doc::CAMTYPE t = CCeledit3Doc::CAMTYPE::CAM_FREE;

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(pdoc->m_Camera[t]->FindComponent(c3::Camera::Type()));
	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(pdoc->m_Camera[t]->FindComponent(c3::Positionable::Type()));

	c3::ModelRenderer *pbr = pdoc->m_Brush ? dynamic_cast<c3::ModelRenderer *>(pdoc->m_Brush->FindComponent(c3::ModelRenderer::Type())) : nullptr;
	c3::Positionable *pbp = pdoc->m_Brush ? dynamic_cast<c3::Positionable *>(pdoc->m_Brush->FindComponent(c3::Positionable::Type())) : nullptr;

	if ((active_tool == CCeledit3App::ToolType::TT_WAND) && pcam && pbr && pbp)
	{
		pdoc->m_Camera[t]->Update();

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

			m_CamYaw += (float)deltax;
			m_CamPitch += (float)-deltay;

			props::IProperty *campitch_min = pdoc->m_Camera[t]->GetProperties()->GetPropertyById('PCAN');
			props::IProperty *campitch_max = pdoc->m_Camera[t]->GetProperties()->GetPropertyById('PCAX');

			float pitchmin = campitch_min ? campitch_min->AsFloat() : -FLT_MAX;
			float pitchmax = campitch_max ? campitch_max->AsFloat() :  FLT_MAX;

			m_CamPitch = std::min(std::max(pitchmin, m_CamPitch), pitchmax);

			pcampos->SetYawPitchRoll(0, 0, 0);
			pcampos->Update(0);
			pcampos->AdjustYaw(glm::radians(m_CamYaw));
			pcampos->Update(0);
			pcampos->AdjustPitch(glm::radians(m_CamPitch));
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
		for (size_t i = 0; i < pdoc->GetNumSelected(); i++)
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


void CCeledit3View::OnTimer(UINT_PTR nIDEvent)
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


BOOL CCeledit3View::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CCeledit3Doc *pdoc = GetDocument();
	ASSERT_VALID(pdoc);

	CCeledit3Doc::CAMTYPE t = CCeledit3Doc::CAMTYPE::CAM_FREE;

	c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(pdoc->m_Camera[t]->FindComponent(c3::Positionable::Type()));
	c3::Camera *pcam = dynamic_cast<c3::Camera *>(pdoc->m_Camera[t]->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		float d = pcam->GetPolarDistance();
		d += ((zDelta < 0) ? 0.5f : -0.5f);
		d = std::max(d, 0.1f);

		pcam->SetPolarDistance(d);
	}

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CCeledit3View::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	c3::Renderer *pr = theApp.m_C3->GetRenderer();
	if (pr->Initialized())
		pr->SetViewport();
}


void CCeledit3View::ClearSelection()
{
	m_Selected.clear();
}


void CCeledit3View::AddToSelection(const c3::Object *obj)
{
	if (std::find(m_Selected.cbegin(), m_Selected.cend(), obj) == m_Selected.cend())
		m_Selected.push_back(obj);
}


void CCeledit3View::RemoveFromSelection(const c3::Object *obj)
{
	TObjectArray::iterator it = std::find(m_Selected.begin(), m_Selected.end(), obj);
	if (it != m_Selected.cend())
		m_Selected.erase(it);
}


size_t CCeledit3View::GetNumSelected()
{
	return m_Selected.size();
}

