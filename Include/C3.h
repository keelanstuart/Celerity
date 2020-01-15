// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#ifdef C3_EXPORTS
#define C3_API __declspec(dllexport)
#else
#define C3_API __declspec(dllimport)
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

#include <stdint.h>

#include <PowerProps.h>
#include <Pool.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define C3MATRIX		glm::mat4x4
#define C3VEC2			glm::fvec2
#define C3VEC3			glm::fvec3
#define C3VEC4			glm::fvec4
#define C3QUAT			glm::quat

#include <C3MatrixStack.h>
#include <C3Texture.h>
#include <C3DepthBuffer.h>
#include <C3FrameBuffer.h>
#include <C3VertexBuffer.h>
#include <C3IndexBuffer.h>
#include <C3Mesh.h>
#include <C3ShaderComponent.h>
#include <C3ShaderProgram.h>
#include <C3Renderer.h>
#include <C3Log.h>
#include <C3Comportment.h>
#include <C3Object.h>
#include <C3Prototype.h>
#include <C3Factory.h>
#include <C3System.h>

