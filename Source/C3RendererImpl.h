// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3GuiImpl.h>
#include <C3MaterialManagerImpl.h>

#include <gl_wrapper.h>

namespace c3
{

	class SystemImpl;

	#define MATRIXUPDATE_WORLDVIEWPROJ		0x01
	#define MATRIXUPDATE_WORLDVIEW			0x02
	#define MATRIXUPDATE_VIEWPROJ			0x04
	#define MATRIXUPDATE_NORMAL				0x08

	#define MATRIXUPDATE_WORLD			(MATRIXUPDATE_WORLDVIEWPROJ | MATRIXUPDATE_WORLDVIEW | MATRIXUPDATE_NORMAL)
	#define MATRIXUPDATE_VIEW			(MATRIXUPDATE_WORLDVIEWPROJ | MATRIXUPDATE_VIEWPROJ | MATRIXUPDATE_NORMAL)
	#define MATRIXUPDATE_PROJ			(MATRIXUPDATE_WORLDVIEWPROJ | MATRIXUPDATE_VIEWPROJ)
	#define MATRIXUPDATE_ALL			(MATRIXUPDATE_WORLD | MATRIXUPDATE_VIEW | MATRIXUPDATE_PROJ)

	class RendererImpl : public Renderer
	{

	protected:
		SystemImpl *m_pSys;
		HANDLE m_event_shutdown;

		tstring m_DeviceName;
		tstring m_VendorName;

		HWND m_hwnd;
		HDC m_hdc;
		HGLRC m_glrc;
		HWND m_hwnd_override;
		RECT m_Viewport;

		GLuint m_glVersionMaj;
		GLuint m_glVersionMin;

		bool m_needsFinish;

		// There's no extra thread here... the "pool" just contains a list of tasks that we're going to execute on this thread when we can
		pool::IThreadPool *m_TaskPool;

		glm::fmat4x4 m_ident;

		glm::fvec3 m_eyepos, m_eyedir;
		glm::fmat4x4 m_proj, m_view, m_world, m_worldview, m_normal, m_viewproj, m_worldviewproj;
		props::TFlags32 m_matupflags;

		glm::fvec4 m_clearColor;
		float m_clearZ;
		uint8_t m_clearStencil;

		bool m_Initialized;
		WNDCLASS m_glARBWndClass;

		FrameBuffer *m_CurFB;
		GLuint m_CurFBID;

		ShaderProgram *m_CurProg;
		GLuint m_CurProgID;

		ShaderComponent *m_vsBounds;
		ShaderComponent *m_fsBounds;
		ShaderProgram *m_spBounds;

		VertexBuffer *m_CurVB;
		GLuint m_CurVBID;

		IndexBuffer *m_CurIB;
		GLuint m_CurIBID;

		GLuint m_VAOglID;

		bool m_Config;

		GuiImpl *m_Gui;

		DepthMode m_DepthMode;
		Test m_DepthTest;

		WindingOrder m_WindingOrder;
		CullMode m_CullMode;
		BlendMode m_BlendMode;
		BlendEquation m_BlendEq;

		float m_AlphaPassMin, m_AlphaPassMax;
		bool m_StencilEnabled;
		Test m_StencilTest;
		uint8_t m_StencilRef, m_StencilMask;
		StencilOperation m_StencilFailOp, m_StencilZFailOp, m_StencilZPassOp;

		float m_AlphaCoverage;
		bool m_AlphaCoverageInv;

		VertexBuffer *m_CubeVB;
		Mesh *m_BoundsMesh;
		Mesh *m_CubeMesh;

		VertexBuffer *m_FSPlaneVB;
		VertexBuffer *m_PlanesVB;
		Mesh *m_XYPlaneMesh;
		Mesh *m_YZPlaneMesh;
		Mesh *m_XZPlaneMesh;

		VertexBuffer *m_HemisphereVB;
		Mesh *m_HemisphereMesh;

		Texture2D *m_BlackTex;
		Texture2D *m_GreyTex;
		Texture2D *m_WhiteTex;
		Texture2D *m_BlueTex;
		Texture2D *m_GridTex;

		MaterialManagerImpl *m_MatMan;
		Material *m_mtlWhite;

		size_t m_VertsPerFrame, m_IndicesPerFrame, m_TrisPerFrame, m_LinesPerFrame, m_PointsPerFrame;

	public:

		COpenGL gl;
		bool isnv;

		RendererImpl(SystemImpl *psys);

		virtual ~RendererImpl();

		virtual System *GetSystem();

		virtual bool Initialize(HWND hwnd, props::TFlags64 flags);

		virtual bool Initialized();

		virtual void FlushErrors(const TCHAR *msgformat, ...);

		virtual void Shutdown();

		virtual const TCHAR* GetVendorName() const;
		virtual const TCHAR* GetDeviceName() const;

		pool::IThreadPool *GetTaskPool();

		virtual Gui *GetGui();

		virtual void SetViewport(const RECT *viewport = nullptr);
		virtual const RECT *GetViewport(RECT *viewport = nullptr) const;

		virtual void SetOverrideHwnd(HWND hwnd);
		virtual HWND GetOverrideHwnd() const;

		virtual bool BeginScene(props::TFlags64 flags);
		virtual bool EndScene(props::TFlags64 flags);
		virtual bool Present();

		virtual void SetClearColor(const glm::fvec4 *color = nullptr);
		virtual const glm::fvec4 *GetClearColor(glm::fvec4 *color = nullptr) const;

		virtual void SetClearDepth(float depth);
		virtual float GetClearDepth() const;

		virtual void SetClearStencil(uint8_t stencil);
		virtual uint8_t GetClearStencil() const;

		virtual void SetDepthMode(DepthMode mode);
		virtual DepthMode GetDepthMode() const;

		virtual void SetDepthTest(Test test);
		virtual Test GetDepthTest() const;

		virtual void SetStencilEnabled(bool en);
		virtual bool GetStencilEnabled() const;

		virtual void SetStencilTest(Test test, uint8_t ref = 0, uint8_t mask = 0xff);
		virtual Test GetStencilTest(uint8_t *ref = nullptr, uint8_t *mask = nullptr) const;

		virtual void SetStencilOperation(StencilOperation stencil_fail, StencilOperation zfail, StencilOperation zpass);
		virtual void GetStencilOperation(StencilOperation &stencil_fail, StencilOperation &zfail, StencilOperation &zpass) const;

		virtual void SetWindingOrder(WindingOrder mode);
		virtual WindingOrder GetWindingOrder() const;

		virtual void SetCullMode(CullMode mode);
		virtual CullMode GetCullMode() const;

		virtual void SetBlendMode(BlendMode mode);
		virtual BlendMode GetBlendMode() const;

		virtual void SetAlphaPassRange(float minalpha, float maxalpha);
		virtual void GetAlphaPassRange(float &minalpha, float &maxalpha);

		virtual void SetAlphaCoverage(float coverage, bool invert);
		virtual void GetAlphaCoverage(float &coverage, bool &invert);

		virtual void SetBlendEquation(BlendEquation eq);
		virtual BlendEquation GetBlendEquation() const;

		size_t PixelSize(TextureType type);
		GLenum GLType(TextureType type);
		GLenum GLInternalFormat(TextureType type);
		GLenum GLFormat(TextureType type);

		virtual Texture2D *CreateTexture2D(size_t width, size_t height, TextureType type, size_t mipcount, props::TFlags64 flags);
		virtual TextureCube *CreateTextureCube(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount, props::TFlags64 flags);
		virtual Texture3D *CreateTexture3D(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount, props::TFlags64 flags);

		virtual Texture2D *CreateTexture2DFromFile(const TCHAR *filename, props::TFlags64 flags);

		virtual DepthBuffer *CreateDepthBuffer(size_t width, size_t height, DepthType type, props::TFlags64 flags);

		virtual FrameBuffer *CreateFrameBuffer(props::TFlags64 flags);

		virtual VertexBuffer *CreateVertexBuffer(props::TFlags64 flags);
		virtual IndexBuffer *CreateIndexBuffer(props::TFlags64 flags);
		virtual Mesh *CreateMesh();

		virtual ShaderProgram *CreateShaderProgram();
		virtual ShaderComponent *CreateShaderComponent(ShaderComponentType type);

		virtual void UseFrameBuffer(FrameBuffer *pfb, props::TFlags64 flags = UFBFLAG_FINISHLAST);
		virtual FrameBuffer *GetActiveFrameBuffer();

		virtual void UseProgram(ShaderProgram *pprog);
		virtual ShaderProgram *GetActiveProgram();

		virtual void UseVertexBuffer(VertexBuffer *pvbuf);
		virtual void UseIndexBuffer(IndexBuffer *pibuf);

		virtual void UseTexture(uint64_t sampler, Texture *ptex = nullptr);

		virtual bool DrawPrimitives(PrimType type, size_t count = -1);
		virtual bool DrawIndexedPrimitives(PrimType type, size_t offset = -1, size_t count = -1);

		virtual void SetProjectionMatrix(const glm::fmat4x4 *m);
		virtual void SetViewMatrix(const glm::fmat4x4 *m);
		virtual void SetWorldMatrix(const glm::fmat4x4 *m);

		virtual const glm::fmat4x4 *GetProjectionMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetViewMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetWorldMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetWorldViewMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetNormalMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetViewProjectionMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetWorldViewProjectionMatrix(glm::fmat4x4 *m = nullptr);

		virtual void SetEyePosition(const glm::fvec3 *pos);
		virtual void SetEyeDirection(const glm::fvec3 *dir);

		virtual const glm::fvec3 *GetEyePosition(glm::fvec3 *pos = nullptr) const;
		virtual const glm::fvec3 *GetEyeDirection(glm::fvec3 *dir = nullptr) const;

		virtual VertexBuffer *GetFullscreenPlaneVB();

		VertexBuffer *GetCubeVB();
		virtual Mesh *GetBoundsMesh();
		virtual Mesh *GetCubeMesh();

		VertexBuffer *GetPlanesVB();
		virtual Mesh *GetXYPlaneMesh();
		virtual Mesh *GetYZPlaneMesh();
		virtual Mesh *GetXZPlaneMesh();

		VertexBuffer *GetHemisphereVB();
		virtual Mesh *GetHemisphereMesh();

		virtual Texture2D *GetBlackTexture();
		virtual Texture2D *GetGreyTexture();
		virtual Texture2D *GetWhiteTexture();
		virtual Texture2D *GetBlueTexture();
		virtual Texture2D *GetGridTexture();

		virtual MaterialManager *GetMaterialManager();
		virtual const Material *GetWhiteMaterial();

		virtual ShaderProgram *GetBoundsShader();

	};

};
