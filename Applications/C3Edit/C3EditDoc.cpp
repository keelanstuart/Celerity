
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

	for (TWndMappedObject::iterator it = m_Camera.begin(), last_it = m_Camera.end(); it != last_it; it++)
		it->second->Release();

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

	return TRUE;
}




// C3EditDoc serialization

void C3EditDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		genio::IOutputStream *os = genio::IOutputStream::Create(ar.GetFile());
		if (os->BeginBlock('CEL0'))
		{
			uint16_t len;

			len = (uint16_t)m_Name.length();
			os->WriteUINT16(len);
			os->WriteString((TCHAR *)(m_Name.c_str()));

			len = (uint16_t)m_Description.length();
			os->WriteUINT16(len);
			os->WriteString((TCHAR *)(m_Description.c_str()));

			len = (uint16_t)m_Author.length();
			os->WriteUINT16(len);
			os->WriteString((TCHAR *)(m_Author.c_str()));

			len = (uint16_t)m_Website.length();
			os->WriteUINT16(len);
			os->WriteString((TCHAR *)(m_Website.c_str()));

			len = (uint16_t)m_Copyright.length();
			os->WriteUINT16(len);
			os->WriteString((TCHAR *)(m_Name.c_str()));

			if (os->BeginBlock('CAM0'))
			{
				c3::Object *cam = m_Camera.begin()->second;
				cam->Save(os, 0);

				os->WriteFloat(m_CamPitch);
				os->WriteFloat(m_CamYaw);

				os->EndBlock();
			}

			if (os->BeginBlock('ENV0'))
			{
				os->Write(&m_ClearColor, sizeof(glm::fvec4), 1);

				os->Write(&m_ShadowColor, sizeof(glm::fvec4), 1);

				os->Write(&m_FogColor, sizeof(glm::fvec4), 1);
				os->WriteFloat(m_FogDensity);

				os->EndBlock();
			}

			m_RootObj->Save(os, 0);

			os->EndBlock();
		}
	}
	else
	{
		genio::IInputStream *is = genio::IInputStream::Create(ar.GetFile());
		genio::FOURCHARCODE b = is->NextBlockId();
		if (b == 'CEL0')
		{
			if (is->BeginBlock(b))
			{
				uint16_t len;

				is->ReadUINT16(len);
				m_Name.resize(len);
				is->ReadString((TCHAR *)(m_Name.c_str()));

				is->ReadUINT16(len);
				m_Description.resize(len);
				is->ReadString((TCHAR *)(m_Description.c_str()));

				is->ReadUINT16(len);
				m_Author.resize(len);
				is->ReadString((TCHAR *)(m_Author.c_str()));

				is->ReadUINT16(len);
				m_Website.resize(len);
				is->ReadString((TCHAR *)(m_Website.c_str()));

				is->ReadUINT16(len);
				m_Copyright.resize(len);
				is->ReadString((TCHAR *)(m_Name.c_str()));

				if (is->BeginBlock('CAM0'))
				{
					c3::Object *cam = theApp.m_C3->GetFactory()->Build();
					cam->Load(is);
					m_Camera.insert(TWndMappedObject::value_type(0, cam));

					is->ReadFloat(m_CamPitch);
					is->ReadFloat(m_CamYaw);

					is->EndBlock();
				}

				if (is->BeginBlock('ENV0'))
				{
					is->Read(&m_ClearColor, sizeof(glm::fvec4), 1);

					is->Read(&m_ShadowColor, sizeof(glm::fvec4), 1);

					is->Read(&m_FogColor, sizeof(glm::fvec4), 1);
					is->ReadFloat(m_FogDensity);

					is->EndBlock();
				}

				m_RootObj->Load(is);

				is->EndBlock();
			}
		}
	}
}

c3::Object* C3EditDoc::GetCamera(HWND h)
{
	TWndMappedObject::iterator it = m_Camera.find(h);
	if (it != m_Camera.end())
		return it->second;

	if (!m_Camera.empty())
	{
		it = m_Camera.find(0);
		if (it != m_Camera.end())
		{
			m_Camera.insert(TWndMappedObject::value_type(h, it->second));
			m_Camera.erase(0);
		}
	}

	c3::Factory *pf = theApp.m_C3->GetFactory();
	assert(pf);

	c3::Object *c = pf->Build();
	assert(c);

	c->AddComponent(c3::Positionable::Type());
	c->AddComponent(c3::Camera::Type());
	c->SetName(_T("Camera"));

	m_Camera.insert(TWndMappedObject::value_type(h, c));

	return c;
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
