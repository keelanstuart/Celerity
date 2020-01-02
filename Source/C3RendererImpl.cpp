// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3RendererImpl.h>
#include <C3ShaderProgramImpl.h>
#include <C3ShaderComponentImpl.h>
#include <C3DepthBufferImpl.h>
#include <C3TextureImpl.h>


using namespace c3;


RendererImpl::RendererImpl(SystemImpl *psys)
{
	m_pSys = psys;

	m_hwnd = NULL;
	m_hdc = NULL;
	m_glrc = NULL;

	m_ident = glm::identity<C3MATRIX>();

	m_Initialized = false;
}


RendererImpl::~RendererImpl()
{
	Shutdown();
}


bool RendererImpl::Initialize(size_t width, size_t height, HWND hwnd, props::TFlags64 flags)
{
	m_hwnd = hwnd;
	m_hdc = ::GetDC(hwnd);

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd  
		1,                     // version number  
		PFD_DRAW_TO_WINDOW |   // support window  
		PFD_SUPPORT_OPENGL |   // support OpenGL  
		PFD_DOUBLEBUFFER,      // double buffered  
		PFD_TYPE_RGBA,         // RGBA type  
		24,                    // 24-bit color depth  
		0, 0, 0, 0, 0, 0,      // color bits ignored  
		0,                     // no alpha buffer  
		0,                     // shift bit ignored  
		0,                     // no accumulation buffer  
		0, 0, 0, 0,            // accum bits ignored  
		24,                    // 32-bit z-buffer  
		8,                     // no stencil buffer  
		0,                     // no auxiliary buffer  
		PFD_MAIN_PLANE,        // main layer  
		0,                     // reserved  
		0, 0, 0                // layer masks ignored  
	};

	int  iPixelFormat;

	// get the best available match of pixel format for the device context   
	iPixelFormat = ChoosePixelFormat(m_hdc, &pfd);

	// make that the pixel format of the device context  
	SetPixelFormat(m_hdc, iPixelFormat, &pfd);

	m_glrc = wglCreateContext(m_hdc);

	wglMakeCurrent(m_hdc, m_glrc);

	if (!gl.Initialize())
		return false;

	m_Initialized = true;

	RECT r;
	GetClientRect(m_hwnd, &r);
	if (width != -1)
		r.right = (LONG)width;
	if (height != -1)
		r.bottom = (LONG)height;

	gl.Viewport(r.left, r.top, r.right - r.left, r.bottom - r.top);

	SetProjectionMatrix(&m_ident);
	SetViewMatrix(&m_ident);
	SetWorldMatrix(&m_ident);

	gl.ShadeModel(GL_SMOOTH);
	gl.ClearColor(1, 0, 1, 1);
	gl.ClearDepthf(1);
	gl.Enable(GL_DEPTH_TEST);
	gl.DepthFunc(GL_LEQUAL);

	return true;
}


void RendererImpl::Shutdown()
{
	if (!m_Initialized)
		return;

	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(m_glrc);

	m_glrc = NULL;
	m_hdc = NULL;
	m_hwnd = NULL;
}


bool RendererImpl::BeginScene(props::TFlags64 flags)
{
	if (!m_Initialized)
		return false;

	gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return true;
}


bool RendererImpl::EndScene(props::TFlags64 flags)
{
	if (!m_Initialized)
		return false;

	gl.Finish();

	SwapBuffers(m_hdc);
	return true;
}

size_t RendererImpl::PixelSize(Renderer::TextureType type)
{
	switch (type)
	{
		case Renderer::TextureType::P8_3CH:
		case Renderer::TextureType::U8_1CH:
			return sizeof(uint8_t) * 1;

		case Renderer::TextureType::P16_3CH:
		case Renderer::TextureType::P16_3CHT:
		case Renderer::TextureType::P16_4CH:
		case Renderer::TextureType::U8_2CH:
			return sizeof(uint8_t) * 2;

		case Renderer::TextureType::U8_3CH:
			return sizeof(uint8_t) * 3;

		case Renderer::TextureType::U8_3CHX:
		case Renderer::TextureType::U8_4CH:
			return sizeof(uint8_t) * 4;

		case Renderer::TextureType::F16_1CH:
			return sizeof(uint16_t) * 1;

		case Renderer::TextureType::F16_2CH:
			return sizeof(uint16_t) * 2;

		case Renderer::TextureType::F16_4CH:
			return sizeof(uint16_t) * 4;

		case Renderer::TextureType::F32_1CH:
			return sizeof(float) * 1;

		case Renderer::TextureType::F32_2CH:
			return sizeof(float) * 2;

		case Renderer::TextureType::F32_4CH:
			return sizeof(float) * 4;
	}

	return 0;
}


GLenum RendererImpl::GLType(TextureType type)
{
	switch (type)
	{
		case Renderer::TextureType::P8_3CH:
			return GL_UNSIGNED_BYTE_3_3_2;

		case Renderer::TextureType::U8_1CH:
		case Renderer::TextureType::U8_2CH:
		case Renderer::TextureType::U8_3CH:
		case Renderer::TextureType::U8_3CHX:
		case Renderer::TextureType::U8_4CH:
			return GL_UNSIGNED_BYTE;

		case Renderer::TextureType::P16_3CH:
			return GL_UNSIGNED_SHORT_5_6_5;

		case Renderer::TextureType::P16_3CHT:
			return GL_UNSIGNED_SHORT_5_5_5_1;

		case Renderer::TextureType::P16_4CH:
			return GL_UNSIGNED_SHORT_4_4_4_4;

		case Renderer::TextureType::F16_1CH:
		case Renderer::TextureType::F16_2CH:
		case Renderer::TextureType::F16_3CH:
		case Renderer::TextureType::F16_4CH:
			return GL_HALF_FLOAT;

		case Renderer::TextureType::F32_1CH:
		case Renderer::TextureType::F32_2CH:
		case Renderer::TextureType::F32_3CH:
		case Renderer::TextureType::F32_4CH:
			return GL_FLOAT;
	}

	return 0;
}


GLenum RendererImpl::GLInternalFormat(TextureType type)
{
	switch (type)
	{
		case Renderer::TextureType::P8_3CH:
			return GL_UNSIGNED_BYTE_3_3_2;

		case Renderer::TextureType::U8_1CH:
			return GL_R8UI;

		case Renderer::TextureType::U8_2CH:
			return GL_RG8UI;

		case Renderer::TextureType::U8_3CH:
			return GL_RGB8UI;

		case Renderer::TextureType::P16_3CH:
			return GL_UNSIGNED_SHORT_5_6_5;

		case Renderer::TextureType::P16_3CHT:
			return GL_UNSIGNED_SHORT_5_5_5_1;

		case Renderer::TextureType::P16_4CH:
			return GL_UNSIGNED_SHORT_4_4_4_4;

		case Renderer::TextureType::U8_3CHX:
		case Renderer::TextureType::U8_4CH:
			return GL_RGBA8UI;

		case Renderer::TextureType::F16_1CH:
			return GL_R16F;

		case Renderer::TextureType::F16_2CH:
			return GL_RG16F;

		case Renderer::TextureType::F16_3CH:
			return GL_RGB16F;

		case Renderer::TextureType::F16_4CH:
			return GL_RGBA16F;

		case Renderer::TextureType::F32_1CH:
			return GL_R32F;

		case Renderer::TextureType::F32_2CH:
			return GL_RG32F;

		case Renderer::TextureType::F32_3CH:
			return GL_RGB32F;

		case Renderer::TextureType::F32_4CH:
			return GL_RGBA32F;
	}

	return 0;
}

GLenum RendererImpl::GLFormat(TextureType type)
{
	switch (type)
	{
		case Renderer::TextureType::U8_1CH:
		case Renderer::TextureType::F16_1CH:
		case Renderer::TextureType::F32_1CH:
			return GL_RED;

		case Renderer::TextureType::U8_2CH:
		case Renderer::TextureType::F16_2CH:
		case Renderer::TextureType::F32_2CH:
			return GL_RG;

		case Renderer::TextureType::P8_3CH:
		case Renderer::TextureType::P16_3CH:
		case Renderer::TextureType::P16_3CHT:
		case Renderer::TextureType::U8_3CH:
		case Renderer::TextureType::U8_3CHX:
		case Renderer::TextureType::F16_3CH:
		case Renderer::TextureType::F32_3CH:
			return GL_RGB;

		case Renderer::TextureType::P16_4CH:
		case Renderer::TextureType::U8_4CH:
		case Renderer::TextureType::F16_4CH:
		case Renderer::TextureType::F32_4CH:
			return GL_RGBA;
	}

	return 0;
}


Texture *RendererImpl::CreateTexture2D(size_t width, size_t height, TextureType type, size_t mipcount, props::TFlags64 flags)
{
	return new Texture2DImpl(this, width, height, type, mipcount);
}


Texture *RendererImpl::CreateTextureCube(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount, props::TFlags64 flags)
{
	return new TextureCubeImpl(this, width, height, depth, type, mipcount);
}


Texture *RendererImpl::CreateTexture3D(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount, props::TFlags64 flags)
{
	return new Texture3DImpl(this, width, height, depth, type, mipcount);
}


DepthBuffer* RendererImpl::CreateDepthBuffer(size_t width, size_t height, DepthType type, props::TFlags64 flags)
{
	return new DepthBufferImpl(this, width, height, type);
}


FrameBuffer *RendererImpl::CreateFrameBuffer(props::TFlags64 flags)
{
	return NULL;
}


VertexBuffer *RendererImpl::CreateVertexBuffer(props::TFlags64 flags)
{
	return NULL;
}


IndexBuffer *RendererImpl::CreateIndexBuffer(props::TFlags64 flags)
{
	return NULL;
}


ShaderProgram *RendererImpl::CreateShaderProgram()
{
	return new ShaderProgramImpl(this);
}


ShaderComponent *RendererImpl::CreateShaderComponent(ShaderComponentType type)
{
	return new ShaderComponentImpl(this, type);
}


void RendererImpl::UseFrameBuffer(FrameBuffer *pfb)
{
}


void RendererImpl::UseProgram(ShaderProgram *pprog)
{
	if (pprog)
		gl.UseProgram((c3::ShaderProgramImpl &)*pprog);
}


bool RendererImpl::DrawPrimitives(PrimType type, size_t count)
{
	return true;
}


bool RendererImpl::DrawIndexedPrimitives(PrimType type, size_t offset, size_t count)
{
	return true;
}


void RendererImpl::SetProjectionMatrix(const C3MATRIX *m)
{
	if (*m != m_proj)
	{
		m_proj = *m;

		gl.MatrixMode(GL_PROJECTION);
		gl.LoadMatrixf((GLfloat *)&m_proj);
	}
}


void RendererImpl::SetViewMatrix(const C3MATRIX *m)
{
	if (*m != m_view)
	{
		m_view = *m;
		m_worldview = m_world * m_view;

		gl.MatrixMode(GL_MODELVIEW);
		gl.LoadMatrixf((GLfloat *)&m_worldview);
	}
}


void RendererImpl::SetWorldMatrix(const C3MATRIX *m)
{
	if (*m != m_world)
	{
		m_world = *m;
		m_worldview = m_world * m_view;

		gl.MatrixMode(GL_MODELVIEW);
		gl.LoadMatrixf((GLfloat *)&m_worldview);
	}
}
