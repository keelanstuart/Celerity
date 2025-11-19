// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <c3.h>
#include <C3Animation.h>
#include <optional>


namespace c3
{

	// A Model is a collection of Meshes and Nodes - Nodes simply reference those Meshes and contain transform and material information
	// used in drawing those Meshes.

	// Conceptually, you could have a Model with only one Mesh, but many Nodes, each with a unique transform and Texture / Color configuration

	class Model
	{

	public: 

		using NodeIndex = size_t;
		using MeshIndex = size_t;
		using SubMeshIndex = size_t;

		enum { NO_PARENT = -1 };
		enum { INVALID_INDEX = -2 };

		// Used by ProcessNodes. Takes a NodeIndex. Return true to keep going, false to stop
		using ProcessNodesFunc = std::function<bool(NodeIndex)>;

		// InstanceData lets you adjust the transform and the material when rendering an instance 
		class InstanceData
		{

		public:

			virtual void Release() = NULL;

			virtual const Model *GetSourceModel() = NULL;

			virtual bool GetBoneOffsetTransform(NodeIndex idx, glm::fmat4x4 &mat) const = NULL;

			virtual bool GetTransform(NodeIndex idx, glm::fmat4x4 &mat) = NULL;

			virtual void SetTransform(NodeIndex idx, glm::fmat4x4 &mat) = NULL;

			virtual Material *GetMaterial(NodeIndex nodeidx, MeshIndex meshidx) = NULL;

		};

		// Creates a new Model given a Renderer
		static C3_API Model *Create(Renderer *prend);

		// Releases the Model's resources
		virtual void Release() = NULL;

		// Adds a new Node to the Model - Nodes contain transforms and Mesh references
		virtual NodeIndex AddNode(const TCHAR *name = nullptr) = NULL;

		// Removes a Node by index
		virtual void RemoveNode(NodeIndex nidx) = NULL;

		// Get the number or Nodes in this Model
		virtual size_t GetNodeCount() const = NULL;

		// Looks for a Node by name; returns true and sets the value at pidx if found
		virtual bool FindNode(const TCHAR *name, NodeIndex *pidx = nullptr, bool case_sensitive = false) const = NULL;

		// Sets the Node Name
		virtual void SetNodeName(NodeIndex nidx, const TCHAR *name) = NULL;

		// Gets the Node name
		virtual const TCHAR *GetNodeName(NodeIndex nidx) const = NULL;

		// Sets the visibility of the given Node. If visible is undefined, the default is true
		virtual bool NodeVisibility(NodeIndex nidx, std::optional<bool> visible = std::nullopt) = NULL;

		// Sets the Collidability of the given Node. If collide is undefined, the default is true
		virtual bool NodeCollidability(NodeIndex nidx, std::optional<bool> collide = std::nullopt) = NULL;

		// Sets the transform matrix on the given Node
		virtual void SetTransform(NodeIndex nidx, const glm::fmat4x4 *pmat) = NULL;

		// Gets the transform matrix for the given Node
		virtual const glm::fmat4x4 *GetTransform(NodeIndex nidx, glm::fmat4x4 *pmat = nullptr) const = NULL;

		// Sets the parent Node for the given Node 
		virtual void SetParentNode(NodeIndex nidx, NodeIndex parent_nidx = NO_PARENT) = NULL;

		// Returns the index of the Node's Parent - or NO_PARENT if it is at the top level of the hierarchy
		virtual NodeIndex GetParentNode(NodeIndex nidx) const = NULL;

		// Assigns a Mesh to the given Node. When the Model is drawn, the Mesh will have the Node's transform hierarchy applied
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

		virtual void Draw(const glm::fmat4x4 *pmat = nullptr, bool allow_material_changes = true, const InstanceData *instance_data = nullptr, bool force = false) const = NULL;

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, size_t *pMeshIndex,
							   float *pDistance, glm::fvec3 *pNormal, size_t *pFaceIndex, glm::vec2 *pUV, const InstanceData *inst, bool force = false) const = NULL;

		virtual const Animation *GetDefaultAnim() const = NULL;

		// Goes over all nodes and runs your code on them
		virtual void ProcessNodes(ProcessNodesFunc func) = NULL;

	};

};
