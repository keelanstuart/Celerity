// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2020, Keelan Stuart


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
#include <C3SystemImpl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


using namespace c3;


RendererImpl::RendererImpl(SystemImpl *psys)
{
	m_pSys = psys;

	m_hwnd = NULL;
	m_hdc = NULL;
	m_glrc = NULL;
	m_hwnd_override = NULL;

	m_glVersionMaj = 4;
	m_glVersionMin = 5;

	m_event_shutdown = CreateEvent(nullptr, TRUE, TRUE, nullptr);

	// No extra threads, just run everything in this thread
	m_TaskPool = pool::IThreadPool::Create(0);

	m_needsFinish = false;

	m_clearZ = 1.0f;
	m_DepthMode = DepthMode::DM_READWRITE;
	m_DepthTest = DepthTest::DT_LESSEREQUAL;
	m_CullMode = CullMode::CM_BACK;

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

	m_PlanesVB = nullptr;
	m_XYPlaneMesh = nullptr;
	m_XZPlaneMesh = nullptr;
	m_YZPlaneMesh = nullptr;

	m_HemisphereVB = nullptr;
	m_HemisphereMesh = nullptr;

	m_BlackTex = nullptr;
	m_GreyTex = nullptr;
	m_WhiteTex = nullptr;
	m_BlueTex = nullptr;
	m_GridTex = nullptr;

	memset(&m_glARBWndClass, 0, sizeof(WNDCLASS));

	m_ident = glm::identity<glm::fmat4x4>();

	m_matupflags.Set(MATRIXUPDATE_ALL);

	m_Initialized = false;

	m_Gui = nullptr;
}


RendererImpl::~RendererImpl()
{
	m_TaskPool->Release();

	Shutdown();

	CloseHandle(m_event_shutdown);
}


c3::System *RendererImpl::GetSystem()
{
	return (c3::System *)m_pSys;
}


bool RendererImpl::Initialize(HWND hwnd, props::TFlags64 flags)
{
	m_hwnd = hwnd;
	m_hdc = ::GetDC(hwnd);

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd  
		1,								// version number  
		PFD_DRAW_TO_WINDOW |			// support window  
		PFD_SUPPORT_OPENGL |			// support OpenGL  
		PFD_SUPPORT_COMPOSITION |		// support compositing
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

			// We want to get the highest version of opengl available...
			typedef const GLubyte * (APIENTRY *PFNGLGETSTRINGPROC) (GLenum name);
			PFNGLGETSTRINGPROC glGetString = nullptr;
			HMODULE module = LoadLibrary(_T("opengl32.dll"));
			glGetString = reinterpret_cast<PFNGLGETSTRINGPROC>(GetProcAddress(module, "glGetString"));
			if (glGetString)
			{
				std::string vers = (char *)glGetString(GL_VERSION);
				sscanf_s(vers.c_str(), "%d.%d", &m_glVersionMaj, &m_glVersionMin);
			}

			PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
			wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));

			PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
			wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));

			const int pixelAttribList[] =
			{
				WGL_DRAW_TO_WINDOW_ARB,		GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB,		GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
				WGL_TRANSPARENT_ARB,		GL_TRUE,
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
				WGL_CONTEXT_MAJOR_VERSION_ARB,	(int)m_glVersionMaj,
				WGL_CONTEXT_MINOR_VERSION_ARB,	(int)m_glVersionMin,
				WGL_CONTEXT_FLAGS_ARB,			WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
				WGL_CONTEXT_PROFILE_MASK_ARB,	WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0, 0	// end
			};

			m_glrc = wglCreateContextAttribsARB(m_hdc, 0, contextAttribList);

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

	ResetEvent(m_event_shutdown);

	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(wglGetProcAddress("wglSwapIntervalEXT"));
	if (wglSwapIntervalEXT)
		wglSwapIntervalEXT(0);

	SetProjectionMatrix(&m_ident);
	SetViewMatrix(&m_ident);
	SetWorldMatrix(&m_ident);

	// This vertex array is global... only using VBO/IBO pairs. I don't know if this is "correct",
	// but I bind it only once and never touch it again.
	gl.GenVertexArrays(1, &m_VAOglID);
	gl.BindVertexArray(m_VAOglID);

	gl.ClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
	gl.ClearDepthf(m_clearZ);

	gl.Enable(GL_DEPTH_TEST);
	gl.DepthFunc(GL_LEQUAL);
	gl.DepthMask(GL_TRUE);

	gl.FrontFace(GL_CCW);
	gl.Enable(GL_CULL_FACE);
	gl.CullFace(GL_BACK);

	gl.Enable(GL_BLEND);
	gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialize meshes
	GetCubeMesh();
	GetBoundsMesh();
	GetHemisphereMesh();
	GetXYPlaneMesh();
	GetXZPlaneMesh();
	GetYZPlaneMesh();

	// Initizlie textures
	GetBlackTexture();
	GetGreyTexture();
	GetWhiteTexture();
	GetBlueTexture();
	GetGridTexture();

	m_Gui = nullptr; //new GuiImpl(this);

	SetViewport();

	return true;
}


bool RendererImpl::Initialized()
{
	return m_Initialized;
}

bool __cdecl UnloadRenderResource(c3::Resource *pres)
{
	while (pres->GetStatus() == Resource::Status::RS_LOADED)
		pres->DelRef();

	return true;
}

void RendererImpl::Shutdown()
{
	if (!m_Initialized)
		return;

	m_pSys->GetResourceManager()->ForAllResourcesDo(UnloadRenderResource, nullptr, RTFLAG_RUNBYRENDERER, ResourceManager::ResTypeFlagMode::RTFM_ANY);

	SetEvent(m_event_shutdown);

	if (m_Gui)
	{
		delete m_Gui;
		m_Gui = nullptr;
	}

	if (m_BlackTex)
	{
		m_BlackTex->Release();
		m_BlackTex = nullptr;
	}

	if (m_GreyTex)
	{
		m_GreyTex->Release();
		m_GreyTex = nullptr;
	}

	if (m_WhiteTex)
	{
		m_WhiteTex->Release();
		m_WhiteTex = nullptr;
	}

	if (m_BlueTex)
	{
		m_BlueTex->Release();
		m_BlueTex = nullptr;
	}

	if (m_GridTex)
	{
		m_GridTex->Release();
		m_GridTex = nullptr;
	}

	if (m_BoundsMesh)
	{
		m_BoundsMesh->AttachVertexBuffer(nullptr);
		m_BoundsMesh->Release();
		m_BoundsMesh = nullptr;
	}

	if (m_CubeMesh)
	{
		m_CubeMesh->AttachVertexBuffer(nullptr);
		m_CubeMesh->Release();
		m_CubeMesh = nullptr;
	}

	if (m_CubeVB)
	{
		m_CubeVB->Release();
		m_CubeVB = nullptr;
	}

	if (m_XYPlaneMesh)
	{
		m_XYPlaneMesh->AttachVertexBuffer(nullptr);
		m_XYPlaneMesh->Release();
		m_XYPlaneMesh = nullptr;
	}

	if (m_XZPlaneMesh)
	{
		m_XZPlaneMesh->AttachVertexBuffer(nullptr);
		m_XZPlaneMesh->Release();
		m_XZPlaneMesh = nullptr;
	}

	if (m_YZPlaneMesh)
	{
		m_YZPlaneMesh->AttachVertexBuffer(nullptr);
		m_YZPlaneMesh->Release();
		m_YZPlaneMesh = nullptr;
	}

	if (m_PlanesVB)
	{
		m_PlanesVB->Release();
		m_PlanesVB = nullptr;
	}

	if (m_HemisphereMesh)
	{
		m_HemisphereMesh->AttachVertexBuffer(nullptr);
		m_HemisphereMesh->Release();
		m_HemisphereMesh = nullptr;
	}

	if (m_HemisphereVB)
	{
		m_HemisphereVB->Release();
		m_HemisphereVB = nullptr;
	}

	// Delete the global vertex array
	gl.BindVertexArray(0);
	gl.DeleteVertexArrays(1, &m_VAOglID);

	wglMakeCurrent(NULL, NULL);

	if (m_glrc)
		wglDeleteContext(m_glrc);

	m_glrc = NULL;
	m_hdc = NULL;
	m_hwnd = NULL;

	m_Initialized = false;
}


pool::IThreadPool *RendererImpl::GetTaskPool()
{
	return m_TaskPool;
}


Gui *RendererImpl::GetGui()
{
	return m_Gui;
}


void RendererImpl::SetViewport(const RECT *viewport)
{
	RECT r;
	if (!viewport)
	{
		GetClientRect(m_hwnd, &r);
		viewport = &r;
	}

	memcpy(&m_Viewport, viewport, sizeof(RECT));

	LONG w = r.right - r.left;
	LONG h = r.bottom - r.top;
	gl.Viewport(r.left, r.top, w, h);

	if (m_Gui)
	{
		m_Gui->SetDisplaySize((float)w, (float)h);
	}
}


const RECT *RendererImpl::GetViewport(RECT *viewport) const
{
	if (viewport)
	{
		memcpy(viewport, &m_Viewport, sizeof(RECT));
		return viewport;
	}

	return &m_Viewport;
}


void RendererImpl::SetOverrideHwnd(HWND hwnd)
{
	m_hwnd_override = hwnd;
}


HWND RendererImpl::GetOverrideHwnd()
{
	return m_hwnd_override;
}


bool RendererImpl::BeginScene(props::TFlags64 flags)
{
	if (!m_Initialized)
		return false;

	m_needsFinish = true;

	m_TaskPool->Flush();

	if (m_Gui)
		m_Gui->BeginFrame();

	gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return true;
}


bool RendererImpl::EndScene(props::TFlags64 flags)
{
	if (!m_Initialized)
		return false;

	if (!m_needsFinish)
		return false;

	if (m_Gui)
	{
		static bool show_metrics = true;
		if (show_metrics)
		{
			ImGui::ShowMetricsWindow(&show_metrics);
		}

		m_Gui->Render();
	}

	gl.Finish();

	m_pSys->SetCurrentFrameNumber(m_pSys->GetCurrentFrameNumber() + 1);

	return true;
}


bool RendererImpl::Present()
{
	if (!m_Initialized)
		return false;

	HDC tmpdc = m_hwnd_override ? GetDC(m_hwnd_override) : m_hdc;
	SwapBuffers(tmpdc);
	return true;
}


void RendererImpl::SetClearColor(const glm::fvec4 *color)
{
	const static glm::fvec4 defcolor = glm::fvec4(0, 0, 0, 1);

	if (!color)
		color = &defcolor;

	if (m_clearColor != *color)
	{
		m_clearColor = *color;
		gl.ClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
	}
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


void RendererImpl::SetDepthMode(DepthMode mode)
{
	if (mode != m_DepthMode)
	{
		if ((m_DepthMode == DM_DISABLED) || (m_DepthMode == DM_READONLY))
			gl.DepthMask(GL_TRUE);

		if ((m_DepthMode == DM_DISABLED) || (m_DepthMode == DM_WRITEONLY))
			gl.Enable(GL_DEPTH_TEST);

		if ((mode == DM_DISABLED) || (mode == DM_READONLY))
			gl.DepthMask(GL_FALSE);

		if ((mode == DM_DISABLED) || (mode == DM_WRITEONLY))
			gl.Disable(GL_DEPTH_TEST);

		m_DepthMode = mode;
	}
}


Renderer::DepthMode RendererImpl::GetDepthMode()
{
	return m_DepthMode;
}


void RendererImpl::SetDepthTest(DepthTest test)
{
	if (test != m_DepthTest)
	{
		const static GLenum dtvals[DT_NUMTESTS] ={DT_NEVER, DT_LESSER, DT_LESSEREQUAL, DT_EQUAL, DT_NOTEQUAL, DT_GREATEREQUAL, DT_GREATER, DT_ALWAYS};

		gl.DepthFunc(dtvals[test]);

		m_DepthTest = test;
	}
}


Renderer::DepthTest RendererImpl::GetDepthTest()
{
	return m_DepthTest;
}


void RendererImpl::SetCullMode(CullMode mode)
{
	if (mode != m_CullMode)
	{
		if (mode == CM_DISABLED)
			gl.Disable(GL_CULL_FACE);
		else if (m_CullMode == CM_DISABLED)
			gl.Enable(GL_CULL_FACE);

		switch (mode)
		{
			case CM_FRONT:
				gl.CullFace(GL_FRONT);
				break;

			case CM_BACK:
				gl.CullFace(GL_BACK);
				break;

			case CM_ALL:
				gl.CullFace(GL_FRONT_AND_BACK);
				break;
		}

		m_CullMode = mode;
	}
}


Renderer::CullMode RendererImpl::GetCullMode()
{
	return m_CullMode;
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


Texture2D *RendererImpl::CreateTexture2DFromFile(const TCHAR *filename, props::TFlags64 flags)
{
	Texture2D *ret = nullptr;

	char *_filename;
	CONVERT_TCS2MBCS(filename, _filename);

	unsigned char *data;
	int width, height, numchannels;
	data = stbi_load(_filename, &width, &height, &numchannels, 0);
	if (data)
	{
		Renderer::TextureType tt;

		switch (numchannels)
		{
			case 1:
				tt = Renderer::TextureType::U8_1CH;
				break;

			case 2:
				tt = Renderer::TextureType::U8_2CH;
				break;

			case 3:
				tt = Renderer::TextureType::U8_3CH;
				break;

			case 4:
				tt = Renderer::TextureType::U8_4CH;
				break;
		}

		ret = CreateTexture2D(width, height, tt, 0, flags);
		
		if (ret)
		{
			void *buf;
			Texture2D::SLockInfo li;
			if ((ret->Lock(&buf, li, 0, TEXLOCKFLAG_WRITE | TEXLOCKFLAG_GENMIPS) == Texture2D::RETURNCODE::RET_OK) && buf)
			{
				memcpy(buf, data, width * height * numchannels);

				ret->Unlock();
			}
		}

		free(data);
	}

	return ret;
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


void RendererImpl::UseFrameBuffer(FrameBuffer *pfb, props::TFlags64 flags)
{
	if (pfb == m_CurFB)
		return;

	GLuint glid = 0;
	if (pfb)
		glid = (c3::FrameBufferImpl &)*pfb;

	if (flags.IsSet(UFBFLAG_FINISHLAST))
		gl.Finish();

	DepthBuffer *pdb = pfb ? pfb->GetDepthTarget() : nullptr;
	if (pdb)
	{
		RECT r;

		r.left = 0;
		r.top = 0;
		r.right = (LONG)pdb->Width();
		r.bottom = (LONG)pdb->Height();

		SetViewport(&r);
	}
	else
	{
		SetViewport();
	}

	m_CurFB = pfb;
	m_CurFBID = glid;

	gl.BindFramebuffer(GL_FRAMEBUFFER, glid);

	if (flags.AnySet(UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH))
		gl.Clear((flags.IsSet(UFBFLAG_CLEARCOLOR) ? GL_COLOR_BUFFER_BIT : 0) | (flags.IsSet(UFBFLAG_CLEARCOLOR) ? GL_DEPTH_BUFFER_BIT : 0));
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


void RendererImpl::UseTexture(uint64_t sampler, Texture *ptex)
{
	if (sampler >= 32)
		return;

	static const GLenum sampleridlu[32] =
	{
		GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7,
		GL_TEXTURE8, GL_TEXTURE9, GL_TEXTURE10, GL_TEXTURE11, GL_TEXTURE12, GL_TEXTURE13, GL_TEXTURE14, GL_TEXTURE15,
		GL_TEXTURE16, GL_TEXTURE17, GL_TEXTURE18, GL_TEXTURE19, GL_TEXTURE20, GL_TEXTURE21, GL_TEXTURE22, GL_TEXTURE23,
		GL_TEXTURE24, GL_TEXTURE25, GL_TEXTURE26, GL_TEXTURE27, GL_TEXTURE28, GL_TEXTURE29, GL_TEXTURE30, GL_TEXTURE31
	};

	static Texture *ptexcache[32] ={0};

	if (ptexcache[sampler] == ptex)
		return;

	Texture *luptex = ptex ? ptex : ptexcache[sampler];

	GLuint glid = GL_INVALID_VALUE;
	GLenum textype;

	c3::Texture2DImpl *tex2d = dynamic_cast<c3::Texture2DImpl *>(luptex);
	if (!tex2d)
	{
		c3::TextureCubeImpl *texcube = dynamic_cast<c3::TextureCubeImpl *>(luptex);
		if (!texcube)
		{
			c3::Texture3DImpl *tex3d = dynamic_cast<c3::Texture3DImpl *>(luptex);
			if (!tex3d)
			{
				return;
			}
			else
			{
				textype = GL_TEXTURE_3D;
				glid = (GLuint)(c3::ShaderProgramImpl &)*tex3d;
			}
		}
		else
		{
			textype = GL_TEXTURE_CUBE_MAP;
			glid = (GLuint)(c3::ShaderProgramImpl &)*texcube;
		}
	}
	else
	{
		textype = GL_TEXTURE_2D;
		glid = (GLuint)(c3::ShaderProgramImpl &)*tex2d;
	}

	ptexcache[sampler] = ptex;

	gl.ActiveTexture(sampleridlu[sampler]);

	gl.BindTexture(textype, ptex ? glid : 0);
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

			//assert(vloc == i);

			bool is_color = (pcd->m_Usage >= VertexBuffer::ComponentDescription::Usage::VU_COLOR0) && (pcd->m_Usage <= VertexBuffer::ComponentDescription::Usage::VU_COLOR3);
			bool is_byte = (pcd->m_Type >= VertexBuffer::ComponentDescription::VCT_U8) && (pcd->m_Type <= VertexBuffer::ComponentDescription::VCT_S8);
			GLuint norm = (is_color && is_byte);

			gl.EnableVertexAttribArray(vloc);
			gl.VertexAttribPointer(vloc, (GLint)pcd->m_Count, t[pcd->m_Type], norm, (GLsizei)vsz, (void *)vo);
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
		m_viewproj = m_proj * m_view;
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
		m_worldviewproj = *GetViewProjectionMatrix() * m_world;
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
			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 2, c3::VertexBuffer::ComponentDescription::Usage::VU_TEXCOORD0},

			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_NONE, 0, c3::VertexBuffer::ComponentDescription::Usage::VU_NONE} // terminator
		};

		typedef struct
		{
			glm::fvec3 pos;
			glm::fvec3 norm;
			glm::fvec2 uv;
		} SCubeVert;

		static const SCubeVert v[6 * 4] =
		{
			// TOP +Z 0
			{ { -1, -1,  1 }, {  0,  0,  1 }, {  0,  0 } },
			{ {  1, -1,  1 }, {  0,  0,  1 }, {  0,  1 } },
			{ {  1,  1,  1 }, {  0,  0,  1 }, {  1,  1 } },
			{ { -1,  1,  1 }, {  0,  0,  1 }, {  1,  0 } },

			// RIGHT +X 4
			{ {  1,  1,  1 }, {  1,  0,  0 }, {  0,  0 } },
			{ {  1,  1, -1 }, {  1,  0,  0 }, {  0,  1 } },
			{ {  1, -1, -1 }, {  1,  0,  0 }, {  1,  1 } },
			{ {  1, -1,  1 }, {  1,  0,  0 }, {  1,  0 } },

			// BOTTOM -Z 8
			{ { -1, -1, -1 }, {  0,  0, -1 }, {  0,  0 } },
			{ {  1, -1, -1 }, {  0,  0, -1 }, {  0,  1 } },
			{ {  1,  1, -1 }, {  0,  0, -1 }, {  1,  1 } },
			{ { -1,  1, -1 }, {  0,  0, -1 }, {  1,  0 } },

			// LEFT -X 12
			{ { -1, -1, -1 }, { -1,  0,  0 }, {  0,  0 } },
			{ { -1, -1,  1 }, { -1,  0,  0 }, {  0,  1 } },
			{ { -1,  1,  1 }, { -1,  0,  0 }, {  1,  1 } },
			{ { -1,  1, -1 }, { -1,  0,  0 }, {  1,  0 } },

			// FRONT +Y 16
			{ { -1,  1, -1 }, {  0,  1,  0 }, {  0,  0 } },
			{ {  1,  1, -1 }, {  0,  1,  0 }, {  0,  1 } },
			{ {  1,  1,  1 }, {  0,  1,  0 }, {  1,  1 } },
			{ { -1,  1,  1 }, {  0,  1,  0 }, {  1,  0 } },

			// FRONT -Y 20
			{ { -1, -1, -1 }, {  0, -1,  0 }, {  0,  0 } },
			{ {  1, -1, -1 }, {  0, -1,  0 }, {  0,  1 } },
			{ {  1, -1,  1 }, {  0, -1,  0 }, {  1,  1 } },
			{ { -1, -1,  1 }, {  0, -1,  0 }, {  1,  0 } }
		};

		void *buf;
		if (m_CubeVB->Lock(&buf, 6 * 4, comps, VBLOCKFLAG_WRITE) == VertexBuffer::RETURNCODE::RET_OK)
		{
			memcpy(buf, v, sizeof(SCubeVert) * 6 * 4);

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
				// do front and back, then connect the corners
				static const uint16_t i[12][2] ={
					{0 , 1 },
					{1 , 2 },
					{2 , 3 },
					{3 , 0 },
					{8 , 9 },
					{9 , 10},
					{10, 11},
					{11, 8 },
					{0 , 8 },
					{1 , 9 },
					{2 , 10},
					{3 , 11}
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
				static const uint16_t i[6][2][3] =
				{
					 { {  0,  2,  1 }, {  0,  3, 2  } }		// top
					,{ {  4,  5,  6 }, {  4,  6, 7  } }		// right
					,{ {  8,  9, 10 }, {  8, 10, 11 } }		// bottom
					,{ { 12, 14, 13 }, { 12, 15, 14 } }		// left fix
					,{ { 16, 17, 18 }, { 16, 18, 19 } }		// front
					,{ { 20, 22, 21 }, { 20, 23, 22 } }		// back
				};

				void *buf;
				if (pib->Lock(&buf, 2 * 3 * 6, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE) == IndexBuffer::RETURNCODE::RET_OK)
				{
					memcpy(buf, i[0], sizeof(uint16_t) * 2 * 3 * 6);

					pib->Unlock();
				}

				m_CubeMesh->AttachIndexBuffer(pib);
			}
		}
	}

	return m_CubeMesh;
}


VertexBuffer *RendererImpl::GetPlanesVB()
{
	if (!m_PlanesVB)
	{
		m_PlanesVB = CreateVertexBuffer(0);

		c3::VertexBuffer::ComponentDescription comps[4] ={
			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_POSITION},
			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_NORMAL},
			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 2, c3::VertexBuffer::ComponentDescription::Usage::VU_TEXCOORD0},

			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_NONE, 0, c3::VertexBuffer::ComponentDescription::Usage::VU_NONE} // terminator
		};

		typedef struct
		{
			glm::fvec3 pos;
			glm::fvec3 norm;
			glm::fvec2 uv;
		} SPlaneVert;

		static const SPlaneVert v[3 * 4] =
		{
			// XY
			{ { -1, -1,  0 }, {  0,  0,  1 }, {  0,  0 } },
			{ {  1, -1,  0 }, {  0,  0,  1 }, {  0,  1 } },
			{ {  1,  1,  0 }, {  0,  0,  1 }, {  1,  1 } },
			{ { -1,  1,  0 }, {  0,  0,  1 }, {  1,  0 } },

			// YZ
			{ {  0,  1,  1 }, {  1,  0,  0 }, {  0,  0 } },
			{ {  0,  1, -1 }, {  1,  0,  0 }, {  0,  1 } },
			{ {  0, -1, -1 }, {  1,  0,  0 }, {  1,  1 } },
			{ {  0, -1,  1 }, {  1,  0,  0 }, {  1,  0 } },

			// XZ
			{ { -1,  0, -1 }, {  0,  1,  0 }, {  0,  0 } },
			{ {  1,  0, -1 }, {  0,  1,  0 }, {  0,  1 } },
			{ {  1,  0,  1 }, {  0,  1,  0 }, {  1,  1 } },
			{ { -1,  0,  1 }, {  0,  1,  0 }, {  1,  0 } }
		};

		void *buf;
		if (m_PlanesVB->Lock(&buf, 3 * 4, comps, VBLOCKFLAG_WRITE) == VertexBuffer::RETURNCODE::RET_OK)
		{
			memcpy(buf, v, sizeof(SPlaneVert) * 3 * 4);

			m_PlanesVB->Unlock();
		}
	}

	return m_PlanesVB;
}

Mesh *RendererImpl::GetXYPlaneMesh()
{
	if (!m_XYPlaneMesh)
	{
		m_XYPlaneMesh = CreateMesh();

		m_XYPlaneMesh->AttachVertexBuffer(GetPlanesVB());

		if (m_PlanesVB)
		{
			IndexBuffer *pib = CreateIndexBuffer(0);
			if (pib)
			{
				static const uint16_t i[2][3] =
				{
					 {  0,  2,  1 }, {  0,  3, 2  }
				};

				void *buf;
				if (pib->Lock(&buf, 2 * 3 * 6, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE) == IndexBuffer::RETURNCODE::RET_OK)
				{
					memcpy(buf, i[0], sizeof(uint16_t) * 2 * 3 * 6);

					pib->Unlock();
				}

				m_XYPlaneMesh->AttachIndexBuffer(pib);
			}
		}
	}

	return m_XYPlaneMesh;
}


Mesh *RendererImpl::GetYZPlaneMesh()
{
	if (!m_YZPlaneMesh)
	{
		m_YZPlaneMesh = CreateMesh();

		m_YZPlaneMesh->AttachVertexBuffer(GetPlanesVB());

		if (m_PlanesVB)
		{
			IndexBuffer *pib = CreateIndexBuffer(0);
			if (pib)
			{
				static const uint16_t i[2][3] =
				{
					 {  8,  10,  9 }, {  8,  11, 10  }
				};

				void *buf;
				if (pib->Lock(&buf, 2 * 3 * 6, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE) == IndexBuffer::RETURNCODE::RET_OK)
				{
					memcpy(buf, i[0], sizeof(uint16_t) * 2 * 3 * 6);

					pib->Unlock();
				}

				m_YZPlaneMesh->AttachIndexBuffer(pib);
			}
		}
	}

	return m_YZPlaneMesh;
}


Mesh *RendererImpl::GetXZPlaneMesh()
{
	if (!m_XZPlaneMesh)
	{
		m_XZPlaneMesh = CreateMesh();

		m_XZPlaneMesh->AttachVertexBuffer(GetPlanesVB());

		if (m_PlanesVB)
		{
			IndexBuffer *pib = CreateIndexBuffer(0);
			if (pib)
			{
				static const uint16_t i[2][3] =
				{
					 {  4,  6,  5 }, {  4,  7, 6  }
				};

				void *buf;
				if (pib->Lock(&buf, 2 * 3 * 6, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE) == IndexBuffer::RETURNCODE::RET_OK)
				{
					memcpy(buf, i[0], sizeof(uint16_t) * 2 * 3 * 6);

					pib->Unlock();
				}

				m_XZPlaneMesh->AttachIndexBuffer(pib);
			}
		}
	}

	return m_XZPlaneMesh;
}


size_t hemisphereSectorCount = 40;
size_t hemisphereStackCount = 20;

VertexBuffer *RendererImpl::GetHemisphereVB()
{
	typedef struct
	{
		glm::fvec3 pos;
		glm::fvec3 norm;
		glm::fvec2 uv;
	} SHemisphereVert;

	typedef std::vector<SHemisphereVert> TVertexArray;
	TVertexArray verts;

	SHemisphereVert v;

	glm::fvec4 n(0, 0, 1, 0);

	v.pos.x = v.pos.y = v.norm.x = v.norm.y = 0.0f;
	v.uv.x = v.uv.y = 0.5f;
	v.pos.z = v.norm.z = 1.0f;

	verts.push_back(v);

	glm::fmat4x4 mz = (glm::fmat4x4)glm::angleAxis(C3_PI * 2.0f / (float)hemisphereSectorCount, glm::fvec3(0.0f, 0.0f, 1.0f));

	for (size_t i = 1, maxi = hemisphereStackCount + 1; i < maxi; i++)
	{
		glm::fmat4x4 mx = (glm::fmat4x4)glm::angleAxis(C3_PI / 2.0f / (float)(hemisphereStackCount + 1) * i, glm::fvec3(0.0f, 1.0f, 0.0f));
		n = glm::normalize(glm::fvec4(0, 0, 1, 0) * mx);

		for (size_t j = 0, maxj = hemisphereSectorCount + 1; j < maxj; j++)
		{
			v.norm = v.pos = n;
			//v.uv.x = (float)j;
			v.uv = glm::fvec2(n.x, n.y) * glm::fvec2(0.5f, 0.5f) + glm::fvec2(0.5f, 0.5f);

			verts.push_back(v);

			n = glm::normalize(n * mz);
		}

		//v.uv.y = (float)i;
	}

	c3::VertexBuffer::ComponentDescription comps[4] ={
		{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_POSITION},
		{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_NORMAL},
		{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 2, c3::VertexBuffer::ComponentDescription::Usage::VU_TEXCOORD0},

		{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_NONE, 0, c3::VertexBuffer::ComponentDescription::Usage::VU_NONE} // terminator
	};

	m_HemisphereVB = CreateVertexBuffer(0);
	if (m_HemisphereVB)
	{
		void *buf;
		if ((m_HemisphereVB->Lock(&buf, verts.size(), comps, VBLOCKFLAG_WRITE) == VertexBuffer::RETURNCODE::RET_OK) && buf)
		{
			memcpy(buf, &(verts.at(0)), sizeof(SHemisphereVert) * verts.size());

			m_HemisphereVB->Unlock();
		}
	}

	return m_HemisphereVB;
}


Mesh *RendererImpl::GetHemisphereMesh()
{
	if (!m_HemisphereMesh)
	{
		m_HemisphereMesh = CreateMesh();

		m_HemisphereMesh->AttachVertexBuffer(GetHemisphereVB());

		if (m_HemisphereVB)
		{
			IndexBuffer *pib = CreateIndexBuffer(0);
			if (pib)
			{
				uint16_t *buf;

				size_t ic_top = 3 * (hemisphereSectorCount + 1), ic = ic_top + ((ic_top * 2) * (hemisphereStackCount - 1));

				if (pib->Lock((void **)&buf, ic, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE) == IndexBuffer::RETURNCODE::RET_OK)
				{
					// Do the top slice first because it has only one triangle per sector
					for (uint16_t i = 0, maxi = (uint16_t)(hemisphereSectorCount + 1); i < maxi; i++)
					{
						*(buf++) = 0;
						*(buf++) = i;
						*(buf++) = i + 1;
					}

					uint16_t ss = 1;
					for (uint16_t i = 1, maxi = (uint16_t)(hemisphereStackCount + 1); i < maxi; i++)
					{
						for (uint16_t j = 0, maxj = (uint16_t)(hemisphereSectorCount + 1); j < maxj; j++)
						{
							uint16_t a = ss;
							uint16_t b = a + (uint16_t)hemisphereSectorCount;

							*(buf++) = a;
							*(buf++) = b;
							*(buf++) = b + 1;

							*(buf++) = a;
							*(buf++) = b + 1;
							*(buf++) = a + 1;

							ss++;
						}
					}

					pib->Unlock();
				}

				m_HemisphereMesh->AttachIndexBuffer(pib);
			}
		}
	}

	return m_HemisphereMesh;
}


Texture2D *RendererImpl::GetBlackTexture()
{
	if (!m_BlackTex)
	{
		m_BlackTex = CreateTexture2D(16, 16, TextureType::U8_4CH, 1, 0);
		if (m_BlackTex)
		{
			uint32_t *buf;
			Texture2D::SLockInfo li;
			if ((m_BlackTex->Lock((void **)&buf, li, 0, TEXLOCKFLAG_WRITE) == Texture2D::RETURNCODE::RET_OK) && buf)
			{
				for (size_t y = 0, maxy = li.height; y < maxy; y++)
				{
					for (size_t x = 0, maxx = li.width; x < maxx; x++)
					{
						buf[x] = 0xFF000000;
					}

					buf = (uint32_t *)((BYTE *)buf + li.stride);
				}

				m_BlackTex->Unlock();
			}
		}
	}

	return m_BlackTex;
}


Texture2D *RendererImpl::GetGreyTexture()
{
	if (!m_GreyTex)
	{
		m_GreyTex = CreateTexture2D(16, 16, TextureType::U8_4CH, 1, 0);
		if (m_GreyTex)
		{
			uint32_t *buf;
			Texture2D::SLockInfo li;
			if ((m_GreyTex->Lock((void **)&buf, li, 0, TEXLOCKFLAG_WRITE) == Texture2D::RETURNCODE::RET_OK) && buf)
			{
				for (size_t y = 0, maxy = li.height; y < maxy; y++)
				{
					for (size_t x = 0, maxx = li.width; x < maxx; x++)
					{
						buf[x] = 0xFF808080;
					}

					buf = (uint32_t *)((BYTE *)buf + li.stride);
				}

				m_GreyTex->Unlock();
			}
		}
	}

	return m_GreyTex;
}


Texture2D *RendererImpl::GetWhiteTexture()
{
	if (!m_WhiteTex)
	{
		m_WhiteTex = CreateTexture2D(16, 16, TextureType::U8_4CH, 1, 0);
		if (m_WhiteTex)
		{
			uint32_t *buf;
			Texture2D::SLockInfo li;
			if ((m_WhiteTex->Lock((void **)&buf, li, 0, TEXLOCKFLAG_WRITE) == Texture2D::RETURNCODE::RET_OK) && buf)
			{
				for (size_t y = 0, maxy = li.height; y < maxy; y++)
				{
					for (size_t x = 0, maxx = li.width; x < maxx; x++)
					{
						buf[x] = 0xFFFFFFFF;
					}

					buf = (uint32_t *)((BYTE *)buf + li.stride);
				}

				m_WhiteTex->Unlock();
			}
		}
	}

	return m_WhiteTex;
}


Texture2D *RendererImpl::GetBlueTexture()
{
	if (!m_BlueTex)
	{
		m_BlueTex = CreateTexture2D(16, 16, TextureType::U8_4CH, 1, 0);
		if (m_BlueTex)
		{
			uint32_t *buf;
			Texture2D::SLockInfo li;
			if ((m_BlueTex->Lock((void **)&buf, li, 0, TEXLOCKFLAG_WRITE) == Texture2D::RETURNCODE::RET_OK) && buf)
			{
				for (size_t y = 0, maxy = li.height; y < maxy; y++)
				{
					for (size_t x = 0, maxx = li.width; x < maxx; x++)
					{
						buf[x] = 0xFFFF0000;
					}

					buf = (uint32_t *)((BYTE *)buf + li.stride);
				}

				m_BlueTex->Unlock();
			}
		}
	}

	return m_BlueTex;
}


Texture2D *RendererImpl::GetGridTexture()
{
	if (!m_GridTex)
	{
		m_GridTex = CreateTexture2D(16, 16, TextureType::U8_4CH, 0, TEXCREATEFLAG_WRAP_U | TEXCREATEFLAG_WRAP_V);
		if (m_GridTex)
		{
			uint32_t *buf;
			Texture2D::SLockInfo li;
			if ((m_GridTex->Lock((void **)&buf, li, 0, TEXLOCKFLAG_WRITE | TEXLOCKFLAG_GENMIPS) == Texture2D::RETURNCODE::RET_OK) && buf)
			{
				for (size_t y = 0, maxy = li.height, maxym1 = (maxy - 1); y < maxy; y++)
				{
					for (size_t x = 0, maxx = li.width, maxxm1 = (maxx - 1); x < maxx; x++)
					{
						buf[x] = (((y > 0) && (y < maxym1) && (x > 0) && (x < maxxm1)) ? 0x00000000 : 0xFFFFFFFF);
					}

					buf = (uint32_t *)((BYTE *)buf + li.stride);
				}

				m_GridTex->Unlock();
			}
		}
	}

	return m_GridTex;
}
