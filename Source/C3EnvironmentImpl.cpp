// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3EnvironmentImpl.h>
#include <cmath>

using namespace c3;

static std::function<glm::fvec3(glm::fvec3 &, glm::fvec3 &, float)> fvec3_interp = [](glm::fvec3 &a, glm::fvec3 &b, float t) -> glm::fvec3
{
	return glm::lerp(a, b, t);
};

static std::function<glm::fvec4(glm::fvec4 &, glm::fvec4 &, float)> fvec4_interp = [](glm::fvec4 &a, glm::fvec4 &b, float t) -> glm::fvec4
{
	return glm::lerp(a, b, t);
};

static std::function<float(float &, float &, float)> float_interp = [](float &a, float &b, float t) -> float
{
	return glm::lerp(a, b, t);
};


EnvironmentImpl::EnvironmentImpl() :
	m_Gravity(glm::fvec3(0, 0, -9.8f), fvec3_interp),
	m_BackgroundColor(Color::fBlack, fvec3_interp),
	m_AmbientColor(Color::fDarkGrey, fvec3_interp),
	m_SunColor(Color::fNaturalSunlight, fvec3_interp),
	m_SunDirection(glm::normalize(glm::fvec3(-0.1f, 0.2f, -1.0f)), fvec3_interp),
	m_FogColor(Color::fGrey, fvec4_interp),
	m_FogDensity(0.0f, float_interp),
	m_FogStart(0.0f, float_interp),
	m_FogEnd(1.0f, float_interp)
{
}


EnvironmentImpl::~EnvironmentImpl()
{
}


void EnvironmentImpl::Update(float elapsed_seconds)
{
	m_Gravity.Update(elapsed_seconds);
	m_BackgroundColor.Update(elapsed_seconds);
	m_AmbientColor.Update(elapsed_seconds);
	m_SunColor.Update(elapsed_seconds);
	m_SunDirection.Update(elapsed_seconds);
	m_FogColor.Update(elapsed_seconds);
	m_FogDensity.Update(elapsed_seconds);
	m_FogStart.Update(elapsed_seconds);
	m_FogEnd.Update(elapsed_seconds);
}

const glm::fvec3 *EnvironmentImpl::GetGravity(glm::fvec3 *gravity) const
{
	if (!gravity)
		return &m_Gravity.ret;

	*gravity = m_Gravity.ret;
	return gravity;
}

const glm::fvec3 *EnvironmentImpl::GetBackgroundColor(glm::fvec3 *bgcolor) const
{
	if (!bgcolor)
		return &m_BackgroundColor.ret;

	*bgcolor = m_BackgroundColor.ret;
	return bgcolor;
}

const glm::fvec3 *EnvironmentImpl::GetAmbientColor(glm::fvec3 *ambcolor) const
{
	if (!ambcolor)
		return &m_AmbientColor.ret;

	*ambcolor = m_AmbientColor.ret;
	return ambcolor;
}

const glm::fvec3 *EnvironmentImpl::GetSunDirection(glm::fvec3 *sundir) const
{
	if (!sundir)
		return &m_SunDirection.ret;

	*sundir = m_SunDirection.ret;
	return sundir;
}

const glm::fvec3 *EnvironmentImpl::GetSunColor(glm::fvec3 *suncolor) const
{
	if (!suncolor)
		return &m_SunColor.ret;

	*suncolor = m_SunColor.ret;
	return suncolor;
}

const glm::fvec4 *EnvironmentImpl::GetFogColor(glm::fvec4 *fogcolor) const
{
	if (!fogcolor)
		return &m_FogColor.ret;

	*fogcolor = m_FogColor.ret;
	return fogcolor;
}

float EnvironmentImpl::GetFogDensity() const
{
	return m_FogDensity.ret;
}

float EnvironmentImpl::GetFogStart() const
{
	return m_FogStart.ret;
}

float EnvironmentImpl::GetFogEnd() const
{
	return m_FogEnd.ret;
}


void EnvironmentImpl::PushGravity(const glm::fvec3 gravity, float transition_time)
{
	m_Gravity.Push(gravity, transition_time);
}

void EnvironmentImpl::PushBackgroundColor(glm::fvec3 bgcolor, float transition_time)
{
	m_BackgroundColor.Push(bgcolor, transition_time);
}

void EnvironmentImpl::PushAmbientColor(glm::fvec3 ambcolor, float transition_time)
{
	m_AmbientColor.Push(ambcolor, transition_time);
}

void EnvironmentImpl::PushSunDirection(glm::fvec3 sundir, float transition_time)
{
	m_SunDirection.Push(sundir, transition_time);
}

void EnvironmentImpl::PushSunColor(glm::fvec3 suncolor, float transition_time)
{
	m_SunColor.Push(suncolor, transition_time);
}

void EnvironmentImpl::PushFogColor(glm::fvec4 fogcolor, float transition_time)
{
	m_FogColor.Push(fogcolor, transition_time);
}

void EnvironmentImpl::PushFogDensity(float fogdensity, float transition_time)
{
	m_FogDensity.Push(fogdensity, transition_time);
}

void EnvironmentImpl::PushFogStart(float fogstart, float transition_time)
{
	m_FogStart.Push(fogstart, transition_time);
}

void EnvironmentImpl::PushFogEnd(float fogend, float transition_time)
{
	m_FogEnd.Push(fogend, transition_time);
}


void EnvironmentImpl::SetGravity(glm::fvec3 gravity, float transition_time)
{
	m_Gravity.Set(gravity, transition_time);
}

void EnvironmentImpl::SetBackgroundColor(glm::fvec3 bgcolor, float transition_time)
{
	m_BackgroundColor.Set(bgcolor, transition_time);
}

void EnvironmentImpl::SetAmbientColor(glm::fvec3 ambcolor, float transition_time)
{
	m_AmbientColor.Set(ambcolor, transition_time);
}

void EnvironmentImpl::SetSunDirection(glm::fvec3 sundir, float transition_time)
{
	m_SunDirection.Set(sundir, transition_time);
}

void EnvironmentImpl::SetSunColor(glm::fvec3 suncolor, float transition_time)
{
	m_SunColor.Set(suncolor, transition_time);
}

void EnvironmentImpl::SetFogColor(glm::fvec4 fogcolor, float transition_time)
{
	m_FogColor.Set(fogcolor, transition_time);
}

void EnvironmentImpl::SetFogDensity(float fogdensity, float transition_time)
{
	m_FogDensity.Set(fogdensity, transition_time);
}

void EnvironmentImpl::SetFogStart(float fogstart, float transition_time)
{
	m_FogStart.Set(fogstart, transition_time);
}

void EnvironmentImpl::SetFogEnd(float fogend, float transition_time)
{
	m_FogEnd.Set(fogend, transition_time);
}


void EnvironmentImpl::PopGravity(float transition_time)
{
	m_Gravity.Pop(transition_time);
}

void EnvironmentImpl::PopBackgroundColor(float transition_time)
{
	m_BackgroundColor.Pop(transition_time);
}

void EnvironmentImpl::PopAmbientColor(float transition_time)
{
	m_AmbientColor.Pop(transition_time);
}

void EnvironmentImpl::PopSunDirection(float transition_time)
{
	m_SunDirection.Pop(transition_time);
}

void EnvironmentImpl::PopSunColor(float transition_time)
{
	m_SunColor.Pop(transition_time);
}

void EnvironmentImpl::PopFogColor(float transition_time)
{
	m_FogColor.Pop(transition_time);
}

void EnvironmentImpl::PopFogDensity(float transition_time)
{
	m_FogDensity.Pop(transition_time);
}

void EnvironmentImpl::PopFogStart(float transition_time)
{
	m_FogStart.Pop(transition_time);
}

void EnvironmentImpl::PopFogEnd(float transition_time)
{
	m_FogEnd.Pop(transition_time);
}
