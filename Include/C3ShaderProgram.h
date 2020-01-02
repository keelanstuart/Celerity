// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Renderer.h>

namespace c3
{

	class ShaderProgram
	{

	public:

		enum RETURNCODE
		{
			RET_OK = 0,

			RET_CREATE_FAILED,
			RET_NULL_SHADER,
			RET_PROGRAM_INCOMPLETE
		};

		virtual void Release() = NULL;

		virtual RETURNCODE AttachShader(ShaderComponent *pshader) = NULL;
		virtual RETURNCODE Link() = NULL;

	};

};
