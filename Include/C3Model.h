// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <c3.h>


namespace c3
{

	class Model
	{

	public: 

		virtual void Release() = NULL;

		virtual size_t AddMesh(const Mesh *pmesh, const TCHAR *name) = NULL;

		virtual size_t GetMeshCount() const = NULL;

		virtual const Mesh *GetMesh(size_t index) const = NULL;

		virtual const TCHAR *GetMeshName(const Mesh *mesh) const = NULL;

		virtual void SetMaterial(const Mesh *mesh, const Material *pmaterial) = NULL;

		virtual const Material *GetMaterial(const Mesh *mesh) const = NULL;

		virtual void SetTransform(const Mesh *mesh, const glm::fmat4 *pmat) = NULL;

		virtual const glm::fmat4 *GetTransform(const Mesh *mesh, glm::fmat4 *pmat = nullptr) const = NULL;

		virtual void SetParent(const Mesh *mesh, const Mesh *parent_mesh) = NULL;

		virtual const Mesh *GetParent(const Mesh *mesh) const = NULL;

		virtual const Frustum *GetBounds() const = NULL;

		virtual void Draw(glm::fmat4 *pmat = nullptr) const = NULL;

	};

};
