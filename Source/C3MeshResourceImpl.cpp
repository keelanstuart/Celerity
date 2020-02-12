// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3Resource.h>
#include <C3RendererImpl.h>
#include <C3VertexBufferImpl.h>
#include <C3IndexBufferImpl.h>
#include <C3MeshImpl.h>


using namespace c3;


DECLARE_RESOURCETYPE(MeshResource, MeshResource);


MeshResource::MeshResource(System *psys)
{
	m_pMesh = nullptr;
}


MeshResource::~MeshResource()
{

}


void MeshResource::Release()
{

}


Status MeshResource::GetStatus()
{
	return Status::RS_VACANT;
}


void *MeshResource::GetData()
{
	return nullptr;
}
