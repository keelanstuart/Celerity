// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3GlobalObjectRegistryImpl.h>

using namespace c3;


GlobalObjectRegistryImpl::GlobalObjectRegistryImpl()
{
	memset(m_RegisteredObjects, 0, sizeof(Object *) * GlobalObjectRegistry::OD_NUMDESIGNATIONS);
}


GlobalObjectRegistryImpl::~GlobalObjectRegistryImpl()
{
}


bool GlobalObjectRegistryImpl::RegisterObject(ObjectDesignation designation, Object *obj)
{
	m_RegisteredObjects[designation] = obj;

	return true;
}


Object *GlobalObjectRegistryImpl::GetRegisteredObject(ObjectDesignation designation) const
{
	return m_RegisteredObjects[designation];
}
