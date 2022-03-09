// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


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

		/// Destroys the given material
		virtual bool DestroyMaterial(Material *mtl) = NULL;

	};

};