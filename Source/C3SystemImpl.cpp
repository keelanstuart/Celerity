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

	m_FileMapper = nullptr;
	m_ResourceManager = nullptr;
	m_Renderer = nullptr;
	m_Factory = nullptr;
	m_PluginManager = nullptr;
	m_Pool = nullptr;
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
		/// UNREGISTER NATIVE COMPORTMENTS BETWEEN THESE LINES
		// *************************************************
		UNREGISTER_COMPORTMENTTYPE(Positionable, m_Factory);
		UNREGISTER_COMPORTMENTTYPE(Camera, m_Factory);
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

		/// REGISTER NATIVE COMPORTMENTS BETWEEN THESE LINES
		// *************************************************
		REGISTER_COMPORTMENTTYPE(Positionable, m_Factory);
		REGISTER_COMPORTMENTTYPE(Camera, m_Factory);
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