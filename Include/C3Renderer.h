// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <c3.h>

namespace c3
{
	class System;

	class Texture;
	class Texture2D;
	class TextureCube;
	class Texture3D;
	class DepthBuffer;
	class VertexBuffer;
	class IndexBuffer;
	class Mesh;
	class FrameBuffer;
	class ShaderComponent;
	class ShaderProgram;
	class Gui;

	class Renderer
	{

	public:

		typedef enum EPrimType
		{
			POINTLIST = 0,
			LINELIST,
			LINESTRIP,
			TRILIST,
			TRISTRIP,
			TRIFAN,

			NUM_PRIMTYPES

		} PrimType;

		typedef enum ETextureType
		{
			P8_3CH = 0,			// packed 8-bit, R3G3B2
			P16_3CH,			// packed 16-bit, R5G6B5
			P16_3CHT,			// packed 16-bit, R5G5B5A1
			P16_4CH,			// packed 16-bit, R4G4B4A4
			U8_1CH,				// 8-bit uint, 1 channel
			U8_2CH,				// 8-bit uint, 2 channel
			U8_3CH,				// 24-bit uint, 3 channel
			U8_3CHX,			// 32-bit uint, 3 channel *primary
			U8_4CH,				// 32-bit uint, 4 channel
			F16_1CH,			// 16-bit float, 1 channel
			F16_2CH,			// 16-bit float, 2 channel
			F16_3CH,			// 16-bit float, 3 channel
			F16_4CH,			// 16-bit float, 4 channel
			F32_1CH,			// 32-bit float, 1 channel
			F32_2CH,			// 32-bit float, 2 channel
			F32_3CH,			// 32-bit float, 3 channel
			F32_4CH,			// 32-bit float, 4 channel

			NUM_TEXTURETYPES

		} TextureType;

		typedef enum EDepthType
		{
			U16_D = 0,			// 16-bit uint, depth
			U32_D,				// 32-bit uint, depth
			U32_DS,				// 24-bit uint, depth, 8-bit stencil
			F32_D,				// 32-bit float, depth
			F32_DS,				// 32-bit float, depth

			NUM_DEPTHTYPES

		} DepthType;

		typedef enum EShaderComponentType
		{
			ST_NONE = -1,

			ST_VERTEX,
			ST_FRAGMENT,
			ST_GEOMETRY,
			ST_TESSEVAL,
			ST_TESSCONTROL,

			ST_NUMTYPES

		} ShaderComponentType;

		typedef enum EDepthTest
		{
			DT_NEVER = 0,
			DT_LESSER,
			DT_LESSEREQUAL,
			DT_EQUAL,
			DT_NOTEQUAL,
			DT_GREATEREQUAL,
			DT_GREATER,
			DT_ALWAYS,

			DT_NUMTESTS

		} DepthTest;

		typedef enum EDepthMode
		{
			DM_DISABLED = 0,
			DM_READWRITE,
			DM_READONLY,
			DM_WRITEONLY,

			DM_NUMMODES

		} DepthMode;

		typedef enum ECullMode
		{
			CM_DISABLED = 0,
			CM_FRONT,
			CM_BACK,
			CM_ALL,

			CM_NUMMODES

		} CullMode;

		/// Returns the System that created this Renderer
		virtual System *GetSystem() = NULL;

		/// Initializes the Renderer
		virtual bool Initialize(HWND hwnd, props::TFlags64 flags) = NULL;

		virtual bool Initialized() = NULL;

		virtual void Shutdown() = NULL;

		virtual Gui *GetGui() = NULL;

		virtual void SetViewport(const RECT *viewport_rect = nullptr) = NULL;

		virtual void SetOverrideHwnd(HWND hwnd = NULL) = NULL;
		virtual HWND GetOverrideHwnd() = NULL;

		/// Prepares the Renderer for rendering
		virtual bool BeginScene(props::TFlags64 flags = 0) = NULL;

		/// Finalizes rendering and presents the result to the display
		virtual bool EndScene(props::TFlags64 flags = 0) = NULL;

		virtual bool Present() = NULL;

		virtual void SetClearColor(const glm::fvec4 *color = nullptr) = NULL;
		virtual const glm::fvec4 *GetClearColor(glm::fvec4 *color = nullptr) = NULL;

		virtual void SetClearDepth(float depth = 1.0f) = NULL;
		virtual float GetClearDepth() = NULL;

		virtual void SetDepthMode(DepthMode mode) = NULL;
		virtual DepthMode GetDepthMode() = NULL;

		virtual void SetDepthTest(DepthTest test) = NULL;
		virtual DepthTest GetDepthTest() = NULL;

		virtual void SetCullMode(CullMode mode) = NULL;
		virtual CullMode GetCullMode() = NULL;

		virtual Texture2D *CreateTexture2D(size_t width, size_t height, TextureType type, size_t mipcount = 0, props::TFlags64 flags = 0) = NULL;
		virtual TextureCube *CreateTextureCube(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount = 0, props::TFlags64 flags = 0) = NULL;
		virtual Texture3D *CreateTexture3D(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount = 0, props::TFlags64 flags = 0) = NULL;

		virtual Texture2D *CreateTexture2DFromFile(const TCHAR *filename, props::TFlags64 flags = 0) = NULL;

		virtual DepthBuffer *CreateDepthBuffer(size_t width, size_t height, DepthType type, props::TFlags64 flags = 0) = NULL;

		/// Creates a frame buffer that facilitates rendering to textures
		virtual FrameBuffer *CreateFrameBuffer(props::TFlags64 flags = 0) = NULL;

		virtual VertexBuffer *CreateVertexBuffer(props::TFlags64 flags = 0) = NULL;
		virtual IndexBuffer *CreateIndexBuffer(props::TFlags64 flags = 0) = NULL;
		virtual Mesh *CreateMesh() = NULL;

		virtual ShaderProgram *CreateShaderProgram() = NULL;
		virtual ShaderComponent *CreateShaderComponent(ShaderComponentType type) = NULL;


		#define UFBFLAG_CLEARCOLOR			0x0001
		#define UFBFLAG_CLEARDEPTH			0x0002
		#define UFBFLAG_FINISHLAST			0x0004

		virtual void UseFrameBuffer(FrameBuffer *pfb, props::TFlags64 flags = UFBFLAG_FINISHLAST) = NULL;
		virtual FrameBuffer *GetActiveFrameBuffer() = NULL;

		virtual void UseProgram(ShaderProgram *pprog) = NULL;
		virtual void UseVertexBuffer(VertexBuffer *pvbuf) = NULL;
		virtual void UseIndexBuffer(IndexBuffer *pibuf) = NULL;

		virtual void UseTexture(uint64_t sampler, Texture *ptex = nullptr) = NULL;

		virtual bool DrawPrimitives(PrimType type, size_t count = -1) = NULL;

		virtual bool DrawIndexedPrimitives(PrimType type, size_t offset = -1, size_t count = -1) = NULL;

		virtual void SetProjectionMatrix(const glm::fmat4x4 *m) = NULL;
		virtual void SetViewMatrix(const glm::fmat4x4 *m) = NULL;
		virtual void SetWorldMatrix(const glm::fmat4x4 *m) = NULL;

		virtual const glm::fmat4x4 *GetProjectionMatrix(glm::fmat4x4 *m = nullptr) = NULL;
		virtual const glm::fmat4x4 *GetViewMatrix(glm::fmat4x4 *m = nullptr) = NULL;
		virtual const glm::fmat4x4 *GetWorldMatrix(glm::fmat4x4 *m = nullptr) = NULL;
		virtual const glm::fmat4x4 *GetViewProjectionMatrix(glm::fmat4x4 *m = nullptr) = NULL;
		virtual const glm::fmat4x4 *GetWorldViewProjectionMatrix(glm::fmat4x4 *m = nullptr) = NULL;

		virtual Mesh *GetBoundsMesh() = NULL;
		virtual Mesh *GetCubeMesh() = NULL;

		virtual Mesh *GetXYPlaneMesh() = NULL;
		virtual Mesh *GetYZPlaneMesh() = NULL;
		virtual Mesh *GetXZPlaneMesh() = NULL;

		virtual Mesh *GetHemisphereMesh() = NULL;

		virtual Texture2D *GetBlackTexture() = NULL;
		virtual Texture2D *GetGreyTexture() = NULL;
		virtual Texture2D *GetWhiteTexture() = NULL;
		virtual Texture2D *GetBlueTexture() = NULL;
		virtual Texture2D *GetGridTexture() = NULL;

	};

};
