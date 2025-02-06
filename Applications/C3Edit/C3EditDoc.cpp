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

#include "C3EditDoc.h"
#include "C3EditView.h"
#include "C3EditFrame.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// C3EditDoc

IMPLEMENT_DYNCREATE(C3EditDoc, CDocument)

BEGIN_MESSAGE_MAP(C3EditDoc, CDocument)
END_MESSAGE_MAP()


// C3EditDoc construction/destruction

C3EditDoc::C3EditDoc() noexcept
{
	m_RootObj = nullptr;
	m_OperationalRootObj = nullptr;
	m_Brush = nullptr;
	m_TimeWarp = 1.0f;
	m_Paused = false;

	m_ClearColor = glm::fvec4(0, 0, 0, 1);
	m_FogColor = glm::fvec4(0, 0, 0, 0);
	m_FogDensity = 0.0f;
	m_ShadowColor = glm::fvec4(0, 0, 0, 0.1f);
}

C3EditDoc::~C3EditDoc()
{
	for (auto it : m_PerViewInfo)
	{
		it.second.m_Camera->Release();
		it.second.m_GUICamera->Release();
	}

	C3_SAFERELEASE(m_Brush);
	C3_SAFERELEASE(m_RootObj);
	m_OperationalRootObj = nullptr;
}

BOOL C3EditDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	c3::Factory *pf = theApp.m_C3->GetFactory();
	if (!pf)
		return FALSE;

	m_RootObj = pf->Build();
	m_RootObj->AddComponent(c3::Positionable::Type());
	m_RootObj->AddComponent(c3::Scriptable::Type());
	m_RootObj->Flags().Set(OF_LIGHT | OF_CASTSHADOW | OF_EXPANDED);

	m_OperationalRootObj = m_RootObj;

	ResetViews();

	return TRUE;
}




// C3EditDoc serialization

void C3EditDoc::Serialize(CArchive& ar)
{
}

C3EditDoc::SPerViewInfo *C3EditDoc::GetPerViewInfo(HWND h)
{
	CRect r;
	GetClientRect(h, r);

	TWndMappedObject::iterator it = m_PerViewInfo.find(h);
	if (it != m_PerViewInfo.end())
		return &it->second;

	if (!m_PerViewInfo.empty())
	{
		it = m_PerViewInfo.find(0);
		if (it != m_PerViewInfo.end())
		{
			m_PerViewInfo.insert(TWndMappedObject::value_type(h, it->second));
			m_PerViewInfo.erase(0);
		}
	}

	c3::Factory *pf = theApp.m_C3->GetFactory();
	assert(pf);

	SPerViewInfo pvi;
	pvi.pitch = pvi.yaw = 0;

	pvi.m_Camera = pf->Build();
	pvi.m_Camera->AddComponent(c3::Positionable::Type());
	pvi.m_Camera->AddComponent(c3::Camera::Type());
	pvi.m_Camera->SetName(_T("Camera"));

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(pvi.m_Camera->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		pcam->SetFOV(65.0f);
		pcam->SetViewMode(c3::Camera::ViewMode::VM_POLAR);
		pcam->SetPolarDistance(1.0f);
	}

	c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(pvi.m_Camera->FindComponent(c3::Positionable::Type()));
	if (ppos)
	{
		ppos->SetYawPitchRoll(0, 0, 0);
	}

	props::IProperty *campitch_min = pvi.m_Camera->GetProperties()->CreateProperty(_T("PitchCameraAngleMin"), 'PCAN');
	campitch_min->SetFloat(-88.0f);
	props::IProperty *campitch_max = pvi.m_Camera->GetProperties()->CreateProperty(_T("PitchCameraAngleMax"), 'PCAX');
	campitch_max->SetFloat(88.0f);


	pvi.m_GUICamera = pf->Build(pvi.m_Camera);

	pcam = dynamic_cast<c3::Camera *>(pvi.m_GUICamera->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		pcam->SetProjectionMode(c3::Camera::EProjectionMode::PM_ORTHOGRAPHIC);
		pcam->SetOrthoDimensions((float)r.Width(), (float)r.Height());
		pcam->SetViewMode(c3::Camera::EViewMode::VM_LOOKAT);
	}

	ppos = dynamic_cast<c3::Positionable *>(pvi.m_GUICamera->FindComponent(c3::Positionable::Type()));
	if (ppos)
	{
		ppos->SetYawPitchRoll(0, glm::radians(-90.0f), 0);
		ppos->SetPos((float)r.Width() / 2.0f, (float)r.Height() / 2.0f, 10.0f);
	}

	auto ret = m_PerViewInfo.insert(TWndMappedObject::value_type(h, pvi));

	return &ret.first->second;
}

#ifdef SHARED_HANDLERS

// Support for Search Handlers
void C3EditDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data.
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void C3EditDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// C3EditDoc diagnostics

#ifdef _DEBUG
void C3EditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void C3EditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// C3EditDoc commands


BOOL C3EditDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnSaveDocument(lpszPathName))
		return FALSE;

	genio::IOutputStream *os = genio::IOutputStream::Create();
	if (!os || !os->Assign(lpszPathName) || !os->Open() || !os->CanAccess())
		return FALSE;

	c3::Object::MetadataSaveFunc savemd = [&](tstring &name, tstring &description, tstring &author, tstring &website, tstring &copyright)
	{
		name = m_Name;
		description = m_Description;
		author = m_Author;
		website = m_Website;
		copyright = m_Copyright;
	};

	c3::Object::CameraSaveFunc savecam = [&](c3::Object **camera, float &yaw, float &pitch)
	{
		SPerViewInfo &pvi = m_PerViewInfo.begin()->second;

		assert(camera);
		*camera = pvi.m_Camera;
		yaw = pvi.yaw;
		pitch = pvi.pitch;
	};

	c3::Object::EnvironmentSaveFunc saveenv = [&](glm::fvec4 &clearcolor, glm::fvec4 &shadowcolor, glm::fvec4 &fogcolor, float &fogdensity)
	{
		clearcolor = m_ClearColor;
		shadowcolor = m_ShadowColor;
		fogcolor = m_FogColor;
		fogdensity = m_FogDensity;
	};

	m_RootObj->Save(os, 0, savemd, savecam, saveenv);

	os->Close();

	SetModifiedFlag(FALSE);

	return TRUE;
}


BOOL C3EditDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	c3::Factory *pf = theApp.m_C3->GetFactory();
	if (!pf)
		return FALSE;

	m_RootObj = pf->Build();
	if (m_RootObj)
	{
		m_RootObj->AddComponent(c3::Positionable::Type());
		m_RootObj->Flags().Set(OF_LIGHT | OF_CASTSHADOW | OF_EXPANDED);
	}
	m_OperationalRootObj = m_RootObj;

	genio::IInputStream *is = genio::IInputStream::Create();
	if (!is || !is->Assign(lpszPathName) || !is->Open() || !is->CanAccess())
		return FALSE;

	c3::Object::MetadataLoadFunc loadmd = [&](const tstring &name, const tstring &description, const tstring &author, const tstring &website, const tstring &copyright)
	{
		m_Name = name;
		m_Description = description;
		m_Author = author;
		m_Website = website;
		m_Copyright = copyright;
	};

	c3::Object::CameraLoadFunc loadcam = [&](c3::Object *camera, float yaw, float pitch)
	{
		SPerViewInfo &pvi = m_PerViewInfo.begin()->second;

		pvi.m_Camera = camera;
		pvi.yaw = yaw;
		pvi.pitch = pitch;
	};

	c3::Object::EnvironmentLoadFunc loadenv = [&](const glm::fvec4 &clearcolor, const glm::fvec4 &shadowcolor, const glm::fvec4 &fogcolor, const float &fogdensity)
	{
		m_ClearColor = clearcolor;
		m_ShadowColor = shadowcolor;
		m_FogColor = fogcolor;
		m_FogDensity = fogdensity;
	};

	if (m_RootObj)
	{
		m_RootObj->Load(is, loadmd, loadcam, loadenv);
		m_RootObj->Flags().Set(OF_EXPANDED);
	}

	is->Close();

	ResetViews();

	return TRUE;
}


void C3EditDoc::ResetViews()
{
	ClearSelection();

	POSITION p = m_viewList.GetHeadPosition();
	while (p)
	{
		C3EditView *pv = dynamic_cast<C3EditView *>((CView *)m_viewList.GetAt(p));
		if (pv)
		{
			pv->AdjustYawPitch(0, 0, false);
		}

		m_viewList.GetNext(p);
	}

	theApp.UpdateObjectList();
}


void C3EditDoc::SetBrush(c3::Object *pobj)
{
	C3_SAFERELEASE(m_Brush);

	m_Brush = pobj;
}

void C3EditDoc::SetBrush(const TCHAR *protoname)
{
	c3::Prototype *pp = theApp.m_C3->GetFactory()->FindPrototype(protoname);

	SetBrush(pp);
}


void C3EditDoc::SetBrush(const c3::Prototype *pproto)
{
	if (pproto == m_BrushProto)
		return;

	c3::Object *pobj = nullptr;

	if (pproto)
		pobj = theApp.m_C3->GetFactory()->Build(pproto);

	SetBrush(pobj);
}


void C3EditDoc::ClearSelection()
{
	m_Selected.clear();

	theApp.SetActiveObject(nullptr);

	C3EditFrame *pef = (C3EditFrame *)theApp.m_pMainWnd;
	if (pef->GetSafeHwnd() && pef->m_wndObjects.GetSafeHwnd())
		pef->m_wndObjects.UpdateContents();

	theApp.UpdateObjectList();
}


bool C3EditDoc::IsSelected(const c3::Object *obj) const
{
	return (std::find(m_Selected.cbegin(), m_Selected.cend(), obj) != m_Selected.cend());
}


void C3EditDoc::AddToSelection(const c3::Object *obj)
{
	if (!obj)
		return;

	if (std::find(m_Selected.cbegin(), m_Selected.cend(), obj) == m_Selected.cend())
		m_Selected.push_back((c3::Object *)obj);

	// Only put the first selection's script in the editor window
	if (m_Selected.size() == 1)
	{
		props::IProperty *psrcf_prop = m_Selected[0]->GetProperties()->GetPropertyById('SRCF');
		if (psrcf_prop)
		{
			c3::Resource *psrcf_res = theApp.m_C3->GetResourceManager()->GetResource(psrcf_prop->AsString(), RESF_DEMANDLOAD);
			C3EditFrame *pef = (C3EditFrame *)theApp.m_pMainWnd;
			if (pef->GetSafeHwnd() && pef->m_wndScripting.GetSafeHwnd())
				pef->m_wndScripting.EditScriptResource(psrcf_res);
		}
	}

	theApp.SetActiveObject(m_Selected[0]);

	UpdateStatusMessage();
	theApp.UpdateObjectList();
}


void C3EditDoc::RemoveFromSelection(const c3::Object *obj)
{
	TObjectArray::iterator it = std::find(m_Selected.begin(), m_Selected.end(), obj);
	if (it != m_Selected.cend())
		m_Selected.erase(it);

	if (m_Selected.size() == 1)
		theApp.SetActiveObject(m_Selected[0]);
	else
		theApp.SetActiveObject(nullptr);

	UpdateStatusMessage();
	theApp.UpdateObjectList();
}


size_t C3EditDoc::GetNumSelected()
{
	return m_Selected.size();
}


c3::Object *C3EditDoc::GetSelection(size_t index) const
{
	if (index >= m_Selected.size())
		return nullptr;

	return (c3::Object *)m_Selected.at(index);
}


void C3EditDoc::SortSelectionsByDescendingDepth()
{
	std::function<size_t(const c3::Object *)> _ObjectDepth = [](const c3::Object *po)
	{
		size_t ret = 0;
		while (po)
		{
			po = po->GetParent();
			ret++;
		}
		return ret;
	};

	std::sort(m_Selected.begin(), m_Selected.end(), [&](const c3::Object *a, const c3::Object *b) -> bool
	{
		return _ObjectDepth(a) > _ObjectDepth(b);
	});
}


void C3EditDoc::DoForAllSelected(SelectionFunction func)
{
	for (auto o : m_Selected)
	{
		if (o)
			func(o);
	}
}


void C3EditDoc::DoForAllSelectedBreakable(SelectionFunctionBreakable func)
{
	for (auto o : m_Selected)
	{
		if (o && !func(o))
			return;
	}
}


void C3EditDoc::UpdateStatusMessage(const c3::Object *pobj)
{
	static TCHAR msgbuf[256];

	size_t nums = GetNumSelected();
	if ((nums == 1) || pobj)
	{
		if (!pobj)
			pobj = GetSelection(0);

		GUID g = pobj->GetGuid();
		const TCHAR *n = pobj->GetName();

		_stprintf_s(msgbuf, _T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X} \"%s\""), g.Data1, g.Data2, g.Data3,
					g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7], n ? n : _T(""));
	}
	else switch (nums)
	{
		case 0:
			msgbuf[0] = _T('\0');
			break;

		default:
			_stprintf_s(msgbuf, _T("%zu Objects Selected"), nums);
			break;
	}

	theApp.UpdateStatusMessage(msgbuf);
}
