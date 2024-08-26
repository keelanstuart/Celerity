// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include <C3AppCommon.h>
#include <array>
#include <renderdoc_app.h>

#pragma once

#define BLURTARGS	4


class CDeferredPipeline
{

public:

	CDeferredPipeline(c3::System *psys, c3::Configuration *pconfig, CWnd *pwnd, bool editor);

	virtual ~CDeferredPipeline();

	void OnDraw(CDC* pDC, c3::Object *root, TObjectArray *selections, c3::Object *camera);

	void InitializeGraphics();
	bool Initialized();

	void CreateSurfaces();

	void DestroySurfaces();

	void UpdateShaderSurfaces();

	bool CapturesAvailable();
	void TriggerCapture();

	void ShowDebug(bool show);
	bool DebugShowing();

protected:
	CWnd *m_pWnd;
	c3::System *m_pSys;
	c3::Configuration *m_pConfig;
	bool m_bInitialized;
	c3::Renderer::LOG_FUNC m_OldLogFunc;

	float m_WindowsUIScale;

	bool m_ShowDebug;

	c3::FrameBuffer *m_GBuf;
	c3::FrameBuffer *m_LCBuf;
	c3::FrameBuffer *m_AuxBuf;
	c3::FrameBuffer *m_SSBuf;
	c3::DepthBuffer *m_DepthTarg;
	c3::DepthBuffer *m_ShadowTarg;
	std::array<c3::Texture2D *, BLURTARGS> m_BTex;
	std::array<c3::FrameBuffer *, BLURTARGS> m_BBuf;
	c3::ShaderComponent *m_VS_resolve;
	c3::ShaderComponent *m_FS_resolve;
	c3::ShaderProgram *m_SP_resolve;
	c3::ShaderComponent *m_VS_blur;
	c3::ShaderComponent *m_FS_blur;
	c3::ShaderProgram *m_SP_blur;
	c3::ShaderComponent *m_VS_combine;
	c3::ShaderComponent *m_FS_combine;
	c3::ShaderProgram *m_SP_combine;
	c3::ShaderComponent *m_VS_bounds;
	c3::ShaderComponent *m_FS_bounds;
	c3::ShaderProgram *m_SP_bounds;

	int32_t m_ulSunDir;
	int32_t m_ulSunColor;
	int32_t m_ulAmbientColor;

	int32_t m_uBlurTex;
	int32_t m_uBlurScale;

	RENDERDOC_API_1_4_0 *m_pRenderDoc;
	bool m_CaptureFrame;

	bool m_bSurfacesCreated, m_bSurfacesReady;

};
