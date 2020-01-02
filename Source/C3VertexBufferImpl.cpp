// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


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
}


VertexBufferImpl::~VertexBufferImpl()
{
	if (m_Buffer)
		Unlock();

	m_NumVerts = 0;
	m_VertSize = 0;

	if (m_Rend && (m_glID != GL_INVALID_VALUE))
	{
		m_Rend->gl.DeleteBuffers(1, &m_glID);
		m_glID = GL_INVALID_VALUE;
	}
}


void VertexBufferImpl::Release()
{
	delete this;
}


VertexBuffer::RETURNCODE VertexBufferImpl::Lock(void **buffer, size_t count, const SVertexComponentDescription *components, props::TFlags64 flags)
{
	if (m_Buffer)
		return RET_ALREADY_LOCKED;

	if (!count)
		return RET_ZERO_ELEMENTS;

	if (!buffer)
		return RET_NULL_BUFFER;

	m_Components.clear();

	size_t sz = 0;
	const SVertexComponentDescription *c = components;
	size_t cc = 0;
	while (c && (c->m_Type != VertexBuffer::SVertexComponentDescription::VCT_NONE) && (c->m_Count > 0))
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

	GLint mode;
	if (flags.IsSet(VBLOCKFLAG_READ | VBLOCKFLAG_WRITE))
		mode = GL_READ_WRITE;
	else if (flags.IsSet(VBLOCKFLAG_READ))
		mode = GL_READ_ONLY;
	else if (flags.IsSet(VBLOCKFLAG_WRITE))
		mode = GL_WRITE_ONLY;

	// bind the buffer
	Bind();

	if (init)
	{
		// make sure that it is allocated
		m_Rend->gl.BufferData(GL_ARRAY_BUFFER, sz * count, NULL, GL_STATIC_DRAW);
	}

	m_Buffer = m_Rend->gl.MapBuffer(GL_ARRAY_BUFFER, mode);
	if (!m_Buffer)
		return RET_MAPBUFFER_FAILED;

	m_VertSize = sz;
	m_NumVerts = count;

	*buffer = m_Buffer;

	return RET_OK;
}


void VertexBufferImpl::Unlock()
{
	if (m_Buffer)
	{
		m_Rend->gl.UnmapBuffer(GL_ARRAY_BUFFER);

		Unbind();

		m_Buffer = NULL;
	}
}


size_t VertexBufferImpl::Count()
{
	return m_NumVerts;
}


void VertexBufferImpl::Bind()
{
	m_Rend->gl.BindBuffer(GL_ARRAY_BUFFER, m_glID);
}


void VertexBufferImpl::Unbind()
{
	m_Rend->gl.BindBuffer(GL_ARRAY_BUFFER, 0);
}


size_t VertexBufferImpl::NumComponents()
{
	return m_Components.size();
}


const VertexBuffer::SVertexComponentDescription *VertexBufferImpl::Component(size_t compidx)
{
	if (compidx >= m_Components.size())
		return NULL;

	return &(m_Components.at(compidx));
}


size_t VertexBufferImpl::VertexSize()
{
	return m_VertSize;
}
