// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include <pch.h>

#include <C3MatrixStackImpl.h>


using namespace c3;


MatrixStackImpl::MatrixStackImpl()
{
	pre.push_back(glm::identity<glm::fmat4x4>());
	post.push_back(glm::identity<glm::fmat4x4>());
}

MatrixStackImpl::~MatrixStackImpl()
{
}

void MatrixStackImpl::Release()
{
	delete this;
}

void MatrixStackImpl::Push(const glm::fmat4x4 *m)
{
	static glm::fmat4x4 i;
	if (!m)
		m = &i;

	pre.push_back(*m);
	post.push_back(post.back() * *m);
}

bool MatrixStackImpl::Pop()
{
	//C3ASSERT(pre.size() == post.size());

	if (pre.size() <= 1)
		return false;

	pre.pop_back();
	post.pop_back();

	return true;
}


const glm::fmat4x4 *MatrixStackImpl::Top(glm::fmat4x4 *m) const
{
	if (!m)
		return &post.back();

	*m = post.back();
	return m;
}


MatrixStack *MatrixStack::Create()
{
	return new MatrixStackImpl();
}
