// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3ModelImpl.h>
#include <C3RendererImpl.h>

namespace c3
{

	namespace Terrain
	{
		inline float SampleHeight(Texture2D *pheights, props::TVec2I pos);

		Mesh *CreateTerrainChunk(System *psys, props::TVec2I dim, props::TVec2I ofs, Texture2D *pheights);

		Model *CreateTerrainModel(System *psys, const tinyxml2::XMLNode *terrain_defs_root);
	};

};
