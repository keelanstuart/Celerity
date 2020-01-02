// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include <pch.h>

#include <C3MatrixStackImpl.h>


using namespace c3;


MatrixStackImpl::MatrixStackImpl()
{
	C3MATRIX m = glm::identity<C3MATRIX>();
	pre.push_back(m);
	post.push_back(m);
}

MatrixStackImpl::~MatrixStackImpl()
{
}

void MatrixStackImpl::Release()
{
	delete this;
}

void MatrixStackImpl::Push(const C3MATRIX *m)
{
	static C3MATRIX i;
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

const C3MATRIX *MatrixStackImpl::Top(C3MATRIX *m) const
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
