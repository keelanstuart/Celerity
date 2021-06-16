// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


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
		Texture2D *m_MtlTex;


	public:

		MaterialManagerImpl(Renderer *prend);

		virtual ~MaterialManagerImpl();

		void UpdateTextures();

		/// Creates a new Material and returns the index
		virtual Material *CreateMaterial();

		/// Removes a material from those that are managed
		virtual bool RemoveMaterial(const Material *mtl);

		/// Returns the Texture2D that contains the material colors
		virtual Texture2D *GetMaterialColorTexture(const Material *pmtl);

		/// Returns the texture coordinates for the given Material color component in the MaterialManager's texture
		/// mul_type_add is the amount at that you must multiply by the color type (Material::ColorComponentType) and add to coord
		virtual bool GetMaterialTexCoordInfo(const Material *pmaterial, glm::fvec2 &coord, glm::fvec2 &mul_type_add);

	};

};