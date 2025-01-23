// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3PositionableImpl.h>
#include <C3BoundingBoxImpl.h>

namespace c3
{

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
		bool BeginSampling();
		void EndSampling();
		float Sample(float u, float v);

		Texture2D *tex;

		using TerrainSampleFunc = std::function<float(const void *h, Texture2D::SLockInfo *li, props::TVec2I &pos)>;
		TerrainSampleFunc samp;
	};

	bool LoadHeightMap(const TCHAR *filename, HeightMap &heightmap);

	void DeleteQuadNode(CTerrainQuadNode *node);

	void RenderQuad(CTerrainQuadNode *node, props::TFlags64 rendflags);

	CTerrainQuadNode *GenerateGeometryQuadSet(size_t startx, size_t starty, size_t xdim, size_t ydim);
		void  GenerateGeometry();
#endif

};