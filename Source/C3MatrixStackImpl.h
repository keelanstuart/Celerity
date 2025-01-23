// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class MatrixStackImpl : public MatrixStack
	{

	protected:

		typedef std::deque<glm::fmat4x4> TMatrixStack;

		// pre is each matrix that has been pushed, post is the result of a multiply of that and the previous top
		TMatrixStack pre, post, inv, invtrans;

	public:

		MatrixStackImpl();
		virtual ~MatrixStackImpl();

		virtual void Release();

		virtual void Push(const glm::fmat4x4 *m = nullptr);
		virtual bool Pop();

		virtual const glm::fmat4x4 *Top(glm::fmat4x4 *m = nullptr) const;

	};

};
