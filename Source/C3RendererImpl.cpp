// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


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
#include <C3CommonVertexDefs.h>
#include <C3RenderMethodImpl.h>
#include <C3FontImpl.h>

#include "resource.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <Shlwapi.h>
#include <ddraw.h>


using namespace c3;


const static GLenum testvals[Renderer::DT_NUMTESTS] = {GL_NEVER, GL_LESS, GL_LEQUAL, GL_EQUAL, GL_NOTEQUAL, GL_GEQUAL, GL_GREATER, GL_ALWAYS};
const static GLenum stencilvals[Renderer::SO_NUMOPMODES] = {GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_INCR_WRAP, GL_DECR, GL_DECR_WRAP, GL_INVERT};

Texture *RendererImpl::s_TexCache[32] = {0};
uint32_t RendererImpl::s_SamplerCache[32] = {0};

RendererImpl::RendererImpl(SystemImpl *psys)
{
	m_pSys = psys;

	m_hwnd = NULL;
	m_hdc = NULL;
	m_glrc = NULL;
	m_hwnd_override = NULL;

	m_LastPresentTime = 0;

	m_glVersionMaj = 4;
	m_glVersionMin = 6;

	m_event_shutdown = CreateEvent(nullptr, TRUE, TRUE, nullptr);

	// No extra threads, just run everything in this thread
	m_TaskPool[0] = pool::IThreadPool::Create(0);
	m_TaskPool[1] = pool::IThreadPool::Create(0);
	m_ActiveTaskPool = 0;

	m_needsFinish = false;

	m_clearZ = 1.0f;
	m_DepthMode = DepthMode::DM_NUMMODES;
	m_DepthTest = Test::DT_NUMTESTS;
	m_WindingOrder = WindingOrder::WO_NUMMODES;
	m_CullMode = CullMode::CM_NUMMODES;
	m_BlendMode = BlendMode::BM_NUMMODES;
	m_BlendEq = BlendEquation::BE_NUMMODES;
	m_AlphaBlendMode = BlendMode::BM_NUMMODES;
	m_AlphaBlendEq = BlendEquation::BE_NUMMODES;
	m_FillMode = FillMode::FM_FILL;

	m_DirtyStates = 0;

	m_AlphaPassMin = 0.2f;
	m_AlphaPassMax = 1.0f;

	m_AlphaCoverage = 1.0f;
	m_AlphaCoverageInv = false;

	m_StencilEnabled = false;
	m_StencilTest = Test::DT_ALWAYS;
	m_StencilRef = 0;
	m_StencilMask = 0xff;
	m_StencilFailOp = StencilOperation::SO_KEEP;
	m_StencilZFailOp = StencilOperation::SO_KEEP;
	m_StencilZPassOp = StencilOperation::SO_KEEP;

	m_CurFB = nullptr;
	m_CurFBID = NULL;

	m_CurIB = nullptr;
	m_CurIBID = NULL;

	m_CurVB = nullptr;
	m_CurVBID = NULL;

	m_CurProg = nullptr;
	m_CurProgID = NULL;

	m_CubeVB = nullptr;
	m_CubeMesh = nullptr;
	m_RefCubeVB = nullptr;
	m_RefCubeMesh = nullptr;
	m_BoundsMesh = nullptr;

	m_FSPlaneVB = nullptr;
	m_PlanesVB = nullptr;
	m_FullScreenPlaneMesh = nullptr;
	m_XYPlaneMesh = nullptr;
	m_XZPlaneMesh = nullptr;
	m_YZPlaneMesh = nullptr;

	m_GuiRectMesh = nullptr;

	for (size_t i = 0; i < EResLevel::RL_RESCOUNT; i++)
	{
		m_HemisphereVB[i] = nullptr;
		m_HemisphereMesh[i] = nullptr;
		m_SphereMesh[i] = nullptr;
	}

	m_BlackTex = nullptr;
	m_GreyTex = nullptr;
	m_DefaultDescTex = nullptr;
	m_WhiteTex = nullptr;
	m_BlueTex = nullptr;
	m_GridTex = nullptr;
	m_LinearGradientTex = nullptr;
	m_OrthoRefTex = nullptr;
	m_UtilityColorTex = nullptr;
	m_SphereSpriteTex = nullptr;
	m_SphereSpriteNormalTex = nullptr;

	m_MatMan = nullptr;
	m_mtlWhite = nullptr;

	m_ActiveRenderMethod = nullptr;
	m_ActiveMaterial = nullptr;

	memset(&m_glARBWndClass, 0, sizeof(WNDCLASS));

	m_ident = glm::identity<glm::fmat4x4>();
	m_texturetransform = m_ident;

	m_matupflags.Set(MATRIXUPDATE_ALL);

	m_Initialized = false;

	m_Gui = nullptr;

	m_VertsPerFrame = 0;
	m_IndicesPerFrame = 0;
	m_TrisPerFrame = 0;
	m_LinesPerFrame = 0;
	m_PointsPerFrame = 0;

	// The default RenderMethod is a passthru to make the Draw calls a little more elegant looking - it doesn't actually set any states itself
	m_DefaultRenderMethod = CreateRenderMethod();
	assert(m_DefaultRenderMethod);
	RenderMethod::Technique *pt = m_DefaultRenderMethod->AddTechnique();
	m_DefaultRenderMethod->SetActiveTechnique(0);
	pt->AddPass();

	m_Frustum = BoundingBox::Create();

	m_DepthBias = 0;
}


RendererImpl::~RendererImpl()
{
	m_TaskPool[0]->Release();
	m_TaskPool[1]->Release();

	Shutdown();

	m_Frustum->Release();

	CloseHandle(m_event_shutdown);
}


c3::System *RendererImpl::GetSystem()
{
	return (c3::System *)m_pSys;
}


bool RendererImpl::Initialize(HWND hwnd, props::TFlags64 flags)
{
	m_hwnd = m_hwnd_override = hwnd;
	m_hdc = ::GetDC(hwnd);
	m_RendThreadId = GetCurrentThreadId();

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
		m_glARBWndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
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

			int contextAttribList[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB,	(int)m_glVersionMaj,
				WGL_CONTEXT_MINOR_VERSION_ARB,	(int)m_glVersionMin,
#if defined(_DEBUG) && FALSE	// the NV driver is suuuper annoying and spams the console with nothing-burgers
				WGL_CONTEXT_FLAGS_ARB,			WGL_CONTEXT_DEBUG_BIT_ARB | WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#else
				WGL_CONTEXT_FLAGS_ARB,			WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#endif
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

	gl.SetLogFunc([&](const TCHAR *msg)
	{
		m_pSys->GetLog()->Print(L"[GL] ");
		m_pSys->GetLog()->Print(msg);
		m_pSys->GetLog()->Print(L"\n");
	});

	// make use of opengl messages, log them
	gl.DebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		static size_t errct = 10;
		if (errct)
		{
			c3::System* psys = (c3::System*)userParam;
			if (message)
			{
				// keep these log messages; NV info spam
				if (strstr(message, "Buffer detailed info") != message)
					errct--;

				TCHAR *tmp;
				CONVERT_MBCS2TCS(message, tmp);

				// until making the logging system thread-safe (including the editor's output window), don't log.
				// nvidia's messages come from a thread that's not the main one grrrr
				psys->GetLog()->Print(_T("[GL] DEBUG: %s\n"), tmp);

				if (!errct)
					psys->GetLog()->Print(_T("\n[GL] Too many errors; error logging disabled.\n"), tmp);
			}
		}
	}, m_pSys);

	// turn on all opengl mesages unless we're on nvidia - they report too much
	if (isnv)
	{
		gl.DebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		gl.DebugMessageControl(GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	else
		gl.DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	m_Initialized = true;

	ResetEvent(m_event_shutdown);

	char *devname = _strdup((const char *)gl.GetString(GL_RENDERER));
	if (devname && *devname)
	{
		char *c = devname;
		while (*c) { *c = tolower(*c); c++; }
		TCHAR *tmp;
		CONVERT_MBCS2TCS(devname, tmp);
		m_DeviceName = tmp;
		free(devname);
	}

	char *vendorname = _strdup((const char *)gl.GetString(GL_VENDOR));
	if (vendorname && *vendorname)
	{
		char *c = vendorname;
		while (*c) { *c = tolower(*c); c++; }
		TCHAR *tmp;
		CONVERT_MBCS2TCS(vendorname, tmp);
		m_VendorName = tmp;
		free(vendorname);
	}

	const TCHAR *vn = GetVendorName();
	if (_tcsstr(vn, _T("nvidia")) != nullptr)
		isnv = true;

	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(wglGetProcAddress("wglSwapIntervalEXT"));
	if (wglSwapIntervalEXT)
		wglSwapIntervalEXT(0);

	SetProjectionMatrix(&m_ident);
	SetViewMatrix(&m_ident);
	SetWorldMatrix(&m_ident);

	gl.ClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);
	gl.ClearDepthf(m_clearZ);
	gl.ClearStencil((GLint)m_clearStencil);

	SetDepthMode(DepthMode::DM_READWRITE);
	SetDepthTest(Test::DT_LESSEREQUAL);

	SetWindingOrder(WindingOrder::WO_CCW);
	SetCullMode(CullMode::CM_BACK);

	SetBlendMode(BM_ALPHA);
	SetBlendEquation(BE_ADD);
	SetAlphaBlendMode(BM_ADD);
	SetAlphaBlendEquation(BE_ADD);

	// Initialize meshes
	GetCubeMesh();
	GetBoundsMesh();
	GetXYPlaneMesh();
	GetXZPlaneMesh();
	GetYZPlaneMesh();
	GetGuiRectMesh();
	GetFullScreenPlaneMesh();


	c3::ResourceManager *rm = m_pSys->GetResourceManager();
	const c3::ResourceType *rt;

	// Initizlie utility textures and register them with the resource manager
	rt = rm->FindResourceTypeByName(_T("Texture2D"));
	rm->GetResource(_T("[black.tex]"), RESF_CREATEENTRYONLY, rt, GetBlackTexture());
	rm->GetResource(_T("[grey.tex]"), RESF_CREATEENTRYONLY, rt, GetGreyTexture());
	rm->GetResource(_T("[white.tex]"), RESF_CREATEENTRYONLY, rt, GetWhiteTexture());
	rm->GetResource(_T("[blue.tex]"), RESF_CREATEENTRYONLY, rt, GetBlueTexture());
	rm->GetResource(_T("[grid.tex]"), RESF_CREATEENTRYONLY, rt, GetGridTexture());
	rm->GetResource(_T("[lineargradient.tex]"), RESF_CREATEENTRYONLY, rt, GetLinearGradientTexture());
	rm->GetResource(_T("[utilitycolor.tex]"), RESF_CREATEENTRYONLY, rt, GetUtilityColorTexture());
	rm->GetResource(_T("[orthoref.tex]"), RESF_CREATEENTRYONLY, rt, GetOrthoRefTexture());

	// Initialize the reference cube model and register it with the resource manager
	rt = rm->FindResourceTypeByName(_T("Model"));

	{
		c3::Material *refmtl = GetMaterialManager()->CreateMaterial();
		refmtl->SetTexture(c3::Material::ETextureComponentType::TCT_DIFFUSE, GetOrthoRefTexture());
		refmtl->SetWindingOrder(c3::Renderer::EWindingOrder::WO_CCW);
		c3::Model *refcube = c3::Model::Create(this);
		c3::Model::MeshIndex mi = refcube->AddMesh(GetRefCubeMesh());
		c3::Model::NodeIndex ni = refcube->AddNode();
		refcube->AssignMeshToNode(ni, mi);
		refcube->SetMaterial(mi, refmtl);
		rm->GetResource(_T("[refcube.model]"), RESF_CREATEENTRYONLY, rt, refcube);
	}

	{
		TCHAR *hemispherename[EResLevel::RL_RESCOUNT] = {_T("[hemisphere_lo.model]"), _T("[hemisphere_med.model]"), _T("[hemisphere_hi.model]")};

		c3::Material *refmtl = GetMaterialManager()->CreateMaterial();
		refmtl->SetTexture(c3::Material::ETextureComponentType::TCT_DIFFUSE, GetGridTexture());
		refmtl->SetWindingOrder(c3::Renderer::EWindingOrder::WO_CCW);

		for (size_t i = 0; i < EResLevel::RL_RESCOUNT; i++)
		{
			c3::Model *hemisphere = c3::Model::Create(this);
			c3::Model::MeshIndex mi = hemisphere->AddMesh(GetHemisphereMesh((EResLevel)i));
			c3::Model::NodeIndex ni = hemisphere->AddNode();
			hemisphere->AssignMeshToNode(ni, mi);
			hemisphere->SetMaterial(mi, refmtl);
			rm->GetResource(hemispherename[i], RESF_CREATEENTRYONLY, rt, hemisphere);
		}
	}

	{
		TCHAR *spherename[EResLevel::RL_RESCOUNT] = {_T("[sphere_lo.model]"), _T("[sphere_med.model]"), _T("[sphere_hi.model]")};

		c3::Material *refmtl = GetMaterialManager()->CreateMaterial();
		refmtl->SetTexture(c3::Material::ETextureComponentType::TCT_DIFFUSE, GetWhiteTexture());
		refmtl->SetTexture(c3::Material::ETextureComponentType::TCT_SURFACEDESC, GetBlackTexture());
		refmtl->SetWindingOrder(c3::Renderer::EWindingOrder::WO_CCW);

		for (size_t i = 0; i < EResLevel::RL_RESCOUNT; i++)
		{
			c3::Model *sphere = c3::Model::Create(this);
			c3::Model::MeshIndex mi = sphere->AddMesh(GetSphereMesh((EResLevel)i));
			c3::Model::NodeIndex ni = sphere->AddNode();
			sphere->AssignMeshToNode(ni, mi);
			sphere->SetMaterial(mi, refmtl);
			rm->GetResource(spherename[i], RESF_CREATEENTRYONLY, rt, sphere);
		}
	}

	{
		c3::Model *boundscube = c3::Model::Create(this);
		c3::Model::MeshIndex mi = boundscube->AddMesh(GetBoundsMesh());
		c3::Model::NodeIndex ni = boundscube->AddNode();
		boundscube->AssignMeshToNode(ni, mi);
		rm->GetResource(_T("[bounds.model]"), RESF_CREATEENTRYONLY, rt, boundscube);
	}

	{
		c3::Model *plane_model = c3::Model::Create(this);
		c3::Model::MeshIndex mi = plane_model->AddMesh(GetXYPlaneMesh());
		c3::Model::NodeIndex ni = plane_model->AddNode();
		plane_model->AssignMeshToNode(ni, mi);
		plane_model->SetMaterial(mi, GetWhiteMaterial());
		rm->GetResource(_T("[xyplane.model]"), RESF_CREATEENTRYONLY, rt, plane_model);
	}

	{
		c3::Model *plane_model = c3::Model::Create(this);
		c3::Model::MeshIndex mi = plane_model->AddMesh(GetYZPlaneMesh());
		c3::Model::NodeIndex ni = plane_model->AddNode();
		plane_model->AssignMeshToNode(ni, mi);
		plane_model->SetMaterial(mi, GetWhiteMaterial());
		rm->GetResource(_T("[yzplane.model]"), RESF_CREATEENTRYONLY, rt, plane_model);
	}

	{
		c3::Model *plane_model = c3::Model::Create(this);
		c3::Model::MeshIndex mi = plane_model->AddMesh(GetXZPlaneMesh());
		c3::Model::NodeIndex ni = plane_model->AddNode();
		plane_model->AssignMeshToNode(ni, mi);
		plane_model->SetMaterial(mi, GetWhiteMaterial());
		rm->GetResource(_T("[xzplane.model]"), RESF_CREATEENTRYONLY, rt, plane_model);
	}

	{
		c3::Model *plane_model = c3::Model::Create(this);
		c3::Model::MeshIndex mi = plane_model->AddMesh(GetGuiRectMesh());
		c3::Model::NodeIndex ni = plane_model->AddNode();
		plane_model->AssignMeshToNode(ni, mi);
		plane_model->SetMaterial(mi, GetWhiteMaterial());
		rm->GetResource(_T("[guirect.model]"), RESF_CREATEENTRYONLY, rt, plane_model);
	}

	m_Gui = new GuiImpl(this);

	SetViewport();

	m_FrameNum = 0;

	return true;
}


bool RendererImpl::Initialized()
{
	return m_Initialized;
}


void RendererImpl::FlushErrors(const TCHAR *msgformat, ...)
{
	using TGlErrMap = std::map<GLenum, size_t>;
	static TGlErrMap errmap;

	GLenum err;
	bool wrotehdr = false;
	// check OpenGL error
	do
	{
		err = gl.GetError();
		if (err != GL_NO_ERROR)
		{
			TGlErrMap::iterator errit = errmap.find(err);
			if (errit == errmap.end())
			{
				std::pair<TGlErrMap::iterator, bool> r = errmap.insert(TGlErrMap::value_type(err, 5));
				errit = r.first;
			}

			if (!errit->second)
				continue;

			errit->second--;
			if (!errit->second)
			{
				m_pSys->GetLog()->Print(_T("\nToo many errors of type: %d\n"), err);
				continue;
			}

			if (!wrotehdr)
			{
#define PRINT_BUFSIZE	1024
				TCHAR buf[PRINT_BUFSIZE];	// Temporary buffer for output

				va_list marker;
				va_start(marker, msgformat);
				_vsntprintf_s(buf, PRINT_BUFSIZE - 1, msgformat, marker);

				m_pSys->GetLog()->Print(_T("\n*** frame: %lu  //  %s\n"), m_FrameNum, buf);
				wrotehdr = true;
			}

			m_pSys->GetLog()->Print(_T("\tOpenGL error: %d\n"), err);
		}
	}
	while (err != GL_NO_ERROR);
}


Renderer::LOG_FUNC RendererImpl::GetLogFunc() const
{
	return m_LogFunc;
}


void RendererImpl::SetLogFunc(Renderer::LOG_FUNC logfunc)
{
	m_LogFunc = logfunc;
	gl.SetLogFunc(logfunc);
}


void RendererImpl::PushDebugGroup(const char *groupname)
{
	char tmpname[64];
	if (!groupname)
	{
		_snprintf_s(tmpname, 64, "gfxdbg%zu", m_DebugGroupStack.size());
		groupname = tmpname;
	}

	m_DebugGroupStack.push_back(groupname);

	gl.PushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, groupname);
}


void RendererImpl::PopDebugGroup()
{
	if (!m_DebugGroupStack.empty())
		m_DebugGroupStack.pop_back();

	gl.PopDebugGroup();
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

	if (m_Gui)
	{
		delete m_Gui;
		m_Gui = nullptr;
	}

	for (auto sit : m_FontMap)
	{
		if (sit.second)
			delete sit.second;
	}

	m_pSys->GetResourceManager()->ForAllResourcesDo(UnloadRenderResource, nullptr, RTFLAG_RUNBYRENDERER, ResourceManager::ResTypeFlagMode::RTFM_ANY);

	SetEvent(m_event_shutdown);

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

	C3_SAFERELEASE(m_CubeVB);

	if (m_RefCubeMesh)
	{
		m_RefCubeMesh->AttachVertexBuffer(nullptr);
		m_RefCubeMesh->Release();
		m_RefCubeMesh = nullptr;
	}

	C3_SAFERELEASE(m_RefCubeVB);

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

	C3_SAFERELEASE(m_GuiRectMesh);

	C3_SAFERELEASE(m_PlanesVB);
	C3_SAFERELEASE(m_FSPlaneVB);

	for (size_t i = 0; i < EResLevel::RL_RESCOUNT; i++)
	{
		C3_SAFERELEASE(m_HemisphereMesh[i]);
		C3_SAFERELEASE(m_SphereMesh[i]);
	}

	if (m_MatMan)
	{
		delete m_MatMan;
		m_MatMan = nullptr;
	}

	for (auto sit : m_TexFlagsToSampler)
	{
		gl.DeleteSamplers(1, &sit.second);
	}
	m_TexFlagsToSampler.clear();

	wglMakeCurrent(NULL, NULL);

	if (m_glrc)
		wglDeleteContext(m_glrc);

	m_glrc = NULL;
	m_hdc = NULL;
	m_hwnd = NULL;

	m_Initialized = false;
}


const TCHAR *RendererImpl::GetVendorName() const
{
	return m_VendorName.c_str();
}


const TCHAR *RendererImpl::GetDeviceName() const
{
	return m_DeviceName.c_str();
}


size_t RendererImpl::LastTriagleCount() const
{
	return m_TrisPerFrame;
}


size_t RendererImpl::LastLineCount() const
{
	return m_LinesPerFrame;
}


size_t RendererImpl::LastPointCount() const
{
	return m_PointsPerFrame;
}


pool::IThreadPool *RendererImpl::GetTaskPool()
{
	return m_TaskPool[m_ActiveTaskPool ^ 1];
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
		GetClientRect(m_hwnd_override ? m_hwnd_override : m_hwnd, &r);
		viewport = &r;
	}

	memcpy(&m_Viewport, viewport, sizeof(RECT));

	LONG w = m_Viewport.right - m_Viewport.left;
	LONG h = m_Viewport.bottom - m_Viewport.top;
	gl.Viewport(0, 0, w, h);
	gl.Scissor(0, 0, w, h);

	if (m_Gui)
	{
		m_Gui->SetDisplaySize((float)w, (float)h);
	}

	m_matupflags.Set(MATRIXUPDATE_FRUSTUM);
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
	if (m_hwnd_override != hwnd)
	{
		m_hwnd_override = hwnd;
		m_hdc = GetDC(hwnd ? hwnd : m_hwnd);
		wglMakeCurrent(m_hdc, m_glrc);
	}
}


HWND RendererImpl::GetOverrideHwnd() const
{
	return m_hwnd_override;
}


bool RendererImpl::BeginScene(props::TFlags64 flags)
{
	if (!m_Initialized || m_needsFinish)
		return false;

	m_needsFinish = true;

	m_VertsPerFrame = 0;
	m_IndicesPerFrame = 0;
	m_TrisPerFrame = 0;
	m_LinesPerFrame = 0;
	m_PointsPerFrame = 0;

	m_ActiveTaskPool ^= 1;
	m_TaskPool[m_ActiveTaskPool]->Flush();

	m_BeginSceneFlags = flags;

#if 0	// should this really be done at some point?
	for (size_t i = 0; i < 32; i++)
		UseTexture(i, nullptr);
	UseRenderMethod(nullptr);
#endif
		 
	if (m_Gui && flags.IsSet(BSFLAG_SHOWGUI))
	{
		m_Gui->BeginFrame();
	}

	if (flags.AnySet(UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH | UFBFLAG_CLEARSTENCIL))
		gl.Clear((flags.IsSet(UFBFLAG_CLEARCOLOR) ? GL_COLOR_BUFFER_BIT : 0) |
			(flags.IsSet(UFBFLAG_CLEARCOLOR) ? GL_DEPTH_BUFFER_BIT : 0) |
			(flags.IsSet(UFBFLAG_CLEARSTENCIL) ? GL_STENCIL_BUFFER_BIT : 0));

	return true;
}


bool RendererImpl::EndScene(props::TFlags64 flags)
{
	if (!m_Initialized)
		return false;

	if (!m_needsFinish)
		return false;

	m_needsFinish = false;

	if (m_Gui && m_BeginSceneFlags.IsSet(BSFLAG_SHOWGUI))
	{
		int32_t mx, my;
		m_pSys->GetInputManager()->GetMousePos(mx, my);
		ImGui::GetIO().MousePos = glm::fvec2(float(mx), float(my));

		static bool show_metrics = false;
		if (show_metrics)
		{
			ImGui::GetIO().MetricsRenderVertices = (int)m_VertsPerFrame;
			ImGui::GetIO().MetricsRenderIndices = (int)m_IndicesPerFrame;

			ImGui::ShowMetricsWindow(&show_metrics);
		}

		m_Gui->EndFrame();
		m_Gui->Render();
	}

#if defined(NEEDS_GLFINISH)
	gl.Finish();
#endif

	return true;
}


bool RendererImpl::Present()
{
	if (!m_Initialized)
		return false;

	HWND tmphwnd = m_hwnd_override ? m_hwnd_override : m_hwnd;
	HDC tmpdc = m_hwnd_override ? GetDC(m_hwnd_override) : m_hdc;

	SwapBuffers(tmpdc);
	ReleaseDC(tmphwnd, tmpdc);

	m_FrameNum++;

	return true;
}


size_t RendererImpl::GetCurrentFrameNumber()
{
	return m_FrameNum;
}


void RendererImpl::UpdateDirtyRenderStates(bool refresh)
{
	if (m_CurFB)
		((FrameBufferImpl *)m_CurFB)->UpdateDirtyRenderStates(refresh);

	// DEPTH MODE
	if (m_DirtyStates.IsSet(RSOF_DEPTHMODE))
	{
		if ((m_DepthMode == DM_READWRITE) || (m_DepthMode == DM_WRITEONLY))
			gl.DepthMask(GL_TRUE);
		else
			gl.DepthMask(GL_FALSE);

		if ((m_DepthMode == DM_READWRITE) || (m_DepthMode == DM_READONLY))
			gl.Enable(GL_DEPTH_TEST);
		else
			gl.Disable(GL_DEPTH_TEST);
	}

	// DEPTH TEST
	if (m_DirtyStates.IsSet(RSOF_DEPTHTEST))
	{
		gl.DepthFunc(testvals[m_DepthTest]);
	}

	// FILL MODE
	if (m_DirtyStates.IsSet(RSOF_FILLMODE))
	{
		switch (m_FillMode)
		{
			case FillMode::FM_FILL:
				gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				break;

			case FillMode::FM_WIRE:
				gl.PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				break;

			case FillMode::FM_POINT:
				gl.PolygonMode(GL_FRONT_AND_BACK, GL_POINT);
				break;
		}
	}

	// STENCIL ENABLED
	if (m_DirtyStates.IsSet(RSOF_STENCIL))
	{
		if (m_StencilEnabled)
			gl.Enable(GL_STENCIL_TEST);
		else
			gl.Disable(GL_STENCIL_TEST);
	}

	if (m_DirtyStates.IsSet(RSOF_STENCILFUNC))
	{
		gl.StencilFunc(testvals[m_StencilTest], m_StencilRef, this->m_StencilMask);
	}

	if (m_DirtyStates.IsSet(RSOF_STENCILOP))
	{
		gl.StencilOp(stencilvals[m_StencilFailOp], stencilvals[m_StencilZFailOp], stencilvals[m_StencilZPassOp]);
	}

	if (m_DirtyStates.IsSet(RSOF_WINDINGORDER))
	{
		gl.FrontFace((m_WindingOrder == WO_CW) ? GL_CW : GL_CCW);
	}

	if (m_DirtyStates.IsSet(RSOF_CULLMODE))
	{
		bool enable_cull = false;

		switch (m_CullMode)
		{
			case CM_DISABLED:
				enable_cull = false;
				break;

			case CM_FRONT:
				enable_cull = true;
				gl.CullFace(GL_FRONT);
				break;

			case CM_BACK:
				enable_cull = true;
				gl.CullFace(GL_BACK);
				break;

			case CM_ALL:
				enable_cull = true;
				gl.CullFace(GL_FRONT_AND_BACK);
				break;
		}

		if (enable_cull)
			gl.Enable(GL_CULL_FACE);
		else
			gl.Disable(GL_CULL_FACE);
	}

	if (m_DirtyStates.AnySet(RSOF_BLENDMODE | RSOF_ALPHABLENDMODE))
	{
		bool enable_blend;
		bool enable_coverage = (m_BlendMode == BlendMode::BM_ALPHATOCOVERAGE);

		auto GLEnumsFromMode = [](BlendMode mode, GLenum &src, GLenum &dst) -> bool
		{
			bool ret = true;
			switch (mode)
			{
				case BlendMode::BM_ALPHA:
					src = GL_SRC_ALPHA;
					dst = GL_ONE_MINUS_SRC_ALPHA;
					break;

				case BlendMode::BM_ADD:
					src = GL_ONE;
					dst = GL_ONE;
					break;

				case BlendMode::BM_ADDALPHA:
					src = GL_SRC_ALPHA;
					dst = GL_ONE;
					break;

				default:
				case BlendMode::BM_ALPHATOCOVERAGE:
				case BlendMode::BM_REPLACE:
					src = GL_ONE;
					dst = GL_ZERO;
					break;

				case BlendMode::BM_DISABLED:
					ret = false;
					src = GL_ZERO;
					dst = GL_ZERO;
					break;
			}

			return ret;
		};

		GLenum color_src, color_dst;
		GLenum alpha_src, alpha_dst;

		enable_blend = GLEnumsFromMode(m_BlendMode, color_src, color_dst);
		GLEnumsFromMode(m_AlphaBlendMode, alpha_src, alpha_dst);

		gl.BlendFuncSeparate(color_src, color_dst, alpha_src, alpha_dst);

		if (enable_blend)
			gl.Enable(GL_BLEND);
		else
			gl.Disable(GL_BLEND);

		if (enable_coverage)
		{
			gl.Enable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			gl.SampleCoverage(m_AlphaCoverage, m_AlphaCoverageInv);
		}
		else
			gl.Disable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	}

	if (m_DirtyStates.IsSet(RSOF_BLENDEQ))
	{
		static GLenum GLFuncLU[BlendEquation::BE_NUMMODES + 1] =
		{
			GL_FUNC_ADD,
			GL_FUNC_SUBTRACT,
			GL_FUNC_REVERSE_SUBTRACT,
			GL_MIN,
			GL_MAX,
			GL_FUNC_ADD
		};

		gl.BlendEquationSeparate(GLFuncLU[m_BlendEq], GLFuncLU[m_AlphaBlendEq]);
	}

	if (m_DirtyStates.IsSet(RSOF_COLORMASK))
	{
		GLboolean r = m_ChannelWriteMask.IsSet(CM_RED);
		GLboolean g = m_ChannelWriteMask.IsSet(CM_GREEN);
		GLboolean b = m_ChannelWriteMask.IsSet(CM_BLUE);
		GLboolean a = m_ChannelWriteMask.IsSet(CM_ALPHA);

		gl.ColorMask(r, g, b, a);
	}

	m_DirtyStates = 0;
}


void RendererImpl::SetClearColor(const glm::fvec4 *color)
{
	const static glm::fvec4 defcolor = glm::fvec4(0, 0, 0, 1);

	if (!color)
		color = &defcolor;

	if (m_clearColor != *color)
	{
		m_clearColor = *color;
		gl.ClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);
	}
}


const glm::fvec4 *RendererImpl::GetClearColor(glm::fvec4 *color) const
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
		gl.ClearDepthf(m_clearZ);
	}
}


float RendererImpl::GetClearDepth() const
{
	return m_clearZ;
}


void RendererImpl::SetClearStencil(uint8_t stencil)
{
	if (stencil != m_clearStencil)
	{
		m_clearStencil = stencil;
		gl.ClearStencil((GLint)m_clearStencil);
	}
}


uint8_t RendererImpl::GetClearStencil() const
{
	return m_clearStencil;
}


void RendererImpl::SetDepthMode(DepthMode mode)
{
	if (mode != m_DepthMode)
	{
		m_DirtyStates.Set(RSOF_DEPTHMODE);
		m_DepthMode = mode;
	}
}


Renderer::DepthMode RendererImpl::GetDepthMode() const
{
	return m_DepthMode;
}


void RendererImpl::SetDepthTest(Test test)
{
	if (test != m_DepthTest)
	{
		m_DirtyStates.Set(RSOF_DEPTHTEST);
		m_DepthTest = test;
	}
}


Renderer::Test RendererImpl::GetDepthTest() const
{
	return m_DepthTest;
}


void RendererImpl::SetFillMode(FillMode mode)
{
	if (mode != m_FillMode)
	{
		m_DirtyStates.Set(RSOF_FILLMODE);
		m_FillMode = mode;
	}
}


Renderer::FillMode RendererImpl::GetFillMode() const
{
	return m_FillMode;
}

void RendererImpl::SetStencilEnabled(bool en)
{
	if (m_StencilEnabled != en)
	{
		m_DirtyStates.Set(RSOF_STENCIL);
		m_StencilEnabled = en;
	}
}


bool RendererImpl::GetStencilEnabled() const
{
	return m_StencilEnabled;
}


void RendererImpl::SetStencilTest(Test test, uint8_t ref, uint8_t mask)
{
	if ((m_StencilTest != test) || (m_StencilRef != ref) || (m_StencilMask != mask))
	{
		m_DirtyStates.Set(RSOF_STENCILFUNC);

		m_StencilTest = test;
		m_StencilRef = ref;
		m_StencilMask = mask;
	}
}


Renderer::Test RendererImpl::GetStencilTest(uint8_t *ref, uint8_t *mask) const
{
	if (ref)
		*ref = m_StencilRef;
	if (mask)
		*mask = m_StencilMask;

	return m_StencilTest;
}


void RendererImpl::SetStencilOperation(StencilOperation stencil_fail, StencilOperation zfail, StencilOperation zpass)
{
	if ((m_StencilFailOp != stencil_fail) || (m_StencilZFailOp != zfail) || (m_StencilZPassOp != zpass))
	{
		m_DirtyStates.Set(RSOF_STENCILOP);

		m_StencilFailOp = stencil_fail;
		m_StencilZFailOp = zfail;
		m_StencilZPassOp = zpass;
	}
}


void RendererImpl::GetStencilOperation(StencilOperation &stencil_fail, StencilOperation &zfail, StencilOperation &zpass) const
{
	stencil_fail = m_StencilFailOp;
	zfail = m_StencilZFailOp;
	zpass = m_StencilZPassOp;
}


void RendererImpl::SetWindingOrder(WindingOrder mode)
{
	if (mode != m_WindingOrder)
	{
		m_DirtyStates.Set(RSOF_WINDINGORDER);

		m_WindingOrder = mode;
	}
}


Renderer::WindingOrder RendererImpl::GetWindingOrder() const
{
	return m_WindingOrder;
}


void RendererImpl::SetCullMode(CullMode mode)
{
	if (mode != m_CullMode)
	{
		m_DirtyStates.Set(RSOF_CULLMODE);

		m_CullMode = mode;
	}
}


Renderer::CullMode RendererImpl::GetCullMode() const
{
	return m_CullMode;
}


void RendererImpl::SetBlendMode(BlendMode mode)
{
	if (mode != m_BlendMode)
	{
		m_DirtyStates.Set(RSOF_BLENDMODE);

		m_BlendMode = mode;
	}
}


Renderer::BlendMode RendererImpl::GetBlendMode() const
{
	return m_BlendMode;
}


void RendererImpl::SetAlphaBlendMode(BlendMode mode)
{
	if (mode != m_AlphaBlendMode)
	{
		m_DirtyStates.Set(RSOF_ALPHABLENDMODE);

		m_AlphaBlendMode = mode;
	}
}


Renderer::BlendMode RendererImpl::GetAlphaBlendMode() const
{
	return m_AlphaBlendMode;
}


void RendererImpl::SetAlphaPassRange(float minalpha, float maxalpha)
{
	m_AlphaPassMin = minalpha;
	m_AlphaPassMax = maxalpha;
}


void RendererImpl::GetAlphaPassRange(float &minalpha, float &maxalpha)
{
	minalpha = m_AlphaPassMin;
	maxalpha = m_AlphaPassMax;
}


void RendererImpl::SetAlphaCoverage(float coverage, bool invert)
{
	if ((m_AlphaCoverage != coverage) || (m_AlphaCoverageInv != invert))
	{
		m_AlphaCoverage = coverage;
		m_AlphaCoverageInv = invert;
	}
}


void RendererImpl::GetAlphaCoverage(float& coverage, bool& invert)
{
	coverage = m_AlphaCoverage;
	invert = m_AlphaCoverageInv;
}


void RendererImpl::SetBlendEquation(Renderer::BlendEquation eq)
{
	if (eq != m_BlendEq)
	{
		m_DirtyStates.Set(RSOF_BLENDEQ);

		m_BlendEq = eq;
	}
}


Renderer::BlendEquation RendererImpl::GetBlendEquation() const
{
	return m_BlendEq;
}


void RendererImpl::SetAlphaBlendEquation(Renderer::BlendEquation eq)
{
	if (eq != m_AlphaBlendEq)
	{
		m_DirtyStates.Set(RSOF_ALPHABLENDEQ);

		m_AlphaBlendEq = eq;
	}
}


Renderer::BlendEquation RendererImpl::GetAlphaBlendEquation() const
{
	return m_AlphaBlendEq;
}


void RendererImpl::SetChannelWriteMask(ChannelMask mask)
{
	if (mask != m_ChannelWriteMask)
	{
		m_DirtyStates.Set(RSOF_COLORMASK);

		m_ChannelWriteMask = mask;
	}
}


void RendererImpl::SetDepthBias(float bias)
{
	if (bias != m_DepthBias)
	{
		gl.PolygonOffset(1.0f, bias);

		m_DepthBias = bias;
	}
}


float RendererImpl::GetDepthBias() const
{
	return m_DepthBias;
}


Renderer::ChannelMask RendererImpl::GetChannelWriteMask() const
{
	return m_ChannelWriteMask;
}


size_t RendererImpl::PixelSize(Renderer::TextureType type)
{
	switch (type)
	{
		case Renderer::TextureType::S8_1CH:
			return sizeof(int8_t) * 1;

		case Renderer::TextureType::S8_2CH:
			return sizeof(int8_t) * 2;

		case Renderer::TextureType::S8_3CH:
			return sizeof(int8_t) * 3;

		case Renderer::TextureType::S8_4CH:
			return sizeof(int8_t) * 4;

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
		case Renderer::TextureType::U8_4CH:
			return GL_UNSIGNED_BYTE;

		case Renderer::TextureType::S8_1CH:
		case Renderer::TextureType::S8_2CH:
		case Renderer::TextureType::S8_3CH:
		case Renderer::TextureType::S8_4CH:
			return GL_BYTE;

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

		case Renderer::TextureType::DXT1:
			return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

		case Renderer::TextureType::DXT3:
			return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;

		case Renderer::TextureType::DXT5:
			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
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
		case Renderer::TextureType::S8_1CH:
			return GL_R8;

		case Renderer::TextureType::U8_2CH:
		case Renderer::TextureType::S8_2CH:
			return GL_RG8;

		case Renderer::TextureType::U8_3CH:
		case Renderer::TextureType::S8_3CH:
			return GL_RGB8;

		case Renderer::TextureType::P16_3CH:
			return GL_UNSIGNED_SHORT_5_6_5;

		case Renderer::TextureType::P16_3CHT:
			return GL_UNSIGNED_SHORT_5_5_5_1;

		case Renderer::TextureType::P16_4CH:
			return GL_UNSIGNED_SHORT_4_4_4_4;

		case Renderer::TextureType::U8_4CH:
		case Renderer::TextureType::S8_4CH:
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

		case Renderer::TextureType::DXT1:
			return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

		case Renderer::TextureType::DXT3:
			return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;

		case Renderer::TextureType::DXT5:
			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	}

	return 0;
}

GLenum RendererImpl::GLFormat(TextureType type)
{
	switch (type)
	{
		case Renderer::TextureType::U8_1CH:
		case Renderer::TextureType::S8_1CH:
		case Renderer::TextureType::F16_1CH:
		case Renderer::TextureType::F32_1CH:
			return GL_RED;

		case Renderer::TextureType::U8_2CH:
		case Renderer::TextureType::S8_2CH:
		case Renderer::TextureType::F16_2CH:
		case Renderer::TextureType::F32_2CH:
			return GL_RG;

		case Renderer::TextureType::P8_3CH:
		case Renderer::TextureType::P16_3CH:
		case Renderer::TextureType::P16_3CHT:
		case Renderer::TextureType::U8_3CH:
		case Renderer::TextureType::S8_3CH:
		case Renderer::TextureType::F16_3CH:
		case Renderer::TextureType::F32_3CH:
			return GL_RGB;

		case Renderer::TextureType::P16_4CH:
		case Renderer::TextureType::U8_4CH:
		case Renderer::TextureType::S8_4CH:
		case Renderer::TextureType::F16_4CH:
		case Renderer::TextureType::F32_4CH:
			return GL_RGBA;

		case Renderer::TextureType::DXT1:
			return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

		case Renderer::TextureType::DXT3:
			return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;

		case Renderer::TextureType::DXT5:
			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	}

	return 0;
}


Texture2D *RendererImpl::CreateTexture2D(size_t width, size_t height, TextureType type, size_t mipcount, props::TFlags64 createflags)
{
	return new Texture2DImpl(this, width, height, type, mipcount, createflags);
}


TextureCube *RendererImpl::CreateTextureCube(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount, props::TFlags64 createflags)
{
	return new TextureCubeImpl(this, width, height, depth, type, mipcount, createflags);
}


Texture3D *RendererImpl::CreateTexture3D(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount, props::TFlags64 createflags)
{
	return new Texture3DImpl(this, width, height, depth, type, mipcount, createflags);
}


DepthBuffer* RendererImpl::CreateDepthBuffer(size_t width, size_t height, DepthType type, props::TFlags64 createflags)
{
	return new DepthBufferImpl(this, width, height, type);
}


FrameBuffer *RendererImpl::CreateFrameBuffer(props::TFlags64 createflags, const TCHAR *name)
{
	static uint32_t fbidx = 0;
	static TCHAR fbname[16];
	if (!name)
	{
		_stprintf_s(fbname, _T("fb%d"), fbidx);
		fbidx++;
		name = fbname;
		m_pSys->GetLog()->Print(_T("Anonymous FrameBuffer created as \"%s\"\n"), name);
	}

	FrameBuffer *ret = new FrameBufferImpl(this, name);

	std::pair<TNameToFrameBufferMap::iterator, bool> insret = m_NameToFB.insert(TNameToFrameBufferMap::value_type(name, ret));
	if (!insret.second)
	{
		m_pSys->GetLog()->Print(_T("Did you mean to create a FrameBuffer with an existing name? (\"%s\")\n"), name);
	}

	return ret;
}


FrameBuffer *RendererImpl::FindFrameBuffer(const TCHAR *name) const
{
	TNameToFrameBufferMap::const_iterator it = m_NameToFB.find(name);
	if (it != m_NameToFB.cend())
		return it->second;

	return nullptr;
}


void RendererImpl::RemoveFrameBuffer(const TCHAR *name)
{
	m_NameToFB.erase(name);
}


VertexBuffer *RendererImpl::CreateVertexBuffer(props::TFlags64 createflags)
{
	return new VertexBufferImpl(this);
}


IndexBuffer *RendererImpl::CreateIndexBuffer(props::TFlags64 createflags)
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


RenderMethod *RendererImpl::CreateRenderMethod()
{
	return new RenderMethodImpl(this);
}


void RendererImpl::UseRenderMethod(const RenderMethod *method, size_t techidx)
{
	m_ActiveRenderMethod = method ? (RenderMethod *)method : m_DefaultRenderMethod;
	if (m_ActiveRenderMethod)
		m_ActiveRenderMethod->SetActiveTechnique(techidx);
	else
		UseProgram(nullptr);
}


RenderMethod *RendererImpl::GetActiveRenderMethod() const
{
	return m_ActiveRenderMethod;
}


void RendererImpl::UseMaterial(const Material *material)
{
	m_ActiveMaterial = (Material *)material;
}


Material *RendererImpl::GetActiveMaterial() const
{
	return m_ActiveMaterial;
}


void RendererImpl::UseFrameBuffer(FrameBuffer *pfb, props::TFlags64 flags)
{
	if (pfb == m_CurFB)
		return;

	GLuint glid = 0;
	if (pfb)
	{
		c3::FrameBufferImpl &fb = (c3::FrameBufferImpl &)*pfb;
		glid = fb;
	}

	if (flags.IsSet(UFBFLAG_FINISHLAST))
		gl.Finish();

	if (pfb)
	{
		// Stop using any textures that we want to render to
		for (size_t i = 0, maxi = pfb->GetNumColorTargets(); i < maxi; i++)
		{
			for (uint64_t s = 0; s < 32; s++)
			{
				if (s_TexCache[s] == pfb->GetColorTarget(i))
					UseTexture(s, nullptr);
			}
		}
	}

	if (flags.IsSet(UFBFLAG_UPDATEVIEWPORT))
	{
		// we set the initial viewport based on the dimensions of the depth target -- or the hwnd, in lieu of that
		DepthBuffer* pdb = pfb ? pfb->GetDepthTarget() : nullptr;
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
	}

	m_CurFB = pfb;
	m_CurFBID = glid;

	gl.BindFramebuffer(GL_FRAMEBUFFER, glid);

	if (flags.AnySet(UFBFLAG_CLEARCOLOR | UFBFLAG_CLEARDEPTH | UFBFLAG_CLEARSTENCIL))
	{
		DepthMode dm = GetDepthMode();
		bool changed_dm = false;

		// the depth buffer must be writable if the request was made to clear it
		if (flags.IsSet(UFBFLAG_CLEARDEPTH) && (dm != DepthMode::DM_READWRITE) && (dm != DepthMode::DM_WRITEONLY))
		{
			SetDepthMode(DepthMode::DM_WRITEONLY);
			changed_dm = true;
		}

		BlendMode bm = m_BlendMode;
		SetBlendMode(BlendMode::BM_REPLACE);
		if (m_CurFB)
			m_CurFB->SetBlendMode(BlendMode::BM_REPLACE);

		UpdateDirtyRenderStates(true);

		if (!m_CurFB || isnv)
		{
			gl.Clear((flags.IsSet(UFBFLAG_CLEARCOLOR) ? GL_COLOR_BUFFER_BIT : 0) |
					 (flags.IsSet(UFBFLAG_CLEARDEPTH) ? GL_DEPTH_BUFFER_BIT : 0) |
					 (flags.IsSet(UFBFLAG_CLEARSTENCIL) ? GL_STENCIL_BUFFER_BIT : 0));
		}

		if (m_CurFB)
		{
			m_CurFB->Clear((uint64_t)flags | (isnv ? UFBFLAG_STRICTCOMPLIANCE : 0));
		}

		SetBlendMode(bm);

		if (changed_dm)
			SetDepthMode(dm);
	}
	else
	{
		UpdateDirtyRenderStates(true);
	}
}


FrameBuffer *RendererImpl::GetActiveFrameBuffer()
{
	return m_CurFB;
}


void RendererImpl::UseProgram(ShaderProgram *pprog)
{
	if ((pprog == m_CurProg))
		return;

	GLuint glid = pprog ? (GLuint)(c3::ShaderProgramImpl &)*pprog : NULL;

	m_CurProg = pprog;
	m_CurProgID = glid;

	gl.UseProgram(m_CurProgID);
}


ShaderProgram *RendererImpl::GetActiveProgram()
{
	return m_CurProg;
}


void RendererImpl::UseVertexBuffer(VertexBuffer *pvbuf)
{
	if (pvbuf == m_CurVB)
		return;

	GLuint glid_vb = pvbuf ? ((VertexBufferImpl *)pvbuf)->VBglID() : NULL;
	GLuint glid_vao = pvbuf ? ((VertexBufferImpl *)pvbuf)->VAOglID() : NULL;

	m_CurVB = pvbuf;
	m_CurVBID = glid_vb;
	m_VAOglID = glid_vao;

	gl.BindBuffer(GL_ARRAY_BUFFER, m_CurVBID);
	gl.BindVertexArray(m_VAOglID);
}


void RendererImpl::UseIndexBuffer(IndexBuffer *pibuf)
{
	if (pibuf == m_CurIB)
		return;

	GLuint glid = pibuf ? ((c3::IndexBufferImpl *)pibuf)->IBglID() : NULL;

	m_CurIB = pibuf;
	m_CurIBID = glid;

	gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, glid);
}


void RendererImpl::UseTexture(uint64_t texunit, Texture *ptex, props::TFlags32 texflags)
{
	if (texunit >= 32)
		return;

	static const GLenum texunitidlu[32] =
	{
		GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7,
		GL_TEXTURE8, GL_TEXTURE9, GL_TEXTURE10, GL_TEXTURE11, GL_TEXTURE12, GL_TEXTURE13, GL_TEXTURE14, GL_TEXTURE15,
		GL_TEXTURE16, GL_TEXTURE17, GL_TEXTURE18, GL_TEXTURE19, GL_TEXTURE20, GL_TEXTURE21, GL_TEXTURE22, GL_TEXTURE23,
		GL_TEXTURE24, GL_TEXTURE25, GL_TEXTURE26, GL_TEXTURE27, GL_TEXTURE28, GL_TEXTURE29, GL_TEXTURE30, GL_TEXTURE31
	};

	if (ptex)
	{
		if (texflags != s_SamplerCache[texunit])
		{
			GLuint sampid;

			TTexFlagsToSamplerMap::iterator it = m_TexFlagsToSampler.find((uint32_t)texflags);
			if (it == m_TexFlagsToSampler.end())
			{
				//make a new sampler object
				gl.GenSamplers(1, &sampid);

				if (texflags.IsSet(TEXFLAG_MIRROR_U | TEXFLAG_WRAP_U))
					gl.SamplerParameteri(sampid, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
				else if (texflags.IsSet(TEXFLAG_WRAP_U))
					gl.SamplerParameteri(sampid, GL_TEXTURE_WRAP_S, GL_REPEAT);
				else if (texflags.IsSet(TEXFLAG_MIRROR_U))
					gl.SamplerParameteri(sampid, GL_TEXTURE_WRAP_S, GL_MIRROR_CLAMP_TO_EDGE);
				else
					gl.SamplerParameteri(sampid, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

				if (texflags.IsSet(TEXFLAG_MIRROR_V | TEXFLAG_WRAP_V))
					gl.SamplerParameteri(sampid, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
				else if (texflags.IsSet(TEXFLAG_WRAP_V))
					gl.SamplerParameteri(sampid, GL_TEXTURE_WRAP_T, GL_REPEAT);
				else if (texflags.IsSet(TEXFLAG_MIRROR_V))
					gl.SamplerParameteri(sampid, GL_TEXTURE_WRAP_T, GL_MIRROR_CLAMP_TO_EDGE);
				else
					gl.SamplerParameteri(sampid, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				if (texflags.IsSet(TEXFLAG_MIRROR_W | TEXFLAG_WRAP_W))
					gl.SamplerParameteri(sampid, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
				else if (texflags.IsSet(TEXFLAG_WRAP_W))
					gl.SamplerParameteri(sampid, GL_TEXTURE_WRAP_R, GL_REPEAT);
				else if (texflags.IsSet(TEXFLAG_MIRROR_W))
					gl.SamplerParameteri(sampid, GL_TEXTURE_WRAP_R, GL_MIRROR_CLAMP_TO_EDGE);
				else
					gl.SamplerParameteri(sampid, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

				if (texflags.IsSet(TEXFLAG_MAGFILTER_LINEAR))
					gl.SamplerParameteri(sampid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				else
					gl.SamplerParameteri(sampid, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				if (ptex->MipCount() > 1)
				{
					if (texflags.IsSet(TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPLINEAR))
						gl.SamplerParameteri(sampid, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					else if (texflags.IsSet(TEXFLAG_MINFILTER_LINEAR))
						gl.SamplerParameteri(sampid, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
					else if (texflags.IsSet(TEXFLAG_MINFILTER_MIPLINEAR))
						gl.SamplerParameteri(sampid, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
					else
						gl.SamplerParameteri(sampid, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				}
				else
				{
					if (texflags.IsSet(TEXFLAG_MINFILTER_LINEAR))
						gl.SamplerParameteri(sampid, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					else
						gl.SamplerParameteri(sampid, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}

				m_TexFlagsToSampler.insert(TTexFlagsToSamplerMap::value_type((uint32_t)texflags, sampid));
			}
			else
			{
				sampid = it->second;
			}

			gl.BindSampler((GLuint)texunit, sampid);

			s_SamplerCache[texunit] = texflags;
		}
	}

	if (s_TexCache[texunit] == ptex)
		return;

	Texture *luptex = ptex ? ptex : s_TexCache[texunit];

	GLuint glid = NULL;
	GLenum textype;

	if (luptex)
	{
		if (c3::Texture2DImpl *tex2d = dynamic_cast<c3::Texture2DImpl *>(luptex))
		{
			textype = GL_TEXTURE_2D;
			glid = (GLuint)(c3::Texture2DImpl &)*tex2d;
		}
		else if (c3::TextureCubeImpl *texcube = dynamic_cast<c3::TextureCubeImpl *>(luptex))
		{
			textype = GL_TEXTURE_CUBE_MAP;
			glid = (GLuint)(c3::TextureCubeImpl &)*texcube;
		}
		else if (c3::Texture3DImpl *tex3d = dynamic_cast<c3::Texture3DImpl *>(luptex))
		{
			textype = GL_TEXTURE_3D;
			glid = (GLuint)(c3::Texture3DImpl &)*tex3d;
		}
		else if (c3::DepthBufferImpl *texdep = dynamic_cast<c3::DepthBufferImpl *>((c3::DepthBuffer *)luptex))
		{
			textype = GL_TEXTURE_2D;
			glid = (GLuint)(c3::DepthBufferImpl &)*texdep;
		}
	}
	else
	{
		textype = GL_TEXTURE_2D;
		ptex = m_BlackTex;
		glid = (GLuint)(c3::Texture2DImpl &)*m_BlackTex;
	}

	s_TexCache[texunit] = ptex;

	gl.ActiveTexture(texunitidlu[texunit]);
	gl.BindTexture(textype, ptex ? glid : 0);
}


static const GLenum typelu[Renderer::PrimType::NUM_PRIMTYPES] = { GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN };

bool RendererImpl::DrawPrimitives(PrimType type, size_t count)
{
	if (m_CurVBID)
	{
		if (count == -1)
			count = m_CurVB->Count();

		std::function<void(Renderer::RenderStateOverrideFlags)> _draw = [&](Renderer::RenderStateOverrideFlags rsof)
		{
			if (m_CurProg)
			{
				if (m_ActiveMaterial)
					m_ActiveMaterial->Apply(m_CurProg, rsof);

				m_CurProg->ApplyUniforms();
			}

			UpdateDirtyRenderStates();

			gl.DrawArrays(typelu[type], 0, (GLsizei)count);
		};

		RenderMethod::Technique *tech = m_ActiveRenderMethod ? m_ActiveRenderMethod->GetActiveTechnique() : nullptr;
		if (tech)
		{
			size_t passct;
			if (tech->Begin(passct))
			{
				for (size_t passidx = 0; passidx < passct; passidx++)
				{
					Renderer::RenderStateOverrideFlags rsof = tech->ApplyPass(passidx);

					_draw(rsof);
				}

				tech->End();
			}
		}
		else
		{
			_draw(0);
		}

		m_VertsPerFrame += m_CurVB->Count();

		switch (type)
		{
			case PrimType::POINTLIST:
				m_PointsPerFrame += count;
				break;

			case PrimType::LINELIST:
				m_LinesPerFrame += (count / 2);
				break;

			case PrimType::LINESTRIP:
				m_LinesPerFrame += (count / 2) + 1;
				break;

			case PrimType::TRILIST:
				m_TrisPerFrame += (count / 3);
				break;

			case PrimType::TRISTRIP:
				m_TrisPerFrame += (count / 3) + 1;
				break;

			case PrimType::TRIFAN:
				m_TrisPerFrame += (count / 3) + 2;
				break;

			default:
				break;
		}

		return true;
	}

	return false;
}


bool RendererImpl::DrawIndexedPrimitives(PrimType type, size_t offset, size_t count)
{
	if ((m_CurIBID == NULL) || (m_CurVBID == NULL))
		return false;

	if (count == -1)
		count = m_CurIB->Count();

	size_t idxs = m_CurIB->GetIndexSize();
	if (!idxs)
		return false;

	GLuint glidxs;
	switch (idxs)
	{
		case IndexBuffer::IndexSize::IS_8BIT: glidxs = GL_UNSIGNED_BYTE; break;
		case IndexBuffer::IndexSize::IS_16BIT: glidxs = GL_UNSIGNED_SHORT; break;
		case IndexBuffer::IndexSize::IS_32BIT: glidxs = GL_UNSIGNED_INT; break;
	}

	std::function<void(Renderer::RenderStateOverrideFlags)> _draw = [&](Renderer::RenderStateOverrideFlags rsof)
	{
		if (m_CurProg)
		{
			if (m_ActiveMaterial)
				m_ActiveMaterial->Apply(m_CurProg, rsof);

			m_CurProg->ApplyUniforms();
		}

		UpdateDirtyRenderStates();

		gl.DrawElements(typelu[type], (GLsizei)count, glidxs, NULL);
	};

	RenderMethod::Technique *tech = m_ActiveRenderMethod ? m_ActiveRenderMethod->GetActiveTechnique() : nullptr;
	if (tech)
	{
		size_t passct;
		if (tech->Begin(passct))
		{
			for (size_t passidx = 0; passidx < passct; passidx++)
			{
				Renderer::RenderStateOverrideFlags rsof = tech->ApplyPass(passidx);

				_draw(rsof);
			}
			tech->End();
		}
	}
	else
	{
		_draw(0);
	}

	m_VertsPerFrame += m_CurVB->Count();
	m_IndicesPerFrame += m_CurIB->Count();

	switch (type)
	{
		case PrimType::POINTLIST:
			m_PointsPerFrame += count;
			break;

		case PrimType::LINELIST:
			m_LinesPerFrame += (count / 2);
			break;

		case PrimType::LINESTRIP:
			m_LinesPerFrame += (count / 2) + 1;
			break;

		case PrimType::TRILIST:
			m_TrisPerFrame += (count / 3);
			break;

		case PrimType::TRISTRIP:
			m_TrisPerFrame += (count / 3) + 1;
			break;

		case PrimType::TRIFAN:
			m_TrisPerFrame += (count / 3) + 2;
			break;

		default:
			break;
	}

	return true;
}

const BoundingBox *RendererImpl::GetClipFrustum()
{
	if (m_matupflags.IsSet(MATRIXUPDATE_FRUSTUM))
	{
		m_Frustum->SetAsFrustum(&m_view, &m_proj, &m_Viewport);
		m_matupflags.Clear(MATRIXUPDATE_FRUSTUM);
	}

	return m_Frustum;
}

void RendererImpl::SetProjectionMatrix(const glm::fmat4x4 *m)
{
	if (*m != m_proj)
	{
		m_proj = *m;
		m_matupflags.Set(MATRIXUPDATE_PROJ | MATRIXUPDATE_FRUSTUM);
	}
}


void RendererImpl::SetViewMatrix(const glm::fmat4x4 *m)
{
	if (*m != m_view)
	{
		m_view = *m;
		m_matupflags.Set(MATRIXUPDATE_VIEW | MATRIXUPDATE_FRUSTUM);
	}
}


void RendererImpl::SetWorldMatrix(const glm::fmat4x4 *m)
{
	if (!m)
		m = &m_ident;

	if (*m != m_world)
	{
		m_world = *m;
		m_matupflags.Set(MATRIXUPDATE_WORLD);
	}
}


void RendererImpl::SetSunShadowMatrix(const glm::fmat4x4 *m)
{
	if (m)
		m_sunshadow = *m;
	else
		m_sunshadow = m_ident;
}


void RendererImpl::SetTextureTransformMatrix(const glm::fmat4x4 *m)
{
	if (m)
		m_texturetransform = *m;
	else
		m_texturetransform = m_ident;
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


const glm::fmat4x4 *RendererImpl::GetWorldViewMatrix(glm::fmat4x4 *m)
{
	if (m_matupflags.AnySet(MATRIXUPDATE_WORLDVIEW))
	{
		m_worldview = m_view * m_world;
		m_matupflags.Clear(MATRIXUPDATE_WORLDVIEW);
	}

	if (!m)
		return &m_worldview;

	*m = m_worldview;
	return m;
}


const glm::fmat4x4 *RendererImpl::GetNormalMatrix(glm::fmat4x4 *m)
{
	if (m_matupflags.AnySet(MATRIXUPDATE_NORMAL))
	{
		m_normal = glm::inverseTranspose(m_world);
		m_matupflags.Clear(MATRIXUPDATE_NORMAL);
	}

	if (!m)
		return &m_normal;

	*m = m_normal;
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


const glm::fmat4x4 *RendererImpl::GetWorldProjectionMatrix(glm::fmat4x4 *m)
{
	if (m_matupflags.AnySet(MATRIXUPDATE_WORLDPROJ))
	{
		m_worldproj = m_proj * m_world;
		m_matupflags.Clear(MATRIXUPDATE_WORLDPROJ);
	}

	if (!m)
		return &m_worldproj;

	*m = m_worldproj;
	return m;
}


const glm::fmat4x4 *RendererImpl::GetWorldViewProjectionMatrix(glm::fmat4x4 *m)
{
	if (m_matupflags.AnySet(MATRIXUPDATE_WORLDVIEWPROJ))
	{
		m_worldviewproj = *GetViewProjectionMatrix() * m_world;
		m_matupflags.Clear(MATRIXUPDATE_WORLDVIEWPROJ);
	}

	if (!m)
		return &m_worldviewproj;

	*m = m_worldviewproj;
	return m;
}


const glm::fmat4x4 *RendererImpl::GetSunShadowMatrix(glm::fmat4x4 *m)
{
	if (!m)
		return &m_sunshadow;

	*m = m_sunshadow;
	return m;
}


const glm::fmat4x4 *RendererImpl::GetTextureTransformMatrix(glm::fmat4x4 *m)
{
	if (!m)
		return &m_texturetransform;

	*m = m_texturetransform;
	return m;
}


void RendererImpl::SetEyePosition(const glm::fvec3* pos)
{
	if (!pos)
		return;

	m_eyepos = *pos;
}


void RendererImpl::SetEyeDirection(const glm::fvec3* dir)
{
	if (!dir)
		return;

	m_eyedir = *dir;
}


void RendererImpl::SetModelInstanceData(const Model::InstanceData *pinstdata)
{
	m_pModelInstData = pinstdata;
}


const Model::InstanceData *RendererImpl::GetModelInstanceData()
{
	return m_pModelInstData;
}


const glm::fvec3* RendererImpl::GetEyePosition(glm::fvec3* pos) const
{
	if (!pos)
		return &m_eyepos;

	*pos = m_eyepos;
	return pos;
}


const glm::fvec3* RendererImpl::GetEyeDirection(glm::fvec3* dir) const
{
	if (!dir)
		return &m_eyedir;

	*dir = m_eyedir;
	return dir;
}


void ComputeTangentBinorm(Vertex::PNYT1::s *v, const Vertex::PNYT1::s *adj)
{
	v->tang = glm::normalize(adj->pos - v->pos);
	v->binorm = glm::normalize(glm::cross(v->norm, v->binorm));
}

VertexBuffer *RendererImpl::GetCubeVB()
{
	if (!m_CubeVB)
	{
		m_CubeVB = CreateVertexBuffer(0);

		static Vertex::PNYT1::s v[6 * 4] =
		{
			// TOP +Z 0
			{ { -1, -1,  1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  0 } },
			{ {  1, -1,  1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  1 } },
			{ {  1,  1,  1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  1,  1 } },
			{ { -1,  1,  1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  1,  0 } },

			// RIGHT +X 4
			{ {  1,  1,  1 }, {  1,  0,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  0 } },
			{ {  1,  1, -1 }, {  1,  0,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  1 } },
			{ {  1, -1, -1 }, {  1,  0,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  1,  1 } },
			{ {  1, -1,  1 }, {  1,  0,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  1,  0 } },

			// BOTTOM -Z 8
			{ { -1, -1, -1 }, {  0,  0, -1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  0 } },
			{ {  1, -1, -1 }, {  0,  0, -1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  1 } },
			{ {  1,  1, -1 }, {  0,  0, -1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  1,  1 } },
			{ { -1,  1, -1 }, {  0,  0, -1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  1,  0 } },

			// LEFT -X 12
			{ { -1, -1, -1 }, { -1,  0,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  0 } },
			{ { -1, -1,  1 }, { -1,  0,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  1 } },
			{ { -1,  1,  1 }, { -1,  0,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  1,  1 } },
			{ { -1,  1, -1 }, { -1,  0,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  1,  0 } },

			// FRONT +Y 16
			{ { -1,  1, -1 }, {  0,  1,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  0 } },
			{ {  1,  1, -1 }, {  0,  1,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  1 } },
			{ {  1,  1,  1 }, {  0,  1,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  1,  1 } },
			{ { -1,  1,  1 }, {  0,  1,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  1,  0 } },

			// FRONT -Y 20
			{ { -1, -1, -1 }, {  0, -1,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  0 } },
			{ {  1, -1, -1 }, {  0, -1,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  1 } },
			{ {  1, -1,  1 }, {  0, -1,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  1,  1 } },
			{ { -1, -1,  1 }, {  0, -1,  0 }, {  0,  0,  1 }, {  0,  0,  1 }, {  1,  0 } }
		};

		for (size_t i = 0; i < 6; i++)
		{
			for (size_t j = 0; j < 4; j++)
			{
				size_t idx = i * 4 + j;
				size_t nidx = (idx + 1) % 4;
				Vertex::PNYT1::s *a = &v[idx], *b = &v[nidx];
				ComputeTangentBinorm(a, b);
			}
		}

		void *buf;
		if (m_CubeVB->Lock(&buf, 6 * 4, Vertex::PNYT1::d, VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE) == VertexBuffer::RETURNCODE::RET_OK)
		{
			memcpy(buf, v, sizeof(Vertex::PNYT1::s) * 6 * 4);

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
				if (pib->Lock(&buf, 12 * 2, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
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
					 { {  0,  1,  2 }, {  0,  2,  3 } }		// top
					,{ {  4,  6,  5 }, {  4,  7,  6 } }		// right
					,{ {  8, 10,  9 }, {  8, 11, 10 } }		// bottom
					,{ { 12, 13, 14 }, { 12, 14, 15 } }		// left fix
					,{ { 16, 18, 17 }, { 16, 19, 18 } }		// front
					,{ { 20, 21, 22 }, { 20, 22, 23 } }		// back
				};

				void *buf;
				if (pib->Lock(&buf, 2 * 3 * 6, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
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


VertexBuffer *RendererImpl::GetRefCubeVB()
{
	if (!m_RefCubeVB)
	{
		m_RefCubeVB = CreateVertexBuffer(0);

		constexpr int w = 30;
		constexpr int xofs = 2;
		constexpr int yofs = 34;
		constexpr int zofs = 66;
		constexpr int h = 29;
		constexpr int pos = 1;
		constexpr int neg = 33;
		constexpr int imgw = 128;
		constexpr int imgh = 64;

		constexpr float hpp = 1.0f / (float)imgh;
		constexpr float wpp = 1.0f / (float)imgw;

		constexpr float xpu0 = wpp * (float)(xofs);
		constexpr float xpu1 = wpp * (float)(xofs + w);
		constexpr float xpv0 = hpp * (float)(pos);
		constexpr float xpv1 = hpp * (float)(pos + h);

		constexpr float ypu0 = wpp * (float)(yofs);
		constexpr float ypu1 = wpp * (float)(yofs + w);
		constexpr float ypv0 = hpp * (float)(pos);
		constexpr float ypv1 = hpp * (float)(pos + h);

		constexpr float zpu0 = wpp * (float)(zofs);
		constexpr float zpu1 = wpp * (float)(zofs + w);
		constexpr float zpv0 = hpp * (float)(pos);
		constexpr float zpv1 = hpp * (float)(pos + h);

		constexpr float xnu0 = wpp * (float)(xofs);
		constexpr float xnu1 = wpp * (float)(xofs + w);
		constexpr float xnv0 = hpp * (float)(neg);
		constexpr float xnv1 = hpp * (float)(neg + h);

		constexpr float ynu0 = wpp * (float)(yofs);
		constexpr float ynu1 = wpp * (float)(yofs + w);
		constexpr float ynv0 = hpp * (float)(neg);
		constexpr float ynv1 = hpp * (float)(neg + h);

		constexpr float znu0 = wpp * (float)(zofs);
		constexpr float znu1 = wpp * (float)(zofs + w);
		constexpr float znv0 = hpp * (float)(neg);
		constexpr float znv1 = hpp * (float)(neg + h);

		static Vertex::PNYT1::s v[6 * 4] =
		{
			// RIGHT +X 0
			{ {  1,  1,  1 }, {  1,  0,  0 }, {  0,  1,  0 }, {  0,  0,  1 }, { xpu1, xpv0 } },
			{ {  1,  1, -1 }, {  1,  0,  0 }, {  0,  1,  0 }, {  0,  0,  1 }, { xpu1, xpv1 } },
			{ {  1, -1, -1 }, {  1,  0,  0 }, {  0,  1,  0 }, {  0,  0,  1 }, { xpu0, xpv1 } },
			{ {  1, -1,  1 }, {  1,  0,  0 }, {  0,  1,  0 }, {  0,  0,  1 }, { xpu0, xpv0 } },

			// FRONT +Y 4
			{ {  1,  1,  1 }, {  0,  1,  0 }, {  1,  0,  0 }, {  0,  0,  1 }, { ypu0, ypv0 } },
			{ {  1,  1, -1 }, {  0,  1,  0 }, {  1,  0,  0 }, {  0,  0,  1 }, { ypu0, ypv1 } },
			{ { -1,  1, -1 }, {  0,  1,  0 }, {  1,  0,  0 }, {  0,  0,  1 }, { ypu1, ypv1 } },
			{ { -1,  1,  1 }, {  0,  1,  0 }, {  1,  0,  0 }, {  0,  0,  1 }, { ypu1, ypv0 } },

			// TOP +Z 8
			{ {  1,  1,  1 }, {  0,  0,  1 }, {  1,  0,  0 }, {  0,  1,  0 }, { zpu0, zpv1 } },
			{ {  1, -1,  1 }, {  0,  0,  1 }, {  1,  0,  0 }, {  0,  1,  0 }, { zpu0, zpv0 } },
			{ { -1, -1,  1 }, {  0,  0,  1 }, {  1,  0,  0 }, {  0,  1,  0 }, { zpu1, zpv0 } },
			{ { -1,  1,  1 }, {  0,  0,  1 }, {  1,  0,  0 }, {  0,  1,  0 }, { zpu1, zpv1 } },

			// LEFT -X 12
			{ { -1,  1,  1 }, { -1,  0,  0 }, {  0,  1,  0 }, {  0,  0,  1 }, { xnu0, xnv0 } },
			{ { -1,  1, -1 }, { -1,  0,  0 }, {  0,  1,  0 }, {  0,  0,  1 }, { xnu0, xnv1 } },
			{ { -1, -1, -1 }, { -1,  0,  0 }, {  0,  1,  0 }, {  0,  0,  1 }, { xnu1, xnv1 } },
			{ { -1, -1,  1 }, { -1,  0,  0 }, {  0,  1,  0 }, {  0,  0,  1 }, { xnu1, xnv0 } },

			// FRONT -Y 16
			{ {  1, -1,  1 }, {  0, -1,  0 }, {  1,  0,  0 }, {  0,  0,  1 }, { ynu1, ynv0 } },
			{ {  1, -1, -1 }, {  0, -1,  0 }, {  1,  0,  0 }, {  0,  0,  1 }, { ynu1, ynv1 } },
			{ { -1, -1, -1 }, {  0, -1,  0 }, {  1,  0,  0 }, {  0,  0,  1 }, { ynu0, ynv1 } },
			{ { -1, -1,  1 }, {  0, -1,  0 }, {  1,  0,  0 }, {  0,  0,  1 }, { ynu0, ynv0 } },

			// BOTTOM -Z 20
			{ {  1,  1, -1 }, {  0,  0, -1 }, {  1,  0,  0 }, {  0,  1,  0 }, { znu1, znv1 } },
			{ {  1, -1, -1 }, {  0,  0, -1 }, {  1,  0,  0 }, {  0,  1,  0 }, { znu1, znv0 } },
			{ { -1, -1, -1 }, {  0,  0, -1 }, {  1,  0,  0 }, {  0,  1,  0 }, { znu0, znv0 } },
			{ { -1,  1, -1 }, {  0,  0, -1 }, {  1,  0,  0 }, {  0,  1,  0 }, { znu0, znv1 } },
		};

		void *buf;
		if (m_RefCubeVB->Lock(&buf, 6 * 4, Vertex::PNYT1::d, VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE) == VertexBuffer::RETURNCODE::RET_OK)
		{
			memcpy(buf, v, sizeof(Vertex::PNYT1::s) * 6 * 4);

			m_RefCubeVB->Unlock();
		}
	}

	return m_RefCubeVB;
}


Mesh *RendererImpl::GetRefCubeMesh()
{
	if (!m_RefCubeMesh)
	{
		m_RefCubeMesh = CreateMesh();

		m_RefCubeMesh->AttachVertexBuffer(GetRefCubeVB());

		if (m_RefCubeVB)
		{
			IndexBuffer *pib = CreateIndexBuffer(0);
			if (pib)
			{
				static const uint16_t i[6][2][3] =
				{
					 { {  0,  1,  2 }, {  0,  2,  3 } }		// right
					,{ {  4,  6,  5 }, {  4,  7,  6 } }		// front
					,{ {  8, 9,  10 }, {  8, 10, 11 } }		// top
					,{ { 12, 14, 13 }, { 12, 15, 14 } }		// left
					,{ { 16, 17, 18 }, { 16, 18, 19 } }		// back
					,{ { 20, 22, 21 }, { 20, 23, 22 } }		// bottom
				};

				void *buf;
				if (pib->Lock(&buf, 2 * 3 * 6, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
				{
					memcpy(buf, i[0], sizeof(uint16_t) * 2 * 3 * 6);

					pib->Unlock();
				}

				m_RefCubeMesh->AttachIndexBuffer(pib);
				m_RefCubeMesh->GetBounds();
			}
		}
	}

	return m_RefCubeMesh;
}


VertexBuffer *RendererImpl::GetPlanesVB()
{
	if (!m_PlanesVB)
	{
		m_PlanesVB = CreateVertexBuffer(0);

		static const Vertex::PNT1::s v[3 * 4] =
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
		if (m_PlanesVB->Lock(&buf, 3 * 4, Vertex::PNT1::d, VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE) == VertexBuffer::RETURNCODE::RET_OK)
		{
			memcpy(buf, v, sizeof(Vertex::PNT1::s) * 3 * 4);

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
				if (pib->Lock(&buf, 2 * 3, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
				{
					memcpy(buf, i[0], sizeof(uint16_t) * 2 * 3);

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
				if (pib->Lock(&buf, 2 * 3, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
				{
					memcpy(buf, i[0], sizeof(uint16_t) * 2 * 3);

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
					 {  4,  6, 5 }, {  4,  7, 6  }
				};

				void *buf;
				if (pib->Lock(&buf, 2 * 3, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
				{
					memcpy(buf, i[0], sizeof(uint16_t) * 2 * 3);

					pib->Unlock();
				}

				m_XZPlaneMesh->AttachIndexBuffer(pib);
			}
		}
	}

	return m_XZPlaneMesh;
}


Mesh *RendererImpl::GetGuiRectMesh()
{
	if (!m_GuiRectMesh)
	{
		m_GuiRectMesh = CreateMesh();

		void *buf;

		VertexBuffer *pvb = CreateVertexBuffer(0);
		if (pvb)
		{
			static const Vertex::PNYT1::s v[4] =
			{
				// GUI Rect (on the x/y plane)
				{ { 0, 0, 0 }, { 0, 0, -1 }, { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0 } },
				{ { 0, 1, 0 }, { 0, 0, -1 }, { 1, 0, 0 }, { 0, 1, 0 }, { 0, 1 } },
				{ { 1, 1, 0 }, { 0, 0, -1 }, { 1, 0, 0 }, { 0, 1, 0 }, { 1, 1 } },
				{ { 1, 0, 0 }, { 0, 0, -1 }, { 1, 0, 0 }, { 0, 1, 0 }, { 1, 0 } }
			};

			if (pvb->Lock(&buf, 4, Vertex::PNYT1::d, VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE) == VertexBuffer::RETURNCODE::RET_OK)
			{
				memcpy(buf, v, sizeof(Vertex::PNYT1::s) * 4);

				pvb->Unlock();
			}
		}

		IndexBuffer *pib = CreateIndexBuffer(0);
		if (pib)
		{
			static const uint16_t i[2][3] =
			{
				 { 0, 2, 1 }, {  0, 3, 2  }
			};

			if (pib->Lock(&buf, 2 * 3, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
			{
				memcpy(buf, i[0], sizeof(uint16_t) * 2 * 3);

				pib->Unlock();
			}
		}

		m_GuiRectMesh->AttachVertexBuffer(pvb);
		m_GuiRectMesh->AttachIndexBuffer(pib);
	}

	return m_GuiRectMesh;
}


VertexBuffer *RendererImpl::GetFullscreenPlaneVB()
{
	if (!m_FSPlaneVB)
	{
		m_FSPlaneVB = CreateVertexBuffer(0);

		static const Vertex::WT1::s v[4] =
		{
			{ {-1.0f,  1.0f, 0.5f, 1.0f}, {0, 1} },
			{ { 1.0f,  1.0f, 0.5f, 1.0f}, {1, 1} },
			{ {-1.0f, -1.0f, 0.5f, 1.0f}, {0, 0} },
			{ { 1.0f, -1.0f, 0.5f, 1.0f}, {1, 0} }
		};

		void *buf;
		if (m_FSPlaneVB->Lock(&buf, 4, Vertex::WT1::d, VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE) == VertexBuffer::RETURNCODE::RET_OK)
		{
			memcpy(buf, v, sizeof(Vertex::WT1::s) * 4);

			m_FSPlaneVB->Unlock();
		}
	}

	return m_FSPlaneVB;
}

Mesh *RendererImpl::GetFullScreenPlaneMesh()
{
	if (!m_FullScreenPlaneMesh)
	{
		m_FullScreenPlaneMesh = CreateMesh();

		m_FullScreenPlaneMesh->AttachVertexBuffer(GetFullscreenPlaneVB());

		if (m_FSPlaneVB)
		{
			IndexBuffer *pib = CreateIndexBuffer(0);
			if (pib)
			{
				static const uint16_t i[2][3] =
				{
					 {  0,  2,  1 }, {  0,  3, 2  }
				};

				void *buf = nullptr;
				if (pib->Lock((void **)&buf, 2 * 3, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
				{
					memcpy(buf, pib, sizeof(i));
					pib->Unlock();
				}

				m_FullScreenPlaneMesh->AttachIndexBuffer(pib);
			}
		}
	}

	return m_FullScreenPlaneMesh;
}

size_t hemisphereSectorCount[Renderer::EResLevel::RL_RESCOUNT] = {8, 16, 24};
size_t hemisphereStackCount[Renderer::EResLevel::RL_RESCOUNT] = {4, 8, 12};

VertexBuffer *RendererImpl::GetHemisphereVB(EResLevel lvl)
{
	if (!m_HemisphereVB[lvl])
	{
		typedef std::vector<Vertex::PNYT1::s> TVertexArray;
		TVertexArray verts;

		Vertex::PNYT1::s v;

		glm::fvec4 n(0, 0, 1, 0);

		v.pos.x = v.pos.y = v.norm.x = v.norm.y = 0.0f;
		v.uv.x = v.uv.y = 0.5f;
		v.pos.z = v.norm.z = 1.0f;

		verts.push_back(v);

		glm::fmat4x4 mz = (glm::fmat4x4)glm::angleAxis(C3_PI * 2.0f / (float)hemisphereSectorCount[lvl], glm::fvec3(0.0f, 0.0f, 1.0f));

		for (size_t i = 1, maxi = hemisphereStackCount[lvl] + 1; i < maxi; i++)
		{
			glm::fmat4x4 mx = (glm::fmat4x4)glm::angleAxis(C3_PI / 2.0f / (float)(hemisphereStackCount[lvl] + 1) * i, glm::fvec3(0.0f, 1.0f, 0.0f));
			n = glm::normalize(glm::fvec4(0, 0, 1, 0) * mx);

			for (size_t j = 0, maxj = hemisphereSectorCount[lvl] + 1; j < maxj; j++)
			{
				v.norm = v.pos = n;
				v.uv.x = (float)j;
				v.uv = glm::fvec2(n.x, n.y) * glm::fvec2(0.5f, 0.5f) + glm::fvec2(0.5f, 0.5f);

				verts.push_back(v);

				n = glm::normalize(n * mz);
			}

			v.uv.y = (float)i;
		}

		m_HemisphereVB[lvl] = CreateVertexBuffer(0);
		if (m_HemisphereVB[lvl])
		{
			Vertex::PNYT1::s *buf;
			if ((m_HemisphereVB[lvl]->Lock((void **)&buf, verts.size(), Vertex::PNYT1::d, VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE) == VertexBuffer::RETURNCODE::RET_OK) && buf)
			{
				for (const auto v : verts)
				{
					memcpy(buf, &v, sizeof(Vertex::PNYT1::s));
					buf++;
				}

				m_HemisphereVB[lvl]->Unlock();
			}
		}
	}

	return m_HemisphereVB[lvl];
}


Mesh *RendererImpl::GetHemisphereMesh(EResLevel lvl)
{
	if (!m_HemisphereMesh[lvl])
	{
		m_HemisphereMesh[lvl] = CreateMesh();

		m_HemisphereMesh[lvl]->AttachVertexBuffer(GetHemisphereVB(lvl));

		if (m_HemisphereVB[lvl])
		{
			IndexBuffer *pib = CreateIndexBuffer(0);
			if (pib)
			{
				uint16_t *buf;

				size_t ic_top = 3 * (hemisphereSectorCount[lvl] + 1), ic = ic_top + (ic_top * hemisphereStackCount[lvl] * 2);

				if (pib->Lock((void **)&buf, ic, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
				{
					// Do the top slice first because it has only one triangle per sector
					for (uint16_t i = 0, maxi = (uint16_t)(hemisphereSectorCount[lvl] + 1); i < maxi; i++)
					{
						*(buf++) = 0;
						*(buf++) = i;
						*(buf++) = i + 1;
					}

					uint16_t ss = 1;
					for (uint16_t i = 1, maxi = (uint16_t)(hemisphereStackCount[lvl]); i < maxi; i++)
					{
						for (uint16_t j = 0, maxj = (uint16_t)(hemisphereSectorCount[lvl] + 1); j < maxj; j++)
						{
							uint16_t a = ss;
							uint16_t b = a + (uint16_t)hemisphereSectorCount[lvl];

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

				m_HemisphereMesh[lvl]->AttachIndexBuffer(pib);
			}
		}
	}

	return m_HemisphereMesh[lvl];
}



Mesh *RendererImpl::GetSphereMesh(EResLevel lvl)
{
	if (!m_SphereMesh[lvl])
	{
		size_t sectors = hemisphereSectorCount[lvl];
		size_t stacks = hemisphereStackCount[lvl] * 2;

		typedef std::vector<Vertex::PNYT1::s> TVertexArray;
		typedef std::vector<uint16_t> TIndexArray;

		TVertexArray vertices;
		TIndexArray indices;

		float sectorStep = 2.0f * glm::pi<float>() / (float)sectors;
		float stackStep = glm::pi<float>() / (float)stacks;
		float radius = 1.0f;

		// Generate vertices
		for (size_t i = 0, max_i = stacks; i <= max_i; ++i)
		{
			float stackAngle = glm::pi<float>() / 2.0f - i * stackStep;
			float xy = radius * cosf(stackAngle);
			float z;
			if (!i)
				z = radius;
			else if (i == max_i)
				z = -radius;
			else
				z = radius * sinf(stackAngle);

			for (size_t j = 0, max_j = (!i || i == max_i) ? 0 : sectors; j <= max_j; ++j)
			{
				float sectorAngle = j * sectorStep;
				float x = xy * cosf(sectorAngle);
				float y = xy * sinf(sectorAngle);

				vertices.emplace_back();
				vertices.back().pos.x = x;
				vertices.back().pos.y = y;
				vertices.back().pos.z = z;
				glm::fvec3 n = glm::fvec3(x, y, z) / radius;
				vertices.back().norm = glm::normalize(n);
				glm::fvec3 t = glm::fvec3(-sinf(sectorAngle), cosf(sectorAngle), 0.0f);
				vertices.back().tang = glm::normalize(t);
				glm::fvec3 b = glm::cross(n, t);
				vertices.back().binorm = glm::normalize(b);
				vertices.back().uv.x = (float)j / (float)sectors;
				vertices.back().uv.y = (float)i / (float)stacks;
			}
		}

		// Generate indices
		for (size_t i = 1, max_i = (stacks - 1); i < max_i; ++i)
		{
			uint16_t k1 = (uint16_t)((i - 1) * (sectors + 1));
			uint16_t k2 = (uint16_t)(k1 + sectors + 1);

			for (size_t j = 0; j <= sectors; ++j, ++k1, ++k2)
			{
				indices.push_back(k1);
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
				indices.push_back(k2 + 1);
			}
		}

		for (size_t j = 0, k1 = 1, k2 = 2; j < sectors; ++j, ++k1, ++k2)
		{
			indices.push_back(0);
			indices.push_back((uint16_t)k2);
			indices.push_back((uint16_t)k1);
		}

		for (size_t j = 0, q = (uint16_t)vertices.size() - 1, k1 = (q - 1), k2 = (k1 - 1); j < sectors; ++j, --k1, --k2)
		{
			indices.push_back((uint16_t)q);
			indices.push_back((uint16_t)k2);
			indices.push_back((uint16_t)k1);
		}

		m_SphereMesh[lvl] = CreateMesh();

		VertexBuffer *pvb = CreateVertexBuffer(0);
		if (m_HemisphereVB[lvl])
		{
			Vertex::PNYT1::s *buf;
			if ((pvb->Lock((void **)&buf, vertices.size(), Vertex::PNYT1::d, VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE) == VertexBuffer::RETURNCODE::RET_OK) && buf)
			{
				for (const auto v : vertices)
					memcpy(buf++, &v, sizeof(Vertex::PNYT1::s));

				pvb->Unlock();
			}

			m_SphereMesh[lvl]->AttachVertexBuffer(pvb);
		}

		IndexBuffer *pib = CreateIndexBuffer(0);
		if (pib)
		{
			uint16_t *buf;
			if (pib->Lock((void **)&buf, indices.size(), IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
			{
				for (TIndexArray::const_iterator it = indices.cbegin(), last_it = indices.cend(); it != last_it; it++)
					*(buf++) = *it;

				pib->Unlock();
			}

			m_SphereMesh[lvl]->AttachIndexBuffer(pib);
		}
	}

	return m_SphereMesh[lvl];
}


Texture2D *RendererImpl::GetBlackTexture()
{
	if (!m_BlackTex)
	{
		m_BlackTex = CreateTexture2D(16, 16, TextureType::U8_4CH, 1, 0);
		if (m_BlackTex)
		{
			m_BlackTex->SetName(_T("black"));

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
			m_GreyTex->SetName(_T("grey"));

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


Texture2D *RendererImpl::GetDefaultDescTexture()
{
	if (!m_DefaultDescTex)
	{
		m_DefaultDescTex = CreateTexture2D(16, 16, TextureType::U8_4CH, 1, 0);
		if (m_DefaultDescTex)
		{
			m_DefaultDescTex->SetName(_T("default_desc"));

			uint32_t *buf;
			Texture2D::SLockInfo li;
			if ((m_DefaultDescTex->Lock((void **)&buf, li, 0, TEXLOCKFLAG_WRITE) == Texture2D::RETURNCODE::RET_OK) && buf)
			{
				for (size_t y = 0, maxy = li.height; y < maxy; y++)
				{
					for (size_t x = 0, maxx = li.width; x < maxx; x++)
					{
						buf[x] = 0x00A020FF;
					}

					buf = (uint32_t *)((BYTE *)buf + li.stride);
				}

				m_DefaultDescTex->Unlock();
			}
		}
	}

	return m_DefaultDescTex;
}


Texture2D *RendererImpl::GetDefaultNormalTexture()
{
	if (!m_DefaultNormalTex)
	{
		m_DefaultNormalTex = CreateTexture2D(16, 16, TextureType::U8_4CH, 1, 0);
		if (m_DefaultNormalTex)
		{
			m_DefaultNormalTex->SetName(_T("default_norm"));

			uint32_t *buf;
			Texture2D::SLockInfo li;
			if ((m_DefaultNormalTex->Lock((void **)&buf, li, 0, TEXLOCKFLAG_WRITE) == Texture2D::RETURNCODE::RET_OK) && buf)
			{
				for (size_t y = 0, maxy = li.height; y < maxy; y++)
				{
					for (size_t x = 0, maxx = li.width; x < maxx; x++)
					{
						buf[x] = 0xFFFF8080;
					}

					buf = (uint32_t *)((BYTE *)buf + li.stride);
				}

				m_DefaultNormalTex->Unlock();
			}
		}
	}

	return m_DefaultNormalTex;
}


Texture2D *RendererImpl::GetWhiteTexture()
{
	if (!m_WhiteTex)
	{
		m_WhiteTex = CreateTexture2D(16, 16, TextureType::U8_4CH, 1, 0);
		if (m_WhiteTex)
		{
			m_WhiteTex->SetName(_T("white"));

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
			m_BlueTex->SetName(_T("blue"));

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
		m_GridTex = CreateTexture2D(16, 16, TextureType::U8_4CH, 0, 0);
		if (m_GridTex)
		{
			m_GridTex->SetName(_T("grid"));

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


Texture2D *RendererImpl::GetLinearGradientTexture()
{
	if (!m_LinearGradientTex)
	{
		m_LinearGradientTex = CreateTexture2D(256, 1, TextureType::U8_4CH, 1, 0);
		if (m_LinearGradientTex)
		{
			m_LinearGradientTex->SetName(_T("linear_gradient"));

			uint32_t *buf;
			Texture2D::SLockInfo li;
			if ((m_LinearGradientTex->Lock((void **)&buf, li, 0, TEXLOCKFLAG_WRITE) == Texture2D::RETURNCODE::RET_OK) && buf)
			{
				for (size_t x = 0, maxx = li.width; x < maxx; x++)
				{
					uint32_t v = (uint32_t)(x | (x << 8) | (x << 16) | (x << 24));
					buf[x] = v;
				}

				m_LinearGradientTex->Unlock();
			}
		}
	}

	return m_LinearGradientTex;
}


Texture2D *RendererImpl::GetOrthoRefTexture()
{
	if (!m_OrthoRefTex)
	{
		HRSRC hres = ::FindResource(g_C3Mod, MAKEINTRESOURCE(IDB_PNG_ORTHOREF), L"PNG");
		if (!hres)
			return GetBlackTexture();

		HGLOBAL hglob = LoadResource(g_C3Mod, hres);
		if (!hglob)
			return GetBlackTexture();

		LPVOID buf = ::LockResource(hglob);
		if (!buf)
		{
			FreeResource(hglob);
			return GetBlackTexture();
		}

		int numchans = 0;
		int imgw = 0, imgh = 0;
		stbi_uc *pimg = stbi_load_from_memory((const stbi_uc *)buf, ::SizeofResource(g_C3Mod, hres), &imgw, &imgh, &numchans, 0);

		ETextureType tt;
		switch (numchans)
		{
			case 1: tt = TextureType::U8_1CH; break;
			case 2: tt = TextureType::U8_2CH; break;
			case 3: tt = TextureType::U8_3CH; break;
			case 4: tt = TextureType::U8_4CH; break;
		}

		m_OrthoRefTex = CreateTexture2D(imgw, imgh, tt, 1, 0);
		if (m_OrthoRefTex)
		{
			m_OrthoRefTex->SetName(_T("OrthoReference"));

			uint8_t *bufd, *bufs = pimg;
			Texture2D::SLockInfo li;
			if ((m_OrthoRefTex->Lock((void **)&bufd, li, 0, TEXLOCKFLAG_WRITE) == Texture2D::RETURNCODE::RET_OK) && bufd)
			{
				for (size_t y = 0; y < li.height; y++)
				{
					memcpy(bufd, bufs, imgw * numchans);
					bufd += li.stride;
					bufs += imgw * numchans;
				}

				m_OrthoRefTex->Unlock();
			}
		}

		free(pimg);
		UnlockResource(hglob);
		FreeResource(hglob);
	}

	return m_OrthoRefTex;
}


#define UTILITY_COLORS_H		16
#define UTILITY_COLORS_W		16
Texture2D *RendererImpl::GetUtilityColorTexture()
{
	if (!m_UtilityColorTex)
	{
		m_UtilityColorTex = CreateTexture2D(UTILITY_COLORS_W, UTILITY_COLORS_H, TextureType::U8_4CH, 1, 0);
		if (m_UtilityColorTex)
		{
			uint32_t *buf;
			Texture2D::SLockInfo li;
			if ((m_UtilityColorTex->Lock((void **)&buf, li, 0, TEXLOCKFLAG_WRITE) == Texture2D::RETURNCODE::RET_OK) && buf)
			{
				size_t idx = 0;

				for (size_t y = 0; y < UTILITY_COLORS_H / 4; y++)
				{
					for (size_t x = 0; x < UTILITY_COLORS_W; x++)
						buf[idx++] = *(uint32_t *)&c3::Color::iRed;
				}

				for (size_t y = 0; y < UTILITY_COLORS_H / 4; y++)
				{
					for (size_t x = 0; x < UTILITY_COLORS_W; x++)
						buf[idx++] = *(uint32_t *)&c3::Color::iGreen;
				}

				for (size_t y = 0; y < UTILITY_COLORS_H / 4; y++)
				{
					for (size_t x = 0; x < UTILITY_COLORS_W; x++)
						buf[idx++] = *(uint32_t *)&c3::Color::iBlue;
				}

				for (size_t y = 0; y < UTILITY_COLORS_H / 4; y++)
				{
					for (size_t x = 0; x < UTILITY_COLORS_W; x++)
						buf[idx++] = *(uint32_t *)&c3::Color::iMagenta;
				}

				m_UtilityColorTex->Unlock();
			}
		}
	}

	return m_UtilityColorTex;
}


#define SPRITE_SPHERE_DIM		64

Texture2D *RendererImpl::GetSphereSpriteTexture()
{
	if (!m_SphereSpriteTex)
	{
		m_SphereSpriteTex = CreateTexture2D(64, 64, TextureType::U8_4CH, 0, 0);
		if (m_SphereSpriteTex)
		{
			uint32_t *buf;
			Texture2D::SLockInfo li;
			if ((m_SphereSpriteTex->Lock((void **)&buf, li, 0, TEXLOCKFLAG_WRITE | TEXLOCKFLAG_GENMIPS) == Texture2D::RETURNCODE::RET_OK) && buf)
			{
				float a = glm::pi<float>() / (float)SPRITE_SPHERE_DIM;

				float ry = 0.0f;
				for (size_t y = 0, maxy = li.height; y < maxy; y++)
				{
					float rx = 0.0f;
					for (size_t x = 0, maxx = li.width; x < maxx; x++)
					{
						float sx = sin(rx);
						float sy = sin(ry);
						uint8_t tmp = (uint8_t)(sx * sy * 255.0f);

						buf[x] = tmp | (tmp << 8) | (tmp << 16);
						if (fabs(cos(rx) * cos(sy)) > 0.707)
							buf[x] |= 0xFF000000;

						rx += a;
					}

					buf = (uint32_t *)((BYTE *)buf + li.stride);
					ry += a;
				}

				m_SphereSpriteTex->Unlock();
			}
		}
	}

	return m_SphereSpriteTex;
}


Texture2D *RendererImpl::GetSphereSpriteNormalTexture()
{
	if (!m_SphereSpriteNormalTex)
	{
		m_SphereSpriteNormalTex = CreateTexture2D(SPRITE_SPHERE_DIM, SPRITE_SPHERE_DIM, TextureType::U8_4CH, 0, 0);
		if (m_SphereSpriteNormalTex)
		{
			uint32_t *buf;
			Texture2D::SLockInfo li;
			if ((m_SphereSpriteNormalTex->Lock((void **)&buf, li, 0, TEXLOCKFLAG_WRITE | TEXLOCKFLAG_GENMIPS) == Texture2D::RETURNCODE::RET_OK) && buf)
			{
				float a = glm::pi<float>() / (float)SPRITE_SPHERE_DIM;

				float ry = glm::pi<float>() / -2.0f;
				for (size_t y = 0, maxy = li.height; y < maxy; y++)
				{
					float rx = glm::pi<float>() / -2.0f;
					for (size_t x = 0, maxx = li.width; x < maxx; x++)
					{
						float sx = sin(rx);
						float sy = sin(ry);
						glm::fvec3 r = glm::normalize(glm::fvec3(sx, sy, 1.0));
						if (fabs(sx * sy) > 0.707f)
							buf[x] = ((uint8_t)(r.x * 127.0f) + 128) | (((uint8_t)(r.y * 127.0f) + 128) << 8) | (((uint8_t)(r.y * 127.0f) + 128) << 16);
						else
							buf[x] = 0;

						rx += a;
					}

					buf = (uint32_t *)((BYTE *)buf + li.stride);
					ry += a;
				}

				m_SphereSpriteNormalTex->Unlock();
			}
		}
	}

	return m_SphereSpriteNormalTex;
}


MaterialManager *RendererImpl::GetMaterialManager()
{
	if (!m_MatMan)
		m_MatMan = new MaterialManagerImpl(this);

	return m_MatMan;
}


const Material *RendererImpl::GetWhiteMaterial()
{
	if (!m_mtlWhite)
	{
		m_mtlWhite = GetMaterialManager()->CreateMaterial();

		if (m_mtlWhite)
		{
			m_mtlWhite->SetColor(Material::ColorComponentType::CCT_DIFFUSE, &Color::fWhite);
			m_mtlWhite->SetWindingOrder(Renderer::WindingOrder::WO_CCW);
			m_mtlWhite->RenderModeFlags().Set(Material::RENDERMODEFLAG(Material::RMF_RENDERFRONT));
		}
	}

	return m_mtlWhite;
}


const Material *RendererImpl::GetBlackMaterial()
{
	if (!m_mtlBlack)
	{
		m_mtlBlack = GetMaterialManager()->CreateMaterial();

		if (m_mtlBlack)
		{
			m_mtlBlack->SetColor(Material::ColorComponentType::CCT_DIFFUSE, &Color::fBlack);
			m_mtlBlack->SetWindingOrder(Renderer::WindingOrder::WO_CCW);
			m_mtlBlack->RenderModeFlags().Set(Material::RENDERMODEFLAG(Material::RMF_RENDERFRONT));
		}
	}

	return m_mtlBlack;
}


ShaderProgram *RendererImpl::GetBoundsShader()
{
	if (!m_spBounds)
	{
		m_spBounds = CreateShaderProgram();

		if (m_spBounds)
		{
			m_vsBounds = (c3::ShaderComponent *)((m_pSys->GetResourceManager()->GetResource(_T("bounds.vsh"), RESF_DEMANDLOAD))->GetData());
			m_fsBounds = (c3::ShaderComponent *)((m_pSys->GetResourceManager()->GetResource(_T("bounds.fsh"), RESF_DEMANDLOAD))->GetData());

			m_spBounds->AttachShader(m_vsBounds);
			m_spBounds->AttachShader(m_fsBounds);
			m_spBounds->Link();
		}
	}

	return m_spBounds;
}


Font *RendererImpl::GetFont(const TCHAR *name, size_t size)
{
	uint32_t key = Crc32::CalculateString(name);
	key = Crc32::Calculate((const uint8_t *)&size, sizeof(size_t), key);

	FontMap::iterator it = m_FontMap.find(key);
	if (it == m_FontMap.end())
	{
		Font *f = new FontImpl(this, name, size);
		if (f)
			((FontImpl *)f)->Initialize();

		std::pair<FontMap::iterator, bool> insret = m_FontMap.insert(FontMap::value_type(key, f));
		it = insret.first;
	}

	return (it != m_FontMap.end()) ? it->second : nullptr;
}
