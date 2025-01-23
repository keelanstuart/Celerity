// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	class C3_API PhysicsManager
	{

	public:

		virtual bool Initialize() = NULL;

		virtual void Shutdown() = NULL;

		virtual void Update(float time = 0.0f) = NULL;

	};

};