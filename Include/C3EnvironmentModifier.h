// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Component.h>


namespace c3
{

	// An EnvironmentModifier is a volume that, when the camera enters it, pushes environment settings...
	// and when the camera leaves pops them. You could use this to affect lighting, audio, fog, etc.
	// See C3Enviroment.h for the various settings available.

	// Properties:
	//		'eGRV' - Gravity, VEC3 type
	//		'eBGC' - Background Color, Color aspect, sets the color that the back buffer is cleared with
	//		'eAMB' - Ambient Color, Color aspect, sets the color of the ambient light source
	//		'eSND' - Sun Direction, VEC3 type, sets the direction for the global sun light
	//		'eSNC' - Sun Color, Color aspect, sets the color of the global sun light
	//		'eFGC' - Fog Color, Color aspect, sets the color of any fog
	//		'eFGD' - Fog Desnity, float type [0..1], sets the density of any fog 
	//		'eFGS' - Fog Start, float type, sets the starting depth of any fog [0..1]
	//		'eFGE' - Fog End, float type, sets the ending depth of any fog (max density) [0..1]

	class C3_API EnvironmentModifier : public Component
	{

	public:

		static const ComponentType *Type();

	};

};
