// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


#pragma once

#include <C3Model.h>
#include <C3RendererImpl.h>
#include <C3Resource.h>
#include <C3BoundingBox.h>


namespace c3
{

	class ModelImpl : public Model
	{

	protected:
		Renderer *m_pRend;

		typedef struct sMeshInfo
		{
			sMeshInfo()
			{
				pmesh = nullptr;
				pmtl = nullptr;
			}

			const Mesh *pmesh;
			const Material *pmtl;

		} SMeshInfo;

		using NodeFlag = enum
		{
			VISIBLE = 0x0001,
			COLLIDE = 0x0002,
		};

		using SNodeInfo = struct sNodeInfo
		{
			sNodeInfo()
			{
				parent = NO_PARENT;
				mat = glm::identity<glm::fmat4x4>();
			}

			NodeIndex parent;
			typedef std::vector<NodeIndex> TNodeIndexArray;
			TNodeIndexArray children;

			typedef std::vector<MeshIndex> TMeshIndexArray;
			TMeshIndexArray meshes;

			tstring name;
			props::TFlags64 flags;

			glm::fmat4x4 mat;

		};

		typedef std::vector<SNodeInfo *> TNodeInfoArray;
		TNodeInfoArray m_Nodes;

		typedef std::vector<SMeshInfo *> TMeshInfoArray;
		TMeshInfoArray m_Meshes;

		typedef std::vector<Material *> TMaterialArray;
		TMaterialArray m_Materials;

		BoundingBox *m_Bounds, *m_ScratchBounds;
		glm::fvec3 m_BoundingCentroid;
		float m_BoundingRadius;

		const Animation *m_DefaultAnim;

	public:

		std::vector<glm::fmat4x4> m_BoneOffsetMat;		// this is the offset for skeletal nodes

		class InstanceDataImpl : Model::InstanceData
		{
		public:

			const ModelImpl *m_pSourceModel;

			std::vector<glm::fmat4x4> m_NodeMat;			// this is used for animation
			std::vector<std::vector<Material *>> m_NodeMtl;
			std::vector<NodeIndex> m_NodeParent;

			InstanceDataImpl(const Model *psource);

			virtual void Release();

			virtual const Model *GetSourceModel();

			virtual bool GetBoneOffsetTransform(NodeIndex idx, glm::fmat4x4 &mat) const;

			virtual bool GetTransform(NodeIndex idx, glm::fmat4x4 &mat);

			virtual void SetTransform(NodeIndex idx, glm::fmat4x4 &mat);

			virtual Material *GetMaterial(NodeIndex nodeidx, MeshIndex meshidx);

		};


		ModelImpl(RendererImpl *prend);

		virtual ~ModelImpl();

		virtual void Release();

		virtual InstanceData *CloneInstanceData();

		virtual NodeIndex AddNode();

		virtual void RemoveNode(NodeIndex nidx);

		virtual size_t GetNodeCount() const;

		virtual bool FindNode(const TCHAR *name, NodeIndex *pidx = nullptr, bool case_sensitive = false) const;

		virtual void SetNodeName(NodeIndex nidx, const TCHAR *name);

		virtual const TCHAR *GetNodeName(NodeIndex nidx) const;

		virtual bool NodeVisibility(NodeIndex nidx, std::optional<bool> visible = std::nullopt);

		virtual bool NodeCollidability(NodeIndex nidx, std::optional<bool> collide = std::nullopt);

		virtual void SetTransform(NodeIndex nidx, const glm::fmat4x4 *pmat);

		virtual const glm::fmat4x4 *GetTransform(NodeIndex nidx, glm::fmat4x4 *pmat) const;

		virtual void SetParentNode(NodeIndex nidx, NodeIndex parent_nidx = -1);

		virtual NodeIndex GetParentNode(NodeIndex nidx) const;

		virtual SubMeshIndex AssignMeshToNode(NodeIndex nidx, MeshIndex midx);

		virtual MeshIndex GetMeshFromNode(NodeIndex nidx, SubMeshIndex midx) const;

		virtual size_t GetMeshCountOnNode(NodeIndex nidx) const;

		virtual MeshIndex AddMesh(const Mesh *pmesh);

		virtual const Mesh *GetMesh(MeshIndex midx) const;

		virtual void RemoveMesh(MeshIndex midx);

		virtual size_t GetMeshCount() const;

		virtual void SetMaterial(MeshIndex midx, const Material *pmtl);

		virtual const Material *GetMaterial(MeshIndex midx) const;

		virtual const BoundingBox *GetBounds(BoundingBox *pbb = nullptr) const;

		virtual void GetBoundingSphere(glm::fvec3 *centroid = nullptr, float *radius = nullptr) const;

		virtual void Draw(const glm::fmat4x4 *pmat, bool allow_material_changes, const InstanceData *inst = nullptr, bool force = false) const;

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, size_t *pMeshIndex,
							   float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV, const InstanceData *inst, bool force = false) const;

		virtual const Animation *GetDefaultAnim() const;

		virtual void ProcessNodes(ProcessNodesFunc func);

		void SetDefaultAnim(const Animation *panim);

		void SetBounds(glm::fvec3 &bmin, glm::fvec3 &bmax);

	protected:
		bool DrawNode(NodeIndex nodeidx, const glm::fmat4x4 *pmat, bool allow_material_changes, const InstanceData *inst, bool force) const;

		bool IntersectNode(NodeIndex nodeidx, const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat,
						   float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV, const Model::InstanceData *inst, bool force = false) const;

	};

	DEFINE_RESOURCETYPE(Model, RTFLAG_RUNBYRENDERER, GUID({0x2ea22c05, 0xbf99, 0x493c, { 0xb7, 0xa, 0x26, 0x6f, 0x89, 0xaf, 0x30, 0x91 }}), "Model", "3D Models", "3ds;dae;fbx;gltb;gltf;glb;obj;x", "fbx");

};
