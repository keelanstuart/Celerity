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
	m_Rend = nullptr;

	m_ClearColor = glm::fvec4(0, 0, 0, 1);

	m_pCam = nullptr;
	m_pCamPos = nullptr;
	m_CamPitch = 0.0f;
	m_CamYaw = 0.0f;

	m_FB = nullptr;
	ZeroMemory(m_ColorTarg, 3 * sizeof(c3::Texture2D *));
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

	if (m_Rend && m_Rend->Initialized())
	{
		m_Rend->SetClearColor(&m_ClearColor);
		m_Rend->SetClearDepth(1.0f);

		pDoc->m_Observer->Update();
		pDoc->m_RootObj->Update();

		c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(pDoc->m_Observer->FindFeature(c3::Positionable::Type()));
		c3::Camera *pcam = dynamic_cast<c3::Camera *>(pDoc->m_Observer->FindFeature(c3::Camera::Type()));
		if (pcam)
		{
			m_Rend->SetViewMatrix(pcam->GetViewMatrix());
			m_Rend->SetProjectionMatrix(pcam->GetProjectionMatrix());
		}

		theApp.m_C3->UpdateTime();

		m_Rend->UseFrameBuffer(m_FB);

		if (m_Rend->BeginScene())
		{
			if (pDoc->m_RootObj->Prerender())
				pDoc->m_RootObj->Render();

			m_Rend->UseFrameBuffer(nullptr);
			m_Rend->UseProgram(m_SP_copyback);

			int32_t ul;
			if (c3::ShaderProgram::INVALID_UNIFORM != (ul = m_SP_copyback->GetUniformLocation(_T("uSamplerDiffuse"))))
				m_SP_copyback->SetUniformTexture(ul, 0, m_ColorTarg[0]);
			if (c3::ShaderProgram::INVALID_UNIFORM != (ul = m_SP_copyback->GetUniformLocation(_T("uSamplerNormal"))))
				m_SP_copyback->SetUniformTexture(ul, 1, m_ColorTarg[1]);
			if (c3::ShaderProgram::INVALID_UNIFORM != (ul = m_SP_copyback->GetUniformLocation(_T("uSamplerPosDepth"))))
				m_SP_copyback->SetUniformTexture(ul, 2, m_ColorTarg[2]);

			m_SP_copyback->ApplyUniforms(false);

			m_Rend->UseVertexBuffer(m_Rend->GetFullscreenPlaneVB());
			m_Rend->SetCullMode(c3::Renderer::ECullMode::CM_DISABLED);
			m_Rend->DrawPrimitives(c3::Renderer::EPrimType::TRISTRIP, 4);

			m_Rend->EndScene();
			m_Rend->Present();

			theApp.m_C3->SetCurrentFrameNumber(theApp.m_C3->GetCurrentFrameNumber() + 1);
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

	m_Rend = theApp.m_C3->GetRenderer();
	if (!m_Rend)
		return;

	c3::ResourceManager *rm = theApp.m_C3->GetResourceManager();

	if (!m_Rend->Initialized())
	{
		CRect r;
		GetClientRect(r);

		if (m_Rend->Initialize(GetSafeHwnd(), 0))
		{
			const size_t w = 2048;
			const size_t h = 2048;

			m_FB = m_Rend->CreateFrameBuffer(0);

			m_ColorTarg[0] = m_Rend->CreateTexture2D(w, h, c3::Renderer::TextureType::U8_4CH, 1, TEXCREATEFLAG_RENDERTARGET);
			m_ColorTarg[1] = m_Rend->CreateTexture2D(w, h, c3::Renderer::TextureType::F16_4CH, 1, TEXCREATEFLAG_RENDERTARGET);
			m_ColorTarg[2] = m_Rend->CreateTexture2D(w, h, c3::Renderer::TextureType::F32_4CH, 1, TEXCREATEFLAG_RENDERTARGET);
			m_DepthTarg = m_Rend->CreateDepthBuffer(w, h, c3::Renderer::DepthType::U32_DS);

			m_FB->AttachDepthTarget(m_DepthTarg);
			for (size_t i = 0; i < 3; i++)
				m_FB->AttachColorTarget(m_ColorTarg[i], i);

			m_FB->Seal();

			props::TFlags64 rf = c3::ResourceManager::RESFLAG(c3::ResourceManager::DEMANDLOAD);
			m_VS_copyback = (c3::ShaderComponent *)((rm->GetResource(_T("copyback.vsh"), rf))->GetData());
			m_FS_copyback = (c3::ShaderComponent *)((rm->GetResource(_T("copyback.fsh"), rf))->GetData());
			m_SP_copyback = m_Rend->CreateShaderProgram();
		}
	}

	CCeledit3Doc *pdoc = GetDocument();
	if (!pdoc)
		return;

	m_pCamPos = dynamic_cast<c3::Positionable *>(pdoc->m_Observer->FindFeature(c3::Positionable::Type()));
	m_pCamPos->SetPos(-3.0f, 0, -10.0f);
	m_pCam = dynamic_cast<c3::Camera *>(pdoc->m_Observer->FindFeature(c3::Camera::Type()));
	m_pCam->SetFOV(glm::radians(70.0f));
	m_pCam->SetViewMode(c3::Camera::ViewMode::VM_POLAR);
	m_pCam->SetPolarDistance(1.0f);

	SetTimer('DRAW', 16, nullptr);
}


void CCeledit3View::OnDestroy()
{
	KillTimer('DRAW');

	for (size_t i = 0; i < 3; i++)
	{
		if (m_ColorTarg[i])
		{
			m_ColorTarg[i]->Release();
			m_ColorTarg[i] = nullptr;
		}
	}

	if (m_DepthTarg)
	{
		m_DepthTarg->Release();
		m_DepthTarg = nullptr;
	}

	if (m_FB)
	{
		m_FB->Release();
		m_FB = nullptr;
	}

	if (m_SP_copyback)
	{
		m_SP_copyback->Release();
		m_SP_copyback = nullptr;
	}

	if (m_Rend && m_Rend->Initialized())
	{
		m_Rend->Shutdown();
		m_Rend = nullptr;
	}

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

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(pdoc->m_Observer->FindFeature(c3::Camera::Type()));
	c3::ModelRenderer *pbr = pdoc->m_Brush ? dynamic_cast<c3::ModelRenderer *>(pdoc->m_Brush->FindFeature(c3::ModelRenderer::Type())) : nullptr;
	c3::Positionable *pbp = pdoc->m_Brush ? dynamic_cast<c3::Positionable *>(pdoc->m_Brush->FindFeature(c3::Positionable::Type())) : nullptr;

	if ((active_tool == CCeledit3App::ToolType::TT_WAND) && pcam && pbr && pbp)
	{
		pdoc->m_Observer->Update();

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

			props::IProperty *campitch_min = pdoc->m_Observer->GetProperties()->GetPropertyById('PCAN');
			props::IProperty *campitch_max = pdoc->m_Observer->GetProperties()->GetPropertyById('PCAX');

			float pitchmin = campitch_min ? campitch_min->AsFloat() : -FLT_MAX;
			float pitchmax = campitch_max ? campitch_max->AsFloat() :  FLT_MAX;

			m_CamPitch = std::min(std::max(pitchmin, m_CamPitch), pitchmax);

			m_pCamPos->SetYawPitchRoll(0, 0, 0);
			m_pCamPos->Update(0);
			m_pCamPos->AdjustYaw(glm::radians(m_CamYaw));
			m_pCamPos->Update(0);
			m_pCamPos->AdjustPitch(glm::radians(m_CamPitch));
			m_pCamPos->Update(0);

			RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
		}
		else
		{
			m_pCamPos->AdjustPitch(glm::radians((float)deltay));
			m_pCamPos->AdjustYaw(glm::radians((float)deltax));
		}
	}
	else if (nFlags & MK_MBUTTON)
	{
		// If the user is holding down the middle mouse button, zoom the camera when they move up/down

		m_pCam->SetPolarDistance(m_pCam->GetPolarDistance() + (float)deltay);

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

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(pdoc->m_Observer->FindFeature(c3::Camera::Type()));
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

