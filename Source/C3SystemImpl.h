// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>

#include <C3RendererImpl.h>
#include <C3LogImpl.h>
#include <C3FactoryImpl.h>
#include <C3PluginManagerImpl.h>
#include <C3ResourceManagerImpl.h>
#include <C3FileMapperImpl.h>
#include <C3ResourceManagerImpl.h>


namespace c3
{

	class SystemImpl : public System
	{

	protected:
		RendererImpl *m_Renderer;
		FactoryImpl *m_Factory;
		PluginManagerImpl *m_PluginManager;
		ResourceManagerImpl *m_ResourceManager;
		pool::IThreadPool *m_Pool;
		LogImpl *m_Log;
		FileMapperImpl *m_FileMapper;

	public:
		SystemImpl();

		virtual ~SystemImpl();

		virtual void Release();

		virtual ResourceManager *GetResourceManager();

		virtual Renderer *GetRenderer();

		virtual Factory *GetFactory();

		virtual PluginManager *GetPluginManager();

		virtual pool::IThreadPool *GetThreadPool();

		virtual FileMapper *GetFileMapper();

		virtual Configuration *CreateConfiguration(const TCHAR *filename);

		virtual Log *GetLog();

	};

};