// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3SystemImpl.h>
#include <C3ConfigurationImpl.h>

#include <C3PositionableImpl.h>
#include <C3CameraImpl.h>

#include <C3TextureImpl.h>
#include <C3ModelImpl.h>

using namespace c3;


System *System::Create(props::TFlags64 flags)
{
	System *ret = new SystemImpl();

	return ret;
}

void InitializeCrc16Table();
void InitializeCrc32Table();

SystemImpl::SystemImpl()
{
	// Initialize out CRC tables before we get rollin'
	InitializeCrc16Table();
	InitializeCrc32Table();

	// we have a log by default... it will pipe to WARNINGS if nothing else
	m_Log = new LogImpl(this);

	m_FileMapper = nullptr;
	m_ResourceManager = nullptr;
	m_Renderer = nullptr;
	m_Factory = nullptr;
	m_PluginManager = nullptr;
	m_Pool = nullptr;

	m_FrameNum = 0;

	QueryPerformanceFrequency(&m_PerfFreq);
	QueryPerformanceCounter(&m_PerfCount);

	m_CurrentTime = 0.0f;
	m_LastTime = 0.0f;
	m_ElapsedTime = 0.0f;
}


SystemImpl::~SystemImpl()
{
	if (m_Log)
	{
		delete m_Log;
		m_Log = nullptr;
	}
}


void SystemImpl::Release()
{
	if (m_ResourceManager)
	{
		/// UNREGISTER NATIVE RESOURCE TYPES BETWEEN THESE LINES
		// *************************************************
		UNREGISTER_RESOURCETYPE(Texture2D, m_ResourceManager);
		UNREGISTER_RESOURCETYPE(Model, m_ResourceManager);
		// *************************************************

		delete m_ResourceManager;
		m_ResourceManager = nullptr;
	}

	if (m_FileMapper)
	{
		delete m_FileMapper;
		m_FileMapper = nullptr;
	}

	if (m_Renderer)
	{
		delete m_Renderer;
		m_Renderer = nullptr;
	}

	if (m_Factory)
	{
		/// UNREGISTER NATIVE FEATURES BETWEEN THESE LINES
		// *************************************************
		UNREGISTER_FEATURETYPE(Positionable, m_Factory);
		UNREGISTER_FEATURETYPE(Camera, m_Factory);
		// *************************************************

		delete m_Factory;
		m_Factory = nullptr;
	}

	if (m_PluginManager)
	{
		delete m_PluginManager;
		m_PluginManager = nullptr;
	}

	if (m_Pool)
	{
		m_Pool->WaitForAllTasks(INFINITE);
		m_Pool->Release();
		m_Pool = nullptr;
	}

	delete this;
}


FileMapper *SystemImpl::GetFileMapper()
{
	if (!m_FileMapper)
	{
		m_FileMapper = new FileMapperImpl(this);
	}

	return m_FileMapper;
}


ResourceManager *SystemImpl::GetResourceManager()
{
	if (!m_ResourceManager)
	{
		m_ResourceManager = new ResourceManagerImpl(this);

		/// REGISTER NATIVE RESOURCE TYPES BETWEEN THESE LINES
		// *************************************************
		REGISTER_RESOURCETYPE(Texture2D, m_ResourceManager);
		REGISTER_RESOURCETYPE(Model, m_ResourceManager);
		// *************************************************
	}

	return m_ResourceManager;
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

		/// REGISTER NATIVE FEATURES BETWEEN THESE LINES
		// *************************************************
		REGISTER_FEATURETYPE(Positionable, m_Factory);
		REGISTER_FEATURETYPE(Camera, m_Factory);
		// *************************************************
	}

	return m_Factory;
}


PluginManager *SystemImpl::GetPluginManager()
{
	if (!m_PluginManager)
	{
		m_PluginManager = new PluginManagerImpl(this);
	}

	return m_PluginManager;
}


pool::IThreadPool *SystemImpl::GetThreadPool()
{
	if (!m_Pool)
	{
		m_Pool = pool::IThreadPool::Create(1, 0);
	}

	return m_Pool;
}


Configuration *SystemImpl::CreateConfiguration(const TCHAR *filename)
{
	Configuration *ret = new ConfigurationImpl(filename);

	return ret;
}

Log *SystemImpl::GetLog()
{
	return m_Log;
}


size_t SystemImpl::GetCurrentFrameNumber()
{
	return m_FrameNum;
}


void SystemImpl::SetCurrentFrameNumber(size_t framenum)
{
	m_FrameNum = framenum;
}


float SystemImpl::GetCurrentTime()
{
	return m_CurrentTime;
}


float SystemImpl::GetElapsedTime()
{
	return m_ElapsedTime;
}


void SystemImpl::UpdateTime()
{
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	m_PerfDelta.QuadPart = c.QuadPart - m_PerfCount.QuadPart;
	m_PerfCount = c;

	m_ElapsedTime = (float)m_PerfDelta.QuadPart * 100.0f / (float)m_PerfFreq.QuadPart;
	m_LastTime = m_CurrentTime;
	m_CurrentTime += m_ElapsedTime;
}
