// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#include "pch.h"

#include "C3ShaderBuilderImpl.h"
#include <C3CRC.h>

using namespace c3;


ShaderBuilderImpl::ShaderBuilderImpl()
{
}


ShaderBuilderImpl::~ShaderBuilderImpl()
{
}


ShaderBuilderImpl::ShaderProgramID ShaderBuilderImpl::ComputeShaderProgramID(const VertexBuffer::ComponentDescription *comps, size_t numcomps)
{
	ShaderBuilderImpl::ShaderProgramID ret = Crc32::Calculate((const uint8_t *)comps, sizeof(VertexBuffer::ComponentDescription) * numcomps);
	return 0;
}


bool ShaderBuilderImpl::MakeShader(VertexBuffer::ComponentDescription *comps, size_t numcomps, ShaderBuilderImpl::ShaderProgramID &id)
{
	return false;
}


bool ShaderBuilderImpl::FindShader(ShaderBuilderImpl::ShaderProgramID id, ShaderProgram **program)
{
	return false;
}
