// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#include "pch.h"

#include <C3ModelImpl.h>
#include <C3Resource.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace c3;


ModelImpl::ModelImpl(RendererImpl *prend)
{
	m_pRend = prend;
	m_MatStack = MatrixStack::Create();
}


ModelImpl::~ModelImpl()
{
	m_MatStack->Release();
}


void ModelImpl::Release()
{

	delete this;
}


Model::NodeIndex ModelImpl::AddNode()
{
	SNodeInfo *pni = new SNodeInfo();

	NodeIndex ret = m_Nodes.size();
	m_Nodes.push_back(pni);

	return ret;
}


void ModelImpl::RemoveNode(NodeIndex nidx)
{
	if (nidx < m_Nodes.size())
	{
		SNodeInfo *node = m_Nodes[nidx];
		if (node)
			delete node;

		m_Nodes[nidx] = nullptr;
	}
}


size_t ModelImpl::GetNodeCount() const
{
	return m_Nodes.size();
}


void ModelImpl::SetNodeName(NodeIndex nidx, const TCHAR *name)
{
	if (nidx < m_Nodes.size())
	{
		SNodeInfo *node = m_Nodes[nidx];
		if (node)
			node->name = name ? name : _T("");
	}
}


const TCHAR *ModelImpl::GetNodeName(NodeIndex nidx) const
{
	if (nidx < m_Nodes.size())
	{
		SNodeInfo *node = m_Nodes[nidx];
		if (node)
			return node->name.c_str();
	}

	return nullptr;
}


void ModelImpl::SetTransform(NodeIndex nidx, const glm::fmat4x4 *pmat)
{
	if (nidx < m_Nodes.size())
	{
		SNodeInfo *node = m_Nodes[nidx];
		if (node)
			node->mat = pmat ? *pmat : glm::identity<glm::fmat4x4>();
	}
}


const glm::fmat4x4 *ModelImpl::GetTransform(NodeIndex nidx, glm::fmat4x4 *pmat) const
{
	if (nidx < m_Nodes.size())
	{
		SNodeInfo *node = m_Nodes[nidx];
		if (node)
		{
			if (pmat)
			{
				*pmat = node->mat;
				return pmat;
			}

			return &(node->mat);
		}
	}

	return nullptr;
}


void ModelImpl::SetParent(NodeIndex nidx, NodeIndex parent_nidx)
{
	if (nidx < m_Nodes.size())
	{
		SNodeInfo *node = m_Nodes[nidx];
		if (node && (node->parent != parent_nidx))
		{
			if (node->parent != NO_PARENT)
			{
				SNodeInfo *old_par = m_Nodes[node->parent];
				if (old_par)
				{
					SNodeInfo::TNodeIndexArray::iterator it = std::find(old_par->children.begin(), old_par->children.end(), nidx);
					if (it != old_par->children.end())
						old_par->children.erase(it);
				}
			}

			node->parent = parent_nidx;
			if (node->parent != NO_PARENT)
			{
				SNodeInfo *new_par = m_Nodes[node->parent];
				if (new_par)
				{
					SNodeInfo::TNodeIndexArray::iterator it = std::find(new_par->children.begin(), new_par->children.end(), nidx);
					if (it == new_par->children.end())
						new_par->children.push_back(nidx);
				}
			}
		}
	}
}


Model::NodeIndex ModelImpl::GetParent(Model::NodeIndex nidx) const
{
	if (nidx < m_Nodes.size())
	{
		SNodeInfo *node = m_Nodes[nidx];
		if (node)
			return node->parent;
	}

	return NO_PARENT;
}


Model::SubMeshIndex ModelImpl::AssignMeshToNode(NodeIndex nidx, MeshIndex midx)
{
	if (midx >= m_Meshes.size())
		return INVALID_INDEX;

	if (nidx < m_Nodes.size())
	{
		SNodeInfo *node = m_Nodes[nidx];
		if (node)
		{
			size_t ret = node->meshes.size();
			node->meshes.push_back(midx);
			return ret;
		}
	}

	return INVALID_INDEX;
}


Model::MeshIndex ModelImpl::GetMeshFromNode(NodeIndex nidx, SubMeshIndex midx) const
{
	if (nidx < m_Nodes.size())
	{
		SNodeInfo *node = m_Nodes[nidx];
		if (node)
		{
			size_t ret = node->meshes.size();
			node->meshes.push_back(midx);
			return ret;
		}
	}

	return INVALID_INDEX;
}


size_t ModelImpl::GetMeshCountOnNode(NodeIndex nidx) const
{
	size_t ret = 0;

	if (nidx < m_Nodes.size())
	{
		SNodeInfo *node = m_Nodes[nidx];
		if (node)
			ret = node->meshes.size();
	}

	return ret;
}


Model::MeshIndex ModelImpl::AddMesh(const Mesh *pmesh)
{
	SMeshInfo *pmi = new SMeshInfo();
	if (!pmi)
		return INVALID_INDEX;

	pmi->pmesh = pmesh;

	MeshIndex ret = m_Meshes.size();
	m_Meshes.push_back(pmi);

	return ret;
}


const Mesh *ModelImpl::GetMesh(MeshIndex midx) const
{
	if (midx < m_Meshes.size())
	{
		SMeshInfo *pmi = m_Meshes[midx];
		if (pmi)
			return pmi->pmesh;
	}

	return nullptr;
}


void ModelImpl::RemoveMesh(MeshIndex midx)
{
	if (midx < m_Meshes.size())
	{
		SMeshInfo *pmi = m_Meshes[midx];
		if (pmi)
		{
			if (pmi->pmesh)
			{
				((Mesh *)(pmi->pmesh))->Release();
				pmi->pmesh = nullptr;
			}

			if (pmi->pmtl)
			{
				((Material *)(pmi->pmtl))->Release();
				pmi->pmtl = nullptr;
			}

			delete pmi;
		}

		m_Meshes[midx] = nullptr;
	}
}


size_t ModelImpl::GetMeshCount() const
{
	return m_Meshes.size();
}


void ModelImpl::SetMaterial(MeshIndex midx, const Material *pmtl)
{
	if (midx < m_Meshes.size())
	{
		SMeshInfo *pmi = m_Meshes[midx];
		if (pmi)
		{
			if (pmi->pmtl)
			{
				((Material *)(pmi->pmtl))->Release();
				pmi->pmtl = nullptr;
			}

			pmi->pmtl = pmtl;
		}
	}
}


const Material *ModelImpl::GetMaterial(MeshIndex midx) const
{
	if (midx < m_Meshes.size())
	{
		SMeshInfo *pmi = m_Meshes[midx];
		if (pmi)
			return pmi->pmtl;
	}

	return nullptr;
}


const Frustum *ModelImpl::GetBounds() const
{
	return nullptr;
}


void ModelImpl::Draw(const glm::fmat4x4 *pmat) const
{
	if (pmat)
		m_MatStack->Push(pmat);

	for (const auto cit : m_Nodes)
	{
		const SNodeInfo *node = cit;
		if (!node)
			continue;

		// from this point, only draw top-level nodes
		if (node->parent == NO_PARENT)
			DrawNode(cit);
	}

	if (pmat)
		m_MatStack->Pop();
}


bool ModelImpl::DrawNode(const SNodeInfo *pnode) const
{
	if (!pnode)
		return false;

	// push the node's transform to build the hierarchy correctly
	m_MatStack->Push(&pnode->mat);

	glm::fmat4x4 m;

	// set up the world matrix to draw meshes at this node level
	m_pRend->SetWorldMatrix(m_MatStack->Top(&m));

	for (const auto &mit : pnode->meshes)
	{
		// render each of the meshes on this node
		const SMeshInfo *mesh = m_Meshes[mit];
		if (!mesh)
			continue;

		if (mesh->pmtl)
		{
			// apply the material for this mesh now
		}

		mesh->pmesh->Draw();
	}

	for (const auto &cit : pnode->children)
	{
		// recursively draw each of the child nodes here
		const SNodeInfo *child = m_Nodes[cit];
		if (!child)
			continue;

		DrawNode(child);
	}

	// pop the node's transform
	m_MatStack->Pop();

	return true;
}



DECLARE_RESOURCETYPE(Model);

typedef std::map<const aiNode *, Model::NodeIndex> TNodeIndexMap;
typedef std::map<size_t, Model::MeshIndex> TMeshIndexMap;

void AddModelNode(ModelImpl *pm, Model::NodeIndex parent_nidx, aiNode *pn, TNodeIndexMap &nidxmap, TMeshIndexMap &midxmap)
{
	if (!pn || !pm)
		return;

	Model::NodeIndex nidx = pm->AddNode();
	nidxmap.insert(TNodeIndexMap::value_type(pn, nidx));

	TCHAR *name;
	CONVERT_MBCS2TCS(pn->mName.C_Str(), name);

	pm->SetNodeName(nidx, name);
	pm->SetParent(nidx, parent_nidx);
	pm->SetTransform(nidx, (const glm::fmat4x4 *)&(pn->mTransformation));

	for (size_t i = 0; i < pn->mNumChildren; i++)
		AddModelNode(pm, nidx, pn->mChildren[i], nidxmap, midxmap);

	for (size_t j = 0; j < pn->mNumMeshes; j++)
	{
		TMeshIndexMap::const_iterator cit = midxmap.find(size_t(pn->mMeshes[j]));
		if (cit != midxmap.cend())
			pm->AssignMeshToNode(nidx, cit->second);
	}
}


c3::ResourceType::LoadResult RESOURCETYPENAME(Model)::ReadFromFile(c3::System *psys, const TCHAR *filename, void **returned_data) const
{
	if (returned_data)
	{
		*returned_data = nullptr;

		char *s;
		CONVERT_TCS2MBCS(filename, s);
		std::string fn = s;

		Assimp::Importer import;
		const aiScene *scene = import.ReadFile(fn,
			aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | 
			aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_ImproveCacheLocality);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			return ResourceType::LoadResult::LR_ERROR;
		}

		ModelImpl *pmi = new ModelImpl((RendererImpl *)(psys->GetRenderer()));

		*returned_data = pmi;
		if (!*returned_data)
			return ResourceType::LoadResult::LR_ERROR;

		TMeshIndexMap mim;

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			VertexBuffer *pvb = psys->GetRenderer()->CreateVertexBuffer();
			IndexBuffer *pib = psys->GetRenderer()->CreateIndexBuffer();
			Mesh *pm = psys->GetRenderer()->CreateMesh();

			Model::MeshIndex midx = pmi->AddMesh(pm);
			if (midx == Model::INVALID_INDEX)
				continue;

			// associate the aiMesh with a MeshIndex so we can attach it to the appropriate ModelIndex
			mim.insert(TMeshIndexMap::value_type(i, midx));

			VertexBuffer::ComponentDescription vcd[VertexBuffer::ComponentDescription::EUsage::VU_NUM_USAGES + 1];
			memset(vcd, 0, sizeof(VertexBuffer::ComponentDescription) * (VertexBuffer::ComponentDescription::EUsage::VU_NUM_USAGES + 1));

			size_t ci = 0;

			if (scene->mMeshes[i]->HasPositions())
			{
				vcd[ci].m_Count = 3;
				vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_F32;
				vcd[ci].m_Usage = VertexBuffer::ComponentDescription::EUsage::VU_POSITION;
				ci++;
			}

			if (scene->mMeshes[i]->HasNormals())
			{
				vcd[ci].m_Count = 3;
				vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_F32;
				vcd[ci].m_Usage = VertexBuffer::ComponentDescription::EUsage::VU_NORMAL;
				ci++;
			}

#if 0
			if (scene->mMeshes[i]->HasTangentsAndBitangents())
			{
				vcd[ci].m_Count = 3;
				vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_F32;
				vcd[ci].m_Usage = VertexBuffer::ComponentDescription::EUsage::VU_TANGENT;
				ci++;

				vcd[ci].m_Count = 3;
				vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_F32;
				vcd[ci].m_Usage = VertexBuffer::ComponentDescription::EUsage::VU_BINORMAL;
				ci++;
			}
#endif

			for (unsigned int t = 0; t < 4; t++)
			{
				if (scene->mMeshes[i]->HasTextureCoords(t))
				{
					vcd[ci].m_Count = scene->mMeshes[i]->mNumUVComponents[t];
					vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_F32;
					vcd[ci].m_Usage = (VertexBuffer::ComponentDescription::EUsage)(VertexBuffer::ComponentDescription::EUsage::VU_TEXCOORD0 + t);
					ci++;
				}
			}

			for (unsigned int c = 0; c < 4; c++)
			{
				if (scene->mMeshes[i]->HasVertexColors(c))
				{
					vcd[ci].m_Count = 4;
					vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_U8;
					vcd[ci].m_Usage = (VertexBuffer::ComponentDescription::EUsage)(VertexBuffer::ComponentDescription::EUsage::VU_COLOR0 + c);
					ci++;
				}
			}

			ci = 0;
			size_t vsz = 0;
			while (vcd[ci].m_Count)
			{
				vsz += vcd[ci++].size();
			}

			size_t vct = scene->mMeshes[i]->mNumVertices;

			BYTE *vbuf;
			if (pvb->Lock((void **)&vbuf, vct, vcd, VBLOCKFLAG_WRITE) == VertexBuffer::RETURNCODE::RET_OK)
			{
				ci = 0;
				size_t ofs = 0;
				while (vcd[ci].m_Count)
				{
					BYTE *pv = vbuf + ofs;

					aiVector3D *pmd_3f = nullptr;
					aiColor4D *pmd_c = nullptr;

					switch (vcd[ci].m_Usage)
					{
						case VertexBuffer::ComponentDescription::EUsage::VU_POSITION:
							pmd_3f = scene->mMeshes[i]->mVertices;
							break;

						case VertexBuffer::ComponentDescription::EUsage::VU_NORMAL:
							pmd_3f = scene->mMeshes[i]->mNormals;
							break;

						case VertexBuffer::ComponentDescription::EUsage::VU_BINORMAL:
							pmd_3f = scene->mMeshes[i]->mBitangents;
							break;

						case VertexBuffer::ComponentDescription::EUsage::VU_TANGENT:
							pmd_3f = scene->mMeshes[i]->mTangents;
							break;

						case VertexBuffer::ComponentDescription::EUsage::VU_TEXCOORD0:
						case VertexBuffer::ComponentDescription::EUsage::VU_TEXCOORD1:
						case VertexBuffer::ComponentDescription::EUsage::VU_TEXCOORD2:
						case VertexBuffer::ComponentDescription::EUsage::VU_TEXCOORD3:
							pmd_3f = scene->mMeshes[i]->mTextureCoords[(size_t)(vcd[ci].m_Usage - VertexBuffer::ComponentDescription::EUsage::VU_TEXCOORD0)];
							break;

						case VertexBuffer::ComponentDescription::EUsage::VU_COLOR0:
						case VertexBuffer::ComponentDescription::EUsage::VU_COLOR1:
						case VertexBuffer::ComponentDescription::EUsage::VU_COLOR2:
						case VertexBuffer::ComponentDescription::EUsage::VU_COLOR3:
							pmd_c = scene->mMeshes[i]->mColors[(size_t)(vcd[ci].m_Usage - VertexBuffer::ComponentDescription::EUsage::VU_COLOR0)];
							break;
					}

					if (pmd_3f)
					{
						for (size_t j = 0; j < vct; j++)
						{
							memcpy(pv, &pmd_3f[j], sizeof(float) * vcd[ci].m_Count);

							pv += vsz;
						}
					}
					else if (pmd_c)
					{
						for (size_t j = 0; j < vct; j++)
						{
							BYTE r = BYTE(std::min(std::max(0.0f, pmd_c[j].r), 1.0f) * 255.0f);
							BYTE g = BYTE(std::min(std::max(0.0f, pmd_c[j].g), 1.0f) * 255.0f);
							BYTE b = BYTE(std::min(std::max(0.0f, pmd_c[j].b), 1.0f) * 255.0f);
							BYTE a = BYTE(std::min(std::max(0.0f, pmd_c[j].a), 1.0f) * 255.0f);
							uint32_t c = r | (g << 8) | (b << 16) | (a << 24);

							((uint32_t *)pv)[j] = c;

							pv += vsz;
						}
					}

					ofs += vcd[ci].size();
					ci++;
				}

				pvb->Unlock();
				pm->AttachVertexBuffer(pvb);
			}

			size_t numfaces = 0;
			for (size_t k = 0; k < scene->mMeshes[i]->mNumFaces; k++)
			{
				aiFace pf = scene->mMeshes[i]->mFaces[k];
				if (pf.mNumIndices == 3)
					numfaces++;
			}

			BYTE *ibuf;
			bool large_indices = (scene->mMeshes[i]->mNumVertices >= SHRT_MAX);
			if (pib->Lock((void **)&ibuf, numfaces * 3, large_indices ? IndexBuffer::EIndexSize::IS_32BIT : IndexBuffer::EIndexSize::IS_16BIT, IBLOCKFLAG_WRITE) == IndexBuffer::RETURNCODE::RET_OK)
			{
				size_t ii = 0;

				if (large_indices)
				{
					for (size_t k = 0; k < numfaces; k++)
					{
						aiFace pf = scene->mMeshes[i]->mFaces[k];
						if (pf.mNumIndices != 3)
							continue;

						memcpy(&ibuf[ii], &pf.mIndices[0], sizeof(uint32_t) * 3);
						ii += 3;
					}
				}
				else
				{
					for (size_t k = 0; k < numfaces; k++)
					{
						aiFace pf = scene->mMeshes[i]->mFaces[k];
						if (pf.mNumIndices != 3)
							continue;

						((uint16_t *)ibuf)[ii++] = (uint16_t)pf.mIndices[0];
						((uint16_t *)ibuf)[ii++] = (uint16_t)pf.mIndices[1];
						((uint16_t *)ibuf)[ii++] = (uint16_t)pf.mIndices[2];
					}
				}

				pib->Unlock();
				pm->AttachIndexBuffer(pib);
			}
		}

		TNodeIndexMap nim;
		AddModelNode(pmi, Model::NO_PARENT, scene->mRootNode, nim, mim);
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(Model)::ReadFromMemory(c3::System *psys, const BYTE *buffer, size_t buffer_length, void **returned_data) const
{
	return ResourceType::LoadResult::LR_ERROR;
}


bool RESOURCETYPENAME(Model)::WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const
{
	return false;
}


void RESOURCETYPENAME(Model)::Unload(void *data) const
{
	((Mesh *)data)->Release();
}
