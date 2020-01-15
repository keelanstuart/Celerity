// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3SystemImpl.h>

#include <C3PositionableImpl.h>


using namespace c3;


System *System::Create(props::TFlags64 flags)
{
	System *ret = new SystemImpl();

	return ret;
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

	if (m_Log)
	{
		delete m_Log;
		m_Log = nullptr;
	}
}


void SystemImpl::Release()
{
	if (m_Renderer)
	{
		delete m_Renderer;
		m_Renderer = nullptr;
	}

	if (m_Factory)
	{
		/// REGISTER NATIVE COMPORTMENTS BETWEEN THESE LINES
		// *************************************************
		UNREGISTER_COMPORTMENTTYPE(Positionable, m_Factory);
		// *************************************************

		delete m_Factory;
		m_Factory = nullptr;
	}

	if (m_Pool)
	{
		m_Pool->WaitForAllTasks(INFINITE);
		m_Pool->Release();
		m_Pool = nullptr;
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

		/// REGISTER NATIVE COMPORTMENTS BETWEEN THESE LINES
		// *************************************************
		REGISTER_COMPORTMENTTYPE(Positionable, m_Factory);
		// *************************************************
	}

	return m_Factory;
}


pool::IThreadPool *SystemImpl::GetThreadPool()
{
	if (!m_Pool)
	{
		m_Pool = pool::IThreadPool::Create(1, 0);
	}

	return m_Pool;
}


Log *SystemImpl::GetLog()
{
	return m_Log;
}