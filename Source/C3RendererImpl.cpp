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
#include <C3VertexBufferImpl.h>
#include <C3IndexBufferImpl.h>
#include <C3MeshImpl.h>
#include <C3FrameBufferImpl.h>

using namespace c3;


RendererImpl::RendererImpl(SystemImpl *psys)
{
	m_pSys = psys;

	m_hwnd = NULL;
	m_hdc = NULL;
	m_glrc = NULL;

	m_clearZ = 1.0f;

	m_CurFB = nullptr;
	m_CurFBID = 0;

	m_CurIB = nullptr;
	m_CurIBID = 0;

	m_CurVB = nullptr;
	m_CurVBID = 0;

	m_CurProg = nullptr;
	m_CurProgID = 0;

	m_CubeVB = nullptr;
	m_BoundsMesh = nullptr;
	m_CubeMesh = nullptr;

	memset(&m_glARBWndClass, 0, sizeof(WNDCLASS));

	m_ident = glm::identity<glm::fmat4x4>();

	m_matupflags.Set(MATRIXUPDATE_ALL);

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

	PIXELFORMATDESCRIPTOR pfd =
	{
	   sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd  
	   1,								// version number  
	   PFD_DRAW_TO_WINDOW |			// support window  
	   PFD_SUPPORT_OPENGL |			// support OpenGL  
	   PFD_DOUBLEBUFFER,				// double buffered  
	   PFD_TYPE_RGBA,					// RGBA type  
	   32,								// 32-bit color depth  
	   0, 0, 0, 0, 0, 0,				// color bits ignored  
	   0,								// no alpha buffer  
	   0,								// shift bit ignored  
	   0,								// no accumulation buffer  
	   0, 0, 0, 0,						// accum bits ignored  
	   24,								// 24-bit z-buffer  
	   8,								// 8-bit stencil buffer  
	   0,								// no auxiliary buffer  
	   PFD_MAIN_PLANE,					// main layer  
	   0,								// reserved  
	   0, 0, 0							// layer masks ignored  
	};

	int  iPixelFormat;

#define GLARBWND_CLASSNAME		_T("CELERITY3_OPENGL_ARB_WNDCLASS")

	HMODULE hmod_temp = GetModuleHandle(NULL);

	if (!GetClassInfo(hmod_temp, GLARBWND_CLASSNAME, &m_glARBWndClass))
	{
		m_glARBWndClass.style = CS_HREDRAW | CS_VREDRAW;
		m_glARBWndClass.lpfnWndProc = DefWindowProc;
		m_glARBWndClass.cbClsExtra = 0;
		m_glARBWndClass.cbWndExtra = 0;
		m_glARBWndClass.hInstance = hmod_temp;
		m_glARBWndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		m_glARBWndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		m_glARBWndClass.hbrBackground = (struct HBRUSH__ *)GetStockObject(BLACK_BRUSH);
		m_glARBWndClass.lpszMenuName = NULL;
		m_glARBWndClass.lpszClassName = GLARBWND_CLASSNAME;

		if (!RegisterClass(&m_glARBWndClass))
			return false;

		HWND hwnd_temp = CreateWindow(GLARBWND_CLASSNAME, GLARBWND_CLASSNAME,
									  WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
									  0, 0, 1, 1, NULL, NULL, hmod_temp, NULL);
		if (hwnd_temp != NULL)
		{
			HDC hdc_temp = GetDC(hwnd_temp);

			iPixelFormat = ChoosePixelFormat(hdc_temp, &pfd);

			SetPixelFormat(hdc_temp, iPixelFormat, &pfd);

			HGLRC hglrc_temp = wglCreateContext(hdc_temp);

			wglMakeCurrent(hdc_temp, hglrc_temp);

			PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
			wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));

			PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
			wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));

			const int pixelAttribList[] =
			{
				WGL_DRAW_TO_WINDOW_ARB,		GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB,		GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
				WGL_PIXEL_TYPE_ARB,			WGL_TYPE_RGBA_ARB,
				WGL_ACCELERATION_ARB,		WGL_FULL_ACCELERATION_ARB,
				WGL_RED_BITS_ARB,			8,
				WGL_GREEN_BITS_ARB,			8,
				WGL_BLUE_BITS_ARB,			8,
				WGL_ALPHA_BITS_ARB,			8,
				WGL_DEPTH_BITS_ARB,			24,
				WGL_STENCIL_BITS_ARB,		8,
				WGL_SAMPLE_BUFFERS_ARB,		GL_TRUE,
				WGL_SAMPLES_ARB,			4,

				0, 0	// end
			};

			UINT numFormats;
			wglChoosePixelFormatARB(m_hdc, pixelAttribList, NULL, 1, &iPixelFormat, &numFormats);
			DescribePixelFormat(m_hdc, iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

			// make that the pixel format of the device context  
			SetPixelFormat(m_hdc, iPixelFormat, &pfd);

			const int contextAttribList[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB,	4,
				WGL_CONTEXT_MINOR_VERSION_ARB,	5,
				WGL_CONTEXT_FLAGS_ARB,			WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
				WGL_CONTEXT_PROFILE_MASK_ARB,	WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0, 0	// end
			};

			m_glrc = wglCreateContextAttribsARB(m_hdc, 0, contextAttribList);
			m_glrc_aux = wglCreateContextAttribsARB(m_hdc, 0, contextAttribList);
			wglShareLists(m_glrc_aux, m_glrc);

			wglMakeCurrent(NULL, NULL);

			wglDeleteContext(hglrc_temp);
			ReleaseDC(hwnd_temp, hdc_temp);
			DestroyWindow(hwnd_temp);

			UnregisterClass(m_glARBWndClass.lpszClassName, hmod_temp);

			wglMakeCurrent(m_hdc, m_glrc);
		}
	}

	if (!gl.Initialize())
		return false;

	m_Initialized = true;

	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(wglGetProcAddress("wglSwapIntervalEXT"));
	if (wglSwapIntervalEXT)
		wglSwapIntervalEXT(0);

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

	// This vertex array is global... only using VBO/IBO pairs. I don't know if this is "correct"
	gl.GenVertexArrays(1, &m_VAOglID);
	gl.BindVertexArray(m_VAOglID);

	SetClearColor();
	gl.ClearDepthf(m_clearZ);
	gl.Enable(GL_DEPTH_TEST);
	gl.DepthFunc(GL_LEQUAL);

	return true;
}


bool RendererImpl::Initialized()
{
	return m_Initialized;
}


void RendererImpl::Shutdown()
{
	if (!m_Initialized)
		return;

	if (m_BoundsMesh)
	{
		m_BoundsMesh->GetIndexBuffer()->Release();
		m_BoundsMesh->AttachIndexBuffer(nullptr);
		m_BoundsMesh->AttachVertexBuffer(nullptr);
		m_BoundsMesh->Release();
		m_BoundsMesh = nullptr;
	}

	if (m_CubeMesh)
	{
		m_CubeMesh->GetIndexBuffer()->Release();
		m_CubeMesh->AttachIndexBuffer(nullptr);
		m_CubeMesh->AttachVertexBuffer(nullptr);
		m_CubeMesh->Release();
		m_CubeMesh = nullptr;
	}

	if (m_CubeVB)
	{
		m_CubeVB->Release();
		m_CubeVB = nullptr;
	}

	// Delete the global vertex array
	gl.BindVertexArray(0);
	gl.DeleteVertexArrays(1, &m_VAOglID);

	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(m_glrc_aux);
	wglDeleteContext(m_glrc);

	m_glrc = NULL;
	m_glrc_aux = NULL;
	m_hdc = NULL;
	m_hwnd = NULL;
}


c3::System *RendererImpl::GetSystem()
{
	return (c3::System *)m_pSys;
}


void RendererImpl::SetClearColor(const glm::fvec4 *color)
{
	const static glm::fvec4 defcolor = glm::fvec4(0, 0, 0, 1);

	bool update = false;
	if (!color)
		color = &defcolor;

	if (m_clearColor != *color)
	{
		m_clearColor = *color;
		update = true;
	}

	if (update)
		gl.ClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
}


const glm::fvec4 *RendererImpl::GetClearColor(glm::fvec4 *color)
{
	glm::fvec4 *ret = color;

	if (ret)
		*ret = m_clearColor;

	return ret;
}


void RendererImpl::SetClearDepth(float depth)
{
	if (depth != m_clearZ)
	{
		m_clearZ = depth;
		gl.ClearDepth(m_clearZ);
	}
}


float RendererImpl::GetClearDepth()
{
	return m_clearZ;
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

	if (m_needFinish)
	{
		gl.Finish();
		m_needFinish = false;
	}

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
			return GL_R8;

		case Renderer::TextureType::U8_2CH:
			return GL_RG8;

		case Renderer::TextureType::U8_3CH:
			return GL_RGB8;

		case Renderer::TextureType::P16_3CH:
			return GL_UNSIGNED_SHORT_5_6_5;

		case Renderer::TextureType::P16_3CHT:
			return GL_UNSIGNED_SHORT_5_5_5_1;

		case Renderer::TextureType::P16_4CH:
			return GL_UNSIGNED_SHORT_4_4_4_4;

		case Renderer::TextureType::U8_3CHX:
		case Renderer::TextureType::U8_4CH:
			return GL_RGBA8;

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


Texture2D *RendererImpl::CreateTexture2D(size_t width, size_t height, TextureType type, size_t mipcount, props::TFlags64 flags)
{
	return new Texture2DImpl(this, width, height, type, mipcount, flags);
}


TextureCube *RendererImpl::CreateTextureCube(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount, props::TFlags64 flags)
{
	return new TextureCubeImpl(this, width, height, depth, type, mipcount, flags);
}


Texture3D *RendererImpl::CreateTexture3D(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount, props::TFlags64 flags)
{
	return new Texture3DImpl(this, width, height, depth, type, mipcount, flags);
}


DepthBuffer* RendererImpl::CreateDepthBuffer(size_t width, size_t height, DepthType type, props::TFlags64 flags)
{
	return new DepthBufferImpl(this, width, height, type);
}


FrameBuffer *RendererImpl::CreateFrameBuffer(props::TFlags64 flags)
{
	return new FrameBufferImpl(this);
}


VertexBuffer *RendererImpl::CreateVertexBuffer(props::TFlags64 flags)
{
	return new VertexBufferImpl(this);
}


IndexBuffer *RendererImpl::CreateIndexBuffer(props::TFlags64 flags)
{
	return new IndexBufferImpl(this);
}


Mesh *RendererImpl::CreateMesh()
{
	return new MeshImpl(this);
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
	if (pfb == m_CurFB)
		return;

	GLuint glid = 0;
	if (pfb)
		glid = (c3::FrameBufferImpl &)*pfb;

	m_CurFB = pfb;
	m_CurFBID = glid;

	gl.BindFramebuffer(GL_FRAMEBUFFER, glid);
}


FrameBuffer *RendererImpl::GetActiveFrameBuffer()
{
	return m_CurFB;
}


void RendererImpl::UseProgram(ShaderProgram *pprog)
{
	if (pprog == m_CurProg)
		return;

	m_CurProg = pprog;
	GLuint glid = m_CurProg ? (GLuint)(c3::ShaderProgramImpl &)*pprog : 0;
	if (glid == GL_INVALID_VALUE)
		glid = 0;

	if (glid == m_CurProgID)
		return;

	m_Config = true;

	m_CurProgID = glid;

	gl.UseProgram(m_CurProgID);
}


void RendererImpl::UseVertexBuffer(VertexBuffer *pvbuf)
{
	if (pvbuf == m_CurVB)
		return;

	m_CurVB = pvbuf;
	GLuint glid = m_CurVB ? (GLuint)(c3::VertexBufferImpl &)*pvbuf : 0;
	if (glid == GL_INVALID_VALUE)
		glid = 0;

	if (glid == m_CurVBID)
		return;

	m_Config = true;

	m_CurVBID = glid;

	gl.BindBuffer(GL_ARRAY_BUFFER, glid);
}


void RendererImpl::UseIndexBuffer(IndexBuffer *pibuf)
{
	if (pibuf == m_CurIB)
		return;

	m_CurIB = pibuf;
	GLuint glid = m_CurIB ? (GLuint)(c3::IndexBufferImpl &)*pibuf : 0;
	if (glid == GL_INVALID_VALUE)
		glid = 0;

	if (glid == m_CurIBID)
		return;

	m_CurIBID = glid;

	gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, glid);
}


bool RendererImpl::ConfigureDrawing()
{
	if (!m_Config)
		return true;

	if (m_CurVB && m_CurProg)
	{
		static const GLenum t[VertexBuffer::ComponentDescription::ComponentType::VCT_NUM_TYPES] = { 0, GL_UNSIGNED_BYTE, GL_BYTE, GL_HALF_FLOAT, GL_UNSIGNED_INT, GL_FLOAT };

		size_t vsz = m_CurVB->VertexSize();
		size_t vo = 0;

		static const char *vattrname[VertexBuffer::ComponentDescription::Usage::VU_NUM_USAGES] =
		{
				"",
				"vPos",
				"vNorm",
				"vTex0",
				"vTex1",
				"vTex2",
				"vTex3",
				"vTan",
				"vBinorm",
				"vIndex",
				"vWeight",
				"vColor0",
				"vColor1",
				"vColor2",
				"vColor3",
				"vSize"
		};

		for (size_t i = 0, maxi = m_CurVB->NumComponents(); i < maxi; i++)
		{
			const VertexBuffer::ComponentDescription *pcd = m_CurVB->Component(i);
			if (!pcd || !pcd->m_Count || (pcd->m_Type == VertexBuffer::ComponentDescription::VCT_NONE) || (pcd->m_Usage == VertexBuffer::ComponentDescription::Usage::VU_NONE))
				break;

			GLint vloc = gl.GetAttribLocation(m_CurProgID, vattrname[pcd->m_Usage]);

			if (vloc < 0)
				return false;

			assert(vloc == i);

			gl.EnableVertexAttribArray(vloc);
			gl.VertexAttribPointer(vloc, (GLint)pcd->m_Count, GL_FLOAT, GL_FALSE, (GLsizei)vsz, (void *)vo);
			vo += pcd->size();
		}
	}

	m_Config = false;
	return true;
}


static const GLenum typelu[Renderer::PrimType::NUM_PRIMTYPES] = { GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN };

bool RendererImpl::DrawPrimitives(PrimType type, size_t count)
{
	if (m_CurVBID)
	{
		if (count == -1)
			count = m_CurVB->Count();

		if (m_Config)
			ConfigureDrawing();

		gl.DrawArrays(typelu[type], 0, (GLsizei)count);

		return true;
	}

	return false;
}


bool RendererImpl::DrawIndexedPrimitives(PrimType type, size_t offset, size_t count)
{
	if (m_CurIBID && m_CurVBID)
	{
		if (count == -1)
			count = m_CurIB->Count();

		size_t idxs = m_CurIB->GetIndexSize();
		if (!idxs)
			return false;
		idxs--;

		static const GLuint glidxs[3] = {GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT};

		if (m_Config)
			ConfigureDrawing();

		gl.DrawElements(typelu[type], (GLsizei)count, glidxs[idxs], NULL);

		return true;
	}

	return false;
}


void RendererImpl::SetProjectionMatrix(const glm::fmat4x4 *m)
{
	if (*m != m_proj)
	{
		m_proj = *m;
		m_matupflags.Set(MATRIXUPDATE_PROJ);
	}
}


void RendererImpl::SetViewMatrix(const glm::fmat4x4 *m)
{
	if (*m != m_view)
	{
		m_view = *m;
		m_matupflags.Set(MATRIXUPDATE_VIEW);
	}
}


void RendererImpl::SetWorldMatrix(const glm::fmat4x4 *m)
{
	if (*m != m_world)
	{
		m_world = *m;
		m_matupflags.Set(MATRIXUPDATE_WORLD);
	}
}


const glm::fmat4x4 *RendererImpl::GetProjectionMatrix(glm::fmat4x4 *m)
{
	if (!m)
		return &m_proj;

	*m = m_proj;
	return m;
}


const glm::fmat4x4 *RendererImpl::GetViewMatrix(glm::fmat4x4 *m)
{
	if (!m)
		return &m_view;

	*m = m_view;
	return m;
}


const glm::fmat4x4 *RendererImpl::GetWorldMatrix(glm::fmat4x4 *m)
{
	if (!m)
		return &m_world;

	*m = m_world;
	return m;
}


const glm::fmat4x4 *RendererImpl::GetViewProjectionMatrix(glm::fmat4x4 *m)
{
	if (m_matupflags.AnySet(MATRIXUPDATE_VIEWPROJ))
	{
		m_viewproj = m_view * m_proj;
		m_matupflags.Clear(MATRIXUPDATE_VIEWPROJ);
	}

	if (!m)
		return &m_viewproj;

	*m = m_viewproj;
	return m;
}


const glm::fmat4x4 *RendererImpl::GetWorldViewProjectionMatrix(glm::fmat4x4 *m)
{
	if (m_matupflags.AnySet(MATRIXUPDATE_ALL))
	{
		m_worldviewproj = m_world * *GetViewProjectionMatrix();
		m_matupflags.Clear(MATRIXUPDATE_ALL);
	}

	if (!m)
		return &m_worldviewproj;

	*m = m_worldviewproj;
	return m;
}


VertexBuffer *RendererImpl::GetCubeVB()
{
	if (!m_CubeVB)
	{
		m_CubeVB = CreateVertexBuffer(0);

		c3::VertexBuffer::ComponentDescription comps[4] ={
			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_POSITION},
			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_NORMAL},

			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_NONE, 0, c3::VertexBuffer::ComponentDescription::Usage::VU_NONE} // terminator
		};

		typedef struct
		{
			glm::vec3 pos;
			glm::vec3 norm;
		} SCubeVert;

		static const SCubeVert v[8 * 3] =
		{
			{ { -1, -1, -1 }, { -1,  0,  0 } },		// A
			{ { -1, -1, -1 }, {  0, -1,  0 } },
			{ { -1, -1, -1 }, {  0,  0, -1 } },

			{ { -1, -1,  1 }, { -1,  0,  0 } },		// E
			{ { -1, -1,  1 }, {  0, -1,  0 } },
			{ { -1, -1,  1 }, {  0,  0,  1 } },

			{ { -1,  1, -1 }, { -1,  0,  0 } },		// B
			{ { -1,  1, -1 }, {  0,  1,  0 } },
			{ { -1,  1, -1 }, {  0,  0, -1 } },

			{ { -1,  1,  1 }, { -1,  0,  0 } },		// F
			{ { -1,  1,  1 }, {  0,  1,  0 } },
			{ { -1,  1,  1 }, {  0,  0,  1 } },

			{ {  1,  1, -1 }, {  1,  0,  0 } },		// C
			{ {  1,  1, -1 }, {  0,  1,  0 } },
			{ {  1,  1, -1 }, {  0,  0, -1 } },

			{ {  1,  1,  1 }, {  1,  0,  0 } },		// G
			{ {  1,  1,  1 }, {  0,  1,  0 } },
			{ {  1,  1,  1 }, {  0,  0,  1 } },

			{ {  1, -1, -1 }, {  1,  0,  0 } },		// D
			{ {  1, -1, -1 }, {  0, -1,  0 } },
			{ {  1, -1, -1 }, {  0,  0, -1 } },

			{ {  1, -1,  1 }, {  1,  0,  0 } },		// H
			{ {  1, -1,  1 }, {  0, -1,  0 } },
			{ {  1, -1,  1 }, {  0,  0,  1 } }
		};

		void *buf;
		if (m_CubeVB->Lock(&buf, 8 * 3, comps, VBLOCKFLAG_WRITE) == VertexBuffer::RETURNCODE::RET_OK)
		{
			memcpy(buf, v, sizeof(SCubeVert) * 8 * 3);

			m_CubeVB->Unlock();
		}
	}

	return m_CubeVB;
}

Mesh *RendererImpl::GetBoundsMesh()
{
	if (!m_BoundsMesh)
	{
		m_BoundsMesh = CreateMesh();

		m_BoundsMesh->AttachVertexBuffer(GetCubeVB());

		if (m_CubeVB)
		{
			IndexBuffer *pib = CreateIndexBuffer(0);
			if (pib)
			{
				static const uint16_t i[12][2] ={
					{0 , 6 },
					{6 , 12},
					{12, 18},
					{18, 0 },
					{3 , 9 },
					{9 , 15},
					{15, 21},
					{21, 3 },
					{0 , 3 },
					{6 , 9 },
					{12, 15},
					{18, 21}
				};

				void *buf;
				if (pib->Lock(&buf, 12 * 2, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE) == IndexBuffer::RETURNCODE::RET_OK)
				{
					memcpy(buf, i, sizeof(uint16_t) * 12 * 2);

					pib->Unlock();
				}

				m_BoundsMesh->AttachIndexBuffer(pib);
			}
		}
	}

	return m_BoundsMesh;
}

Mesh *RendererImpl::GetCubeMesh()
{
	if (!m_CubeMesh)
	{
		m_CubeMesh = CreateMesh();

		m_CubeMesh->AttachVertexBuffer(GetCubeVB());

		if (m_CubeVB)
		{
			IndexBuffer *pib = CreateIndexBuffer(0);
			if (pib)
			{
				static const uint16_t i[6][2][3] ={
					{ {0 , 6 , 9 }, {0 , 9 , 3 } },	// -x
					{ {6 , 12, 15}, {6 , 15, 9 } },	// +y
					{ {12, 18, 21}, {12, 21, 15} },	// +x
					{ {18, 0 , 3 }, {18, 9 , 3 } },	// -y
					{ {0 , 12, 6 }, {0 , 18, 12} },	// -z
					{ {3 , 15, 9 }, {3 , 21, 15} },	// +z
				};

				void *buf;
				if (pib->Lock(&buf, 6 * 2 * 3, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE) == IndexBuffer::RETURNCODE::RET_OK)
				{
					memcpy(buf, i, sizeof(uint16_t) * 12 * 2);

					pib->Unlock();
				}

				m_CubeMesh->AttachIndexBuffer(pib);
			}
		}
	}

	return m_CubeMesh;
}
