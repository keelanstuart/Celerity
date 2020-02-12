// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3Resource.h>
#include <C3Renderer.h>

namespace c3
{

	class MeshResource : public Resource
	{

	protected:
		Mesh *m_pMesh;

	public:

		MeshResource(System *psys);

		virtual ~MeshResource();

		virtual void Release();

		virtual Status GetStatus();

		virtual void *GetData();

	};

	DEFINE_RESOURCETYPE(MeshResource, MeshResource, GUID({0xbbf49113, 0xdb66, 0x4b7d, { 0xb4, 0xc7, 0x33, 0x68, 0x80, 0x97, 0x6, 0x49 }}), _T("Mesh"), _T("3D Meshes"), _T("3ds,max,x,dae,obj,bland"), _T("dae"));

};