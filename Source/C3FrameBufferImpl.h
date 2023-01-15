// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3FrameBuffer.h>
#include <C3RendererImpl.h>
#include <C3DepthBufferImpl.h>


namespace c3
{

	class FrameBufferImpl : public FrameBuffer
	{

	protected:
		tstring m_Name;
		RendererImpl *m_Rend;
		GLuint m_glID;

		DepthBuffer *m_DepthTarget;
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
					uint16_t hr, hg, hb, ha;
				};
				uint16_t h[4];
				uint64_t ph;
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
			Texture2D *tex;
			UClearColor clearcolor;
		} STargetData;

		typedef std::vector<STargetData> TColorTargetArray;

		TColorTargetArray m_ColorTarget;

		const static GLuint targenum[MAX_COLORTARGETS];

		Renderer::BlendMode m_BlendMode;
		Renderer::BlendEquation m_BlendEq;

	public:

		FrameBufferImpl(RendererImpl *prend, const TCHAR *name);
		virtual ~FrameBufferImpl();

		virtual void Release();

		virtual const TCHAR *GetName() const;

		virtual RETURNCODE Setup(size_t numtargs, const TargetDesc *ptargdescs, DepthBuffer *pdb, RECT &r);

		virtual RETURNCODE AttachColorTarget(Texture2D *target, size_t position);

		virtual size_t GetNumColorTargets();

		virtual Texture2D* GetColorTarget(size_t position);

		virtual Texture2D *GetColorTargetByName(const TCHAR *name);

		virtual RETURNCODE AttachDepthTarget(DepthBuffer *pdepth);

		virtual DepthBuffer *GetDepthTarget();

		virtual RETURNCODE Seal();

		virtual void SetClearColor(size_t position, const uint32_t color);

		virtual uint32_t GetClearColor(size_t position) const;

		virtual void SetClearDepth(float depth = 1.0f);

		virtual float GetClearDepth() const;

		virtual void SetClearStencil(int8_t stencil = 0);

		virtual int8_t GetClearStencil() const;

		virtual void Clear(props::TFlags64 flags);

		virtual void SetBlendMode(Renderer::BlendMode mode);

		virtual Renderer::BlendMode GetBlendMode() const;

		virtual void SetBlendEquation(Renderer::BlendEquation eq);

		virtual Renderer::BlendEquation GetBlendEquation() const;

		operator GLuint() const { return m_glID; }

	};

};