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
	/// ParticleEmitter is a Component that makes an Object draw particles. Particles have the concept of lifetime and "peak"
	/// behavior, where, as a percentage of the individual lifetime, the properties are interpolated from start to peak, then peak to
	/// end.
	/// 
	/// Requires: Positionable
	/// 
	/// Recognized Properties:
	///		- 'PEsh' : "Shape"
	///			ENUM(SPHERE, RAY, CONE, CYLINDER, PLANE) - The shape of the emitter, determines the emission vector
	///
	///		- 'PErm' : "ParticleRenderMethod"
	///			STRING ; FILENAME - the .c3rm render method file used for rendering the particles
	///
	///		- 'PEmp' : "MaxParticles"
	/// 		INT - The maximum number of particles at any instant
	/// 
	///		- 'PErt' : "EmitRate"
	/// 		FVEC2 min/max - the minimum and maximum number of particles spawned per second
	/// 
	///		- 'PEsp' : "EmitSpeed"
	/// 		FVEC2 min/max - the minimum and maximum velocity magnitude for newly-spawned particles along the emission vector
	/// 
	///		- 'PElf' : "ParticleLife"
	/// 		FVEC2 min/max - the minimum and maximum lifetime of particles in seconds
	/// 
	///		- 'PEac' : "Acceleration"
	/// 		FVEC2 min/max - the minimum and maximum acceleration magnitude for newly-spawned particles 
	/// 
	///		- 'PEgr' : "Gravity"
	/// 		FLOAT - the amount that gravity affects particles
	/// 
	///		- 'PErl' : "Roll"
	/// 		FVEC2 min/max - the minimum and maximum roll about the facing vector
	/// 
	///		- 'PEss' : "StartScale"
	/// 		FLOAT - the start scale for particles
	/// 
	///		- 'PEps' : "PeakScale"
	/// 		FLOAT - the peak scale for particles
	/// 
	///		- 'PEes' : "EndScale"
	/// 		FLOAT - the end scale for particles
	/// 
	///		- 'PEsc' : "StartColor"
	/// 		COLOR - The start color for particles
	/// 
	///		- 'PEpc' : "PeakColor"
	/// 		COLOR - The peak color for particles
	/// 
	///		- 'PEec' : "EndColor"
	/// 		COLOR - The end color for particles
	/// 
	///		- 'PErd' : "Radius"
	/// 		FVEC2 min/max - the minimum and maximum radius (meaning differs by shape)
	/// 
	///		- 'PEtx' : "Texture"
	/// 		STRING FILENAME - the texture filename for the texture applied to particles
	/// 
	///		- 'PEpp' : "Peak"
	/// 		FLOAT - the percentage of a particle's lifetime that it reaches "peak"
	/// </summary>

	class C3_API ParticleEmitter : public Component
	{

	public:

		static const ComponentType *Type();

	};

};