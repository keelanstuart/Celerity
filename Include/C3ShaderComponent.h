// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


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

		/// Returns the type of shader this is: VERTEX, FRAGMENT, GEOMETRY, etc.
		virtual Renderer::ShaderComponentType Type() = NULL;

		/// Compiles the shader component
		///   
		/// NOTES:
		///  - Do not include the GLSL version in your program; the version indicated by the renderer will be prepended automatically
		///  - Do not include inputs for VERTEX shaders or outputs for FRAGMENT shaders; they are set based on the VertexBuffer components and the attached FrameBuffer
		///  - The preamble should normally be set by the ShaderProgram that owns this ShaderComponent
		///   
		/// Vertex component names in the default VERTEX SHADER preamble:
		///   VU_POSITION:		"vPos"
		///   VU_NORMAL:		"vNorm"
		///   VU_TEXCOORD0:		"vTex0"
		///   VU_TEXCOORD1:		"vTex1"
		///   VU_TEXCOORD2:		"vTex2"
		///   VU_TEXCOORD3:		"vTex3"
		///   VU_TANGENT:		"vTan"
		///   VU_BINORMAL:		"vBinorm"
		///   VU_INDEX:			"vIndex"
		///   VU_WEIGHT:		"vWeight"
		///   VU_COLOR0:		"vColor0"
		///   VU_COLOR1:		"vColor1"
		///   VU_COLOR2:		"vColor2"
		///   VU_COLOR3:		"vColor3"
		///   VU_SIZE:			"vSize"
		///   
		/// Fragment output names in the default FRAGMENT SHADER preamble are in the form of ("fragment%d", fragindex)
		///   
		virtual RETURNCODE CompileProgram(const TCHAR *program, const TCHAR *preamble = nullptr) = NULL;

		// 
		virtual const TCHAR *GetProgramText() const = NULL;

		virtual uint32_t GetProgramCRC() const = NULL;

		virtual bool IsCompiled() const = NULL;

	};

};