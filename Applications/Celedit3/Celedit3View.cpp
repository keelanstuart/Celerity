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

#include <C3Renderable.h>

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

	m_pProjectorCam = nullptr;
	m_pProjectorCamPos = nullptr;

	m_FB = nullptr;
	m_DB = nullptr;
	m_DiffuseTarg = nullptr;
	m_NormSpecTarg = nullptr;
	m_PosDepthTarg = nullptr;

	memset(m_SP, 0, sizeof(c3::ShaderProgram *) * NUMSHADERS);
	memset(m_VS, 0, sizeof(c3::ShaderComponent *) * NUMSHADERS);
	memset(m_FS, 0, sizeof(c3::ShaderComponent *) * NUMSHADERS);
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

		pDoc->m_Projector->Update();
		pDoc->m_Observer->Update();
		pDoc->m_RootObj->Update();
		c3::Positionable *pos = (c3::Positionable *)(pDoc->m_RootObj->FindFeature(c3::Positionable::Type()));
#if 1
		if (pos)
		{
			pos->SetPosX(sinf(float(theApp.m_C3->GetCurrentFrameNumber()) / 10.0f) * 10.0f);
		}
#endif

		if (m_Rend->BeginScene())
		{
			m_Rend->UseFrameBuffer(m_FB, UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARCOLOR);

			// render the actual content here
			if (m_pProjectorCam)
			{
				m_Rend->SetProjectionMatrix(m_pProjectorCam->GetProjectionMatrix());
				m_Rend->SetViewMatrix(m_pProjectorCam->GetViewMatrix());
			}

			m_Rend->UseProgram(m_SP[SCENE]);
			if (pDoc->m_RootObj->Prerender())
				pDoc->m_RootObj->Render();

			// this is the observer... render the dome with the projector's texture on it
			m_Rend->UseFrameBuffer(nullptr, UFBFLAG_FINISHLAST);
			if (m_pCam)
			{
				glm::fmat4x4 ident = glm::identity<glm::fmat4x4>();
				m_Rend->SetProjectionMatrix(m_pCam->GetProjectionMatrix());
				m_Rend->SetViewMatrix(m_pCam->GetViewMatrix());
				m_Rend->SetWorldMatrix(&ident);

				m_SP[LIGHT]->SetUniformTexture(m_UP_TEX0[LIGHT], 0, m_DiffuseTarg);
				m_SP[LIGHT]->SetUniformTexture(m_UP_TEX1[LIGHT], 1, m_NormSpecTarg);
				m_SP[LIGHT]->SetUniformTexture(m_UP_TEX2[LIGHT], 2, m_PosDepthTarg);

				m_Rend->GetHemisphereMesh()->Draw();

				m_Rend->UseTexture(0, nullptr);
				m_Rend->UseTexture(1, nullptr);
				m_Rend->UseTexture(2, nullptr);
			}

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

	if (!m_Rend->Initialized())
	{
		CRect r;
		GetClientRect(r);

		if (m_Rend->Initialize(GetSafeHwnd(), 0))
		{
			const size_t tw = 2048;
			const size_t th = 2048;

			m_FB = m_Rend->CreateFrameBuffer(0);

			m_DB = m_Rend->CreateDepthBuffer(tw, th, c3::Renderer::DepthType::U32_D, TEXCREATEFLAG_RENDERTARGET);
			m_DiffuseTarg = m_Rend->CreateTexture2D(tw, th, c3::Renderer::TextureType::U8_4CH, 1, TEXCREATEFLAG_RENDERTARGET);
			m_NormSpecTarg = m_Rend->CreateTexture2D(tw, th, c3::Renderer::TextureType::U8_4CH, 1, TEXCREATEFLAG_RENDERTARGET);
			m_PosDepthTarg = m_Rend->CreateTexture2D(tw, th, c3::Renderer::TextureType::U8_4CH, 1, TEXCREATEFLAG_RENDERTARGET);

			m_FB->AttachDepthTarget(m_DB);
			m_FB->AttachColorTarget(m_DiffuseTarg, 0);
			m_FB->AttachColorTarget(m_NormSpecTarg, 1);
			m_FB->AttachColorTarget(m_PosDepthTarg, 2);
			m_FB->Seal();

			m_SP[SCENE] = m_Rend->CreateShaderProgram();

			static const TCHAR *pvst_scene = _T("\
				#version 410\n\
				uniform mat4 matWorldViewProj; \n\
				uniform mat4 matWorld; \n\
				layout (location=0) in vec3 vPos; \n\
				layout (location=1) in vec3 vNorm; \n\
				layout (location=2) in vec2 vTex0; \n\
				layout (location=3) in vec3 vTang; \n\
				out VS_OUT \n\
				{ \n\
					vec3 Pos; \n\
					vec4 Color; \n\
					vec2 Tex0; \n\
					mat3 Trans; \n\
				} o; \n\
				void main()\n\
				{\n\
					vec4 p = matWorldViewProj * vec4(vPos, 1.0); \n\
					gl_Position = p; \n\
					vec3 T = normalize(vec3(matWorld * vec4(vTang, 0.0))); \n\
					vec3 N = normalize(vec3(matWorld * vec4(vNorm, 0.0))); \n\
					vec3 B = normalize(cross(N, T)); \n\
					o.Pos = p.xyz; \n\
					o.Color = vec4(1, 1, 1, 1); \n\
					o.Tex0 = vTex0; \n\
					o.Trans = transpose(mat3(T, B, N)); \n\
				}\n\
			");

			m_VS[SCENE] = m_Rend->CreateShaderComponent(c3::Renderer::ShaderComponentType::ST_VERTEX);
			if (m_VS[SCENE])
				m_VS[SCENE]->CompileProgram(pvst_scene);

			static const TCHAR *pfst_scene = _T("\
				#version 410 \n\
				in VS_OUT \n\
				{ \n\
					vec3 Pos; \n\
					vec4 Color; \n\
					vec2 Tex0; \n\
					mat3 Trans; \n\
				} i; \n\
				uniform sampler2D texDiffuse; \n\
				uniform sampler2D texNormal; \n\
				uniform sampler2D texMaterial; \n\
				layout (location=0) out vec4 diffuse; \n\
				layout (location=1) out vec4 normspec; \n\
				layout (location=2) out vec4 posdepth; \n\
				void main() \n\
				{ \n\
					vec4 d = texture(texDiffuse, i.Tex0) * i.Color; \n\
					if (d.a < 0.01) discard; \n\
					diffuse = d; \n\
					vec3 n = texture(texNormal, i.Tex0).rgb; \n\
					n = normalize(n * 2.0 - 1.0); \n\
					n = normalize(i.Trans * n); \n\
					normspec = vec4(n, 0.2); \n\
					posdepth = vec4(i.Pos, gl_FragCoord.z); \n\
				} \n\
			");

			m_FS[SCENE] = m_Rend->CreateShaderComponent(c3::Renderer::ShaderComponentType::ST_FRAGMENT);
			if (m_FS[SCENE])
				m_FS[SCENE]->CompileProgram(pfst_scene);

			m_SP[SCENE]->AttachShader(m_VS[SCENE]);
			m_SP[SCENE]->AttachShader(m_FS[SCENE]);
			if (m_SP[SCENE]->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
			{
				m_UP_MVP[SCENE] = m_SP[SCENE]->GetUniformLocation(_T("matWorldViewProj"));
				m_UP_M[SCENE] = m_SP[SCENE]->GetUniformLocation(_T("matWorld"));
				m_UP_TEX0[SCENE] = m_SP[SCENE]->GetUniformLocation(_T("texDiffuse"));
				m_UP_TEX1[SCENE] = m_SP[SCENE]->GetUniformLocation(_T("texNormal"));
				m_UP_TEX2[SCENE] = m_SP[SCENE]->GetUniformLocation(_T("texMaterial"));
			}



			m_SP[LIGHT] = m_Rend->CreateShaderProgram();

			static const TCHAR *pvst_light = _T("\
				#version 410\n\
				uniform mat4 matWorldViewProj; \n\
				layout (location=0) in vec3 vPos; \n\
				layout (location=1) in vec3 vNorm; \n\
				out VS_OUT \n\
				{ \n\
					vec3 Pos; \n\
					vec4 Color; \n\
				} o; \n\
				void main()\n\
				{\n\
					vec4 p = matWorldViewProj * vec4(vPos, 1.0); \n\
					gl_Position = p; \n\
					o.Pos = p.xyz; \n\
					o.Color = vec4(1, 1, 1, 1); \n\
				}\n\
			");

			m_VS[LIGHT] = m_Rend->CreateShaderComponent(c3::Renderer::ShaderComponentType::ST_VERTEX);
			if (m_VS[LIGHT])
				m_VS[LIGHT]->CompileProgram(pvst_light);

			static const TCHAR *pfst_light = _T("\
				#version 410 \n\
				in VS_OUT \n\
				{ \n\
					vec3 Pos; \n\
					vec4 Color; \n\
				} i; \n\
				uniform sampler2D texDiffuse; \n\
				uniform sampler2D texNormSpec; \n\
				uniform sampler2D texPosDepth; \n\
				layout (location=0) out vec4 frag; \n\
				void main() \n\
				{ \n\
					vec4 d = texture(texDiffuse, gl_FragCoord.xy) * i.Color; \n\
					frag = d; \n\
				} \n\
			");

			m_FS[LIGHT] = m_Rend->CreateShaderComponent(c3::Renderer::ShaderComponentType::ST_FRAGMENT);
			if (m_FS[LIGHT])
				m_FS[LIGHT]->CompileProgram(pfst_scene);

			m_SP[LIGHT]->AttachShader(m_VS[LIGHT]);
			m_SP[LIGHT]->AttachShader(m_FS[LIGHT]);
			if (m_SP[LIGHT]->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
			{
				m_UP_MVP[LIGHT] = m_SP[LIGHT]->GetUniformLocation(_T("matWorldViewProj"));
				m_UP_TEX0[LIGHT] = m_SP[LIGHT]->GetUniformLocation(_T("texDiffuse"));
				m_UP_TEX1[LIGHT] = m_SP[LIGHT]->GetUniformLocation(_T("texNormSpec"));
				m_UP_TEX2[LIGHT] = m_SP[LIGHT]->GetUniformLocation(_T("texPosDepth"));
			}

		}
	}

	CCeledit3Doc *pdoc = GetDocument();
	if (!pdoc)
		return;

	m_pCamPos = dynamic_cast<c3::Positionable *>(pdoc->m_Observer->FindFeature(c3::Positionable::Type()));
	m_pCamPos->SetPos(-3.0f, 0, -10.0f);
	m_pCam = dynamic_cast<c3::Camera *>(pdoc->m_Observer->FindFeature(c3::Camera::Type()));
	m_pCam->SetFOV(glm::radians(60.0f));
	m_pCam->SetViewMode(c3::Camera::ViewMode::VM_POLAR);
	m_pCam->SetPolarDistance(1.0f);

	m_pProjectorCamPos = dynamic_cast<c3::Positionable *>(pdoc->m_Projector->FindFeature(c3::Positionable::Type()));
	m_pProjectorCamPos->AdjustPitch(glm::radians(-90.0f));
	m_pProjectorCam = dynamic_cast<c3::Camera *>(pdoc->m_Projector->FindFeature(c3::Camera::Type()));
	m_pProjectorCam->SetFOV(glm::radians(170.0f));
	m_pProjectorCam->SetViewMode(c3::Camera::ViewMode::VM_LOOKAT);
	m_pProjectorCam->SetPolarDistance(1.0f);

	SetTimer('DRAW', 33, nullptr);
}


void CCeledit3View::OnDestroy()
{
	KillTimer('DRAW');

	if (m_Rend && m_Rend->Initialized())
	{
		m_Rend->Shutdown();
		m_Rend = nullptr;
	}

#if 0
	if (m_SP)
	{
		m_SP->Release();
		m_SP = nullptr;
	}

	if (m_VS)
	{
		m_VS->Release();
		m_VS = nullptr;
	}

	if (m_FS)
	{
		m_FS->Release();
		m_FS = nullptr;
	}
#endif

	if (m_FB)
	{
		m_FB->Release();
		m_FB = nullptr;
	}

	if (m_DB)
	{
		m_DB->Release();
		m_DB = nullptr;
	}

	if (m_DiffuseTarg)
	{
		m_DiffuseTarg->Release();
		m_DiffuseTarg = nullptr;
	}

	if (m_NormSpecTarg)
	{
		m_NormSpecTarg->Release();
		m_NormSpecTarg = nullptr;
	}

	if (m_PosDepthTarg)
	{
		m_PosDepthTarg->Release();
		m_PosDepthTarg = nullptr;
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
	c3::Renderable *pbr = pdoc->m_Brush ? dynamic_cast<c3::Renderable *>(pdoc->m_Brush->FindFeature(c3::Renderable::Type())) : nullptr;
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

