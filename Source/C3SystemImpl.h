// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>

#include <C3RendererImpl.h>
#include <C3LogImpl.h>

namespace c3
{

	class SystemImpl : public System
	{

	protected:
		RendererImpl *m_Renderer;
		LogImpl *m_Log;

	public:
		SystemImpl();

		virtual ~SystemImpl();

		virtual void Release();

		virtual Renderer *GetRenderer();

		virtual Log *GetLog();

	};

};