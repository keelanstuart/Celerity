// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>

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
	class MaterialManager;
	class Material;
	class RenderMethod;
	class Font;

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
			TEXTYPE_UNKNOWN = 0,

			P8_3CH,				// packed 8-bit, R3G3B2
			P16_3CH,			// packed 16-bit, R5G6B5
			P16_3CHT,			// packed 16-bit, R5G5B5A1
			P16_4CH,			// packed 16-bit, R4G4B4A4
			U8_1CH,				// 8-bit uint, 1 channel
			U8_2CH,				// packed 16-bit uint, 2 channels
			U8_3CH,				// packed 24-bit uint, 3 channels
			U8_4CH,				// packed 32-bit uint, 4 channels
			S8_1CH,				// 8-bit signed int, 1 channel
			S8_2CH,				// packed 16-bit signed int, 2 channels
			S8_3CH,				// packed 24-bit signed int, 3 channels
			S8_4CH,				// packed 32-bit signed int, 4 channels
			F16_1CH,			// 16-bit float, 1 channel
			F16_2CH,			// 16-bit float, 2 channel
			F16_3CH,			// 16-bit float, 3 channel
			F16_4CH,			// 16-bit float, 4 channel
			F32_1CH,			// 32-bit float, 1 channel
			F32_2CH,			// 32-bit float, 2 channel
			F32_3CH,			// 32-bit float, 3 channel
			F32_4CH,			// 32-bit float, 4 channel

			DXT1,				// compressed textures
			DXT3,
			DXT5,

			NUM_TEXTURETYPES

		} TextureType;

		typedef enum EDepthType
		{
			U16_D = 0,			// 16-bit uint, depth
			U32_D,				// 32-bit uint, depth
			U32_DS,				// 24-bit uint, depth, 8-bit stencil
			F32_D,				// 32-bit float, depth
			F32_DS,				// 32-bit float, depth
			F16_SHADOW,			// 16-bit float, shadow
			F32_SHADOW,			// 32-bit float, shadow

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

		typedef enum ETest
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

		} Test;

		typedef enum EDepthMode
		{
			DM_DISABLED = 0,
			DM_READWRITE,
			DM_READONLY,
			DM_WRITEONLY,

			DM_NUMMODES

		} DepthMode;

		typedef enum EStencilOperation
		{
			SO_KEEP = 0,
			SO_ZERO,
			SO_REPLACE,
			SO_INC,
			SO_INC_WRAP,
			SO_DEC,
			SO_DEC_WRAP,
			SO_INVERT,

			SO_NUMOPMODES
		} StencilOperation;

		typedef enum EWindingOrder
		{
			WO_CW = 0,
			WO_CCW,

			WO_NUMMODES
		} WindingOrder;

		typedef enum ECullMode
		{
			CM_DISABLED = 0,
			CM_FRONT,
			CM_BACK,
			CM_ALL,

			CM_NUMMODES

		} CullMode;

		typedef enum EFillMode
		{
			FM_FILL = 0,
			FM_WIRE,
			FM_POINT,

			FM_NUMMODES
		} FillMode;

		typedef enum EBlendMode
		{
			BM_DISABLED = 0,			// blend(zero, zero)
			BM_REPLACE,					// blend(one, zero)
			BM_ALPHA,					// blend(src_alpha, inv_src_alpha)
			BM_ADD,						// blend(one, one)
			BM_ADDALPHA,				// blend(src_alpha, one)
			BM_ALPHATOCOVERAGE,			// replace, but with alpha-to-coverage enabled

			BM_NUMMODES

		} BlendMode;

		typedef enum EBlendEquation
		{
			BE_ADD = 0,					// result = source + destination
			BE_SUBTRACT,				// result = source - destination
			BE_REVERSE_SUBTRACT,		// result = destination - source
			BE_MIN,						// result = min(source, destination)
			BE_MAX,						// result = max(source, destination)

			BE_NUMMODES

		} BlendEquation;

		// There is an interplay between RenderMethods and Materials. Since Materials are (or can be) data driven by Models,
		// and because RenderMethods sit logically on top of them and because it is often desirable to make them override render states set
		// by a Material, RenderMethod Passes will return a set of RenderStateOverrideFlags indicating which states have been overridden
		// and should therefore not be re-set by Materials. This is not really useful for most users of Materials or RenderMethods,
		// but the flags are being defined here because the Apply interfaces are public-facing and these represent only the overlapping parts of both
		using RenderStateOverrideFlags = props::TFlags32;
		#define RSOF_WINDINGORDER	0x0001
		#define RSOF_CULLMODE		0x0002
		#define RSOF_FILLMODE		0x0004
		#define RSOF_DEPTHMODE		0x0008
		#define RSOF_DEPTHTEST		0x0010
		#define RSOF_STENCIL		0x0020
		#define RSOF_BLENDMODE		0x0040
		#define RSOF_BLENDEQ		0x0080
		#define RSOF_COLORAMB		0x0100
		#define RSOF_COLORDIFF		0x0200
		#define RSOF_COLOREMIS		0x0400
		#define RSOF_TEXDIFF		0x1000
		#define RSOF_TEXNORM		0x2000
		#define RSOF_TEXEMIS		0x4000
		#define RSOF_TEXSURF		0x8000



		// Flags for use with UseFrameBuffer
		#define UFBFLAG_CLEARCOLOR			0x0001
		#define UFBFLAG_CLEARDEPTH			0x0002
		#define UFBFLAG_CLEARSTENCIL		0x0004
		#define UFBFLAG_FINISHLAST			0x0008
		#define UFBFLAG_UPDATEVIEWPORT		0x0010
		#define BSFLAG_SHOWGUI				0x0100		// BeginScene flag; Indicates that the GUI should be rendered

		/// Returns the System that created this Renderer
		virtual System *GetSystem() = NULL;

		/// Initializes the Renderer
		virtual bool Initialize(HWND hwnd, props::TFlags64 flags) = NULL;

		// Returns true if the Renderer has been successfully initialized
		virtual bool Initialized() = NULL;

		// Flushes all accumulated errors to the console with a header message that you may provide
		virtual void FlushErrors(const TCHAR *msgformat, ...) = NULL;

		// Call this if you are done using the Renderer or need to reset / re-initialize. It is called automatically when shutting down Celerity otherwise
		virtual void Shutdown() = NULL;

		// Each time Present is called, this is incremented by 1
		virtual size_t GetCurrentFrameNumber() = NULL;

		// Returns the name of the active graphics adapter manufacturer
		virtual const TCHAR* GetVendorName() const = NULL;

		// Returns the name of the active graphics adapter
		virtual const TCHAR* GetDeviceName() const = NULL;

		// Returns the number of triangles rendered in the last frame
		virtual size_t LastTriagleCount() const = NULL;

		// Returns the number of lines rendered in the last frame
		virtual size_t LastLineCount() const = NULL;

		// Returns the number of points rendered in the last frame
		virtual size_t LastPointCount() const = NULL;

		// Returns the immediate mode GUI system
		virtual Gui *GetGui() = NULL;

		virtual void SetViewport(const RECT *viewport = nullptr) = NULL;
		virtual const RECT *GetViewport(RECT *viewport = nullptr) const = NULL;

		virtual void SetOverrideHwnd(HWND hwnd = NULL) = NULL;
		virtual HWND GetOverrideHwnd() const = NULL;

		/// Prepares the Renderer for rendering
		virtual bool BeginScene(props::TFlags64 flags = UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH | UFBFLAG_CLEARSTENCIL) = NULL;

		/// Finalizes rendering and presents the result to the display
		virtual bool EndScene(props::TFlags64 flags = 0) = NULL;

		virtual bool Present() = NULL;

		virtual void SetClearColor(const glm::fvec4 *color = nullptr) = NULL;
		virtual const glm::fvec4 *GetClearColor(glm::fvec4 *color = nullptr) const = NULL;

		virtual void SetClearDepth(float depth = 1.0f) = NULL;
		virtual float GetClearDepth() const = NULL;

		virtual void SetClearStencil(uint8_t stencil = 0) = NULL;
		virtual uint8_t GetClearStencil() const = NULL;

		virtual void SetDepthMode(DepthMode mode) = NULL;
		virtual DepthMode GetDepthMode() const = NULL;

		virtual void SetDepthTest(Test test) = NULL;
		virtual Test GetDepthTest() const = NULL;

		virtual void SetFillMode(FillMode mode) = NULL;
		virtual FillMode GetFillMode() const = NULL;

		virtual void SetStencilEnabled(bool en) = NULL;
		virtual bool GetStencilEnabled() const = NULL;

		virtual void SetStencilTest(Test test, uint8_t ref = 0, uint8_t mask = 0xff) = NULL;
		virtual Test GetStencilTest(uint8_t *ref = nullptr, uint8_t *mask = nullptr) const = NULL;

		virtual void SetStencilOperation(StencilOperation stencil_fail, StencilOperation zfail, StencilOperation zpass) = NULL;
		virtual void GetStencilOperation(StencilOperation &stencil_fail, StencilOperation &zfail, StencilOperation &zpass) const = NULL;

		virtual void SetWindingOrder(WindingOrder mode) = NULL;
		virtual WindingOrder GetWindingOrder() const = NULL;

		virtual void SetCullMode(CullMode mode) = NULL;
		virtual CullMode GetCullMode() const = NULL;

		virtual void SetBlendMode(BlendMode mode) = NULL;
		virtual BlendMode GetBlendMode() const = NULL;

		// Sets the alpha range in which pixels will not be discarded
		virtual void SetAlphaPassRange(float minalpha = 0.0f, float maxalpha = 1.0f) = NULL;

		// Gets the alpha range in which pixels will not be discarded
		virtual void GetAlphaPassRange(float &minalpha, float &maxalpha) = NULL;

		// Sets alpha coverage value
		virtual void SetAlphaCoverage(float coverage = 1.0f, bool invert = false) = NULL;
		virtual void GetAlphaCoverage(float &coverage, bool &invert) = NULL;

		virtual void SetBlendEquation(BlendEquation eq) = NULL;
		virtual BlendEquation GetBlendEquation() const = NULL;

		virtual Texture2D *CreateTexture2D(size_t width, size_t height, TextureType type, size_t mipcount = 0, props::TFlags64 createflags = 0) = NULL;
		virtual TextureCube *CreateTextureCube(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount = 0, props::TFlags64 createflags = 0) = NULL;
		virtual Texture3D *CreateTexture3D(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount = 0, props::TFlags64 createflags = 0) = NULL;

		// Creates a depth surface
		virtual DepthBuffer *CreateDepthBuffer(size_t width, size_t height, DepthType type, props::TFlags64 createflags = 0) = NULL;

		/// Creates a frame buffer that facilitates rendering to textures
		virtual FrameBuffer *CreateFrameBuffer(props::TFlags64 createflags = 0, const TCHAR *name = nullptr) = NULL;

		// Finds a frame buffer by name
		virtual FrameBuffer *FindFrameBuffer(const TCHAR *name) const = NULL;

		virtual VertexBuffer *CreateVertexBuffer(props::TFlags64 createflags = 0) = NULL;
		virtual IndexBuffer *CreateIndexBuffer(props::TFlags64 createflags = 0) = NULL;
		virtual Mesh *CreateMesh() = NULL;

		virtual ShaderProgram *CreateShaderProgram() = NULL;
		virtual ShaderComponent *CreateShaderComponent(ShaderComponentType type) = NULL;

		virtual RenderMethod *CreateRenderMethod() = NULL;
		virtual void UseRenderMethod(const RenderMethod *method = nullptr) = NULL;
		virtual RenderMethod *GetActiveRenderMethod() const = NULL;

		virtual void UseMaterial(const Material *material = nullptr) = NULL;
		virtual Material *GetActiveMaterial() const = NULL;

		virtual void UseFrameBuffer(FrameBuffer *pfb, props::TFlags64 flags = UFBFLAG_FINISHLAST) = NULL;
		virtual FrameBuffer *GetActiveFrameBuffer() = NULL;

		virtual void UseProgram(ShaderProgram *pprog) = NULL;
		virtual void UseVertexBuffer(VertexBuffer *pvbuf) = NULL;
		virtual void UseIndexBuffer(IndexBuffer *pibuf) = NULL;
		virtual ShaderProgram *GetActiveProgram() = NULL;

		virtual bool DrawPrimitives(PrimType type, size_t count = -1) = NULL;

		virtual bool DrawIndexedPrimitives(PrimType type, size_t offset = -1, size_t count = -1) = NULL;

		// Sets the current projection matrix - kind of a clearing house for shaders to pull from
		virtual void SetProjectionMatrix(const glm::fmat4x4 *m) = NULL;
		virtual void SetViewMatrix(const glm::fmat4x4 *m) = NULL;
		virtual void SetWorldMatrix(const glm::fmat4x4 *m) = NULL;
		virtual void SetSunShadowMatrix(const glm::fmat4x4 *m) = NULL;
		virtual void SetTextureTransformMatrix(const glm::fmat4x4 *m) = NULL;

		virtual const glm::fmat4x4 *GetProjectionMatrix(glm::fmat4x4 *m = nullptr) = NULL;
		virtual const glm::fmat4x4 *GetViewMatrix(glm::fmat4x4 *m = nullptr) = NULL;
		virtual const glm::fmat4x4 *GetWorldMatrix(glm::fmat4x4 *m = nullptr) = NULL;
		virtual const glm::fmat4x4 *GetWorldViewMatrix(glm::fmat4x4 *m = nullptr) = NULL;
		virtual const glm::fmat4x4 *GetNormalMatrix(glm::fmat4x4 *m = nullptr) = NULL;
		virtual const glm::fmat4x4 *GetViewProjectionMatrix(glm::fmat4x4 *m = nullptr) = NULL;
		virtual const glm::fmat4x4 *GetWorldViewProjectionMatrix(glm::fmat4x4 *m = nullptr) = NULL;
		virtual const glm::fmat4x4 *GetSunShadowMatrix(glm::fmat4x4 *m = nullptr) = NULL;
		virtual const glm::fmat4x4 *GetTextureTransformMatrix(glm::fmat4x4 *m = nullptr) = NULL;

		virtual void SetEyePosition(const glm::fvec3 *pos) = NULL;
		virtual void SetEyeDirection(const glm::fvec3 *dir) = NULL;

		virtual const glm::fvec3 *GetEyePosition(glm::fvec3 *pos = nullptr) const = NULL;
		virtual const glm::fvec3 *GetEyeDirection(glm::fvec3 *dir = nullptr) const = NULL;

		virtual VertexBuffer *GetFullscreenPlaneVB() = NULL;

		virtual Mesh *GetBoundsMesh() = NULL;
		virtual Mesh *GetCubeMesh() = NULL;
		virtual Mesh *GetRefCubeMesh() = NULL;

		virtual Mesh *GetXYPlaneMesh() = NULL;
		virtual Mesh *GetYZPlaneMesh() = NULL;
		virtual Mesh *GetXZPlaneMesh() = NULL;
		virtual Mesh *GetGuiRectMesh() = NULL;

		virtual Mesh *GetHemisphereMesh() = NULL;
		virtual Mesh *GetSphereMesh() = NULL;

		virtual Texture2D *GetBlackTexture() = NULL;
		virtual Texture2D *GetGreyTexture() = NULL;
		virtual Texture2D *GetDefaultDescTexture() = NULL;
		virtual Texture2D *GetDefaultNormalTexture() = NULL;
		virtual Texture2D *GetWhiteTexture() = NULL;
		virtual Texture2D *GetBlueTexture() = NULL;
		virtual Texture2D *GetGridTexture() = NULL;
		virtual Texture2D *GetLinearGradientTexture() = NULL;
		virtual Texture2D *GetOrthoRefTexture() = NULL;
		virtual Texture2D *GetUtilityColorTexture() = NULL;
		virtual Texture2D *GetSphereSpriteTexture() = NULL;
		virtual Texture2D *GetSphereSpriteNormalTexture() = NULL;

		virtual MaterialManager *GetMaterialManager() = NULL;
		virtual const Material *GetWhiteMaterial() = NULL;
		virtual const Material *GetBlackMaterial() = NULL;

		virtual ShaderProgram *GetBoundsShader() = NULL;

		virtual Font *GetFont(const TCHAR *name, size_t size = 10) = NULL;

	};

};
