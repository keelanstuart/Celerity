// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


#pragma once

#include <C3FrameBuffer.h>
#include <C3RendererImpl.h>
#include <C3DepthBufferImpl.h>
#include <C3Publisher.h>


namespace c3
{

	class FrameBufferImpl : public FrameBuffer
	{

	protected:
		tstring m_Name;
		RendererImpl *m_Rend;
		GLuint m_glID;

		DepthBuffer *m_DepthTarget;
		bool m_OwnsDepth;
		float m_ClearDepth;
		int8_t m_ClearStencil;

		typedef union uClearColor
		{
			union
			{
				struct
				{
					uint8_t ur, ug, ub, ua;
				};
				uint8_t u[4];
				uint32_t pu;
			};

			union
			{
				struct
				{
					int8_t sr, sg, sb, sa;
				};
				int8_t s[4];
				uint32_t ps;
			};

			union
			{
				struct
				{
					float fr, fg, fb, fa;
				};
				float f[4];
			};
		} UClearColor;

		typedef struct sTargetData
		{
			sTargetData()
			{
				tex = nullptr;
				clearcolor = {0};
				owns = false;
			}

			Texture2D *tex;
			UClearColor clearcolor;
			bool owns;
		} STargetData;

		typedef std::vector<STargetData> TColorTargetArray;

		TColorTargetArray m_ColorTarget;

		const static GLuint targenum[MAX_COLORTARGETS];

		Renderer::BlendMode m_BlendMode[MAX_COLORTARGETS];
		Renderer::BlendEquation m_BlendEq[MAX_COLORTARGETS];
		Renderer::BlendMode m_AlphaBlendMode[MAX_COLORTARGETS];
		Renderer::BlendEquation m_AlphaBlendEq[MAX_COLORTARGETS];
		Renderer::ChannelMask m_ChannelMask[MAX_COLORTARGETS];

		Renderer::RenderStateOverrideFlags m_DirtyStates[MAX_COLORTARGETS];

		Publisher *m_Pub;

	public:

		FrameBufferImpl(RendererImpl *prend, const TCHAR *name);
		virtual ~FrameBufferImpl();

		virtual void Release();

		virtual const TCHAR *GetName() const;

		virtual RETURNCODE Setup(size_t numtargs, const TargetDesc *ptargdescs, DepthBuffer *pdb, RECT &r);

		virtual RETURNCODE Teardown();

		virtual RETURNCODE AttachColorTarget(Texture2D *target, size_t position);

		virtual size_t GetNumColorTargets();

		virtual Texture2D* GetColorTarget(size_t position);

		virtual Texture2D *GetColorTargetByName(const TCHAR *name);

		virtual RETURNCODE AttachDepthTarget(DepthBuffer *pdepth);

		virtual DepthBuffer *GetDepthTarget();

		virtual RETURNCODE Seal();

		virtual void SetClearColor(glm::fvec4 color, size_t target = -1);

		virtual glm::fvec4 GetClearColor(size_t target = 0) const;

		virtual void SetClearDepth(float depth = 1.0f);

		virtual float GetClearDepth() const;

		virtual void SetClearStencil(int8_t stencil = 0);

		virtual int8_t GetClearStencil() const;

		virtual void Clear(props::TFlags64 flags);

		virtual void SetBlendMode(Renderer::BlendMode mode, size_t target = -1);

		virtual Renderer::BlendMode GetBlendMode(size_t target = 0) const;

		virtual void SetAlphaBlendMode(Renderer::BlendMode mode, size_t target = -1);

		virtual Renderer::BlendMode GetAlphaBlendMode(size_t target = 0) const;

		virtual void SetBlendEquation(Renderer::BlendEquation eq, size_t target = -1);

		virtual Renderer::BlendEquation GetBlendEquation(size_t target = 0) const;

		virtual void SetAlphaBlendEquation(Renderer::BlendEquation eq, size_t target = -1);

		virtual Renderer::BlendEquation GetAlphaBlendEquation(size_t target = 0) const;

		virtual void SetChannelWriteMask(Renderer::ChannelMask mask, size_t target = -1);

		virtual Renderer::ChannelMask GetChannelWriteMask(size_t target = -1) const;

		operator GLuint() const { return m_glID; }

		void Subscribe(Subscription *sub);

		void Unsubscribe(Subscription *sub);

		void UpdateDirtyRenderStates(bool refresh = false);

	};

};