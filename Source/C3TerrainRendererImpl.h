// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3TerrainRenderer.h>
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

	class TerrainRendererImpl : public TerrainRenderer, props::IPropertyChangeListener
	{

	protected:
		Object *m_pOwner = nullptr;
		Positionable *m_pPos = nullptr;

		Material *m_pMtl = nullptr;
		Texture2D *m_pHeightMap;
		Texture2D *m_pCoverMap;
		props::TVec2I m_MapOfs;
		props::TVec2I m_MapRange;

		props::TFlags64 m_Flags;

		static const TCHAR *MakeTerrainModelFilename(TCHAR *namebuf, size_t namebuflen, size_t xdim, size_t ydim);
		static void GenerateTerrainModel(System *psys, size_t xdimchunk, size_t ydimchunk, size_t xdimmap, size_t ydimmap);

	public:

		TerrainRendererImpl();

		virtual ~TerrainRendererImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags, int draworder);

		virtual void Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance) const;

	};

	DEFINE_COMPONENTTYPE(TerrainRenderer, TerrainRendererImpl, GUID({ 0xec67985c, 0xb770, 0x4bae, { 0xaa, 0xa7, 0x1b, 0xb6, 0x3e, 0x1, 0x65, 0xa8 } }), "TerrainRenderer", "Generates and renders a patch of terrain (requires Positionable)", 0);

};