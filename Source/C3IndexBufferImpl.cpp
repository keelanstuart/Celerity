// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


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
	m_Cache = nullptr;
}


IndexBufferImpl::~IndexBufferImpl()
{
	if (m_Buffer)
		Unlock();

	if (m_Cache)
	{
		free(m_Cache);
		m_Cache = nullptr;
	}

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
	if (m_Buffer)
		return RET_ALREADY_LOCKED;

	if (!buffer)
		return RET_NULL_BUFFER;

	if (!numindices)
		return RET_ZERO_ELEMENTS;

	if (flags.IsSet(IBLOCKFLAG_CACHE))
	{
		if (m_Cache)
		{
			size_t cache_sz = _msize(m_Cache);
			if (cache_sz != (sz * numindices))
				free(m_Cache);
		}

		m_Cache = malloc(sz * numindices);
	}

	// if we want read-only access, then use the cache if one is available. this avoids a map/unmap
	if (flags.IsSet(IBLOCKFLAG_READ) && !flags.IsSet(IBLOCKFLAG_WRITE))
	{
		if (m_Cache)
		{
			m_Buffer = m_Cache;
			*buffer = m_Buffer;

			return RET_OK;
		}
		else
		{
			assert(0 && "Cache never created; expect poor performance. Use IBLOCKFLAG_CACHE when calling Lock()!");
		}
	}

	bool init = false;
	if (m_glID == NULL)
	{
		m_Rend->gl.GenBuffers(1, &m_glID);
		init = true;
	}

	if (m_glID == NULL)
		return RET_GENBUFFER_FAILED;

	bool update_now = flags.IsSet(IBLOCKFLAG_UPDATENOW);
	bool user_buffer = flags.IsSet(IBLOCKFLAG_USERBUFFER);
	if (update_now && !user_buffer)
		return RET_UPDATENOW_NEEDS_USERBUFFER;

	GLint mode;
	if (flags.IsSet(IBLOCKFLAG_READ | IBLOCKFLAG_WRITE))
		mode = GL_READ_WRITE;
	else if (flags.IsSet(IBLOCKFLAG_READ))
		mode = GL_READ_ONLY;
	else if (flags.IsSet(IBLOCKFLAG_WRITE))
		mode = GL_WRITE_ONLY;

	m_Rend->UseIndexBuffer(this);

	if (init || update_now)
	{
		// make sure that it is allocated
		m_Rend->gl.BufferData(GL_ELEMENT_ARRAY_BUFFER, sz * numindices, user_buffer ? *buffer : nullptr, flags.IsSet(IBLOCKFLAG_DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		if (update_now)
			return RET_OK;
	}

	m_Buffer = m_Rend->gl.MapBuffer(GL_ELEMENT_ARRAY_BUFFER, mode);
	if (!m_Buffer)
		return RET_MAPBUFFER_FAILED;

	m_IndexSize = sz;
	m_NumIndices = numindices;

	*buffer = m_Buffer;

	return RET_OK;
}


void IndexBufferImpl::Unlock()
{
	if (m_Buffer)
	{
		if (m_Buffer != m_Cache)
		{
			m_Rend->gl.UnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

			if (m_Cache)
				memcpy(m_Cache, m_Buffer, m_IndexSize * m_NumIndices);
		}

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
