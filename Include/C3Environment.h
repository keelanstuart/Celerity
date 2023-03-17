// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	class Environment
	{

	public:

		virtual const glm::fvec3 *GetAmbientColor(glm::fvec3 *ambcolor = nullptr) const = NULL;
		virtual const glm::fvec3 *GetSunDirection(glm::fvec3 *sundir = nullptr) const = NULL;
		virtual const glm::fvec3 *GetSunColor(glm::fvec3 *suncolor = nullptr) const = NULL;
		virtual const glm::fvec3 *GetFogColor(glm::fvec3 *fogcolor = nullptr) const = NULL;

	};

};
