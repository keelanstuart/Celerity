// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3EnvironmentImpl.h>

using namespace c3;


EnvironmentImpl::EnvironmentImpl()
{
	m_AmbientColor = Color::fDarkGrey;
	m_SunColor = Color::fNaturalSunlight;
	m_SunDirection = glm::normalize(glm::fvec3(-0.1f, 0.2f, -1.0f));
	m_FogColor = Color::fGrey;
}


const glm::fvec3 *EnvironmentImpl::GetAmbientColor(glm::fvec3 *ambcolor) const
{
	if (!ambcolor)
		return &m_AmbientColor;

	*ambcolor = m_AmbientColor;
	return ambcolor;
}


const glm::fvec3 *EnvironmentImpl::GetSunDirection(glm::fvec3 *sundir) const
{
	if (!sundir)
		return &m_SunDirection;

	*sundir = m_SunDirection;
	return sundir;
}


const glm::fvec3 *EnvironmentImpl::GetSunColor(glm::fvec3 *suncolor) const
{
	if (!suncolor)
		return &m_SunColor;

	*suncolor = m_SunColor;
	return suncolor;
}


const glm::fvec3 *EnvironmentImpl::GetFogColor(glm::fvec3 *fogcolor) const
{
	if (!fogcolor)
		return &m_FogColor;

	*fogcolor = m_FogColor;
	return fogcolor;
}
