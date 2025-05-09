// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


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

/// Creates a new Material
Material *MaterialManagerImpl::CloneMaterial(const Material *to_clone)
{
	MaterialImpl *ret = new MaterialImpl(this, m_pRend, to_clone);

	return ret;
}

/// Destroys the given material
bool MaterialManagerImpl::DestroyMaterial(Material *mtl)
{
	delete mtl;

	return true;
}
