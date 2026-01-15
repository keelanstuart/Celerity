// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"

#include <C3IndexBufferImpl.h>


using namespace c3;


IndexBufferImpl::IndexBufferImpl(RendererImpl *prend)
{
	m_Rend = prend;
	m_Buffer = NULL;
	m_NumIndices = 0;
	m_IndexSize = IS_16BIT;
	m_glID = NULL;
	m_LastBoundBuffer = 0;
}


IndexBufferImpl::~IndexBufferImpl()
{
	if (m_Buffer)
		Unlock();

	m_NumIndices = 0;
	m_IndexSize = IS_16BIT;

	if (m_Rend && (m_glID != NULL))
	{
		m_Rend->UseIndexBuffer(nullptr);
		m_Rend->gl.DeleteBuffers(1, &m_glID);
		m_glID = 0;
	}
}


void IndexBufferImpl::Release()
{
	delete this;
}


IndexBuffer::RETURNCODE IndexBufferImpl::Lock(void **buffer, size_t numindices, IndexSize sz, props::TFlags64 flags)
{
	if (!buffer)
		return RET_NULL_BUFFER;

	if (flags.IsSet(IBLOCKFLAG_READ) && !flags.IsSet(IBLOCKFLAG_WRITE) && !m_Cache.empty())
	{
		*buffer = m_Cache.data();
		return RET_OK;
	}

	if (m_Buffer)
		return RET_ALREADY_LOCKED;

	if (flags.IsSet(IBLOCKFLAG_WRITE) && !numindices)
		return RET_ZERO_ELEMENTS;

	bool init = (sz != m_IndexSize) || (numindices != m_NumIndices);
	m_IndexSize = sz;
	m_NumIndices = numindices;

	bool update_now = flags.IsSet(IBLOCKFLAG_UPDATENOW);
	bool user_buffer = flags.IsSet(IBLOCKFLAG_USERBUFFER);
	if (update_now && !user_buffer)
		return RET_UPDATENOW_NEEDS_USERBUFFER;

	if (m_glID == NULL)
	{
		m_Rend->gl.GenBuffers(1, &m_glID);
		init = true;
	}

	if (m_glID == NULL)
		return RET_GENBUFFER_FAILED;

	GLbitfield mode = 0;
	if (flags.IsSet(IBLOCKFLAG_READ) || flags.IsSet(IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE))
		mode |= GL_MAP_READ_BIT;
	if (flags.IsSet(IBLOCKFLAG_WRITE))
		mode |= GL_MAP_WRITE_BIT;

	size_t bufsize = m_IndexSize * m_NumIndices;
	if (flags.IsSet(IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE))
	{
		if (bufsize > m_Cache.size())
			m_Cache.resize(bufsize);
	}

	m_Rend->UseIndexBuffer(this);

	if (init || update_now)
	{
		// make sure that it is allocated
		m_Rend->gl.BufferData(GL_ELEMENT_ARRAY_BUFFER, bufsize, user_buffer ? *buffer : nullptr, flags.IsSet(IBLOCKFLAG_DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		if (update_now)
		{
			if (flags.IsSet(IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE))
				memcpy(m_Cache.data(), buffer, bufsize);

			return RET_OK;
		}
	}

	m_Buffer = m_Rend->gl.MapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, bufsize, mode);
	if (!m_Buffer)
		return RET_MAPBUFFER_FAILED;

	*buffer = m_Buffer;

	return RET_OK;
}


void IndexBufferImpl::Unlock()
{
	if (m_Buffer)
	{
		if (m_Buffer != m_Cache.data() && !m_Cache.empty())
		{
			// if there IS a cache, then copy our new buffer contents to it
			memcpy(m_Cache.data(), m_Buffer, m_IndexSize * m_NumIndices);
		}

		m_Rend->gl.UnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

		m_Buffer = NULL;
	}
}


size_t IndexBufferImpl::Count()
{
	return m_NumIndices;
}


size_t IndexBufferImpl::GetIndexSize()
{
	return m_IndexSize;
}
