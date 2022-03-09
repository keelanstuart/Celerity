// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once

#include <C3DepthBuffer.h>
#include <C3RendererImpl.h>


namespace c3
{

	class DepthBufferImpl : public DepthBuffer
	{

	protected:
		RendererImpl *m_Rend;
		GLuint m_glID;
		Renderer::EDepthType m_Type;
		size_t m_Width;
		size_t m_Height;

	public:

		DepthBufferImpl(RendererImpl *prend, size_t width, size_t height, Renderer::EDepthType type);
		virtual ~DepthBufferImpl();

		virtual void Release();

		virtual Renderer::EDepthType Format();

		/// The X length of the texture
		virtual size_t Width();

		/// The Y length of the texture
		virtual size_t Height();

		operator GLuint() const { return m_glID; }

	};

};