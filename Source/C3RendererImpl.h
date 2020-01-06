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

		C3MATRIX m_ident;

		C3MATRIX m_proj, m_view, m_world, m_worldview;

		bool m_Initialized;
		WNDCLASS m_glARBWndClass;


	public:
		RendererImpl(SystemImpl *psys);

		virtual ~RendererImpl();

		virtual bool Initialize(size_t width, size_t height, HWND hwnd, props::TFlags64 flags);

		virtual void Shutdown();

		virtual System *GetSystem();

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

		virtual ShaderProgram *CreateShaderProgram();
		virtual ShaderComponent *CreateShaderComponent(ShaderComponentType type);

		virtual void UseFrameBuffer(FrameBuffer *pfb);

		virtual void UseProgram(ShaderProgram *pprog);

		virtual bool DrawPrimitives(PrimType type, size_t count = -1);

		virtual bool DrawIndexedPrimitives(PrimType type, size_t offset = -1, size_t count = -1);

		virtual void SetProjectionMatrix(const C3MATRIX *m);
		virtual void SetViewMatrix(const C3MATRIX *m);
		virtual void SetWorldMatrix(const C3MATRIX *m);

		COpenGL gl;
	};

};
