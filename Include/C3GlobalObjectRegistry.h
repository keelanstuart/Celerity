// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	class GlobalObjectRegistry
	{

	public:

		typedef enum EObjectDesignation
		{
			OD_WORLDROOT = 0,
			OD_SKYBOXROOT,
			OD_CAMERA_ROOT,
			OD_CAMERA_ARM,
			OD_CAMERA,
			OD_GUI_ROOT,
			OD_GUI_CAMERA,
			OD_PLAYER,

			OD_NUMDESIGNATIONS
		} ObjectDesignation;

		// registers an acction with a name and your callback
		virtual bool RegisterObject(ObjectDesignation designation, Object *obj = nullptr) = NULL;

		// returns the number of currently registered actions
		virtual Object *GetRegisteredObject(ObjectDesignation designation) const = NULL;

	};

};