// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2020, Keelan Stuart


#pragma once

#include <c3.h>


namespace c3
{

	class Model
	{

	public: 

		typedef size_t NodeIndex;
		typedef size_t MeshIndex;
		typedef size_t SubMeshIndex;

		enum { NO_PARENT = -1 };
		enum { INVALID_INDEX = -1 };

		virtual void Release() = NULL;

		virtual NodeIndex AddNode() = NULL;

		virtual void RemoveNode(NodeIndex nidx) = NULL;

		virtual size_t GetNodeCount() const = NULL;

		virtual void SetNodeName(NodeIndex nidx, const TCHAR *name) = NULL;

		virtual const TCHAR *GetNodeName(NodeIndex nidx) const = NULL;

		virtual void SetTransform(NodeIndex nidx, const glm::fmat4x4 *pmat) = NULL;

		virtual const glm::fmat4x4 *GetTransform(NodeIndex nidx, glm::fmat4x4 *pmat = nullptr) const = NULL;

		virtual void SetParent(NodeIndex nidx, NodeIndex parent_nidx = NO_PARENT) = NULL;

		virtual NodeIndex GetParent(NodeIndex nidx) const = NULL;

		virtual SubMeshIndex AssignMeshToNode(NodeIndex nidx, MeshIndex midx) = NULL;

		virtual MeshIndex GetMeshFromNode(NodeIndex nidx, SubMeshIndex midx) const = NULL;

		virtual size_t GetMeshCountOnNode(NodeIndex nidx) const = NULL;

		virtual MeshIndex AddMesh(const Mesh *pmesh) = NULL;

		virtual const Mesh *GetMesh(MeshIndex midx) const = NULL;

		virtual void RemoveMesh(MeshIndex midx) = NULL;

		virtual size_t GetMeshCount() const = NULL;

		virtual void SetMaterial(MeshIndex midx, const Material *pmtl) = NULL;

		virtual const Material *GetMaterial(MeshIndex idx) const = NULL;

		virtual const Frustum *GetBounds() const = NULL;

		virtual void Draw(const glm::fmat4x4 *pmat = nullptr) const = NULL;

	};

};
