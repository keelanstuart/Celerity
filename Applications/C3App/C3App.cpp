
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
	m_FS = m_VS = nullptr;
	m_SP = nullptr;
	m_M = nullptr;
	m_iMatMVP = -1;
	m_iMatN = -1;
	m_TexRes = nullptr;
}

UIControl::~UIControl()
{
	if (m_M)
	{
		//m_M->Release();
		m_M = nullptr;
	}

	if (m_SP)
	{
		m_SP->Release();
		m_SP = nullptr;
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
	c3::ResourceManager *rm = powner->GetSystem()->GetResourceManager();

	props::TFlags64 rf = c3::ResourceManager::RESFLAG(c3::ResourceManager::DEMANDLOAD);
	m_VS = (c3::ShaderComponent *)((rm->GetResource(_T("d:/proj/game data/shaders/std.vsh"), rf))->GetData());
	m_FS = (c3::ShaderComponent *)((rm->GetResource(_T("d:/proj/game data/shaders/std.fsh"), rf))->GetData());

	m_TexRes = rm->GetResource(_T("D:\\IAC\\Proj\\Game Data\\Textures\\GrassObj.tga"));
	m_NormRes = rm->GetResource(_T("D:\\Proj\\C3\\Applications\\C3App\\normal.png"));

#if 0
	m_ModRes = powner->GetSystem()->GetResourceManager()->GetResource(_T("D:/proj/three.js/examples/models/fbx/stanford-bunny.fbx"));
#else
	m_ModRes = powner->GetSystem()->GetResourceManager()->GetResource(_T("D:\\proj\\C3\\third-party\\assimp\\test\\models\\OBJ\\box.obj"));
#endif

	c3::MaterialManager *pmm = prend->GetMaterialManager();
	m_Mtl = pmm->CreateMaterial();
	m_Mtl->SetColor(c3::Material::CCT_DIFFUSE, &c3::Color::White);
	m_Mtl->SetTexture(c3::Material::TCT_DIFFUSE, prend->GetGridTexture());

	m_SP = prend->CreateShaderProgram();
	if (m_SP)
	{
		m_SP->AttachShader(m_VS);
		m_SP->AttachShader(m_FS);

		if (m_SP->Link() == c3::ShaderProgram::RETURNCODE::RET_OK)
		{
			m_iMatMVP = m_SP->GetUniformLocation(_T("MatMVP"));
			m_iMatN = m_SP->GetUniformLocation(_T("MatN"));
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
	if (m_TexRes)
	{
		if (m_TexRes->GetStatus() == c3::Resource::Status::RS_LOADED)
			m_Mtl->SetTexture(c3::Material::TCT_DIFFUSE, (c3::Texture2D *)(m_TexRes->GetData()));
	}

	if (m_NormRes)
	{
		if (m_NormRes->GetStatus() == c3::Resource::Status::RS_LOADED)
			m_Mtl->SetTexture(c3::Material::TCT_NORMAL, (c3::Texture2D *)(m_NormRes->GetData()));
	}

	return true;
}

void UIControl::Render(c3::Object *powner, props::TFlags64 rendflags)
{
	if (m_M && m_SP)
	{
		c3::Renderer *prend = powner->GetSystem()->GetRenderer();
		prend->UseProgram(m_SP);

		m_Mtl->Apply(m_SP);
		m_SP->SetUniformMatrix(m_iMatMVP, prend->GetWorldViewProjectionMatrix());
		m_SP->SetUniformMatrix(m_iMatN, prend->GetNormalMatrix());

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
