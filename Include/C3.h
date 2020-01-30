// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#define CELERITY_VERSION_MAJ		3
#define CELERITY_VERSION_MIN		0
#define CELERITY_VERSION_BLD		0

#define CELERITY_VERSION			((CELERITY_VERSION_MAJ << 24) |  (CELERITY_VERSION_MIN << 16) | CELERITY_VERSION_BLD)


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

// GLM init
#define GLM_FORCE_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/ext.hpp>

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
#include <C3Positionable.h>
#include <C3Camera.h>
#include <C3Object.h>
#include <C3Prototype.h>
#include <C3Factory.h>
#include <C3Plugin.h>
#include <C3PluginManager.h>
#include <C3Configuration.h>
#include <C3FileMapper.h>
#include <C3ResourceManager.h>
#include <C3System.h>

#define IMGUI_USER_CONFIG	"C3ImGuiConfig.h"
#include <../third-party/imgui/imgui.h>