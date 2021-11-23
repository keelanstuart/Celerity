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

// Celedit3Doc.cpp : implementation of the CCeledit3Doc class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Celedit3.h"
#endif

#include "Celedit3Doc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCeledit3Doc

IMPLEMENT_DYNCREATE(CCeledit3Doc, CDocument)

BEGIN_MESSAGE_MAP(CCeledit3Doc, CDocument)
END_MESSAGE_MAP()


// CCeledit3Doc construction/destruction

CCeledit3Doc::CCeledit3Doc() noexcept
{
	m_RootObj = nullptr;
	for (size_t i = 0; i < CAM_NUMCAMS; i++)
		m_Camera[i] = nullptr;
	m_Brush = nullptr;
}

CCeledit3Doc::~CCeledit3Doc()
{
	if (m_Brush)
	{
		m_Brush->Release();
		m_Brush = nullptr;
	}

	for (size_t i = 0; i < CAM_NUMCAMS; i++)
	{
		if (m_Camera[i])
		{
			m_Camera[i]->Release();
			m_Camera[i] = nullptr;
		}
	}

	if (m_RootObj)
	{
		m_RootObj->Release();
		m_RootObj = nullptr;
	}
}

BOOL CCeledit3Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	c3::Factory *pf = theApp.m_C3->GetFactory();
	if (!pf)
		return FALSE;

	for (size_t i = 0; i < CAM_NUMCAMS; i++)
	{
		m_Camera[i] = pf->Build();
		m_Camera[i]->AddComponent(c3::Positionable::Type());
		m_Camera[i]->AddComponent(c3::Camera::Type());
		m_Camera[i]->SetName(_T("Camera"));
	}

	m_RootObj = pf->Build();

	return TRUE;
}


// CCeledit3Doc serialization

void CCeledit3Doc::Serialize(CArchive& ar)
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

			m_RootObj->Save(os, 0);

			for (size_t i = 0; i < CAM_NUMCAMS; i++)
				m_Camera[i]->Save(os, 0);

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

				m_RootObj->Load(is);

				for (size_t i = 0; i < CAM_NUMCAMS; i++)
					m_Camera[i]->Load(is);

				is->EndBlock();
			}
		}
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CCeledit3Doc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CCeledit3Doc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data.
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CCeledit3Doc::SetSearchContent(const CString& value)
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

// CCeledit3Doc diagnostics

#ifdef _DEBUG
void CCeledit3Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCeledit3Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CCeledit3Doc commands
