// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>

#include <gl_wrapper.h>

namespace c3
{

	class SystemImpl;

	#define MATRIXUPDATE_VIEW		0x01
	#define MATRIXUPDATE_PROJ		0x02
	#define MATRIXUPDATE_WORLD		0x04

	#define MATRIXUPDATE_VIEWPROJ		(MATRIXUPDATE_VIEW | MATRIXUPDATE_PROJ)
	#define MATRIXUPDATE_ALL			(MATRIXUPDATE_WORLD | MATRIXUPDATE_VIEW | MATRIXUPDATE_PROJ)

	#define NUMAUXTHREADS			1

	class RendererImpl : public Renderer
	{

	protected:
		SystemImpl *m_pSys;
		HANDLE m_event_shutdown;

		HWND m_hwnd;
		HDC m_hdc;
		HGLRC m_glrc;
		HWND m_hwnd_override;

		bool m_needsFinish;

		HDC m_hdc_aux[NUMAUXTHREADS];
		HGLRC m_glrc_aux[NUMAUXTHREADS];
		pool::IThreadPool *m_AuxPool;

		glm::fmat4x4 m_ident;

		glm::fmat4x4 m_proj, m_view, m_world, m_viewproj, m_worldviewproj;
		props::TFlags32 m_matupflags;

		glm::fvec4 m_clearColor;
		float m_clearZ;

		bool m_Initialized;
		WNDCLASS m_glARBWndClass;

		FrameBuffer *m_CurFB;
		GLuint m_CurFBID;

		ShaderProgram *m_CurProg;
		GLuint m_CurProgID;

		VertexBuffer *m_CurVB;
		GLuint m_CurVBID;

		IndexBuffer *m_CurIB;
		GLuint m_CurIBID;

		GLuint m_VAOglID;

		bool m_Config;

		DepthTest m_DepthTest;
		DepthMode m_DepthMode;
		CullMode m_CullMode;

		typedef std::pair<VertexBuffer *, ShaderProgram *> TVBProgPair;
		typedef struct
		{
			GLint attribloc;
		} SVBProg;
//		typedef std::

		VertexBuffer *m_CubeVB;
		Mesh *m_BoundsMesh;
		Mesh *m_CubeMesh;

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

	public:

		COpenGL gl;

		RendererImpl(SystemImpl *psys);

		virtual ~RendererImpl();

		virtual bool Initialize(size_t width, size_t height, HWND hwnd, props::TFlags64 flags);

		virtual bool Initialized();

		virtual void Shutdown();

		virtual System *GetSystem();

		virtual void SetOverrideHwnd(HWND hwnd);
		virtual HWND GetOverrideHwnd();

		virtual bool BeginScene(props::TFlags64 flags);
		virtual bool EndScene(props::TFlags64 flags);
		virtual bool Present();

		virtual void SetClearColor(const glm::fvec4 *color = nullptr);
		virtual const glm::fvec4 *GetClearColor(glm::fvec4 *color = nullptr);

		virtual void SetClearDepth(float depth);
		virtual float GetClearDepth();

		virtual void SetDepthMode(DepthMode mode);
		virtual DepthMode GetDepthMode();

		virtual void SetDepthTest(DepthTest test);
		virtual DepthTest GetDepthTest();

		virtual void SetCullMode(CullMode mode);
		virtual CullMode GetCullMode();

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

		virtual void UseFrameBuffer(FrameBuffer *pfb);
		virtual FrameBuffer *GetActiveFrameBuffer();

		virtual void UseProgram(ShaderProgram *pprog);
		virtual void UseVertexBuffer(VertexBuffer *pvbuf);
		virtual void UseIndexBuffer(IndexBuffer *pibuf);

		virtual void UseTexture(uint64_t sampler, Texture *ptex = nullptr);

		bool ConfigureDrawing();

		virtual bool DrawPrimitives(PrimType type, size_t count = -1);
		virtual bool DrawIndexedPrimitives(PrimType type, size_t offset = -1, size_t count = -1);

		virtual void SetProjectionMatrix(const glm::fmat4x4 *m);
		virtual void SetViewMatrix(const glm::fmat4x4 *m);
		virtual void SetWorldMatrix(const glm::fmat4x4 *m);

		virtual const glm::fmat4x4 *GetProjectionMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetViewMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetWorldMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetViewProjectionMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetWorldViewProjectionMatrix(glm::fmat4x4 *m = nullptr);

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

	};

};
