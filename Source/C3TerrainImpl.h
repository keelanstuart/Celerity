// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3PositionableImpl.h>
#include <C3BoundingBoxImpl.h>

namespace c3
{

	class TerrainDescription
	{

	public:
		bool Load(tinyxml2::XMLDocument *pdoc);
		Model *GenerateTerrain(System *psys);

		tstring m_Name;
		glm::u32vec2 m_Dim;
		float m_MinZ, m_MaxZ;
		float m_UnitsBetweenSamples;
		std::optional<tstring> m_HeightmapFile;
		glm::uvec2 m_TexDim;

		struct LayerDescription
		{
			std::optional<tstring> m_TexFile[Material::TextureComponentType::NUM_TEXTURETYPES];
			std::optional<tstring> m_LayerOpacityFile;
			float m_Opacity = 1.0f;
			float m_Tiling = 1.0f;
		};

		using LayerArray = std::vector<LayerDescription>;
		LayerArray m_Layers;

	protected:
		bool CompositeTextures(System *psys, Material *pmtl);

	};


#if 0
	template <class T> class TQuadTreeNode
	{

	public:
		enum ChildQuad
		{
			POSX_POSY = 0,
			POSX_NEGY,
			NEGX_POSY,
			NEGX_NEGY,

			NUM_QUADS
		};


	private:
		T m_Data;

		TQuadTreeNode<T> *m_pParent;

		TQuadTreeNode<T> *m_pChild[NUM_QUADS];

	public:
		TQuadTreeNode()
		{
			memset(m_pChild, 0, sizeof(TQuadTreeNode<T> *) * NUM_QUADS);
		}

		TQuadTreeNode(T *data)
		{
			m_Data = data;

			memset(m_pChild, 0, sizeof(TQuadTreeNode<T> *) * NUM_QUADS);
		}

		~TQuadTreeNode()
		{
		}

		void SetChild(ChildQuad quad, TQuadTreeNode<T> *pnode)
		{
			m_pChild[quad] = pnode;
		}

		TQuadTreeNode *GetChild(ChildQuad quad)
		{
			return m_pChild[quad];
		}

		void SetData(T data)
		{
			m_Data = data;
		}

		T GetData()
		{
			return m_Data;
		}
	};

	struct HeightMap
	{
		bool LoadHeightMap(const TCHAR *filename, HeightMap &heightmap);

		bool BeginSampling();
		void EndSampling();
		float Sample(float u, float v);

		Texture2D *tex;

		using TerrainSampleFunc = std::function<float(const void *h, Texture2D::SLockInfo *li, props::TVec2I &pos)>;
		TerrainSampleFunc samp;
	};


	void DeleteQuadNode(CTerrainQuadNode *node);

	void RenderQuad(CTerrainQuadNode *node, props::TFlags64 rendflags);

	CTerrainQuadNode *GenerateGeometryQuadSet(size_t startx, size_t starty, size_t xdim, size_t ydim);
		void  GenerateGeometry();
#endif

};