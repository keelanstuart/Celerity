// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2024, Keelan Stuart


#pragma once

#include <c3.h>
#include <C3Animation.h>


namespace c3
{

	// A Model is a collection of Meshes and Nodes - Nodes simply reference those Meshes and contain transform and material information
	// used in drawing those Meshes.

	// Conceptually, you could have a Model with only one Mesh, but many Nodes, each with a unique transform and Texture / Color configuration

	class Model
	{

	public: 

		typedef size_t NodeIndex;
		typedef size_t MeshIndex;
		typedef size_t SubMeshIndex;

		enum { NO_PARENT = -1 };
		enum { INVALID_INDEX = -2 };

		// InstanceData lets you adjust the transform and the material when rendering an instance 
		class InstanceData
		{

		public:

			virtual void Release() = NULL;

			virtual const Model *GetSourceModel() = NULL;

			virtual bool GetTransform(NodeIndex idx, glm::fmat4x4 &mat) = NULL;

			virtual void SetTransform(NodeIndex idx, glm::fmat4x4 &mat) = NULL;

			virtual Material *GetMaterial(NodeIndex nodeidx, MeshIndex meshidx) = NULL;

		};

		static C3_API Model *Create(Renderer *prend);

		virtual void Release() = NULL;

		virtual NodeIndex AddNode() = NULL;

		virtual void RemoveNode(NodeIndex nidx) = NULL;

		virtual size_t GetNodeCount() const = NULL;

		virtual bool FindNode(const TCHAR *name, NodeIndex *pidx = nullptr, bool case_sensitive = false) const = NULL;

		virtual void SetNodeName(NodeIndex nidx, const TCHAR *name) = NULL;

		virtual const TCHAR *GetNodeName(NodeIndex nidx) const = NULL;

		virtual void SetTransform(NodeIndex nidx, const glm::fmat4x4 *pmat) = NULL;

		virtual const glm::fmat4x4 *GetTransform(NodeIndex nidx, glm::fmat4x4 *pmat = nullptr) const = NULL;

		virtual void SetParentNode(NodeIndex nidx, NodeIndex parent_nidx = NO_PARENT) = NULL;

		virtual NodeIndex GetParentNode(NodeIndex nidx) const = NULL;

		virtual SubMeshIndex AssignMeshToNode(NodeIndex nidx, MeshIndex midx) = NULL;

		virtual MeshIndex GetMeshFromNode(NodeIndex nidx, SubMeshIndex midx) const = NULL;

		virtual size_t GetMeshCountOnNode(NodeIndex nidx) const = NULL;

		virtual MeshIndex AddMesh(const Mesh *pmesh) = NULL;

		virtual const Mesh *GetMesh(MeshIndex midx) const = NULL;

		virtual void RemoveMesh(MeshIndex midx) = NULL;

		virtual size_t GetMeshCount() const = NULL;

		virtual void SetMaterial(MeshIndex midx, const Material *pmtl) = NULL;

		virtual const Material *GetMaterial(MeshIndex idx) const = NULL;

		virtual const BoundingBox *GetBounds(BoundingBox *pbb = nullptr) const = NULL;

		virtual void GetBoundingSphere(glm::fvec3 *centroid = nullptr, float *radius = nullptr) const = NULL;

		virtual void Draw(const glm::fmat4x4 *pmat = nullptr, bool allow_material_changes = true, const InstanceData *instance_data = nullptr) const = NULL;

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats, size_t *pMeshIndex,
							   float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV) const = NULL;

		virtual const Animation *GetDefaultAnim() const = NULL;

	};

};
