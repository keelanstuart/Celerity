// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3FrameBuffer.h>
#include <C3RendererImpl.h>
#include <C3DepthBufferImpl.h>


namespace c3
{

	class FrameBufferImpl : public FrameBuffer
	{

	protected:
		RendererImpl *m_Rend;
		GLuint m_glID;

		DepthBufferImpl *m_DepthTarget;
		typedef std::vector<Texture2D *> TColorTargetArray;
		TColorTargetArray m_ColorTarget;

		const static GLuint targenum[MAX_COLORTARGETS];

	public:

		FrameBufferImpl(RendererImpl *prend);
		virtual ~FrameBufferImpl();

		virtual void Release();

		virtual RETURNCODE AttachColorTarget(Texture2D *target, size_t position);

		virtual size_t GetNumColorTargets();

		virtual Texture2D* GetColorTarget(size_t position);

		virtual RETURNCODE AttachDepthTarget(DepthBuffer *pdepth);

		virtual DepthBuffer *GetDepthTarget();

		virtual RETURNCODE Seal();

		operator GLuint() const { return m_glID; }

	};

};