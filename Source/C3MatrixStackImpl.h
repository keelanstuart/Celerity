// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class MatrixStackImpl : public MatrixStack
	{

	protected:

		typedef std::deque<C3MATRIX> TMatrixStack;

		// pre is each matrix that has been pushed, post is the result of a multiply of that and the previous top
		TMatrixStack pre, post;

	public:

		MatrixStackImpl();
		virtual ~MatrixStackImpl();

		virtual void Release();

		virtual void Push(const C3MATRIX *m = nullptr);
		virtual bool Pop();

		virtual const C3MATRIX *Top(C3MATRIX *m) const;

	};

};
