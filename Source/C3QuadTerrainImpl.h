// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3QuadTerrain.h>
#include <C3PositionableImpl.h>
#include <C3BoundingBoxImpl.h>

namespace c3
{

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

	class QuadTerrainImpl : public QuadTerrain, props::IPropertyChangeListener
	{

	protected:
		Object *m_pOwner = nullptr;
		Positionable *m_pPos = nullptr;

		ShaderComponent *m_VS_terr = nullptr;
		ShaderComponent *m_FS_terr = nullptr;
		ShaderProgram *m_SP_terr = nullptr;

		int32_t m_uSamplerHeight = -1;
		int32_t m_uSamplerHeightStep = -1;
		Material *m_pMtl = nullptr;

#define QTFLAG_DIRTY	0x0001
		props::TFlags64 m_Flags = 0;

		props::TVec2I m_HeightTexDim = { 128, 128 };
		props::TVec2I m_VertDim = { 129, 129 };
		glm::fvec3 m_Scale = { 1.0f, 1.0f, 1.0f };

		float m_TexPeriod = 1.0f;

		Texture2D *m_HeightTex = nullptr;

		VertexBuffer *m_VB = nullptr;

		struct TerrainQuad
		{
			BoundingBoxImpl m_Bounds;

			size_t m_NumVerts = 0;
			size_t m_NumFaces = 0;

			IndexBuffer *m_IB = nullptr;
			Material *m_Mtl = nullptr;
		};

		typedef class TQuadTreeNode<TerrainQuad *> CTerrainQuadNode;

		CTerrainQuadNode *m_Root = nullptr;


	public:

		QuadTerrainImpl();

		virtual ~QuadTerrainImpl();

		virtual void Release();

		virtual ComponentType *GetType();

		virtual props::TFlags64 Flags();

		virtual bool Initialize(Object *pobject);

		virtual void Update(Object *pobject, float elapsed_time = 0.0f);

		virtual bool Prerender(Object *pobject, props::TFlags64 rendflags);

		virtual void Render(Object *pobject, props::TFlags64 rendflags);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance) const;

	protected:

		void DeleteQuadNode(CTerrainQuadNode *node);

		void RenderQuad(CTerrainQuadNode *node, props::TFlags64 rendflags);

		CTerrainQuadNode *GenerateGeometryQuadSet(size_t startx, size_t starty, size_t xdim, size_t ydim);
		void  GenerateGeometry();

	};

	DEFINE_COMPONENTTYPE(QuadTerrain, QuadTerrainImpl, GUID({ 0xec67985c, 0xb770, 0x4bae, { 0xaa, 0xa7, 0x1b, 0xb6, 0x3e, 0x1, 0x65, 0xa8 } }), "QuadTerrain", "Generates and renders a patch of terrain (requires Positionable)");

};