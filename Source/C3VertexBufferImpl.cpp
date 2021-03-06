// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3VertexBufferImpl.h>


using namespace c3;


VertexBufferImpl::VertexBufferImpl(RendererImpl *prend)
{
	m_Rend = prend;
	m_Buffer = NULL;
	m_NumVerts = 0;
	m_VertSize = 0;
	m_glID = GL_INVALID_VALUE;
	m_LastBoundBuffer = 0;
	m_Components.reserve(5);
}


VertexBufferImpl::~VertexBufferImpl()
{
	if (m_Buffer)
		Unlock();

	m_NumVerts = 0;
	m_VertSize = 0;

	if (m_Rend && (m_glID != GL_INVALID_VALUE))
	{
		m_Rend->UseVertexBuffer(nullptr);
		m_Rend->gl.DeleteBuffers(1, &m_glID);
		m_glID = GL_INVALID_VALUE;
	}
}


void VertexBufferImpl::Release()
{
	delete this;
}


VertexBuffer::RETURNCODE VertexBufferImpl::Lock(void **buffer, size_t numverts, const ComponentDescription *components, props::TFlags64 flags)
{
	if (m_Buffer)
		return RET_ALREADY_LOCKED;

	if (!numverts)
		return RET_ZERO_ELEMENTS;

	if (!buffer)
		return RET_NULL_BUFFER;

	m_Components.clear();

	size_t sz = 0;
	const ComponentDescription *c = components;
	size_t cc = 0;
	while (c && (c->m_Type != VertexBuffer::ComponentDescription::VCT_NONE) && (c->m_Count > 0))
	{
		m_Components.push_back(*c);
		sz += c->size();
		c++;
	}

	if (!sz)
		return RET_BAD_VERTEX_DESCRIPTION;

	bool init = false;
	if (m_glID == GL_INVALID_VALUE)
	{
		m_Rend->gl.GenBuffers(1, &m_glID);
		init = true;
	}

	if (m_glID == GL_INVALID_VALUE)
		return RET_GENBUFFER_FAILED;

	bool update_now = flags.IsSet(VBLOCKFLAG_UPDATENOW);
	bool user_buffer = flags.IsSet(VBLOCKFLAG_USERBUFFER);
	if (update_now && !user_buffer)
		return RET_UPDATENOW_NEEDS_USERBUFFER;

	GLint mode;
	if (flags.IsSet(VBLOCKFLAG_READ | VBLOCKFLAG_WRITE))
		mode = GL_READ_WRITE;
	else if (flags.IsSet(VBLOCKFLAG_READ))
		mode = GL_READ_ONLY;
	else if (flags.IsSet(VBLOCKFLAG_WRITE))
		mode = GL_WRITE_ONLY;

	m_VertSize = sz;
	m_NumVerts = numverts;

	// bind the buffer
	m_Rend->UseVertexBuffer(this);

	if (init || update_now)
	{
		// make sure that it is allocated
		m_Rend->gl.BufferData(GL_ARRAY_BUFFER, sz * numverts, user_buffer ? *buffer : nullptr, flags.IsSet(VBLOCKFLAG_DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		if (update_now)
			return RET_OK;
	}

	m_Buffer = m_Rend->gl.MapBuffer(GL_ARRAY_BUFFER, mode);
	if (!m_Buffer)
		return RET_MAPBUFFER_FAILED;

	*buffer = m_Buffer;

	return RET_OK;
}


void VertexBufferImpl::Unlock()
{
	if (m_Buffer)
	{
		m_Rend->gl.UnmapBuffer(GL_ARRAY_BUFFER);

		m_Buffer = NULL;
	}
}


size_t VertexBufferImpl::Count()
{
	return m_NumVerts;
}


size_t VertexBufferImpl::NumComponents()
{
	return m_Components.size();
}


const VertexBuffer::ComponentDescription *VertexBufferImpl::Component(size_t compidx)
{
	if (compidx >= m_Components.size())
		return NULL;

	return &(m_Components.at(compidx));
}


size_t VertexBufferImpl::VertexSize()
{
	return m_VertSize;
}
