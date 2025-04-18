// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3SystemImpl.h>
#include <C3ConfigurationImpl.h>

#include <C3TextureImpl.h>
#include <C3ModelImpl.h>
#include <C3ShaderComponentImpl.h>
#include <C3RenderMethodImpl.h>

#include <C3PositionableImpl.h>
#include <C3CameraImpl.h>
#include <C3ModelRendererImpl.h>
#include <C3QuadTerrainImpl.h>
#include <C3OmniLightImpl.h>
#include <C3ParticleEmitterImpl.h>
#include <C3ScriptableImpl.h>
#include <C3PhysicalImpl.h>
#include <C3SoundPlayerImpl.h>
#include <C3EnvironmentModifierImpl.h>
#include <C3AnimationImpl.h>
#include <C3AnimatorImpl.h>
#include <C3TextRendererImpl.h>
#include <C3InteractableImpl.h>

#include <C3ResourceImpl.h>


using namespace c3;


System *System::Create(HWND owner, props::TFlags64 flags)
{
	SystemImpl *ret = new SystemImpl();

	ret->SetOwner(owner);

	return ret;
}


extern HWND g_hPostSplashWnd;
void SystemImpl::SetOwner(HWND owner)
{
	g_hPostSplashWnd = m_hOwner = owner;
}


HWND SystemImpl::GetOwner() const
{
	return m_hOwner;
}


extern HWND g_hSplashWnd;
bool SystemImpl::IsSplashWnd(HWND h) const
{
	return (g_hSplashWnd && (h == g_hSplashWnd));
}


void InitializeCrc16Table();
void InitializeCrc32Table();

SystemImpl::SystemImpl()
{
	// Initialize out CRC tables before we get rollin'
	InitializeCrc16Table();
	InitializeCrc32Table();

	m_hOwner = GetDesktopWindow();

	// we have a log by default... it will pipe to WARNINGS if nothing else
	m_Log = new LogImpl(this);

	m_FileMapper = nullptr;
	m_ResourceManager = nullptr;
	m_Renderer = nullptr;
	m_Factory = nullptr;
	m_PluginManager = nullptr;
	m_Pool = nullptr;
	m_InputManager = nullptr;
	m_ActionMapper = nullptr;
	m_SoundPlayer = nullptr;

	QueryPerformanceFrequency(&m_PerfFreq);
	QueryPerformanceCounter(&m_PerfCount);

	m_CurrentTime = 0.0f;
	m_LastTime = 0.0f;
	m_ElapsedTime = 0.0f;

	srand(timeGetTime());
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
		UNREGISTER_RESOURCETYPE(Animation, m_ResourceManager);
		UNREGISTER_RESOURCETYPE(AnimStatesDesc, m_ResourceManager);
		UNREGISTER_RESOURCETYPE(DefaultTexture2D, m_ResourceManager);
		UNREGISTER_RESOURCETYPE(DefaultTextureCube, m_ResourceManager);
		UNREGISTER_RESOURCETYPE(DefaultTexture3D, m_ResourceManager);
		UNREGISTER_RESOURCETYPE(Model, m_ResourceManager);
		UNREGISTER_RESOURCETYPE(ShaderComponent, m_ResourceManager);
		UNREGISTER_RESOURCETYPE(RenderMethod, m_ResourceManager);
		UNREGISTER_RESOURCETYPE(Script, m_ResourceManager);
		UNREGISTER_RESOURCETYPE(Sound, m_ResourceManager);
		// *************************************************

		delete m_ResourceManager;
		m_ResourceManager = nullptr;
	}

	if (m_InputManager)
	{
		delete m_InputManager;
		m_InputManager = nullptr;
	}

	if (m_ActionMapper)
	{
		delete m_ActionMapper;
		m_ActionMapper = nullptr;
	}

	if (m_SoundPlayer)
	{
		delete m_SoundPlayer;
		m_SoundPlayer = nullptr;
	}

	if (m_Renderer)
	{
		delete m_Renderer;
		m_Renderer = nullptr;
	}

	if (m_FileMapper)
	{
		delete m_FileMapper;
		m_FileMapper = nullptr;
	}

	if (m_Factory)
	{
		/// UNREGISTER NATIVE COMPONENTS BETWEEN THESE LINES
		// *************************************************
		UNREGISTER_COMPONENTTYPE(Animator, m_Factory);
		UNREGISTER_COMPONENTTYPE(Positionable, m_Factory);
		UNREGISTER_COMPONENTTYPE(Camera, m_Factory);
		UNREGISTER_COMPONENTTYPE(ModelRenderer, m_Factory);
		UNREGISTER_COMPONENTTYPE(OmniLight, m_Factory);
		UNREGISTER_COMPONENTTYPE(ParticleEmitter, m_Factory);
		UNREGISTER_COMPONENTTYPE(Scriptable, m_Factory);
		UNREGISTER_COMPONENTTYPE(Physical, m_Factory);
		UNREGISTER_COMPONENTTYPE(EnvironmentModifier, m_Factory);
		UNREGISTER_COMPONENTTYPE(TextRenderer, m_Factory);
		UNREGISTER_COMPONENTTYPE(Interactable, m_Factory);
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

	if (m_PhysicsManager)
	{
		delete m_PhysicsManager;
		m_PhysicsManager = nullptr;
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
		ResourceImpl::s_pSys = this;

		/// REGISTER NATIVE RESOURCE TYPES BETWEEN THESE LINES
		// *************************************************
		REGISTER_RESOURCETYPE(Animation, m_ResourceManager);
		REGISTER_RESOURCETYPE(AnimStatesDesc, m_ResourceManager);
		REGISTER_RESOURCETYPE(DefaultTexture2D, m_ResourceManager);
		REGISTER_RESOURCETYPE(DefaultTextureCube, m_ResourceManager);
		REGISTER_RESOURCETYPE(DefaultTexture3D, m_ResourceManager);
		REGISTER_RESOURCETYPE(Model, m_ResourceManager);
		REGISTER_RESOURCETYPE(ShaderComponent, m_ResourceManager);
		REGISTER_RESOURCETYPE(RenderMethod, m_ResourceManager);
		REGISTER_RESOURCETYPE(Script, m_ResourceManager);
		REGISTER_RESOURCETYPE(Sound, m_ResourceManager);
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


SoundPlayer *SystemImpl::GetSoundPlayer()
{
	if (!m_SoundPlayer)
	{
		m_SoundPlayer = new SoundPlayerImpl(this);
	}

	return m_SoundPlayer;
}


Environment *SystemImpl::GetEnvironment()
{
	return &m_Environment;
}


PhysicsManager *SystemImpl::GetPhysicsManager()
{
	if (!m_PhysicsManager)
	{
		m_PhysicsManager = new PhysicsManagerImpl(this);

		//m_PhysicsManager->Initialize();
	}

	return m_PhysicsManager;
}


InputManager *SystemImpl::GetInputManager()
{
	if (!m_InputManager)
	{
		m_InputManager = new InputManagerImpl(this);
		if (m_InputManager)
			m_InputManager->Initialize(m_hOwner, GetModuleHandle(NULL));
	}

	return m_InputManager;
}


ActionMapper *SystemImpl::GetActionMapper()
{
	if (!m_ActionMapper)
	{
		m_ActionMapper = new ActionMapperImpl(this);
	}

	return m_ActionMapper;
}


Factory *SystemImpl::GetFactory()
{
	if (!m_Factory)
	{
		m_Factory = new FactoryImpl(this);

		/// REGISTER NATIVE COMPONENTS BETWEEN THESE LINES
		// *************************************************
		REGISTER_COMPONENTTYPE(Animator, m_Factory);
		REGISTER_COMPONENTTYPE(Positionable, m_Factory);
		REGISTER_COMPONENTTYPE(Camera, m_Factory);
		REGISTER_COMPONENTTYPE(ModelRenderer, m_Factory);
		REGISTER_COMPONENTTYPE(OmniLight, m_Factory);
		REGISTER_COMPONENTTYPE(ParticleEmitter, m_Factory);
		REGISTER_COMPONENTTYPE(Scriptable, m_Factory);
		REGISTER_COMPONENTTYPE(Physical, m_Factory);
		REGISTER_COMPONENTTYPE(EnvironmentModifier, m_Factory);
		REGISTER_COMPONENTTYPE(TextRenderer, m_Factory);
		REGISTER_COMPONENTTYPE(Interactable, m_Factory);
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


ScreenManager *SystemImpl::GetScreenManager()
{
	if (!m_ScreenManager)
	{
		m_ScreenManager = new ScreenManagerImpl(this);
	}

	return m_ScreenManager;
}


Configuration *SystemImpl::CreateConfiguration(const TCHAR *filename)
{
	if (!filename)
		return nullptr;

	Configuration *ret = new ConfigurationImpl(filename);

	return ret;
}


Log *SystemImpl::GetLog()
{
	return m_Log;
}


GlobalObjectRegistry *SystemImpl::GetGlobalObjectRegistry()
{
	return &m_GlobalObjectRegistry;
}


float SystemImpl::GetCurrentRunTime()
{
	return m_CurrentTime;
}


float SystemImpl::GetElapsedTime()
{
	return m_ElapsedTime;
}


void SystemImpl::UpdateTime(bool paused)
{
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	m_PerfDelta.QuadPart = c.QuadPart - m_PerfCount.QuadPart;
	m_PerfCount = c;

	m_ElapsedTime = (float)m_PerfDelta.QuadPart / (float)m_PerfFreq.QuadPart;
	m_LastTime = m_CurrentTime;
	m_CurrentTime += m_ElapsedTime;

	if (m_InputManager)
		m_InputManager->Update(m_ElapsedTime);

	if (m_ActionMapper)
		m_ActionMapper->Update();

	if (m_SoundPlayer)
	{
		// The registered camera is the listener
		Object *pcam = m_GlobalObjectRegistry.GetRegisteredObject(GlobalObjectRegistry::OD_CAMERA);
		if (pcam)
		{
			Positionable *ppc = (Positionable *)(pcam->FindComponent(Positionable::Type()));
			if (ppc)
			{
				m_SoundPlayer->SetListenerPos(ppc->GetPosVec());
				m_SoundPlayer->SetListenerDir(ppc->GetFacingVector());
			}
		}

		m_SoundPlayer->Update(paused ? 0 : m_ElapsedTime);
	}

	m_Environment.Update(paused ? 0 : m_ElapsedTime);

	if (m_PhysicsManager)
		m_PhysicsManager->Update(paused ? 0 : m_ElapsedTime);
}
