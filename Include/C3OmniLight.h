// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Component.h>


namespace c3
{

	/// <summary>
	/// OmniLight is a Component that lets an Object act as a deferred point light. OmniLights use the "l" technique in their RenderMethod to render a cube
	/// into the scene. If the camera is inside, the winding order is reversed so the inside of the box is drawn instead. The range of an OmniLight is
	/// determined by it's "Scale" property ('SCL').
	///
	/// Requires: Positionable
	/// 
	/// Recognized Properties:
	///		- 'C3RM' : "RenderMethod" 
	///			+ STRING FILENAME - The render method that defines the graphics behavior (uses the "l" technique)
	///
	///		- 'LCLR' : "LightColor" 
	///			+ COLOR - The light color
	/// 
	///		- 'GRAD' : "Gradient" 
	///			+ STRING FILENAME - The filename of a 1D texture that defines the light graient (maybe linear, maybe harsh falloff, up to you!)
	/// 
	///		- 'LATN' : "LightAttenuation"
	///			+ FLOAT - The light attenuation; higher for more exposure, negative for darkening
	/// </summary>

	class C3_API OmniLight : public Component
	{

	public:

		static const ComponentType *Type();

		virtual void SetSourceFrameBuffer(FrameBuffer *psource) = NULL;

	};

};
