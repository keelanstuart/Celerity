
// C3App.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "C3App.h"
#include "C3Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// C3App

BEGIN_MESSAGE_MAP(C3App, CWinApp)
END_MESSAGE_MAP()

// C3App construction

C3App::C3App()
{
	m_C3 = nullptr;
}


// The one and only C3App object

C3App theApp;



DECLARE_FEATURETYPE(UIControl, UIControl);


UIControl::UIControl()
{
	m_Owner = nullptr;
	m_FS = m_VS[0] = m_VS[1] = nullptr;
	m_SP[0] = m_SP[1] = nullptr;
	m_M = nullptr;
	m_MVP = -1;
	m_Tex = nullptr;
	m_TexRes = nullptr;
}

UIControl::~UIControl()
{
	if (m_M)
	{
		//m_M->Release();
		m_M = nullptr;
	}

	for (int i = 0; i < 2; i++)
	{
		if (m_SP[i])
		{
			m_SP[i]->Release();
			m_SP[i] = nullptr;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		if (m_VS[i])
		{
			m_VS[i]->Release();
			m_VS[i] = nullptr;
		}
	}

	if (m_FS)
	{
		m_FS->Release();
		m_FS = nullptr;
	}

	if (m_TexRes)
	{
		m_TexRes->DelRef();
		m_TexRes = nullptr;
	}
}

void UIControl::Release()
{
	delete this;
}

props::TFlags64 UIControl::Flags()
{
	return props::TFlags64(0);
}

void UIControl::Update(c3::Object *powner, float elapsed_time)
{
}

bool UIControl::Initialize(c3::Object *powner)
{
	m_Owner = powner;
	c3::Renderer *prend = powner->GetSystem()->GetRenderer();

	m_VS[0] = prend->CreateShaderComponent(c3::Renderer::ShaderComponentType::ST_VERTEX);
	if (m_VS[0])
	{
		static const TCHAR *vertex_shader_text = _T(
			/*"#version 410\n"*/
			"uniform mat4 MVP;\n"
			"layout (location=0) in vec3 vPos;\n"
			"layout (location=1) in vec3 vNorm;\n"
			"layout (location=2) in vec2 vTex0;\n"
			"out vec3 fNorm;"
			"out vec4 fColor;\n"
			"out vec2 fTex0;\n"
			"void main()\n"
			"{\n"
			"    gl_Position = MVP * vec4(vPos, 1.0);\n"
			"    vec4 n = MVP * vec4(vNorm, 0.0);\n"
			"    fNorm = n.xyz;\n"
			"    fColor = vec4((n.xyz + vec3(1, 1, 1)) * vec3(0.4, 0.4, 0.4), 1.0);\n"
			"    fTex0 = vTex0;\n"
			"}\n");

		m_VS[0]->CompileProgram(vertex_shader_text);
	}

	m_VS[1] = prend->CreateShaderComponent(c3::Renderer::ShaderComponentType::ST_VERTEX);
	if (m_VS[1])
	{
		static const TCHAR *vertex_shader_text = _T(
			/*"#version 410\n"*/
			"uniform mat4 MVP;\n"
			"layout (location=0) in vec3 vPos;\n"
			"layout (location=1) in vec3 vNorm;\n"
			"layout (location=2) in vec3 vTan;\n"
			"layout (location=3) in vec3 vBinorm;\n"
			"layout (location=4) in vec2 vTex0;\n"
			"out vec3 fNorm;\n"
			"out vec4 fColor;\n"
			"out vec2 fTex0;\n"
			"void main()\n"
			"{\n"
			"    gl_Position = MVP * vec4(vPos, 1.0);\n"
			"    vec4 n = MVP * vec4(vNorm, 0.0);\n"
			"    fNorm = n.xyz;\n"
			"    fTex0 = vTex0;\n"
			"    fColor = vec4((n.xyz + vec3(1, 1, 1)) * vec3(0.4, 0.4, 0.4), 1.0);\n"
			"}\n");

		m_VS[1]->CompileProgram(vertex_shader_text);
	}

	m_FS = prend->CreateShaderComponent(c3::Renderer::ShaderComponentType::ST_FRAGMENT);
	if (m_FS)
	{
		static const TCHAR *fragment_shader_text = _T(
			/*"#version 410\n"*/
			"in vec3 fNorm;\n"
			"in vec4 fColor;\n"
			"in vec2 fTex0;\n"
			"uniform sampler2D TEX0;\n"
			"layout (location=0) out vec4 fragment;\n"
			//"layout (location=1) out vec4 fragment_norm;\n"
			"void main()\n"
			"{\n"
			"    fragment = texture(TEX0, fTex0 * 25.0);\n"//vec2(2.0f, 2.0f));\n"
			"    if (fragment.a < 0.1) discard;\n"
			//			"    fragment_norm = vec4(fNorm, 1.0);\n"
			"}\n");

		m_FS->CompileProgram(fragment_shader_text);
	}

	m_Tex = prend->GetGridTexture();
	m_TexRes = powner->GetSystem()->GetResourceManager()->GetResource(_T("C:\\Proj\\Game Data\\Textures\\drg_ground_stones.png"));

#if 0
	m_ModRes = powner->GetSystem()->GetResourceManager()->GetResource(_T("D:/proj/three.js/examples/models/fbx/stanford-bunny.fbx"));
#else
	m_ModRes = powner->GetSystem()->GetResourceManager()->GetResource(_T("c:\\proj\\Celerity\\third-party\\assimp\\test\\models\\OBJ\\box.obj"));
#endif


	m_SP[0] = prend->CreateShaderProgram();
	if (m_SP[0])
	{
		m_SP[0]->AttachShader(m_VS[0]);
		m_SP[0]->AttachShader(m_FS);

		if (m_SP[0]->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
		{
			m_MVP = m_SP[0]->GetUniformLocation(_T("MVP"));
			m_TEX0 = m_SP[0]->GetUniformLocation(_T("TEX0"));
		}
	}

	m_SP[1] = prend->CreateShaderProgram();
	if (m_SP[1])
	{
		m_SP[1]->AttachShader(m_VS[1]);
		m_SP[1]->AttachShader(m_FS);

		if (m_SP[1]->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
		{
			m_MVP = m_SP[1]->GetUniformLocation(_T("MVP"));
			m_TEX0 = m_SP[1]->GetUniformLocation(_T("TEX0"));
		}
	}

#if 1
	//m_M = prend->GetHemisphereMesh();
	m_M = prend->GetCubeMesh();
#else
	m_M = (c3::Mesh *)(m_ModRes->GetData());
#endif

	return true;
}

bool UIControl::Prerender(c3::Object *powner, props::TFlags64 rendflags)
{
	return true;
}

void UIControl::Render(c3::Object *powner, props::TFlags64 rendflags)
{
	if (m_M && m_SP)
	{
		c3::Renderer *prend = powner->GetSystem()->GetRenderer();
		prend->UseProgram(m_SP[1]);
		prend->UseTexture(0, prend->GetBlueTexture());

		m_SP[1]->SetUniformMatrix(m_MVP, prend->GetWorldViewProjectionMatrix());
		m_SP[1]->SetUniformTexture(m_TEX0, 0, (!m_TexRes || (m_TexRes->GetStatus() != c3::Resource::Status::RS_LOADED)) ? m_Tex : (c3::Texture2D *)(m_TexRes->GetData()));

		prend->SetCullMode(c3::Renderer::CM_DISABLED);

		m_M->Draw(c3::Renderer::PrimType::TRILIST);

//		if (m_ModRes && (m_ModRes->GetStatus() == c3::Resource::RS_LOADED) && m_ModRes->GetData())
			//((c3::Model *)(m_ModRes->GetData()))->Draw();

//		prend->UseProgram(m_SP[0]);
//		m_SP[0]->SetUniformMatrix(m_MVP, prend->GetWorldViewProjectionMatrix());
//		m_SP[0]->SetUniformTexture(m_TEX0, 0, (!m_TexRes || (m_TexRes->GetStatus() != c3::Resource::Status::RS_LOADED)) ? m_Tex : (c3::Texture2D *)(m_TexRes->GetData()));
	}
}


// C3App initialization

BOOL C3App::InitInstance()
{
#if 0
	MessageBox(NULL, L"attach graphics debugger", L"debug", MB_OK);
#endif

	CWinApp::InitInstance();

	m_C3 = c3::System::Create(0);
	if (!m_C3)
		return FALSE;

	c3::Factory *pfactory = m_C3->GetFactory();

	REGISTER_FEATURETYPE(UIControl, pfactory);

	c3::Prototype *ppcam = pfactory->CreatePrototype();
	ppcam->AddFeature(pfactory->FindFeatureType(_T("Positionable")));
	ppcam->AddFeature(pfactory->FindFeatureType(_T("Camera")));
	ppcam->SetName(_T("Camera"));

	c3::Prototype *ppctl = pfactory->CreatePrototype();
	ppctl->AddFeature(pfactory->FindFeatureType(_T("UIControl")));
	ppctl->AddFeature(pfactory->FindFeatureType(_T("Positionable")));
		ppctl->SetName(_T("GenericControl"));

	// Standard initialization

	SetRegistryKey(_T("Celerity"));

	C3Dlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}



int C3App::ExitInstance()
{
	c3::Factory *pfactory = m_C3->GetFactory();

	UNREGISTER_FEATURETYPE(UIControl, pfactory);

	if (m_C3)
	{
		m_C3->Release();
		m_C3 = nullptr;
	}

	return CWinApp::ExitInstance();
}
