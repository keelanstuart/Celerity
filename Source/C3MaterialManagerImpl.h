// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3MaterialManager.h>
#include <C3Renderer.h>
#include <C3Texture.h>

namespace c3
{

	class MaterialManagerImpl : public MaterialManager
	{

	protected:
		Renderer *m_pRend;

		std::deque<Material *> m_Mats;

	public:

		MaterialManagerImpl(Renderer *prend);

		virtual ~MaterialManagerImpl();

		/// Creates a new Material
		virtual Material *CreateMaterial();

		/// Clones a new Material from an existing one
		virtual Material *CloneMaterial(const Material *to_clone);

		/// Destroys the given material
		virtual bool DestroyMaterial(Material *mtl);

	};

};