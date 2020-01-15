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

	class RendererImpl : public Renderer
	{

	protected:
		SystemImpl *m_pSys;

		HWND m_hwnd;
		HDC m_hdc;

		HGLRC m_glrc;
		HGLRC m_glrc_aux;

		bool m_needFinish;

		C3MATRIX m_ident;

		C3MATRIX m_proj, m_view, m_world, m_worldview;

		C3VEC4 m_clearColor;
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

		typedef std::pair<VertexBuffer *, ShaderProgram *> TVBProgPair;
		typedef struct
		{
			GLint attribloc;
		} SVBProg;
//		typedef std::

		VertexBuffer *m_CubeVB;
		Mesh *m_BoundsMesh;
		Mesh *m_CubeMesh;

	public:

		COpenGL gl;

		RendererImpl(SystemImpl *psys);

		virtual ~RendererImpl();

		virtual bool Initialize(size_t width, size_t height, HWND hwnd, props::TFlags64 flags);

		virtual bool Initialized();

		virtual void Shutdown();

		virtual System *GetSystem();

		virtual void SetClearColor(const C3VEC4 *color = nullptr);
		virtual const C3VEC4 *GetClearColor(C3VEC4 *color = nullptr);

		virtual void SetClearDepth(float depth);
		virtual float GetClearDepth();

		virtual bool BeginScene(props::TFlags64 flags);

		virtual bool EndScene(props::TFlags64 flags);

		size_t PixelSize(TextureType type);
		GLenum GLType(TextureType type);
		GLenum GLInternalFormat(TextureType type);
		GLenum GLFormat(TextureType type);

		virtual Texture2D *CreateTexture2D(size_t width, size_t height, TextureType type, size_t mipcount, props::TFlags64 flags);
		virtual TextureCube *CreateTextureCube(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount, props::TFlags64 flags);
		virtual Texture3D *CreateTexture3D(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount, props::TFlags64 flags);

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

		bool ConfigureDrawing();

		virtual bool DrawPrimitives(PrimType type, size_t count = -1);
		virtual bool DrawIndexedPrimitives(PrimType type, size_t offset = -1, size_t count = -1);

		virtual void SetProjectionMatrix(const C3MATRIX *m);
		virtual void SetViewMatrix(const C3MATRIX *m);
		virtual void SetWorldMatrix(const C3MATRIX *m);

		VertexBuffer *GetCubeVB();
		virtual Mesh *GetBoundsMesh();
		virtual Mesh *GetCubeMesh();

	};

};
