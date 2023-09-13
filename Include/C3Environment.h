// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	// Environment lets you change ambient world settings for things that affect rendering, sound, or physics
	// A stack mechanism is utilized so that when entering or leaving an area they can be pushed or popped appropriately

	// Gravity - used for physics, do with it as you will
	// BackgroundColor - the rendering clear color
	// AmbientColor - the amount of ambient light applied to all rendered objects
	// SunDirection - directional light source direction
	// SunColor - directional light source color
	// FogColor - the color of depth-based fog in the rendered scene
	// FogDensity - the maximum density of the fog in rendered scene [0..1]
	// FogStart - the scene depth at which the fog is at it's minimum density [0..]
	// FogEnd - the scene depth at which the fog is at it's maximum density [0..]

	class Environment
	{

	public:

		virtual const glm::fvec3 *GetGravity(glm::fvec3 *gravity = nullptr) const = NULL;
		virtual const glm::fvec3 *GetBackgroundColor(glm::fvec3 *bgcolor = nullptr) const = NULL;
		virtual const glm::fvec3 *GetAmbientColor(glm::fvec3 *ambcolor = nullptr) const = NULL;
		virtual const glm::fvec3 *GetSunDirection(glm::fvec3 *sundir = nullptr) const = NULL;
		virtual const glm::fvec3 *GetSunColor(glm::fvec3 *suncolor = nullptr) const = NULL;
		virtual const glm::fvec4 *GetFogColor(glm::fvec4 *fogcolor = nullptr) const = NULL;
		virtual float GetFogDensity() const = NULL;
		virtual float GetFogStart() const = NULL;
		virtual float GetFogEnd() const = NULL;

		virtual void PushGravity(glm::fvec3 gravity, float transition_time = 0.0f) = NULL;
		virtual void PushBackgroundColor(glm::fvec3 bgcolor, float transition_time = 0.0f) = NULL;
		virtual void PushAmbientColor(glm::fvec3 ambcolor, float transition_time = 0.0f) = NULL;
		virtual void PushSunDirection(glm::fvec3 sundir, float transition_time = 0.0f) = NULL;
		virtual void PushSunColor(glm::fvec3 suncolor, float transition_time = 0.0f) = NULL;
		virtual void PushFogColor(glm::fvec4 fogcolor, float transition_time = 0.0f) = NULL;
		virtual void PushFogDensity(float fogdensity, float transition_time = 0.0f) = NULL;
		virtual void PushFogStart(float fogstart, float transition_time = 0.0f) = NULL;
		virtual void PushFogEnd(float fogend, float transition_time = 0.0f) = NULL;

		virtual void SetGravity(glm::fvec3 gravity, float transition_time = 0.0f) = NULL;
		virtual void SetBackgroundColor(glm::fvec3 bgcolor, float transition_time = 0.0f) = NULL;
		virtual void SetAmbientColor(glm::fvec3 ambcolor, float transition_time = 0.0f) = NULL;
		virtual void SetSunDirection(glm::fvec3 sundir, float transition_time = 0.0f) = NULL;
		virtual void SetSunColor(glm::fvec3 suncolor, float transition_time = 0.0f) = NULL;
		virtual void SetFogColor(glm::fvec4 fogcolor, float transition_time = 0.0f) = NULL;
		virtual void SetFogDensity(float fogdensity, float transition_time = 0.0f) = NULL;
		virtual void SetFogStart(float fogstart, float transition_time = 0.0f) = NULL;
		virtual void SetFogEnd(float fogend, float transition_time = 0.0f) = NULL;

		virtual void PopGravity(float transition_time = 0.0f) = NULL;
		virtual void PopBackgroundColor(float transition_time = 0.0f) = NULL;
		virtual void PopAmbientColor(float transition_time = 0.0f) = NULL;
		virtual void PopSunDirection(float transition_time = 0.0f) = NULL;
		virtual void PopSunColor(float transition_time = 0.0f) = NULL;
		virtual void PopFogColor(float transition_time = 0.0f) = NULL;
		virtual void PopFogDensity(float transition_time = 0.0f) = NULL;
		virtual void PopFogStart(float transition_time = 0.0f) = NULL;
		virtual void PopFogEnd(float transition_time = 0.0f) = NULL;

	};

};
