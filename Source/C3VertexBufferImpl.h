// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3VertexBuffer.h>
#include <C3RendererImpl.h>

namespace c3
{

	class VertexBufferImpl : public VertexBuffer
	{

	protected:
		RendererImpl *m_Rend;

		void *m_Buffer;
		size_t m_NumVerts;
		GLuint m_glID;
		size_t m_VertSize;

		void *m_Cache;

		typedef std::vector<ComponentDescription> TVertexComponentDescriptionArray;
		TVertexComponentDescriptionArray m_Components;

		GLuint m_LastBoundBuffer;

	public:
		VertexBufferImpl(RendererImpl *prend);
		virtual ~VertexBufferImpl();

		virtual void Release();

		virtual RETURNCODE Lock(void **buffer, size_t numverts, const ComponentDescription *components, props::TFlags64 flags);
		virtual void Unlock();

		virtual size_t Count();

		virtual size_t NumComponents();
		virtual const ComponentDescription *Component(size_t compidx);

		virtual size_t VertexSize();

		operator GLuint() const { return m_glID; }

	};

};