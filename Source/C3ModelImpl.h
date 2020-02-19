// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3Model.h>
#include <C3RendererImpl.h>
#include <C3Resource.h>
#include <C3Frustum.h>

namespace c3
{

	class ModelImpl : public Model
	{

	protected:
		Renderer *m_pRend;

		typedef struct
		{
			Mesh *parent;
			Material *pmaterial;
			glm::fmat4 mat;

		} SModelMeshInfo;

		typedef std::multimap<Mesh *, SModelMeshInfo> TMeshesMap;
		TMeshesMap m_Meshes;

		Frustum *m_Bounds;

	public:

		ModelImpl(RendererImpl *prend);

		virtual ~ModelImpl();

		virtual void Release();

		virtual size_t AddMesh(const Mesh *pmesh, const TCHAR *name);

		virtual size_t GetMeshCount() const;

		virtual const Mesh *GetMesh(size_t index) const;

		virtual const TCHAR *GetMeshName(const Mesh *mesh) const;

		virtual void SetMaterial(const Mesh *mesh, const Material *pmaterial);

		virtual const Material *GetMaterial(const Mesh *mesh) const;

		virtual void SetTransform(const Mesh *mesh, const glm::fmat4 *pmat);

		virtual const glm::fmat4 *GetTransform(const Mesh *mesh, glm::fmat4 *pmat = nullptr) const;

		virtual void SetParent(const Mesh *mesh, const Mesh *parent_mesh);

		virtual const Mesh *GetParent(const Mesh *mesh) const;

		virtual const Frustum *GetBounds() const;

		virtual void Draw(glm::fmat4 *pmat = nullptr) const;

	};

	DEFINE_RESOURCETYPE(Model, RTFLAG_RUNBYRENDERER, GUID({0x2ea22c05, 0xbf99, 0x493c, { 0xb7, 0xa, 0x26, 0x6f, 0x89, 0xaf, 0x30, 0x91 }}), "Model", "3D Models", "3ds;dae;fbx;obj;x", "fbx");

};
