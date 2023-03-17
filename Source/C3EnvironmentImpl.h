// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Environment.h>


namespace c3
{

	class EnvironmentImpl : public Environment
	{

	protected:

		glm::fvec3 m_AmbientColor;
		glm::fvec3 m_SunColor;
		glm::fvec3 m_SunDirection;
		glm::fvec3 m_FogColor;

	public:

		EnvironmentImpl();

		virtual const glm::fvec3 *GetAmbientColor(glm::fvec3 *ambcolor) const;
		virtual const glm::fvec3 *GetSunDirection(glm::fvec3 *sundir) const;
		virtual const glm::fvec3 *GetSunColor(glm::fvec3 *suncolor) const;
		virtual const glm::fvec3 *GetFogColor(glm::fvec3 *fogcolor) const;

	};

};