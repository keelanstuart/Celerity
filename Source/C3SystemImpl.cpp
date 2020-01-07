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
	// we have a log by default... it will pipe to WARNINGS if nothing else
	m_Log = new LogImpl(this);

	m_Renderer = nullptr;
	m_Factory = nullptr;
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

	if (m_Log)
	{
		delete m_Log;
		m_Log = nullptr;
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


Factory *SystemImpl::GetFactory()
{
	if (!m_Factory)
	{
		m_Factory = new FactoryImpl(this);
	}

	return m_Factory;
}


Log *SystemImpl::GetLog()
{
	return m_Log;
}