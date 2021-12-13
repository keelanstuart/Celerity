// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3IndexBuffer.h>
#include <C3RendererImpl.h>

namespace c3
{

	class IndexBufferImpl : public IndexBuffer
	{

	protected:
		RendererImpl *m_Rend;
		void *m_Buffer;
		size_t m_NumIndices;
		GLuint m_glID;
		IndexBuffer::IndexSize m_IndexSize;
		GLuint m_LastBoundBuffer;

		void *m_Cache;

	public:
		IndexBufferImpl(RendererImpl *prend);
		virtual ~IndexBufferImpl();

		virtual void Release();

		virtual RETURNCODE Lock(void **buffer, size_t numindices, IndexSize sz, props::TFlags64 flags);
		virtual void Unlock();

		virtual size_t Count();

		virtual size_t GetIndexSize();

		operator GLuint() const { return m_glID; }
		GLuint IBglID() const { return m_glID; }

	};

};