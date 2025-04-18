// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


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

#include <stdint.h>

#include <C3TString.h>

#include <PowerProps.h>		// https://github.com/keelanstuart/PowerProps	==> third-party/PowerProps/Include
#include <Pool.h>			// https://github.com/keelanstuart/Pool			==> third-party/Pool/Include

// GLM init
#define GLM_FORCE_CTOR_INIT
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include <glm/glm.hpp>						// https://github.com/g-truc/glm				==> third-party/glm
#include <glm/ext.hpp>						// https://github.com/g-truc/glm				==> third-party/glm
#include <glm/gtx/matrix_decompose.hpp>		// https://github.com/g-truc/glm				==> third-party/glm

#include <C3MatrixStack.h>
#include <C3BoundingBox.h>
#include <C3Texture.h>
#include <C3DepthBuffer.h>
#include <C3FrameBuffer.h>
#include <C3VertexBuffer.h>
#include <C3IndexBuffer.h>
#include <C3Mesh.h>
#include <C3ShaderComponent.h>
#include <C3ShaderProgram.h>
#include <C3Material.h>
#include <C3MaterialManager.h>
#include <C3RenderMethod.h>
#include <C3Model.h>
#include <C3Renderer.h>
#include <C3SoundPlayer.h>
#include <C3Log.h>
#include <C3Screen.h>
#include <C3ScreenManager.h>
#include <C3Font.h>

#include <C3Component.h>
#include <C3Positionable.h>
#include <C3Camera.h>
#include <C3ModelRenderer.h>
#include <C3OmniLight.h>
#include <C3Scriptable.h>
#include <C3Physical.h>

#include <C3Environment.h>
#include <C3PhysicsManager.h>
#include <C3Object.h>
#include <C3Prototype.h>
#include <C3Factory.h>
#include <C3Plugin.h>
#include <C3PluginManager.h>
#include <C3Configuration.h>
#include <C3FileMapper.h>
#include <C3ResourceManager.h>
#include <C3InputManager.h>
#include <C3ActionMapper.h>
#include <C3GlobalObjectRegistry.h>
#include <C3System.h>
#include <C3ColorDefs.h>

#include <C3FlowGraph.h>
#include <C3FlowNode.h>

#define C3_SAFERELEASE(p)	if (p) { p->Release(); p = nullptr; }
