
// C3EditDoc.cpp : implementation of the C3EditDoc class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "C3Edit.h"
#endif

#include "C3EditDoc.h"

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
	if (m_Brush)
	{
		m_Brush->Release();
		m_Brush = nullptr;
	}

	for (auto it : m_PerViewInfo)
		it.second.obj->Release();

	if (m_RootObj)
	{
		m_RootObj->Release();
		m_RootObj = nullptr;
	}
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
	m_RootObj->Flags().Set(OF_LIGHT | OF_CASTSHADOW);

	return TRUE;
}




// C3EditDoc serialization

void C3EditDoc::Serialize(CArchive& ar)
{
}

C3EditDoc::SPerViewInfo *C3EditDoc::GetPerViewInfo(HWND h)
{
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

	c3::Object *c = pf->Build();
	assert(c);

	c->AddComponent(c3::Positionable::Type());
	c->AddComponent(c3::Camera::Type());
	c->SetName(_T("Camera"));

	c3::Camera *pcam = dynamic_cast<c3::Camera *>(c->FindComponent(c3::Camera::Type()));
	if (pcam)
	{
		pcam->SetFOV(glm::radians(70.0f));
		pcam->SetViewMode(c3::Camera::ViewMode::VM_POLAR);
		pcam->SetPolarDistance(1.0f);
	}

	c3::Positionable *ppos = dynamic_cast<c3::Positionable *>(c->FindComponent(c3::Positionable::Type()));
	if (ppos)
	{
		ppos->SetYawPitchRoll(0, 0, 0);
	}

	props::IProperty *campitch_min = c->GetProperties()->CreateProperty(_T("PitchCameraAngleMin"), 'PCAN');
	campitch_min->SetFloat(-88.0f);
	props::IProperty *campitch_max = c->GetProperties()->CreateProperty(_T("PitchCameraAngleMax"), 'PCAX');
	campitch_max->SetFloat(88.0f);

	SPerViewInfo pvi;
	pvi.obj = c;
	pvi.yaw = pvi.pitch = 0;

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

	if (os->BeginBlock('CEL0'))
	{
		uint16_t len;

		len = (uint16_t)m_Name.length();
		os->WriteUINT16(len);
		if (len)
			os->WriteString((TCHAR *)(m_Name.c_str()));

		len = (uint16_t)m_Description.length();
		os->WriteUINT16(len);
		if (len)
			os->WriteString((TCHAR *)(m_Description.c_str()));

		len = (uint16_t)m_Author.length();
		os->WriteUINT16(len);
		if (len)
			os->WriteString((TCHAR *)(m_Author.c_str()));

		len = (uint16_t)m_Website.length();
		os->WriteUINT16(len);
		if (len)
			os->WriteString((TCHAR *)(m_Website.c_str()));

		len = (uint16_t)m_Copyright.length();
		os->WriteUINT16(len);
		if (len)
			os->WriteString((TCHAR *)(m_Copyright.c_str()));

		if (os->BeginBlock('CAM0'))
		{
			SPerViewInfo *pvi = &m_PerViewInfo.begin()->second;
			c3::Object *cam = pvi->obj;
			cam->Save(os, 0);

			os->WriteFloat(pvi->pitch);
			os->WriteFloat(pvi->yaw);

			os->EndBlock();
		}

		if (os->BeginBlock('ENV0'))
		{
			os->Write(&m_ClearColor, sizeof(glm::fvec4));

			os->Write(&m_ShadowColor, sizeof(glm::fvec4));

			os->Write(&m_FogColor, sizeof(glm::fvec4));
			os->WriteFloat(m_FogDensity);

			os->EndBlock();
		}

		m_RootObj->Save(os, 0);

		os->EndBlock();
	}

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
		m_RootObj->Flags().Set(OF_LIGHT | OF_CASTSHADOW);
	}

	genio::IInputStream *is = genio::IInputStream::Create();
	if (!is || !is->Assign(lpszPathName) || !is->Open() || !is->CanAccess())
		return FALSE;

	genio::FOURCHARCODE b = is->NextBlockId();
	if (b == 'CEL0')
	{
		if (is->BeginBlock(b))
		{
			uint16_t len;

			is->ReadUINT16(len);
			m_Name.resize(len);
			if (len)
				is->ReadString((TCHAR *)(m_Name.c_str()));

			is->ReadUINT16(len);
			m_Description.resize(len);
			if (len)
				is->ReadString((TCHAR *)(m_Description.c_str()));

			is->ReadUINT16(len);
			m_Author.resize(len);
			if (len)
				is->ReadString((TCHAR *)(m_Author.c_str()));

			is->ReadUINT16(len);
			m_Website.resize(len);
			if (len)
				is->ReadString((TCHAR *)(m_Website.c_str()));

			is->ReadUINT16(len);
			m_Copyright.resize(len);
			if (len)
				is->ReadString((TCHAR *)(m_Copyright.c_str()));

			if (is->BeginBlock('CAM0'))
			{
				SPerViewInfo pvi;

				pvi.obj = theApp.m_C3->GetFactory()->Build();
				pvi.obj->Load(is);

				is->ReadFloat(pvi.pitch);
				is->ReadFloat(pvi.yaw);
				m_PerViewInfo.insert(TWndMappedObject::value_type(0, pvi));

				is->EndBlock();
			}

			if (is->BeginBlock('ENV0'))
			{
				is->Read(&m_ClearColor, sizeof(glm::fvec4));
	
				is->Read(&m_ShadowColor, sizeof(glm::fvec4));
	
				is->Read(&m_FogColor, sizeof(glm::fvec4));
				is->ReadFloat(m_FogDensity);
	
				is->EndBlock();
			}

			if (m_RootObj)
				m_RootObj->Load(is);

			is->EndBlock();
		}
	}

	is->Close();

	return TRUE;
}
