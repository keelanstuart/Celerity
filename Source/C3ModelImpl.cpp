// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3ModelImpl.h>
#include <C3Resource.h>
#include <C3MaterialImpl.h>
#include <C3MeshImpl.h>
#include <C3AnimationImpl.h>
#include <C3Math.h>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
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
	m_ScratchBounds = BoundingBox::Create();
	m_DefaultAnim = nullptr;
}


ModelImpl::~ModelImpl()
{
}


void ModelImpl::Release()
{
	C3_SAFERELEASE(m_MatStack);
	C3_SAFERELEASE(m_Bounds);
	C3_SAFERELEASE(m_ScratchBounds);

	delete this;
}


Model::InstanceData *ModelImpl::CloneInstanceData()
{
	ModelImpl::InstanceDataImpl *ret = new ModelImpl::InstanceDataImpl(this);

	return (InstanceData *)ret;
}


Model::NodeIndex ModelImpl::AddNode()
{
	SNodeInfo *pni = new SNodeInfo();
	pni->flags.Set(NodeFlag::VISIBLE | NodeFlag::COLLIDE);

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


bool ModelImpl::FindNode(const TCHAR *name, NodeIndex *pidx, bool case_sensitive) const
{
	std::function<int(const TCHAR *, const TCHAR *)> cmpfunc = case_sensitive ? _tcscmp : _tcsicmp;

	for (size_t i = 0, maxi = m_Nodes.size(); i < maxi; i++)
	{
		if (!cmpfunc(m_Nodes[i]->name.c_str(), name))
		{
			if (pidx)
				*pidx = (int)i;

			return true;
		}
	}

	return false;
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


bool ModelImpl::NodeVisibility(NodeIndex nidx, std::optional<bool> visible)
{
	if (nidx < m_Nodes.size())
	{
		SNodeInfo *node = m_Nodes[nidx];
		if (node)
		{
			if (visible.has_value())
			{
				if (*visible)
					node->flags.Set(NodeFlag::VISIBLE);
				else
					node->flags.Clear(NodeFlag::VISIBLE);
			}

			return node->flags.IsSet(NodeFlag::VISIBLE);
		}
	}

	return false;
}


bool ModelImpl::NodeCollidability(NodeIndex nidx, std::optional<bool> collide)
{
	if (nidx < m_Nodes.size())
	{
		SNodeInfo *node = m_Nodes[nidx];
		if (node)
		{
			if (collide.has_value())
			{
				if (*collide)
					node->flags.Set(NodeFlag::COLLIDE);
				else
					node->flags.Clear(NodeFlag::COLLIDE);
			}

			return node->flags.IsSet(NodeFlag::COLLIDE);
		}
	}

	return false;
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


void ModelImpl::SetParentNode(NodeIndex nidx, NodeIndex parent_nidx)
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


Model::NodeIndex ModelImpl::GetParentNode(Model::NodeIndex nidx) const
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
			if (midx < node->meshes.size())
				return node->meshes[midx];
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


void ModelImpl::Draw(const glm::fmat4x4 *pmat, bool allow_material_changes, const InstanceData *inst) const
{
	if (inst && (((InstanceDataImpl *)inst)->m_pSourceModel != this))
		return;

	if (pmat)
		m_MatStack->Push(pmat);

	for (size_t i = 0, maxi = m_Nodes.size(); i < maxi; i++)
	{
		// from this point, only draw top-level nodes
		if (m_Nodes[i]->parent == NO_PARENT)
			DrawNode(i, allow_material_changes, inst);
	}

	if (pmat)
		m_MatStack->Pop();
}


bool ModelImpl::DrawNode(NodeIndex nodeidx, bool allow_material_changes, const Model::InstanceData *inst) const
{
	const SNodeInfo *pnode = m_Nodes[nodeidx];

	if (!pnode || !pnode->flags.IsSet(NodeFlag::VISIBLE))
		return false;

	// push the node's transform to build the hierarchy correctly
	if (inst)
		m_MatStack->Push(&(((InstanceDataImpl *)inst)->m_NodeMat[nodeidx]));
	else
		m_MatStack->Push(&pnode->mat);

	glm::fmat4x4 m;

	// set up the world matrix to draw meshes at this node level
	m_pRend->SetWorldMatrix(m_MatStack->Top(&m));

	MeshIndex maxi = pnode->meshes.size();
	for (MeshIndex i = 0; i < maxi; i++)
	{
		// render each of the meshes on this node
		const SMeshInfo *mesh = m_Meshes[pnode->meshes[i]];
		assert(mesh != nullptr);

		if (allow_material_changes)
		{
			if (!inst)
				m_pRend->UseMaterial(mesh->pmtl);
			else
				m_pRend->UseMaterial(((InstanceDataImpl *)inst)->m_NodeMtl[nodeidx][i]);
		}

		RenderMethod *prm = m_pRend->GetActiveRenderMethod();

		mesh->pmesh->Draw();
	}

	NodeIndex maxc = pnode->children.size();
	for (NodeIndex c = 0; c < maxc; c++)
	{
		assert(m_Nodes[pnode->children[c]] != nullptr);

		// recursively draw each of the child nodes here
		DrawNode(pnode->children[c], allow_material_changes, inst);
	}

	// pop the node's transform
	m_MatStack->Pop();

	return true;
}

bool ModelImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats, size_t *pMeshIndex,
							float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV) const
{
	bool ret = false;

	float d = FLT_MAX;

	float mindist = FLT_MAX;

	for (size_t ni = 0, max_ni = m_Nodes.size(); ni < max_ni; ni++)
	{
		const SNodeInfo *node = m_Nodes[ni];
		if (!node)
			continue;

		// from this point, only draw top-level nodes
		if (node->parent == NO_PARENT)
		{
			ret = IntersectNode(node, pRayPos, pRayDir, mats, &d, pFaceIndex, pUV);

			if (ret && (d < mindist))
			{
				if (pMeshIndex)
					*pMeshIndex = ni;

				if (pDistance)
					*pDistance = d;

				mindist = d;
			}
		}
	}

	return ret;
}

bool ModelImpl::IntersectNode(const SNodeInfo *pnode, const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats,
				   float *pDistance, size_t *pFaceIndex, glm::vec2 *pUV) const
{
	if (!pnode || !pnode->flags.IsSet(NodeFlag::COLLIDE))
		return false;

	bool ret = false;

	// push the node's transform to build the hierarchy correctly
	mats->Push(&pnode->mat);

	float tmpdist = FLT_MAX;
	if (!pDistance)
		pDistance = &tmpdist;

	float d = FLT_MAX;
	for (const auto &mit : pnode->meshes)
	{
		// render each of the meshes on this node
		const SMeshInfo *mesh = m_Meshes[mit];
		if (!mesh)
			continue;

		float tmpd = d;
		if (mesh->pmesh->Intersect(pRayPos, pRayDir, &tmpd, pFaceIndex, pUV, mats->Top()))
		{
			if (tmpd < d)
				d = tmpd;

			if (d < *pDistance)
			{
				*pDistance = d;
				ret = true;
			}
		}
	}

	for (const auto &cit : pnode->children)
	{
		// recursively draw each of the child nodes here
		const SNodeInfo *child = m_Nodes[cit];
		if (!child)
			continue;

		ret |= IntersectNode(child, pRayPos, pRayDir, mats, pDistance, pFaceIndex, pUV);
	}

	// pop the node's transform
	mats->Pop();

	return ret;
}


void ModelImpl::SetBounds(glm::fvec3 &bmin, glm::fvec3 &bmax)
{
	m_Bounds->SetExtents(&bmin, &bmax);
}


void ModelImpl::GetBoundingSphere(glm::fvec3 *centroid, float *radius) const
{
	if (centroid)
		*centroid = m_BoundingCentroid;

	if (radius)
		*radius = m_BoundingRadius;
}


DECLARE_RESOURCETYPE(Model);

typedef std::map<const aiNode *, Model::NodeIndex> TNodeIndexMap;
typedef std::map<size_t, Model::MeshIndex> TMeshIndexMap;

void AddModelNode(ModelImpl *pm, Model::NodeIndex parent_nidx, aiNode *pn, aiMatrix4x4 *pnxform, TNodeIndexMap &nidxmap, TMeshIndexMap &midxmap)
{
	if (!pn || !pm)
		return;

	Model::NodeIndex nidx;

	TCHAR *name;
	CONVERT_MBCS2TCS(pn->mName.C_Str(), name);

#if 0
	if (pm->FindNode(name, &nidx, true))
		return;
#endif

	nidx = pm->AddNode();
	nidxmap.insert(TNodeIndexMap::value_type(pn, nidx));

	pm->SetNodeName(nidx, name);

	glm::fmat4x4 t;
	if (pnxform)
	{
		for (unsigned int j = 0; j < 4; j++)
			for (unsigned int k = 0; k < 4; k++)
				t[j][k] = (float)((*pnxform)[k][j]);
	}
	else
	{
		t = glm::identity<glm::fmat4x4>();
	}

	pm->SetTransform(nidx, &t);

	for (size_t j = 0; j < pn->mNumMeshes; j++)
	{
		TMeshIndexMap::const_iterator cit = midxmap.find(size_t(pn->mMeshes[j]));
		if (cit != midxmap.cend())
			pm->AssignMeshToNode(nidx, cit->second);
	}

	pm->SetParentNode(nidx, parent_nidx);

	for (size_t i = 0; i < pn->mNumChildren; i++)
		AddModelNode(pm, nidx, pn->mChildren[i], &pn->mChildren[i]->mTransformation, nidxmap, midxmap);
}

inline unsigned int MakeImportFlags(const TCHAR *options, bool &animation_only)
{
	animation_only = false;

	unsigned int impflags = 0
		| aiProcess_Triangulate
		| aiProcess_CalcTangentSpace
		| aiProcess_ImproveCacheLocality
		| aiProcess_OptimizeMeshes
		| aiProcess_GenBoundingBoxes
		| aiProcess_SortByPType
		| aiProcess_JoinIdenticalVertices
		| aiProcess_GenUVCoords
		| aiProcess_FixInfacingNormals
		| aiProcess_GenNormals
		| aiProcess_LimitBoneWeights
		| aiProcess_PopulateArmatureData
		;

	if (options)
	{
		tstring o = options;
		std::transform(o.begin(), o.end(), o.begin(), tolower);

		if (_tcsstr(o.c_str(), _T("force_smooth_normals")))
		{
			impflags &= ~aiProcess_GenNormals;
			impflags |= aiProcess_GenSmoothNormals | aiProcess_ForceGenNormals;
		}
		else if (_tcsstr(o.c_str(), _T("force_flat_normals")))
		{
			impflags |= aiProcess_ForceGenNormals;
		}
		else if (_tcsstr(o.c_str(), _T("animation_only")))
		{
			animation_only = true;
		}
	}

	return impflags;
}


ModelImpl *ImportModel(c3::System *psys, const aiScene *scene, const TCHAR *rootpath, const TCHAR *sourcename, bool animation_only)
{
	if (!rootpath)
		rootpath = _T("");

	if (!sourcename)
		sourcename = _T("");

	if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
		return nullptr;

	RendererImpl *pr = (RendererImpl *)(psys->GetRenderer());
	ModelImpl *pmi = new ModelImpl(pr);

	if (!pmi)
		return nullptr;

	std::function<const TCHAR *(tstring &, const TCHAR *)> MakeTexFilename = [&sourcename](tstring &out, const TCHAR *idx) -> const TCHAR *
	{
		out += _T(":texture[");
		out += idx;
		out += _T("]");
		return out.c_str();
	};

	if (scene->HasTextures())
	{
		for (unsigned int texidx = 0; texidx < scene->mNumTextures; texidx++)
		{
			TCHAR textmp[8];
			_itot_s(texidx, textmp, 10);

			tstring texname = sourcename;
			MakeTexFilename(texname, textmp);
			c3::Texture2D *pt = nullptr;
			DefaultTexture2DResourceType::Type()->ReadFromMemory(psys, texname.c_str(), (const BYTE *)scene->mTextures[texidx]->pcData, scene->mTextures[texidx]->mWidth, nullptr, (void **)&pt);
			if (pt)
			{
				psys->GetResourceManager()->GetResource(texname.c_str(), RESF_CREATEENTRYONLY, DefaultTexture2DResourceType::Type(), pt);
			}
		}
	}

	if (scene->HasAnimations())
	{
		Animation *panim = Animation::Create();
		if (panim)
		{
			pmi->SetDefaultAnim(panim);

			for (size_t aidx = 0; aidx < scene->mNumAnimations; aidx++)
			{
				aiAnimation *pa = scene->mAnimations[aidx];
				if (aidx > 1)
				{
					pr->GetSystem()->GetLog()->Print(_T("Warning: ImportModel found more than one animation.\n"));
					break;
				}

				float ticks = (pa->mTicksPerSecond == 0) ? 30.0f : (float)pa->mTicksPerSecond;

				for (unsigned int aci = 0; aci < pa->mNumChannels; aci++)
				{
					aiNodeAnim* pna = pa->mChannels[aci];

					TCHAR *trackname;
					CONVERT_MBCS2TCS(pna->mNodeName.C_Str(), trackname);
					Animation::TrackIndex trackidx = panim->AddNewTrack(trackname);
					if (trackidx < 0)
						continue;

					AnimTrack *ptrack = panim->GetTrack(trackidx);

					for (size_t i = 0; i < pna->mNumPositionKeys; i++)
					{
						float t = (float)pna->mPositionKeys[i].mTime / ticks;

						auto kv = pna->mPositionKeys[i].mValue;
						glm::fvec3 pos(glm::fvec3(kv.x, kv.y, kv.z));

						ptrack->AddPosKey(t, pos);
					}

					for (size_t i = 0; i < pna->mNumRotationKeys; i++)
					{
						float t = (float)pna->mRotationKeys[i].mTime / ticks;

						auto kv = pna->mRotationKeys[i].mValue;
						glm::fquat ori(kv.w, kv.x, kv.y, kv.z);

						ptrack->AddOriKey(t, ori);
					}

					for (size_t i = 0; i < pna->mNumScalingKeys; i++)
					{
						float t = (float)pna->mScalingKeys[i].mTime / ticks;

						auto kv = pna->mScalingKeys[i].mValue;
						glm::fvec3 scl(kv.x, kv.y, kv.z);

						ptrack->AddSclKey(t, scl);
					}

					ptrack->SortKeys();
				}
			}

			tstring animfilename = rootpath;
			size_t extidx = animfilename.find_last_of(_T('.'), 0);
			if (extidx < animfilename.size())
			{
				animfilename.replace(animfilename.begin() + extidx, animfilename.end(), _T(".c3anim"));

				psys->GetResourceManager()->GetResource(animfilename.c_str(), RESF_CREATEENTRYONLY, RESOURCETYPE(Animation), panim);
			}
		}
	}

	if (animation_only)
		return pmi;

	std::vector<Material *> mtlvec;
	mtlvec.reserve(scene->mNumMaterials);
	MaterialManager *pmm = pr->GetMaterialManager();

	aiString aipath;
	TCHAR *textmp, *texfilename;
	TCHAR texpath[MAX_PATH];
	bool has_difftex = false;

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

		props::TFlags64 rf = RESF_DEMANDLOAD;

		if (true == (has_difftex = (paim->GetTextureCount(aiTextureType_DIFFUSE) > 0)))
			paim->GetTexture(aiTextureType_DIFFUSE, 0, &aipath);
		else if (!has_difftex && (true == (has_difftex = (paim->GetTextureCount(aiTextureType_BASE_COLOR) > 0))))
			paim->GetTexture(aiTextureType_BASE_COLOR, 0, &aipath);
		else if (!has_difftex && (true == (has_difftex = (paim->GetTextureCount(aiTextureType_UNKNOWN) > 0))))
			paim->GetTexture(aiTextureType_UNKNOWN, 0, &aipath);

		// locally-defined function to set texture filenames for specific types of textures
		auto SetTextureFileName = [&](Material::TextureComponentType t, const char *s)
		{
			CONVERT_MBCS2TCS(s, textmp);
			TCHAR idxstr[16];

			if (scene->HasTextures())
			{
				unsigned int texidx = 0;
				if (*textmp == '*')
				{
					textmp++;
					texidx = _ttoi(textmp);
				}
				else
				{
					while ((texidx < scene->mNumTextures) && _stricmp(s, scene->mTextures[texidx]->mFilename.C_Str()))
						texidx++;

					if (texidx >= scene->mNumTextures)
						texidx = 0;
				}

				_itot_s(texidx, idxstr, 10);

				tstring texname = sourcename;
				Resource *ptres = psys->GetResourceManager()->GetResource(MakeTexFilename(texname, idxstr));
				c3::Texture2D *pt = ptres ? dynamic_cast<Texture2D *>((Texture2D *)ptres->GetData()) : nullptr;

				pmtl->SetTexture(t, pt);
			}
			else
			{
				TCHAR fulltexpath[512];
				fulltexpath[0] = _T('\0');

				PathCombine(texpath, rootpath, textmp);
				bool loctex = psys->GetFileMapper()->FindFile(texpath, fulltexpath, 512);
				if (!loctex)
				{
					TCHAR *fnstart = PathFindFileName(textmp);
					PathCombine(texpath, rootpath, fnstart);
					loctex = psys->GetFileMapper()->FindFile(texpath);
				}
				texfilename = loctex ? ((_tcslen(texpath) < _tcslen(fulltexpath)) ? fulltexpath : texpath) : PathFindFileName(texpath);
				if (t == Material::TextureComponentType::TCT_DIFFUSE)
					_tcsncpy_s(difftexpath, textmp, MAX_PATH - 1);
				pmtl->SetTexture(t, psys->GetResourceManager()->GetResource(texfilename, rf));
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

	glm::fvec3 vmin_model(FLT_MAX, FLT_MAX, FLT_MAX), vmax_model(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh *impmesh = scene->mMeshes[i];

		glm::fvec3 vmin(FLT_MAX, FLT_MAX, FLT_MAX), vmax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		Mesh *pm = psys->GetRenderer()->CreateMesh();

		VertexBuffer *pvb = psys->GetRenderer()->CreateVertexBuffer();
		IndexBuffer *pib = psys->GetRenderer()->CreateIndexBuffer();

		Model::MeshIndex midx = pmi->AddMesh(pm);
		if (midx == Model::INVALID_INDEX)
			continue;

		unsigned int mtlidx = impmesh->mMaterialIndex;
		pmi->SetMaterial(midx, (mtlidx < mtlvec.size()) ? mtlvec[mtlidx] : pr->GetWhiteMaterial());

		// associate the aiMesh with a MeshIndex so we can attach it to the appropriate ModelIndex
		mim.insert(TMeshIndexMap::value_type(i, midx));

		VertexBuffer::ComponentDescription vcd[VertexBuffer::ComponentDescription::EUsage::VU_NUM_USAGES + 1];
		memset(vcd, 0, sizeof(VertexBuffer::ComponentDescription) * (VertexBuffer::ComponentDescription::EUsage::VU_NUM_USAGES + 1));

		size_t ci = 0;

		if (impmesh->HasPositions())
		{
			vcd[ci].m_Count = 3;
			vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_F32;
			vcd[ci].m_Usage = VertexBuffer::ComponentDescription::EUsage::VU_POSITION;
			ci++;
		}

		if (impmesh->HasNormals())
		{
			vcd[ci].m_Count = 3;
			vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_F32;
			vcd[ci].m_Usage = VertexBuffer::ComponentDescription::EUsage::VU_NORMAL;
			ci++;
		}

		if (impmesh->HasTangentsAndBitangents())
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
			if (impmesh->HasTextureCoords(t))
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
			if (impmesh->HasVertexColors(c))
			{
				vcd[ci].m_Count = 4;
				vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_U8;
				vcd[ci].m_Usage = (VertexBuffer::ComponentDescription::EUsage)(VertexBuffer::ComponentDescription::EUsage::VU_COLOR0 + c);
				ci++;
			}
			else
				break;	// can't have vertex color set #2 without #1, right?
		}

		// "collect weights on all vertices. Quick and careless" --assimp sample viewer, probably
		using TWeightVector = std::vector<aiVertexWeight>;
		std::vector<TWeightVector> weights_per_vert;
		if (impmesh->HasBones())
		{
			((MeshImpl *)pm)->MakeSkinned();

			weights_per_vert.resize(impmesh->mNumVertices);

			for (unsigned int a = 0; a < impmesh->mNumBones; a++)
			{
				const aiBone* bone = impmesh->mBones[a];
				for (unsigned int b = 0; b < bone->mNumWeights; b++)
					weights_per_vert[bone->mWeights[b].mVertexId].push_back(aiVertexWeight(a, bone->mWeights[b].mWeight));
			}

			vcd[ci].m_Count = 4;
			vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_U8;
			vcd[ci].m_Usage = VertexBuffer::ComponentDescription::EUsage::VU_INDEX;
			ci++;

			vcd[ci].m_Count = 4;
			vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_F32;
			vcd[ci].m_Usage = VertexBuffer::ComponentDescription::EUsage::VU_WEIGHT;
			ci++;
		}

		vcd[ci].m_Type = VertexBuffer::ComponentDescription::ComponentType::VCT_NONE;

		ci = 0;
		size_t vsz = 0;
		while (vcd[ci].m_Count)
		{
			vsz += vcd[ci++].size();
		}

		size_t vct = impmesh->mNumVertices;

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
				bool do_bounds = false;

				switch (vcd[ci].m_Usage)
				{
					case VertexBuffer::ComponentDescription::EUsage::VU_POSITION:
						pmd_3f = impmesh->mVertices;
						do_bounds = true;
						break;

					case VertexBuffer::ComponentDescription::EUsage::VU_NORMAL:
						pmd_3f = impmesh->mNormals;
						break;

					case VertexBuffer::ComponentDescription::EUsage::VU_BINORMAL:
						pmd_3f = impmesh->mBitangents;
						break;

					case VertexBuffer::ComponentDescription::EUsage::VU_TANGENT:
						pmd_3f = impmesh->mTangents;
						break;

					case VertexBuffer::ComponentDescription::EUsage::VU_TEXCOORD0:
					case VertexBuffer::ComponentDescription::EUsage::VU_TEXCOORD1:
					case VertexBuffer::ComponentDescription::EUsage::VU_TEXCOORD2:
					case VertexBuffer::ComponentDescription::EUsage::VU_TEXCOORD3:
						pmd_3f = impmesh->mTextureCoords[(size_t)(vcd[ci].m_Usage - VertexBuffer::ComponentDescription::EUsage::VU_TEXCOORD0)];
						break;

					case VertexBuffer::ComponentDescription::EUsage::VU_COLOR0:
					case VertexBuffer::ComponentDescription::EUsage::VU_COLOR1:
					case VertexBuffer::ComponentDescription::EUsage::VU_COLOR2:
					case VertexBuffer::ComponentDescription::EUsage::VU_COLOR3:
						pmd_c = impmesh->mColors[(size_t)(vcd[ci].m_Usage - VertexBuffer::ComponentDescription::EUsage::VU_COLOR0)];
						break;
				}

				if (pmd_3f)
				{
					for (size_t j = 0; j < vct; j++)
					{
						// this may seem wrong because you could have texture coordinates with 2 components...
						// but only the relevant ones are copied, based on the count defined by the vertex component
						memcpy(pv, &pmd_3f[j], sizeof(float) * vcd[ci].m_Count);

						if (do_bounds)
						{
							if (pmd_3f[j].x < vmin.x)
								vmin.x = pmd_3f[j].x;
							if (pmd_3f[j].y < vmin.y)
								vmin.y = pmd_3f[j].y;
							if (pmd_3f[j].z < vmin.z)
								vmin.z = pmd_3f[j].z;

							if (pmd_3f[j].x > vmax.x)
								vmax.x = pmd_3f[j].x;
							if (pmd_3f[j].y > vmax.y)
								vmax.y = pmd_3f[j].y;
							if (pmd_3f[j].z > vmax.z)
								vmax.z = pmd_3f[j].z;
						}

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
				else if (vcd[ci].m_Usage == VertexBuffer::ComponentDescription::EUsage::VU_INDEX)
				{
					// it's bone indices...
					for (size_t j = 0; j < vct; j++)
					{
						TWeightVector &vwv = weights_per_vert[j];

						uint8_t bi[4] = { 0 };
						for (size_t q = 0, maxq = std::min<size_t>(vwv.size(), 4); q < maxq; q++)
						{
							bi[q] = vwv[q].mVertexId;
						}
						memcpy(pv, bi, sizeof(uint8_t) * 4);

						pv += vsz;
					}
				}
				else if (vcd[ci].m_Usage == VertexBuffer::ComponentDescription::EUsage::VU_WEIGHT)
				{
					// it's bone weights...
					for (size_t j = 0; j < vct; j++)
					{
						TWeightVector &vwv = weights_per_vert[j];

						float bw[4] = { 0 };
						for (size_t q = 0, maxq = std::min<size_t>(vwv.size(), 4); q < maxq; q++)
						{
							bw[q] = vwv[q].mWeight;
						}
						memcpy(pv, bw, sizeof(float) * 4);

						pv += vsz;
					}

					pv += vsz;
				}

				ofs += vcd[ci].size();
				ci++;
			}

			pvb->Unlock();
			pm->AttachVertexBuffer(pvb);
		}

		size_t numfaces = 0;
		for (size_t k = 0; k < impmesh->mNumFaces; k++)
		{
			aiFace pf = impmesh->mFaces[k];
			if (pf.mNumIndices == 3)
				numfaces++;
		}

		BYTE *ibuf;
		bool large_indices = (impmesh->mNumVertices >= SHRT_MAX);
		if (pib->Lock((void **)&ibuf, numfaces * 3, large_indices ? IndexBuffer::EIndexSize::IS_32BIT : IndexBuffer::EIndexSize::IS_16BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
		{
			size_t ii = 0;

			if (large_indices)
			{
				for (size_t k = 0; k < numfaces; k++)
				{
					aiFace pf = impmesh->mFaces[k];
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
					aiFace pf = impmesh->mFaces[k];
					if (pf.mNumIndices != 3)
						continue;

					((uint16_t *)ibuf)[ii++] = (uint16_t)pf.mIndices[0];
					((uint16_t *)ibuf)[ii++] = (uint16_t)pf.mIndices[1];
					((uint16_t *)ibuf)[ii++] = (uint16_t)pf.mIndices[2];
				}
			}

			pib->Unlock();
			pm->AttachIndexBuffer(pib);

			((MeshImpl *)pm)->SetBounds(vmin, vmax);

			if (vmin.x < vmin_model.x)
				vmin_model.x = vmin.x;
			if (vmin.y < vmin_model.y)
				vmin_model.y = vmin.y;
			if (vmin.z < vmin_model.z)
				vmin_model.z = vmin.z;

			if (vmax.x > vmax_model.x)
				vmax_model.x = vmax.x;
			if (vmax.y > vmax_model.y)
				vmax_model.y = vmax.y;
			if (vmax.z > vmax_model.z)
				vmax_model.z = vmax.z;
		}
	}

	pmi->SetBounds(vmin_model, vmax_model);

#if 0
	int upAxis = 0;
	bool hasUpAxis = scene->mMetaData->Get<int>("UpAxis", upAxis);
	int upAxisSign = 1;
	scene->mMetaData->Get<int>("UpAxisSign", upAxisSign);

	int frontAxis = 0;
	bool hasFrontAxis = scene->mMetaData->Get<int>("FrontAxis", frontAxis);
	int frontAxisSign = 1;
	scene->mMetaData->Get<int>("FrontAxisSign", frontAxisSign);

	int rightAxis = 0;
	bool hasRightAxis = scene->mMetaData->Get<int>("RightAxis", rightAxis);
	int rightAxisSign = 1;
	scene->mMetaData->Get<int>("RightAxisSign", rightAxisSign);

	aiVector3D upVec = upAxis == 0 ? aiVector3D(upAxisSign,0,0) : upAxis == 1 ? aiVector3D(0, upAxisSign,0) : aiVector3D(0, 0, upAxisSign);
	aiVector3D forwardVec = frontAxis == 0 ? aiVector3D(frontAxisSign, 0, 0) : frontAxis == 1 ? aiVector3D(0, frontAxisSign, 0) : aiVector3D(0, 0, frontAxisSign);
	aiVector3D rightVec = rightAxis == 0 ? aiVector3D(rightAxisSign, 0, 0) : rightAxis == 1 ? aiVector3D(0, rightAxisSign, 0) : aiVector3D(0, 0, rightAxisSign);
	aiMatrix4x4 mat(rightVec.x, rightVec.y, rightVec.z, 0.0f,
					upVec.x, upVec.y, upVec.z, 0.0f,
					forwardVec.x, forwardVec.y, forwardVec.z, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
#endif

	TNodeIndexMap nim;
	AddModelNode(pmi, Model::NO_PARENT, scene->mRootNode, nullptr, nim, mim);

	return pmi;
}

c3::ResourceType::LoadResult RESOURCETYPENAME(Model)::ReadFromFile(c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const
{
	if (returned_data)
	{
		*returned_data = nullptr;

		char *s;
		CONVERT_TCS2MBCS(filename, s);
		std::string fn = s;

		Assimp::Importer import;
		bool animation_only = false;
		const aiScene *scene = import.ReadFile(fn, MakeImportFlags(options, animation_only));
		if (!scene)
		{
			TCHAR *err;
			CONVERT_MBCS2TCS(import.GetErrorString(), err);
			psys->GetLog()->Print(_T(" ("));
			psys->GetLog()->Print(err);
			psys->GetLog()->Print(_T(") - "));
		}
		else
		{
			TCHAR modbasepath[MAX_PATH];
			_tcscpy_s(modbasepath, filename);
			TCHAR *c = modbasepath;
			while (*c) { if (*c == _T('/')) { *c = _T('\\'); } c++; }
			c = _tcsrchr(modbasepath, _T('\\'));
			if (c)
			{
				*c = _T('\0');
				c++;
			}

			*returned_data = ImportModel(psys, scene, modbasepath, c, animation_only);
		}

		if (!*returned_data)
			return ResourceType::LoadResult::LR_ERROR;
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(Model)::ReadFromMemory(c3::System *psys, const TCHAR *contextname, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
{
	if (returned_data)
	{
		*returned_data = nullptr;

		Assimp::Importer import;
		bool animation_only = false;
		const aiScene *scene = import.ReadFileFromMemory(buffer, buffer_length, MakeImportFlags(options, animation_only));

		tstring sourcename;
		if (!contextname || !*contextname)
		{
			sourcename.resize(64, _T('#'));
			GUID g;
			CoCreateGuid(&g);
			_stprintf_s(sourcename.data(), sourcename.size(), _T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"), g.Data1, g.Data2, g.Data3,
						g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
			contextname = sourcename.c_str();
		}

		*returned_data = ImportModel(psys, scene, nullptr, contextname, animation_only);
		if (!*returned_data)
			return ResourceType::LoadResult::LR_ERROR;
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


size_t ParentlessNodeCount(ModelImpl *pm)
{
	size_t ret = 0;
	for (size_t i = 0; i < pm->GetNodeCount(); i++)
		if (pm->GetParentNode(i) <= 0)
			ret++;

	return ret;
}


// TODO: finish the write function - it's non-trivial because of the assimp data fill-out procedure
bool RESOURCETYPENAME(Model)::WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const
{
	ModelImpl *pmod = dynamic_cast<ModelImpl *>((ModelImpl *)data);
	if (pmod)
	{
		Assimp::Exporter e;
		aiScene s;

		std::vector<aiMesh *> meshes;
		std::vector<aiMaterial *> mtls;
		std::vector<aiNode *> nodes;

		meshes.resize(pmod->GetMeshCount());
		s.mMeshes = meshes.data();
		s.mNumMeshes = (unsigned int)meshes.size();

		aiNode *pn;

		bool made_root = ParentlessNodeCount(pmod) != 1;
		size_t nc = pmod->GetNodeCount() + made_root ? 1 : 0;
		nodes.reserve(nc);
		if (made_root)
		{
			pn = new aiNode();
			nodes.push_back(pn);
			for (unsigned int j = 0; j < 4; j++)
				for (unsigned int k = 0; k < 4; k++)
					pn->mTransformation[k][j] = (j == k) ? 1.0f : 0.0f;
		}

		while (nc)
		{
			size_t ni = nc - 1;

			char *nn;
			CONVERT_TCS2MBCS(pmod->GetNodeName(ni), nn);
			pn = new aiNode(nn);
			nodes.push_back(pn);

			unsigned int snc = (unsigned int)pmod->GetMeshCountOnNode(ni);
			nodes[ni]->mNumMeshes = snc;
			glm::fmat4x4 mat;
			pmod->GetTransform(ni, &mat);

			for (unsigned int j = 0; j < 4; j++)
				for (unsigned int k = 0; k < 4; k++)
					pn->mTransformation[k][j] = mat[k][j];
		}
	}

	return false;
}


void RESOURCETYPENAME(Model)::Unload(void *data) const
{
	((Mesh *)data)->Release();
}


ModelImpl::InstanceDataImpl::InstanceDataImpl(const Model *psource)
{
	assert(psource);

	m_pSourceModel = (const ModelImpl *)psource;

	NodeIndex maxi = psource->GetNodeCount();
	m_NodeMat.resize(maxi);
	m_NodeMtl.resize(maxi);
	m_NodeParent.resize(maxi);

	for (NodeIndex i = 0; i < maxi; i++)
	{
		m_NodeMat[i] = *psource->GetTransform(i);

		MeshIndex maxj = psource->GetMeshCountOnNode(i);
		if (maxj)
		{
			m_NodeMtl[i].resize(maxj, nullptr);
			for (MeshIndex j = 0; j < maxj; j++)
			{
				MeshIndex m = psource->GetMeshFromNode(i, j);
				m_NodeMtl[i][j] = ((ModelImpl *)psource)->m_pRend->GetMaterialManager()->CloneMaterial(psource->GetMaterial(m));
			}
		}
	}
}


void ModelImpl::InstanceDataImpl::Release()
{
	delete this;
}


const Model *ModelImpl::InstanceDataImpl::GetSourceModel()
{
	return m_pSourceModel;
}


bool ModelImpl::InstanceDataImpl::GetTransform(NodeIndex idx, glm::fmat4x4 &mat)
{
	if (idx < m_NodeMat.size())
	{
		mat = m_NodeMat[idx];
		return true;
	}

	return false;
}


void ModelImpl::InstanceDataImpl::SetTransform(NodeIndex idx, glm::fmat4x4 &mat)
{
	if (idx < m_NodeMat.size())
		m_NodeMat[idx] = mat;
}


Material *ModelImpl::InstanceDataImpl::GetMaterial(NodeIndex nodeidx, MeshIndex meshidx)
{
	if ((nodeidx >= m_NodeMtl.size()) || (meshidx >= m_pSourceModel->GetMeshCountOnNode(nodeidx)))
		return nullptr;

	return m_NodeMtl[nodeidx][meshidx];
}


const Animation *ModelImpl::GetDefaultAnim() const
{
	return m_DefaultAnim;
}


void ModelImpl::SetDefaultAnim(const Animation *panim)
{
	m_DefaultAnim = panim;
}

void ModelImpl::ProcessNodes(ProcessNodesFunc func)
{
	assert(func != nullptr);

	for (NodeIndex i = 0, maxi = m_Nodes.size(); i < maxi; i++)
	{
		if (!func(i))
			break;
	}
}
