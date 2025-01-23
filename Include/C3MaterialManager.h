// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <c3.h>
#include <C3Material.h>

namespace c3
{

	class MaterialManager
	{

	public:

		/// Creates a new Material
		virtual Material *CreateMaterial() = NULL;

		/// Clones a new Material from an existing one
		virtual Material *CloneMaterial(const Material *to_clone) = NULL;

		/// Destroys the given material
		virtual bool DestroyMaterial(Material *mtl) = NULL;

	};

};