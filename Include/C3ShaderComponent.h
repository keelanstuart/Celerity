// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Renderer.h>

namespace c3
{

	class ShaderComponent
	{

	public:

		enum RETURNCODE
		{
			RET_OK = 0,

			RET_NULL_PROGRAM,
			RET_CREATE_FAILED,
			RET_COMPILE_FAILED,
		};

		virtual void Release() = NULL;

		virtual Renderer::ShaderComponentType Type() = NULL;

		virtual RETURNCODE CompileProgram(const TCHAR *program) = NULL;

		virtual const TCHAR *GetProgramText() = NULL;

		virtual bool IsCompiled() = NULL;

	};

};