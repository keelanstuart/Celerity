// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3PositionableImpl.h>
#include <C3BoundingBoxImpl.h>
#include <C3Material.h>

namespace c3
{

	class ExtrusionDescription
	{

	public:
		ExtrusionDescription() {	m_SurfDesc.resize(SurfaceType::NUM_SURFTYPES); }

		bool Load(tinyxml2::XMLDocument *pdoc);
		Model *GenerateExtrusion(System *psys);

		tstring m_Name;
		std::optional<tstring> m_MapFile;

		using SurfaceType = enum
		{
			NEGZ = 0,
			POSZ,
			POSY,
			NEGY,
			POSX,
			NEGX,

			NUM_SURFTYPES
		};

		struct SurfaceDescription
		{
			std::optional<tstring> m_TexFile[Material::TextureComponentType::NUM_TEXTURETYPES];
			float m_Opacity = 1.0f;
			glm::fvec2 m_Tiling = { 1.0f, 1.0f };
		};

		using SurfaceDescriptionArray = std::vector<SurfaceDescription>;
		using SurfaceArray = std::vector<SurfaceDescriptionArray>;

		SurfaceArray m_SurfDesc;
	};

};