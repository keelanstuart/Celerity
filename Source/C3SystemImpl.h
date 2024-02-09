// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>

#include <C3RendererImpl.h>
#include <C3LogImpl.h>
#include <C3FactoryImpl.h>
#include <C3PluginManagerImpl.h>
#include <C3ResourceManagerImpl.h>
#include <C3FileMapperImpl.h>
#include <C3ResourceManagerImpl.h>
#include <C3InputManagerImpl.h>
#include <C3ActionMapperImpl.h>
#include <C3GlobalObjectRegistryImpl.h>
#include <C3EnvironmentImpl.h>
#include <C3SoundPlayerImpl.h>
#include <C3ScreenManagerImpl.h>


namespace c3
{

	class SystemImpl : public System
	{

	protected:
		HWND m_hOwner;

		RendererImpl *m_Renderer;
		FactoryImpl *m_Factory;
		PluginManagerImpl *m_PluginManager;
		ResourceManagerImpl *m_ResourceManager;
		InputManagerImpl *m_InputManager;
		ActionMapperImpl *m_ActionMapper;
		pool::IThreadPool *m_Pool;
		LogImpl *m_Log;
		FileMapperImpl *m_FileMapper;
		GlobalObjectRegistryImpl m_GlobalObjectRegistry;
		EnvironmentImpl m_Environment;
		SoundPlayerImpl *m_SoundPlayer;
		ScreenManager *m_ScreenManager;

		LARGE_INTEGER m_PerfFreq, m_PerfCount, m_PerfDelta;
		float m_CurrentTime;
		float m_LastTime;
		float m_ElapsedTime;

	public:
		SystemImpl();

		virtual ~SystemImpl();

		virtual void SetOwner(HWND owner);

		virtual HWND GetOwner() const;

		virtual bool IsSplashWnd(HWND h) const;

		virtual void Release();

		virtual ResourceManager *GetResourceManager();

		virtual Renderer *GetRenderer();

		virtual SoundPlayer *GetSoundPlayer();

		virtual Environment *GetEnvironment();

		virtual InputManager *GetInputManager();

		virtual ActionMapper *GetActionMapper();

		virtual Factory *GetFactory();

		virtual PluginManager *GetPluginManager();

		virtual pool::IThreadPool *GetThreadPool();

		virtual FileMapper *GetFileMapper();

		virtual ScreenManager *GetScreenManager();

		virtual Configuration *CreateConfiguration(const TCHAR *filename);

		virtual Log *GetLog();

		virtual GlobalObjectRegistry *GetGlobalObjectRegistry();

		virtual float GetCurrentRunTime();

		virtual float GetElapsedTime();

		virtual void UpdateTime();

	};

};