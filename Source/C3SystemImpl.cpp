// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3SystemImpl.h>


using namespace c3;


System *System::Create(props::TFlags64 flags)
{
	return new SystemImpl();
}


SystemImpl::SystemImpl()
{
	m_Renderer = nullptr;
}


SystemImpl::~SystemImpl()
{
	Release();
}


void SystemImpl::Release()
{
	if (m_Renderer)
	{
		delete m_Renderer;
		m_Renderer = nullptr;
	}
}


Renderer *SystemImpl::GetRenderer()
{
	if (!m_Renderer)
	{
		m_Renderer = new RendererImpl(this);
	}

	return m_Renderer;
}
