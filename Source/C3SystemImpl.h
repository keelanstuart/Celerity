// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>

#include <C3RendererImpl.h>

namespace c3
{

	class SystemImpl : public System
	{

	protected:
		RendererImpl *m_Renderer;

	public:
		SystemImpl();

		virtual ~SystemImpl();

		virtual void Release();

		virtual Renderer *GetRenderer();

	};

};