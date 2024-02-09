// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3GuiImpl.h>
#include <C3MaterialManagerImpl.h>

#include <gl_wrapper.h>

namespace c3
{

	class SystemImpl;

	#define MATRIXUPDATE_WORLDVIEWPROJ		0x01
	#define MATRIXUPDATE_WORLDVIEW			0x02
	#define MATRIXUPDATE_VIEWPROJ			0x04
	#define MATRIXUPDATE_NORMAL				0x08

	#define MATRIXUPDATE_WORLD				(MATRIXUPDATE_WORLDVIEWPROJ | MATRIXUPDATE_WORLDVIEW | MATRIXUPDATE_NORMAL)
	#define MATRIXUPDATE_VIEW				(MATRIXUPDATE_WORLDVIEWPROJ | MATRIXUPDATE_VIEWPROJ | MATRIXUPDATE_NORMAL)
	#define MATRIXUPDATE_PROJ				(MATRIXUPDATE_WORLDVIEWPROJ | MATRIXUPDATE_VIEWPROJ)
	#define MATRIXUPDATE_ALL				(MATRIXUPDATE_WORLD | MATRIXUPDATE_VIEW | MATRIXUPDATE_PROJ)


	class RendererImpl : public Renderer
	{

	protected:
		SystemImpl *m_pSys;
		HANDLE m_event_shutdown;

		tstring m_DeviceName;
		tstring m_VendorName;

		HWND m_hwnd;
		HDC m_hdc;
		HGLRC m_glrc;
		HWND m_hwnd_override;
		RECT m_Viewport;
		float m_LastPresentTime;

		props::TFlags64 m_BeginSceneFlags;

		size_t m_FrameNum;

		GLuint m_glVersionMaj;
		GLuint m_glVersionMin;

		bool m_needsFinish;

		// There's no extra thread here... the "pool" just contains a list of tasks that we're going to execute on this thread when we can
		// NOTE: there are two pools for double-bufferring so that if there are resource dependencies (e.g., textures loaded by models)
		// they can keep render blocking to a minimum
		pool::IThreadPool *m_TaskPool[2];
		std::atomic<size_t> m_ActiveTaskPool;

		glm::fmat4x4 m_ident;

		glm::fvec3 m_eyepos, m_eyedir;
		glm::fmat4x4 m_proj, m_view, m_world, m_worldview, m_normal, m_viewproj, m_worldviewproj, m_sunshadow, m_texturetransform;
		props::TFlags32 m_matupflags;

		glm::fvec4 m_clearColor;
		float m_clearZ;
		uint8_t m_clearStencil;

		bool m_Initialized;
		WNDCLASS m_glARBWndClass;

		FrameBuffer *m_CurFB;
		GLuint m_CurFBID;

		ShaderProgram *m_CurProg;
		GLuint m_CurProgID;

		ShaderComponent *m_vsBounds;
		ShaderComponent *m_fsBounds;
		ShaderProgram *m_spBounds;

		VertexBuffer *m_CurVB;
		GLuint m_CurVBID;

		IndexBuffer *m_CurIB;
		GLuint m_CurIBID;

		GLuint m_VAOglID;

		bool m_Config;

		GuiImpl *m_Gui;

		DepthMode m_DepthMode;
		Test m_DepthTest;

		WindingOrder m_WindingOrder;
		CullMode m_CullMode;
		BlendMode m_BlendMode;
		BlendEquation m_BlendEq;
		FillMode m_FillMode;

		float m_AlphaPassMin, m_AlphaPassMax;
		bool m_StencilEnabled;
		Test m_StencilTest;
		uint8_t m_StencilRef, m_StencilMask;
		StencilOperation m_StencilFailOp, m_StencilZFailOp, m_StencilZPassOp;

		float m_AlphaCoverage;
		bool m_AlphaCoverageInv;

		VertexBuffer *m_CubeVB;
		VertexBuffer *m_RefCubeVB;
		Mesh *m_BoundsMesh;
		Mesh *m_CubeMesh;
		Mesh *m_RefCubeMesh;

		VertexBuffer *m_FSPlaneVB;
		VertexBuffer *m_PlanesVB;
		Mesh *m_XYPlaneMesh;
		Mesh *m_YZPlaneMesh;
		Mesh *m_XZPlaneMesh;

		VertexBuffer *m_HemisphereVB;
		Mesh *m_HemisphereMesh;

		Mesh *m_SphereMesh;

		Texture2D *m_BlackTex;
		Texture2D *m_GreyTex;
		Texture2D *m_DefaultDescTex;
		Texture2D *m_DefaultNormalTex;
		Texture2D *m_WhiteTex;
		Texture2D *m_BlueTex;
		Texture2D *m_GridTex;
		Texture2D *m_LinearGradientTex;
		Texture2D *m_OrthoRefTex;
		Texture2D *m_UtilityColorTex;
		Texture2D *m_SphereSpriteTex;
		Texture2D *m_SphereSpriteNormalTex;

		MaterialManagerImpl *m_MatMan;
		Material *m_mtlWhite;
		Material *m_mtlBlack;

		size_t m_VertsPerFrame, m_IndicesPerFrame, m_TrisPerFrame, m_LinesPerFrame, m_PointsPerFrame;

		typedef std::map<uint32_t, GLuint> TTexFlagsToSamplerMap;
		TTexFlagsToSamplerMap m_TexFlagsToSampler;

		typedef std::map<tstring, FrameBuffer *> TNameToFrameBufferMap;
		TNameToFrameBufferMap m_NameToFB;

		RenderMethod *m_ActiveRenderMethod;
		RenderMethod *m_DefaultRenderMethod;

		Material *m_ActiveMaterial;

		const Model::InstanceData *m_pModelInstData;

		DWORD m_RendThreadId;

		using FontMap = std::map<uint32_t, Font *>;
		FontMap m_FontMap;


	public:

		COpenGL gl;
		bool isnv;

		RendererImpl(SystemImpl *psys);

		virtual ~RendererImpl();

		virtual System *GetSystem();

		virtual bool Initialize(HWND hwnd, props::TFlags64 flags);

		virtual bool Initialized();

		virtual void FlushErrors(const TCHAR *msgformat, ...);

		virtual void Shutdown();

		virtual const TCHAR* GetVendorName() const;
		virtual const TCHAR* GetDeviceName() const;

		virtual size_t LastTriagleCount() const;
		virtual size_t LastLineCount() const;
		virtual size_t LastPointCount() const;

		pool::IThreadPool *GetTaskPool();

		virtual Gui *GetGui();

		virtual void SetViewport(const RECT *viewport = nullptr);
		virtual const RECT *GetViewport(RECT *viewport = nullptr) const;

		virtual void SetOverrideHwnd(HWND hwnd);
		virtual HWND GetOverrideHwnd() const;

		virtual bool BeginScene(props::TFlags64 flags = 0);
		virtual bool EndScene(props::TFlags64 flags = 0);
		virtual bool Present();

		virtual size_t GetCurrentFrameNumber();

		virtual void SetClearColor(const glm::fvec4 *color = nullptr);
		virtual const glm::fvec4 *GetClearColor(glm::fvec4 *color = nullptr) const;

		virtual void SetClearDepth(float depth);
		virtual float GetClearDepth() const;

		virtual void SetClearStencil(uint8_t stencil);
		virtual uint8_t GetClearStencil() const;

		virtual void SetDepthMode(DepthMode mode);
		virtual DepthMode GetDepthMode() const;

		virtual void SetDepthTest(Test test);
		virtual Test GetDepthTest() const;

		virtual void SetFillMode(FillMode mode);
		virtual FillMode GetFillMode() const;

		virtual void SetStencilEnabled(bool en);
		virtual bool GetStencilEnabled() const;

		virtual void SetStencilTest(Test test, uint8_t ref = 0, uint8_t mask = 0xff);
		virtual Test GetStencilTest(uint8_t *ref = nullptr, uint8_t *mask = nullptr) const;

		virtual void SetStencilOperation(StencilOperation stencil_fail, StencilOperation zfail, StencilOperation zpass);
		virtual void GetStencilOperation(StencilOperation &stencil_fail, StencilOperation &zfail, StencilOperation &zpass) const;

		virtual void SetWindingOrder(WindingOrder mode);
		virtual WindingOrder GetWindingOrder() const;

		virtual void SetCullMode(CullMode mode);
		virtual CullMode GetCullMode() const;

		virtual void SetBlendMode(BlendMode mode);
		virtual BlendMode GetBlendMode() const;

		virtual void SetAlphaPassRange(float minalpha, float maxalpha);
		virtual void GetAlphaPassRange(float &minalpha, float &maxalpha);

		virtual void SetAlphaCoverage(float coverage, bool invert);
		virtual void GetAlphaCoverage(float &coverage, bool &invert);

		virtual void SetBlendEquation(BlendEquation eq);
		virtual BlendEquation GetBlendEquation() const;

		size_t PixelSize(TextureType type);
		GLenum GLType(TextureType type);
		GLenum GLInternalFormat(TextureType type);
		GLenum GLFormat(TextureType type);

		virtual Texture2D *CreateTexture2D(size_t width, size_t height, TextureType type, size_t mipcount, props::TFlags64 flags);
		virtual TextureCube *CreateTextureCube(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount, props::TFlags64 flags);
		virtual Texture3D *CreateTexture3D(size_t width, size_t height, size_t depth, TextureType type, size_t mipcount, props::TFlags64 flags);

		virtual DepthBuffer *CreateDepthBuffer(size_t width, size_t height, DepthType type, props::TFlags64 flags);

		virtual FrameBuffer *CreateFrameBuffer(props::TFlags64 flags, const TCHAR *name);
		virtual FrameBuffer *FindFrameBuffer(const TCHAR *name) const;
		void RemoveFrameBuffer(const TCHAR *name);

		virtual VertexBuffer *CreateVertexBuffer(props::TFlags64 flags);
		virtual IndexBuffer *CreateIndexBuffer(props::TFlags64 flags);
		virtual Mesh *CreateMesh();

		virtual ShaderProgram *CreateShaderProgram();
		virtual ShaderComponent *CreateShaderComponent(ShaderComponentType type);

		virtual RenderMethod *CreateRenderMethod();
		virtual void UseRenderMethod(const RenderMethod *method);
		virtual RenderMethod *GetActiveRenderMethod() const;

		virtual void UseMaterial(const Material *material);
		virtual Material *GetActiveMaterial() const;

		virtual void UseFrameBuffer(FrameBuffer *pfb, props::TFlags64 flags = UFBFLAG_FINISHLAST);
		virtual FrameBuffer *GetActiveFrameBuffer();

		virtual void UseProgram(ShaderProgram *pprog);
		virtual ShaderProgram *GetActiveProgram();

		virtual void UseVertexBuffer(VertexBuffer *pvbuf);
		virtual void UseIndexBuffer(IndexBuffer *pibuf);

		virtual void UseTexture(uint64_t texunit, Texture *ptex, props::TFlags32 texflags = TEXFLAG_WRAP_U | TEXFLAG_WRAP_V | TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPLINEAR);

		virtual bool DrawPrimitives(PrimType type, size_t count = -1);
		virtual bool DrawIndexedPrimitives(PrimType type, size_t offset = -1, size_t count = -1);

		virtual void SetProjectionMatrix(const glm::fmat4x4 *m);
		virtual void SetViewMatrix(const glm::fmat4x4 *m);
		virtual void SetWorldMatrix(const glm::fmat4x4 *m);
		virtual void SetSunShadowMatrix(const glm::fmat4x4 *m);
		virtual void SetTextureTransformMatrix(const glm::fmat4x4 *m);

		virtual const glm::fmat4x4 *GetProjectionMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetViewMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetWorldMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetWorldViewMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetNormalMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetViewProjectionMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetWorldViewProjectionMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetSunShadowMatrix(glm::fmat4x4 *m = nullptr);
		virtual const glm::fmat4x4 *GetTextureTransformMatrix(glm::fmat4x4 *m = nullptr);

		virtual void SetEyePosition(const glm::fvec3 *pos);
		virtual void SetEyeDirection(const glm::fvec3 *dir);

		virtual const glm::fvec3 *GetEyePosition(glm::fvec3 *pos = nullptr) const;
		virtual const glm::fvec3 *GetEyeDirection(glm::fvec3 *dir = nullptr) const;

		virtual const Model::InstanceData *RendererImpl::GetModelInstanceData();
		virtual void SetModelInstanceData(const Model::InstanceData *pinstdata = nullptr);

		virtual VertexBuffer *GetFullscreenPlaneVB();

		VertexBuffer *GetCubeVB();
		VertexBuffer *GetRefCubeVB();
		virtual Mesh *GetBoundsMesh();
		virtual Mesh *GetCubeMesh();
		virtual Mesh *GetRefCubeMesh();

		VertexBuffer *GetPlanesVB();
		virtual Mesh *GetXYPlaneMesh();
		virtual Mesh *GetYZPlaneMesh();
		virtual Mesh *GetXZPlaneMesh();

		VertexBuffer *GetHemisphereVB();
		virtual Mesh *GetHemisphereMesh();
		virtual Mesh *GetSphereMesh();

		virtual Texture2D *GetBlackTexture();
		virtual Texture2D *GetGreyTexture();
		virtual Texture2D *GetDefaultDescTexture();
		virtual Texture2D *GetDefaultNormalTexture();
		virtual Texture2D *GetWhiteTexture();
		virtual Texture2D *GetBlueTexture();
		virtual Texture2D *GetGridTexture();
		virtual Texture2D *GetLinearGradientTexture();
		virtual Texture2D *GetOrthoRefTexture();
		virtual Texture2D *GetUtilityColorTexture();
		virtual Texture2D *GetSphereSpriteTexture();
		virtual Texture2D *GetSphereSpriteNormalTexture();

		virtual MaterialManager *GetMaterialManager();
		virtual const Material *GetWhiteMaterial();
		virtual const Material *GetBlackMaterial();

		virtual ShaderProgram *GetBoundsShader();

		virtual Font *GetFont(const TCHAR *name, size_t size);

	};

};
