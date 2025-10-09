// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3PositionableImpl.h>
#include <C3BoundingBoxImpl.h>
#include <C3Material.h>

namespace c3
{

	class MazeDescription
	{

	public:
		MazeDescription() {	m_SurfDesc.resize(SurfaceType::NUM_SURFTYPES); }

		bool Load(tinyxml2::XMLDocument *pdoc);
		Model *GenerateMaze(System *psys);

		tstring m_Name;
		float m_MinWallZ, m_MaxWallZ, m_FloorZ, m_CeilingZ;
		std::optional<tstring> m_MapFile;

		using SurfaceType = enum
		{
			FLOOR = 0,
			CEILING,
			NORTH,
			SOUTH,
			EAST,
			WEST,

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