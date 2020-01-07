// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>

#include <C3RendererImpl.h>
#include <C3LogImpl.h>
#include <C3FactoryImpl.h>

namespace c3
{

	class SystemImpl : public System
	{

	protected:
		RendererImpl *m_Renderer;
		FactoryImpl *m_Factory;
		LogImpl *m_Log;

	public:
		SystemImpl();

		virtual ~SystemImpl();

		virtual void Release();

		virtual Renderer *GetRenderer();

		virtual Factory *GetFactory();

		virtual Log *GetLog();

	};

};