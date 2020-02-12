// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <c3.h>

namespace c3
{

	class Mesh;
	class Material;

	class Model
	{

	public: 

		virtual int64_t AddMesh(Mesh *pmesh, Material *pmaterial, glm::fmat4 *pmat, int64_t parent) = NULL;

		virtual size_t GetMeshCount() = NULL;

	};

};
