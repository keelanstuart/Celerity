// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3GlobalObjectRegistryImpl.h>

using namespace c3;


GlobalObjectRegistryImpl::GlobalObjectRegistryImpl(System *psys)
{
	m_pSys = psys;

	memset(m_RegisteredObjects, 0, sizeof(Object *) * GlobalObjectRegistry::OD_NUMDESIGNATIONS);
}


GlobalObjectRegistryImpl::~GlobalObjectRegistryImpl()
{
}


bool GlobalObjectRegistryImpl::RegisterObject(ObjectDesignation designation, Object *obj)
{
	Object *po_screen = m_pSys ? m_pSys->GetScreenManager()->GetActiveScreen() : nullptr;
	Screen *pc_screen = po_screen ? dynamic_cast<Screen *>((Screen *)po_screen->FindComponent(Screen::Type())) : nullptr;

	if (!pc_screen)
		m_RegisteredObjects[designation] = obj;
	else
		pc_screen->GetObjectRegistry()->RegisterObject(designation, obj);

	return true;
}


Object *GlobalObjectRegistryImpl::GetRegisteredObject(ObjectDesignation designation) const
{
	Object *po_screen = m_pSys ? m_pSys->GetScreenManager()->GetActiveScreen() : nullptr;
	Screen *pc_screen = po_screen ? dynamic_cast<Screen *>((Screen *)po_screen->FindComponent(Screen::Type())) : nullptr;

	if (!pc_screen)
		return m_RegisteredObjects[designation];
	else
		return pc_screen->GetObjectRegistry()->GetRegisteredObject(designation);

	return nullptr;
}
