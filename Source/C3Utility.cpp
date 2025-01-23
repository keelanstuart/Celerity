// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3Utility.h>
#include <C3Positionable.h>

using namespace c3;


void util::ComputeFinalTransform(Object *proot, glm::fmat4x4 *pmat)
{
	if (!proot)
		return;

	assert(pmat);

	static glm::fmat4x4 imat = glm::identity<glm::fmat4x4>();
	Positionable *ppos = (Positionable *)proot->FindComponent(Positionable::Type());
	const glm::fmat4x4 *mat = ppos ? ppos->GetTransformMatrix() : &imat;
	*pmat = *mat * *pmat;

	ComputeFinalTransform(proot->GetParent(), pmat);
}


void C3_API util::RecursiveObjectAction(Object *proot, std::function<void(Object *)> action_func)
{
	if (!proot)
		return;

	action_func(proot);
	for (size_t i = 0, maxi = proot->GetNumChildren(); i < maxi; i++)
		action_func(proot->GetChild(i));
}
