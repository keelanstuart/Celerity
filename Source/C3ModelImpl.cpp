// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3ModelImpl.h>
#include <C3Resource.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace c3;


ModelImpl::ModelImpl(RendererImpl *prend)
{

}


ModelImpl::~ModelImpl()
{

}


void ModelImpl::Release()
{
	delete this;
}


size_t ModelImpl::AddMesh(const Mesh *pmesh, const TCHAR *name)
{
	return -1;
}


size_t ModelImpl::GetMeshCount() const
{
	return m_Meshes.size();
}


const Mesh *ModelImpl::GetMesh(size_t index) const
{
	return nullptr;
}


const TCHAR *ModelImpl::GetMeshName(const Mesh *mesh) const
{
	return nullptr;
}


void ModelImpl::SetMaterial(const Mesh *mesh, const Material *pmaterial)
{

}


const Material *ModelImpl::GetMaterial(const Mesh *mesh) const
{
	return nullptr;
}


void ModelImpl::SetTransform(const Mesh *mesh, const glm::fmat4 *pmat)
{

}


const glm::fmat4 *ModelImpl::GetTransform(const Mesh *mesh, glm::fmat4 *pmat) const
{
	return nullptr;
}


void ModelImpl::SetParent(const Mesh *mesh, const Mesh *parent_mesh)
{

}


const Mesh *ModelImpl::GetParent(const Mesh *mesh) const
{
	return nullptr;
}


const Frustum *ModelImpl::GetBounds() const
{
	return nullptr;
}


void ModelImpl::Draw(glm::fmat4 *pmat) const
{

}



DECLARE_RESOURCETYPE(Model);

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

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			VertexBuffer *pvb = psys->GetRenderer()->CreateVertexBuffer();
			IndexBuffer *pib = psys->GetRenderer()->CreateIndexBuffer();
			Mesh *pm = psys->GetRenderer()->CreateMesh();

			TCHAR *name;
			CONVERT_MBCS2TCS(scene->mMeshes[i]->mName.C_Str(), name);
			pmi->AddMesh(pm, name);

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
			if (pvb->Lock((void **)&vbuf, vct, vcd, VBLOCKFLAG_WRITE))
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
							for (size_t k = 0; k < vcd[ci].m_Count; k++)
								((float *)pv)[k] = ((float *)(&pmd_3f[j]))[k];

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
				}

				pvb->Unlock();
				pm->AttachVertexBuffer(pvb);
			}

			BYTE *ibuf;
			bool large_indices = (scene->mMeshes[i]->mNumVertices >= SHRT_MAX);
			if (pib->Lock((void **)&ibuf, scene->mMeshes[i]->mNumFaces, large_indices ? IndexBuffer::EIndexSize::IS_16BIT : IndexBuffer::EIndexSize::IS_32BIT, IBLOCKFLAG_WRITE))
			{
				for (size_t k = 0; k < scene->mMeshes[i]->mNumFaces; k++)
				{
					aiFace pf = scene->mMeshes[i]->mFaces[k];
					uint32_t a = pf.mIndices[0];
					uint32_t b = pf.mIndices[1];
					uint32_t c = pf.mIndices[2];
				}

				pib->Unlock();
				pm->AttachIndexBuffer(pib);
			}
		}
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
