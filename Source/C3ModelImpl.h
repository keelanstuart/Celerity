// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


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

		typedef struct sNodeInfo
		{
			sNodeInfo()
			{
				parent = NO_PARENT;
				mat = glm::identity<glm::fmat4x4>();
			}

			typedef std::vector<MeshIndex> TMeshIndexArray;
			TMeshIndexArray meshes;

			tstring name;

			NodeIndex parent;
			typedef std::vector<NodeIndex> TNodeIndexArray;
			TNodeIndexArray children;

			glm::fmat4x4 mat;

		} SNodeInfo;

		typedef std::vector<SNodeInfo *> TNodeInfoArray;
		TNodeInfoArray m_Nodes;

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

		typedef std::vector<SMeshInfo *> TMeshInfoArray;
		TMeshInfoArray m_Meshes;

		BoundingBox *m_Bounds;

		MatrixStack *m_MatStack;

	public:

		ModelImpl(RendererImpl *prend);

		virtual ~ModelImpl();

		virtual void Release();

		virtual NodeIndex AddNode();

		virtual void RemoveNode(NodeIndex nidx);

		virtual size_t GetNodeCount() const;

		virtual void SetNodeName(NodeIndex nidx, const TCHAR *name);

		virtual const TCHAR *GetNodeName(NodeIndex nidx) const;

		virtual void SetTransform(NodeIndex nidx, const glm::fmat4x4 *pmat);

		virtual const glm::fmat4x4 *GetTransform(NodeIndex nidx, glm::fmat4x4 *pmat) const;

		virtual void SetParent(NodeIndex nidx, NodeIndex parent_nidx = -1);

		virtual NodeIndex GetParent(NodeIndex nidx) const;

		virtual SubMeshIndex AssignMeshToNode(NodeIndex nidx, MeshIndex midx);

		virtual MeshIndex GetMeshFromNode(NodeIndex nidx, SubMeshIndex midx) const;

		virtual size_t GetMeshCountOnNode(NodeIndex nidx) const;

		virtual MeshIndex AddMesh(const Mesh *pmesh);

		virtual const Mesh *GetMesh(MeshIndex midx) const;

		virtual void RemoveMesh(MeshIndex midx);

		virtual size_t GetMeshCount() const;

		virtual void SetMaterial(MeshIndex midx, const Material *pmtl);

		virtual const Material *GetMaterial(MeshIndex midx) const;

		virtual const BoundingBox *GetBounds() const;

		virtual void Draw(const glm::fmat4x4 *pmat) const;

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir,
							   float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV,
							   const glm::fmat4x4 *pmat = nullptr) const;

	protected:
		bool DrawNode(const SNodeInfo *pnode) const;

		bool IntersectNode(const SNodeInfo *pnode, const glm::vec3 *pRayPos, const glm::vec3 *pRayDir,
						   float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV) const;

	};

	DEFINE_RESOURCETYPE(Model, RTFLAG_RUNBYRENDERER, GUID({0x2ea22c05, 0xbf99, 0x493c, { 0xb7, 0xa, 0x26, 0x6f, 0x89, 0xaf, 0x30, 0x91 }}), "Model", "3D Models", "3ds;dae;fbx;gltb;gltf;obj;x", "fbx");

};
