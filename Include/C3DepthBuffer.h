// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Renderer.h>

namespace c3
{

	class DepthBuffer
	{

	public:

		virtual void Release() = NULL;

		virtual Renderer::EDepthType Format() = NULL;

		// The X length of the texture
		virtual size_t Width() = NULL;

		// The Y length of the texture
		virtual size_t Height() = NULL;

	};

};