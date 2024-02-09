// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Material.h>
#include <C3Renderer.h>
#include <C3Texture.h>
#include <C3VertexBuffer.h>

namespace c3
{

	class ShaderBuilderImpl
	{

	public:
		typedef uint32_t ShaderProgramID;

		ShaderBuilderImpl();
		virtual ~ShaderBuilderImpl();

		virtual ShaderProgramID ComputeShaderProgramID(const VertexBuffer::ComponentDescription *comps, size_t numcomps);

		virtual bool MakeShader(VertexBuffer::ComponentDescription *comps, size_t numcomps, ShaderProgramID &id);

		virtual bool FindShader(ShaderProgramID id, ShaderProgram **program);

	protected:
		typedef std::unordered_map<ShaderProgramID, ShaderProgram *> IDtoShaderMap;

	};

};
