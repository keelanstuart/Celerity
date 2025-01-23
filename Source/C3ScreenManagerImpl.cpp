// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3ScreenManagerImpl.h>

#include <Shlwapi.h>

using namespace c3;


ScreenManagerImpl::ScreenManagerImpl(System *psys)
{
	m_pSys = psys;
}


ScreenManagerImpl::~ScreenManagerImpl()
{

}


bool ScreenManagerImpl::RegisterScreen(const Screen *pscreen)
{
	if (!pscreen)
	{
		m_pSys->GetLog()->Print(_T("ScreenManager::RegisterScreen - did you mean to register a NULL Screen?\n"));
		return false;
	}

	if (!pscreen->GetName())
	{
		m_pSys->GetLog()->Print(_T("ScreenManager::RegisterScreen - did you mean to register a Screen with an empty name?\n"));
		return false;
	}

	for (TScreenRegistry::const_iterator it = m_ScreenReg.cbegin(); it != m_ScreenReg.cend(); it++)
	{
		if (*it == pscreen)
		{
			m_pSys->GetLog()->Print(_T("ScreenManager::RegisterScreen - that Screen (\"%s\") was already registered.\n"), pscreen->GetName());
			return false;
		}

		if (!_tcscmp((*it)->GetName(), pscreen->GetName()))
		{
			m_pSys->GetLog()->Print(_T("ScreenManager::RegisterScreen - a Screen with the name \"%s\" was already registered.\n"), pscreen->GetName());
			return false;
		}
	}

	m_ScreenReg.push_back((Screen *)pscreen);
	m_pSys->GetLog()->Print(_T("Screen[ \"%s\" ] registered.\n"), pscreen->GetName());
	return true;
}


bool ScreenManagerImpl::UnregisterScreen(const Screen *pscreen)
{
	if (!pscreen)
	{
		m_pSys->GetLog()->Print(_T("ScreenManager::RegisterScreen - did you mean to unregister a NULL Screen?\n"));
		return false;
	}

	for (TScreenRegistry::const_iterator it = m_ScreenReg.cbegin(); it != m_ScreenReg.cend(); it++)
	{
		if (*it == pscreen)
		{
			m_pSys->GetLog()->Print(_T("Screen[ \"%s\" ] unregistered.\n"), pscreen->GetName());
			m_ScreenReg.erase(it);
			return true;
		}
	}

	return false;
}


size_t ScreenManagerImpl::RegisteredScreenCount()
{
	return m_ScreenReg.size();
}


Screen *ScreenManagerImpl::GetRegisteredScreen(size_t idx)
{
	if (idx < m_ScreenReg.size())
		return m_ScreenReg[idx];

	return nullptr;
}


void ScreenManagerImpl::Update(float elapsed_time)
{
	for (TScreenStack::reverse_iterator it = m_ScreenStack.rbegin(); it != m_ScreenStack.rend(); it++)
	{
		it->first->Update(elapsed_time);

		if (!it->second.IsSet(SCRFLAG_UPDATEOVER))
			break;
	}
}


void ScreenManagerImpl::Render()
{
	for (TScreenStack::reverse_iterator it = m_ScreenStack.rbegin(); it != m_ScreenStack.rend(); it++)
	{
		it->first->Render();

		if (!it->second.IsSet(SCRFLAG_DRAWOVER))
			break;
	}
}


bool ScreenManagerImpl::PushScreen(const TCHAR *screen_name, TScreenFlags flags)
{
	for (TScreenRegistry::const_iterator it = m_ScreenReg.cbegin(); it != m_ScreenReg.cend(); it++)
	{
		Screen *ps = *it;
		if (!_tcscmp(ps->GetName(), screen_name))
		{
			m_ScreenStack.push_back(TScreenStack::value_type(ps, flags));
			return true;
		}
	}

	return false;
}


bool ScreenManagerImpl::PopScreen()
{
	if (!m_ScreenStack.empty())
		m_ScreenStack.pop_back();

	return !m_ScreenStack.empty();
}
