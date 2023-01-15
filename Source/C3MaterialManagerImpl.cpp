// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3MaterialManagerImpl.h>
#include <C3MaterialImpl.h>

using namespace c3;


MaterialManagerImpl::MaterialManagerImpl(Renderer *prend)
{
	m_pRend = prend;
}


MaterialManagerImpl::~MaterialManagerImpl()
{
	for (auto mtl : m_Mats)
	{
		mtl->Release();
	}
}

/// Creates a new Material
Material *MaterialManagerImpl::CreateMaterial()
{
	MaterialImpl *ret = new MaterialImpl(this, m_pRend);

	return ret;
}

/// Destroys the given material
bool MaterialManagerImpl::DestroyMaterial(Material *mtl)
{
	delete mtl;

	return true;
}
