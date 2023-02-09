// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3ModelImpl.h>
#include <C3Resource.h>
#include <C3MaterialImpl.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>

#include <Shlwapi.h>

using namespace c3;


Model *Model::Create(Renderer *prend)
{
	return new ModelImpl((RendererImpl *)prend);
}


ModelImpl::ModelImpl(RendererImpl *prend)
{
	m_pRend = prend;
	m_MatStack = MatrixStack::Create();
	m_Bounds = BoundingBox::Create();
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


const BoundingBox *ModelImpl::GetBounds(BoundingBox *pbb) const
{
	if (!pbb)
		return m_Bounds;

	*pbb = *m_Bounds;
	return pbb;
}


void ModelImpl::Draw(const glm::fmat4x4 *pmat, bool allow_material_changes) const
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
			DrawNode(cit, allow_material_changes);
	}

	if (pmat)
		m_MatStack->Pop();
}


bool ModelImpl::DrawNode(const SNodeInfo *pnode, bool allow_material_changes) const
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

		if (allow_material_changes)
			m_pRend->UseMaterial(mesh->pmtl);

		mesh->pmesh->Draw();
	}

	for (const auto &cit : pnode->children)
	{
		// recursively draw each of the child nodes here
		const SNodeInfo *child = m_Nodes[cit];
		if (!child)
			continue;

		DrawNode(child, allow_material_changes);
	}

	// pop the node's transform
	m_MatStack->Pop();

	return true;
}

bool ModelImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, size_t *pMeshIndex,
							float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV,
							const glm::fmat4x4 *pmat) const
{
	bool ret = false;

	float d = FLT_MAX;

	if (pmat)
		m_MatStack->Push(pmat);

	float mindist = FLT_MAX;

	for (size_t ni = 0, max_ni = m_Nodes.size(); ni < max_ni; ni++)
	{
		const SNodeInfo *node = m_Nodes[ni];
		if (!node)
			continue;

		// from this point, only draw top-level nodes
		if (node->parent == NO_PARENT)
		{
			ret = IntersectNode(node, pRayPos, pRayDir, &d, pFaceIndex, pUV);

			if (ret && (d < mindist))
			{
				if (pMeshIndex)
					*pMeshIndex = ni;

				if (pDistance)
					*pDistance = d;

				d = mindist;
			}
		}
	}

	if (pmat)
		m_MatStack->Pop();

	return ret;
}

bool ModelImpl::IntersectNode(const SNodeInfo *pnode, const glm::vec3 *pRayPos, const glm::vec3 *pRayDir,
				   float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV) const
{
	if (!pnode)
		return false;

	bool ret = false;

	// push the node's transform to build the hierarchy correctly
	m_MatStack->Push(&pnode->mat);

	glm::fmat4x4 m;
	m_MatStack->Top(&m);
	m = glm::inverse(m);

	glm::vec4 rp(pRayPos->x, pRayPos->y, pRayPos->z, 1);
	glm::vec3 rpt = m * rp;

	glm::fmat4x4 mit = glm::inverseTranspose(m);
	glm::vec4 rd(pRayDir->x, pRayDir->y, pRayDir->z, 0);
	glm::vec3 rdt = mit * rd;

	for (const auto &mit : pnode->meshes)
	{
		// render each of the meshes on this node
		const SMeshInfo *mesh = m_Meshes[mit];
		if (!mesh)
			continue;

		float d = FLT_MAX;
		if (mesh->pmesh->Intersect(&rpt, &rdt, &d, pFaceIndex, pUV))
		{
			ret = true;

			if (!pDistance)
				break;

			if (*pDistance > d)
				*pDistance = d;
		}
	}

	for (const auto &cit : pnode->children)
	{
		// recursively draw each of the child nodes here
		const SNodeInfo *child = m_Nodes[cit];
		if (!child)
			continue;

		ret |= IntersectNode(child, &rpt, &rdt, pDistance, pFaceIndex, pUV);
	}

	// pop the node's transform
	m_MatStack->Pop();

	return ret;
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

	glm::fmat4x4 t;
	for (unsigned int j = 0; j < 4; j++)
		for (unsigned int k = 0; k < 4; k++)
			t[j][k] = (float)(pn->mTransformation[k][j]);

	pm->SetTransform(nidx, &t);

	for (size_t i = 0; i < pn->mNumChildren; i++)
		AddModelNode(pm, nidx, pn->mChildren[i], nidxmap, midxmap);

	for (size_t j = 0; j < pn->mNumMeshes; j++)
	{
		TMeshIndexMap::const_iterator cit = midxmap.find(size_t(pn->mMeshes[j]));
		if (cit != midxmap.cend())
			pm->AssignMeshToNode(nidx, cit->second);
	}
}


c3::ResourceType::LoadResult RESOURCETYPENAME(Model)::ReadFromFile(c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const
{
	if (returned_data)
	{
		*returned_data = nullptr;

		char *s;
		CONVERT_TCS2MBCS(filename, s);
		std::string fn = s;

		unsigned int impflags = 0
			//| aiProcess_FlipUVs
			| aiProcess_Triangulate
			| aiProcess_CalcTangentSpace
			| aiProcess_ImproveCacheLocality
			| aiProcess_OptimizeMeshes
			| aiProcess_GenBoundingBoxes
			| aiProcess_SortByPType
			| aiProcess_JoinIdenticalVertices
			| aiProcess_GenUVCoords
			| aiProcess_FixInfacingNormals
			;

		if (_tcsstr(options, _T("force_smooth_normals")))
		{
			impflags |= aiProcess_GenSmoothNormals | aiProcess_ForceGenNormals;
		}
		else if (_tcsstr(options, _T("force_flat_normals")))
		{
			impflags |= aiProcess_GenNormals | aiProcess_ForceGenNormals;
		}
		else
		{
			impflags |= aiProcess_GenNormals;
		}

		Assimp::Importer import;
		const aiScene *scene = import.ReadFile(fn, impflags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			return ResourceType::LoadResult::LR_ERROR;
		}

		RendererImpl *pr = (RendererImpl *)(psys->GetRenderer());
		ModelImpl *pmi = new ModelImpl(pr);

		*returned_data = pmi;
		if (!*returned_data)
			return ResourceType::LoadResult::LR_ERROR;

		std::vector<Material *> mtlvec;
		mtlvec.reserve(scene->mNumMaterials);
		MaterialManager *pmm = pr->GetMaterialManager();

		aiString aipath;
		TCHAR *textmp, *texfilename;
		TCHAR modbasepath[MAX_PATH], texpath[MAX_PATH];

		for (size_t j = 0; j < scene->mNumMaterials; j++)
		{
			Material *pmtl = pmm->CreateMaterial();
			aiMaterial *paim = scene->mMaterials[j];

			glm::vec4 diff(1, 1, 1, 1), emis(0, 0, 0, 0), spec(0, 0, 0, 0);
			float shin = 0;
			for (size_t pidx = 0, maxpidx = paim->mNumProperties; pidx < maxpidx; pidx++)
			{
				aiMaterialProperty *pmp = paim->mProperties[pidx];
				aiString key = pmp->mKey;

				if (!_stricmp(key.C_Str(), "$clr.diffuse") || !_stricmp(key.C_Str(), "$clr.base"))
				{
					memcpy(&diff, pmp->mData, sizeof(aiColor3D));
				}
				else if (!_stricmp(key.C_Str(), "$clr.emissive"))
				{
					memcpy(&emis, pmp->mData, sizeof(aiColor3D));
				}
				else if (!_stricmp(key.C_Str(), "$clr.specular"))
				{
					memcpy(&spec, pmp->mData, sizeof(aiColor3D));
				}
				else if (!_stricmp(key.C_Str(), "$clr.shininess"))
				{
					memcpy(&shin, pmp->mData, sizeof(float));
				}
				else if (!_stricmp(key.C_Str(), "$mat.twosided"))
				{
					if (*((uint8_t *)pmp->mData) == 1)
						pmtl->RenderModeFlags().Set(Material::RENDERMODEFLAG(Material::RMF_RENDERBACK));
				}
			}
			pmtl->SetColor(Material::ColorComponentType::CCT_DIFFUSE, &diff);
			pmtl->SetColor(Material::ColorComponentType::CCT_EMISSIVE, &emis);
			pmtl->SetColor(Material::ColorComponentType::CCT_SPECULAR, &spec);

			TCHAR difftexpath[MAX_PATH], texpathtemp[MAX_PATH];

			_tcscpy_s(modbasepath, filename);
			TCHAR *c = modbasepath;
			while (*c) { if (*c == _T('/')) { *c = _T('\\'); } c++; }
			PathRemoveFileSpec(modbasepath);

			props::TFlags64 rf = 0; //c3::ResourceManager::RESFLAG(c3::ResourceManager::DEMANDLOAD);

			bool has_difftex = false;
			if (true == (has_difftex = (paim->GetTextureCount(aiTextureType_DIFFUSE) > 0)))
				paim->GetTexture(aiTextureType_DIFFUSE, 0, &aipath);
			else if (!has_difftex && (true == (has_difftex = (paim->GetTextureCount(aiTextureType_BASE_COLOR) > 0))))
				paim->GetTexture(aiTextureType_BASE_COLOR, 0, &aipath);

			// locally-defined function to set texture filenames for specific types of textures
			auto SetTextureFileName = [&](Material::TextureComponentType t, const char *s)
			{
				CONVERT_MBCS2TCS(s, textmp);
				if (textmp && (*textmp != '*'))
				{
					PathCombine(texpath, modbasepath, textmp);
					bool loctex = psys->GetFileMapper()->FindFile(texpath);
					if (!loctex)
					{
						TCHAR *fnstart = PathFindFileName(textmp);
						PathCombine(texpath, modbasepath, fnstart);
						loctex = psys->GetFileMapper()->FindFile(texpath);
					}
					texfilename = loctex ? texpath : PathFindFileName(texpath);
					if (t == Material::TextureComponentType::TCT_DIFFUSE)
						_tcsncpy_s(difftexpath, textmp, MAX_PATH - 1);
					pmtl->SetTexture(t, psys->GetResourceManager()->GetResource(texfilename, rf));
				}
				else
				{
					textmp++;
					size_t texidx = _ttoi(textmp);
					if (scene->HasTextures() && texidx < scene->mNumTextures)
					{
						int w, h, c;
						stbi_uc *data = stbi_load_from_memory((const stbi_uc *)(scene->mTextures[texidx]->pcData),
							scene->mTextures[texidx]->mWidth, &w, &h, &c, 0);
						if (data)
						{
							c3::Texture2D *pt = psys->GetRenderer()->CreateTexture2D(w, h, (c3::Renderer::TextureType)(c3::Renderer::TextureType::U8_1CH + c - 1));
							if (pt)
							{
								void *buf;
								Texture2D::SLockInfo li;
								if ((pt->Lock(&buf, li, 0, TEXLOCKFLAG_WRITE | TEXLOCKFLAG_GENMIPS) == Texture2D::RETURNCODE::RET_OK) && buf)
								{
									memcpy(buf, data, w * h * c);

									pt->Unlock();
								}

								tstring texname = filename;
								texname += _T(":texture[");
								texname += textmp;
								texname += _T("]");
								pt->SetName(texname.c_str());
								pmtl->SetTexture(t, pt);
								psys->GetResourceManager()->GetResource(texname.c_str(),
									RESF_CREATEENTRYONLY,
									psys->GetResourceManager()->FindResourceTypeByName(_T("Texture2D")),
									pt);
							}

							free(data);
						}
					}
				}
			};

			if (has_difftex)
			{
				SetTextureFileName(Material::TextureComponentType::TCT_DIFFUSE, aipath.C_Str());
			}

			bool has_normtex = false;
			if (true == (has_normtex = (paim->GetTextureCount(aiTextureType_NORMALS) > 0)))
				paim->GetTexture(aiTextureType_NORMALS, 0, &aipath);
			else if (!has_normtex && (true == (has_normtex = (paim->GetTextureCount(aiTextureType_NORMAL_CAMERA) > 0))))
				paim->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &aipath);
			else if (has_difftex && MaterialImpl::s_pfAltTexFilenameFunc && ((has_normtex = MaterialImpl::s_pfAltTexFilenameFunc(difftexpath, Material::TextureComponentType::TCT_NORMAL, texpathtemp, MAX_PATH - 1)) == true))
			{
				char *c;
				CONVERT_TCS2MBCS(texpathtemp, c);
				aipath = c;
			}

			if (has_normtex)
			{
				SetTextureFileName(Material::TextureComponentType::TCT_NORMAL, aipath.C_Str());
			}

			bool has_emistex = false;
			if (true == (has_emistex = (paim->GetTextureCount(aiTextureType_EMISSIVE) > 0)))
				paim->GetTexture(aiTextureType_EMISSIVE, 0, &aipath);
			else if (!has_emistex && (true == (has_emistex = (paim->GetTextureCount(aiTextureType_EMISSION_COLOR) > 0))))
				paim->GetTexture(aiTextureType_EMISSION_COLOR, 0, &aipath);
			else if (has_difftex && MaterialImpl::s_pfAltTexFilenameFunc && ((has_emistex = MaterialImpl::s_pfAltTexFilenameFunc(difftexpath, Material::TextureComponentType::TCT_EMISSIVE, texpathtemp, MAX_PATH - 1)) == true))
			{
				char *c;
				CONVERT_TCS2MBCS(texpathtemp, c);
				aipath = c;
			}

			if (has_emistex)
			{
				SetTextureFileName(Material::TextureComponentType::TCT_EMISSIVE, aipath.C_Str());
			}

			// metalness / roughness / ao -- all together
			unsigned int tidx_pbrmtl = paim->GetTextureCount(aiTextureType_PBR_MTL);
			bool has_surftex = (tidx_pbrmtl != 0);
			unsigned int tidx_metalness, tidx_roughness, tidx_ambocc;
			if (!has_surftex)
			{
				tidx_metalness = paim->GetTextureCount(aiTextureType_METALNESS);
				tidx_roughness = paim->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS);
				tidx_ambocc = paim->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION);
				has_surftex = (tidx_metalness > 0) && (tidx_roughness == tidx_metalness) && (tidx_ambocc == tidx_metalness);
				if (has_surftex)
					paim->GetTexture(aiTextureType_METALNESS, 0, &aipath);
			}
			else
			{
				tidx_metalness = tidx_roughness = tidx_ambocc = tidx_pbrmtl;
				if (has_surftex)
					paim->GetTexture(aiTextureType_PBR_MTL, 0, &aipath);
			}

			if (!has_surftex && has_difftex && MaterialImpl::s_pfAltTexFilenameFunc && ((has_surftex = MaterialImpl::s_pfAltTexFilenameFunc(difftexpath, Material::TextureComponentType::TCT_SURFACEDESC, texpathtemp, MAX_PATH - 1)) == true))
			{
				char *c;
				CONVERT_TCS2MBCS(texpathtemp, c);
				aipath = c;
			}

			if (has_surftex)
			{
				SetTextureFileName(Material::TextureComponentType::TCT_SURFACEDESC, aipath.C_Str());
			}

			mtlvec.push_back(pmtl);
		}

		TMeshIndexMap mim;

		glm::fvec3 vmin(FLT_MAX, FLT_MAX, FLT_MAX), vmax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			Mesh *pm = psys->GetRenderer()->CreateMesh();

			VertexBuffer *pvb = psys->GetRenderer()->CreateVertexBuffer();
			IndexBuffer *pib = psys->GetRenderer()->CreateIndexBuffer();

			Model::MeshIndex midx = pmi->AddMesh(pm);
			if (midx == Model::INVALID_INDEX)
				continue;

			unsigned int mtlidx = scene->mMeshes[i]->mMaterialIndex;
			pmi->SetMaterial(midx, (mtlidx < mtlvec.size()) ? mtlvec[mtlidx] : pr->GetWhiteMaterial());

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

			for (unsigned int t = 0; t < 4; t++)
			{
				if (scene->mMeshes[i]->HasTextureCoords(t))
				{
					vcd[ci].m_Count = scene->mMeshes[i]->mNumUVComponents[t];
					vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_F32;
					vcd[ci].m_Usage = (VertexBuffer::ComponentDescription::EUsage)(VertexBuffer::ComponentDescription::EUsage::VU_TEXCOORD0 + t);
					ci++;
				}
				else
					break;	// can't have uv set #2 without #1, right?
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
				else
					break;	// can't have vertex color set #2 without #1, right?
			}

			vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_NONE;

			ci = 0;
			size_t vsz = 0;
			while (vcd[ci].m_Count)
			{
				vsz += vcd[ci++].size();
			}

			size_t vct = scene->mMeshes[i]->mNumVertices;

			BYTE *vbuf;
			if (pvb->Lock((void **)&vbuf, vct, vcd, VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE) == VertexBuffer::RETURNCODE::RET_OK)
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

							if (pmd_3f->x < vmin.x)
								vmin.x = pmd_3f->x;
							if (pmd_3f->y < vmin.y)
								vmin.y = pmd_3f->y;
							if (pmd_3f->z < vmin.z)
								vmin.z = pmd_3f->z;

							if (pmd_3f->x > vmax.x)
								vmax.x = pmd_3f->x;
							if (pmd_3f->y > vmax.y)
								vmax.y = pmd_3f->y;
							if (pmd_3f->z > vmax.z)
								vmax.z = pmd_3f->z;

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
							// this may seem wrong because you could have texture coordinates with 2 components...
							// but only the relevant ones are copied, based on the count defined by the vertex component
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

							*((uint32_t *)pv) = c;

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
			if (pib->Lock((void **)&ibuf, numfaces * 3, large_indices ? IndexBuffer::EIndexSize::IS_32BIT : IndexBuffer::EIndexSize::IS_16BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
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
						ii += sizeof(uint32_t) * 3;
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


c3::ResourceType::LoadResult RESOURCETYPENAME(Model)::ReadFromMemory(c3::System *psys, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
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
