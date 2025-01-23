// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3GlobalObjectRegistry.h>


namespace c3
{

	class GlobalObjectRegistryImpl : public GlobalObjectRegistry
	{

	protected:
		Object *m_RegisteredObjects[GlobalObjectRegistry::ObjectDesignation::OD_NUMDESIGNATIONS];

	public:

		GlobalObjectRegistryImpl();

		virtual ~GlobalObjectRegistryImpl();

		// registers an Object with a particular designation
		virtual bool RegisterObject(ObjectDesignation designation, Object *obj);

		// returns the Object with the registered designation
		virtual Object *GetRegisteredObject(ObjectDesignation designation) const;

	};

};