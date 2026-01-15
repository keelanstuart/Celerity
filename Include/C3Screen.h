// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Component.h>
#include <C3GlobalObjectRegistry.h>


namespace c3
{

	// A Screen is a Component that has its own Object Registry

	// Probably don't use this except when the Component is added to an Object
	// implicitly by the ScreenManager... because Screens really only make sense there

	class Screen : public Component
	{

	public:

		virtual GlobalObjectRegistry *GetObjectRegistry() = NULL;

		static const ComponentType *Type();

	};

};
