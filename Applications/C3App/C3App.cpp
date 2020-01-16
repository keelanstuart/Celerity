
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



DECLARE_COMPORTMENTTYPE(UIControl, UIControl);


UIControl::UIControl()
{
	m_Owner = nullptr;
	m_FS = m_VS = nullptr;
	m_SP = nullptr;
	m_VB = nullptr;
	m_IB = nullptr;
	m_M = nullptr;
	m_MVP = -1;
}

UIControl::~UIControl()
{
	if (m_M)
	{
		//m_M->Release();
		m_M = nullptr;
	}

	if (m_VB)
	{
		m_VB->Release();
		m_VB = nullptr;
	}

	if (m_IB)
	{
		m_IB->Release();
		m_IB = nullptr;
	}

	if (m_SP)
	{
		m_SP->Release();
		m_SP = nullptr;
	}

	if (m_VS)
	{
		m_VS->Release();
		m_VS = nullptr;
	}

	if (m_FS)
	{
		m_FS->Release();
		m_FS = nullptr;
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

	m_VS = prend->CreateShaderComponent(c3::Renderer::ShaderComponentType::ST_VERTEX);
	if (m_VS)
	{
		static const TCHAR *vertex_shader_text = _T(
			"#version 410\n"
			"uniform mat4 MVP;\n"
			"layout (location=0) in vec3 vPos;\n"
			"layout (location=1) in vec3 vNorm;\n"
			"out vec3 fNorm;"
			"out vec4 fColor;\n"
			"void main()\n"
			"{\n"
			"    gl_Position = MVP * vec4(vPos, 1.0);\n"
			"    vec4 n = MVP * vec4(vNorm, 0.0);\n"
			"    fNorm = n.xyz;\n"
			"    fColor = vec4(n.x, 1.0, n.z, 1.0);\n"
			"}\n" );

		m_VS->CompileProgram(vertex_shader_text);
	}

	m_FS = prend->CreateShaderComponent(c3::Renderer::ShaderComponentType::ST_FRAGMENT);
	if (m_FS)
	{
		static const TCHAR *fragment_shader_text = _T(
			"#version 410\n"
			"in vec3 fNorm;\n"
			"in vec4 fColor;\n"
			"layout (location=0) out vec4 fragment;\n"
			//"layout (location=1) out vec4 fragment_norm;\n"
			"void main()\n"
			"{\n"
			"    fragment = vec4(fColor);\n"
//			"    fragment_norm = vec4(fNorm, 1.0);\n"
			"}\n" );

		m_FS->CompileProgram(fragment_shader_text);
	}

	if (m_VS && m_FS)
	m_SP = prend->CreateShaderProgram();
	if (m_SP)
	{
		m_SP->AttachShader(m_VS);
		m_SP->AttachShader(m_FS);

		if (m_SP->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
		{
			m_MVP = m_SP->GetUniformLocation(_T("MVP"));
		}
	}

	m_VB = prend->CreateVertexBuffer();
	if (m_VB)
	{
		c3::VertexBuffer::ComponentDescription comps[4] ={
			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_POSITION},
			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_COLOR0},

			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_NONE, 0, c3::VertexBuffer::ComponentDescription::Usage::VU_NONE} // terminator
		};

		typedef struct
		{
			glm::vec3 pos;
			glm::vec3 col;
		} SSimpleVert;

		static const SSimpleVert v[3] =
		{
			{ { -0.6f, -0.4f, 0.0f }, { 1.f, 0.f, 0.f } },
			{ {  0.6f, -0.4f, 0.0f }, { 0.f, 1.f, 0.f } },
			{ {  0.0f,  0.6f, 0.0f }, { 0.f, 0.f, 1.f } }
		};

		void *buf;
		if (m_VB->Lock(&buf, 3, comps, VBLOCKFLAG_WRITE) == c3::VertexBuffer::RETURNCODE::RET_OK)
		{
			memcpy(buf, v, sizeof(SSimpleVert) * 3);

			m_VB->Unlock();
		}
	}

#if 1
	m_IB = prend->CreateIndexBuffer();
	if (m_IB)
	{
		int16_t i[1][3] = { {0, 1, 2} };

		void *buf;
		if (m_IB->Lock(&buf, 3, c3::IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE) == c3::IndexBuffer::RETURNCODE::RET_OK)
		{
			memcpy(buf, i, 3 * sizeof(int16_t));

			m_IB->Unlock();
		}
	}
#endif

	m_M = prend->GetBoundsMesh(); //CreateMesh();
	if (m_M)
	{
#if 0
		m_M->AttachVertexBuffer(m_VB);
		m_M->AttachIndexBuffer(m_IB);
#endif
	}

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
		prend->UseProgram(m_SP);

		m_SP->SetUniformMatrix(m_MVP, prend->GetWorldViewProjectionMatrix());

		m_M->Draw(c3::Renderer::PrimType::LINELIST);
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

	REGISTER_COMPORTMENTTYPE(UIControl, pfactory);

	c3::Prototype *ppcam = pfactory->CreatePrototype();
	ppcam->AddComportment(pfactory->FindComportmentType(_T("Positionable")));
	ppcam->AddComportment(pfactory->FindComportmentType(_T("Camera")));
	ppcam->SetName(_T("Camera"));

	c3::Prototype *ppctl = pfactory->CreatePrototype();
	ppctl->AddComportment(pfactory->FindComportmentType(_T("Positionable")));
	ppctl->AddComportment(pfactory->FindComportmentType(_T("UIControl")));
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

	UNREGISTER_COMPORTMENTTYPE(UIControl, pfactory);

	if (m_C3)
	{
		m_C3->Release();
		m_C3 = nullptr;
	}

	return CWinApp::ExitInstance();
}
