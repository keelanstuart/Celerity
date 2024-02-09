// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3VertexBufferImpl.h>
#include <C3RendererImpl.h>

using namespace c3;


VertexBufferImpl::VertexBufferImpl(RendererImpl *prend)
{
	m_Rend = prend;
	m_Buffer = nullptr;
	m_NumVerts = 0;
	m_VertSize = 0;
	m_VAOglID = NULL;
	m_VBglID = NULL;
	m_Components.reserve(5);
	m_Cache = nullptr;
}


VertexBufferImpl::~VertexBufferImpl()
{
	if (m_Buffer)
		Unlock();

	if (m_Cache)
	{
		free(m_Cache);
		m_Cache = nullptr;
	}

	m_NumVerts = 0;
	m_VertSize = 0;

	if (m_Rend)
	{
		if (m_VBglID != NULL)
		{
			m_Rend->UseVertexBuffer(nullptr);
			m_Rend->gl.DeleteBuffers(1, &m_VBglID);
			m_VBglID = NULL;
		}

		if (m_VAOglID != NULL)
		{
			// Delete the global vertex array
			m_Rend->gl.BindVertexArray(0);
			m_Rend->gl.DeleteVertexArrays(1, &m_VAOglID);
		}
	}
}


void VertexBufferImpl::Release()
{
	delete this;
}

bool IdenticalComponents(const VertexBuffer::ComponentDescription* c, const VertexBufferImpl::TVertexComponentDescriptionArray& tc)
{
	if (c && tc.empty())
		return false;

	size_t i = 0;
	const VertexBuffer::ComponentDescription* cc = tc.data();
	while (c && (c->m_Type != VertexBuffer::ComponentDescription::VCT_NONE) && (i < tc.size()))
	{
		if (memcmp(c, cc, sizeof(VertexBuffer::ComponentDescription)))
			return false;
		i++;
	}

	return (i == tc.size());
}

VertexBuffer::RETURNCODE VertexBufferImpl::Lock(void **buffer, size_t numverts, const ComponentDescription *components, props::TFlags64 flags)
{
	if (m_Buffer)
		return RET_ALREADY_LOCKED;

	if (!buffer)
		return RET_NULL_BUFFER;

	size_t sz = m_VertSize;

	if (flags.IsSet(VBLOCKFLAG_WRITE))
	{
		if (!components)
			return RET_NULL_VERTEX_DESCRIPTION;

		if (!numverts)
			return RET_ZERO_ELEMENTS;

		if (!IdenticalComponents(components, m_Components))
		{
			sz = 0;
			m_Components.clear();

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
		}

		if (flags.IsSet(VBLOCKFLAG_CACHE))
		{
			if (m_Cache)
			{
				size_t cache_sz = _msize(m_Cache);
				if (cache_sz != (sz * numverts))
				{
					free(m_Cache);
					m_Cache = nullptr;
				}
			}

			if (!m_Cache)
				m_Cache = malloc(sz * numverts);
		}
	}

	// if we want read-only access, then use the cache if one is available. this avoids a map/unmap
	if (flags.IsSet(VBLOCKFLAG_READ) && !flags.IsSet(VBLOCKFLAG_WRITE))
	{
		if (m_Cache)
		{
			m_Buffer = m_Cache;
			*buffer = m_Buffer;

			return RET_OK;
		}
		else
		{
			assert(0 && "Cache never created; expect poor performance. Use VBLOCKFLAG_CACHE when calling Lock()!");
		}
	}

	if (m_VAOglID == NULL)
	{
		m_Rend->gl.GenVertexArrays(1, &m_VAOglID);
	}

	bool update_now = flags.IsSet(VBLOCKFLAG_UPDATENOW);
	bool user_buffer = flags.IsSet(VBLOCKFLAG_USERBUFFER);
	if (update_now && !user_buffer)
		return RET_UPDATENOW_NEEDS_USERBUFFER;

	bool init = false;

	if (m_VBglID == NULL)
	{
		m_Rend->gl.GenBuffers(1, &m_VBglID);
		init = true;
	}

	if (m_VBglID == NULL)
		return RET_GENBUFFER_FAILED;

	GLint mode = 0;
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
		{
			ConfigureAttributes();

			return RET_OK;
		}
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
		// if the buffer isn't just our cached copy, then unmap it and let the video driver do it's thing
		if (m_Buffer != m_Cache)
		{
			// if there IS a cache, then copy our new buffer contents to it
			if (m_Cache)
				memcpy(m_Cache, m_Buffer, m_VertSize * m_NumVerts);

			m_Rend->gl.UnmapBuffer(GL_ARRAY_BUFFER);

			ConfigureAttributes();
		}

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


void VertexBufferImpl::ConfigureAttributes()
{
	size_t vsz = VertexSize();
	size_t vo = 0;

	constexpr static const GLenum t[VertexBuffer::ComponentDescription::ComponentType::VCT_NUM_TYPES] = { 0, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT };

	for (size_t i = 0, maxi = NumComponents(); i < maxi; i++)
	{
		const VertexBuffer::ComponentDescription *pcd = Component(i);
		if (!pcd || !pcd->m_Count || (pcd->m_Type == VertexBuffer::ComponentDescription::VCT_NONE) || (pcd->m_Usage == VertexBuffer::ComponentDescription::Usage::VU_NONE))
			break;

		bool is_color = (pcd->m_Usage >= VertexBuffer::ComponentDescription::Usage::VU_COLOR0) && (pcd->m_Usage <= VertexBuffer::ComponentDescription::Usage::VU_COLOR3);

		switch (pcd->m_Type)
		{
			case VertexBuffer::ComponentDescription::VCT_F16:
			case VertexBuffer::ComponentDescription::VCT_F32:
			{
				m_Rend->gl.VertexAttribPointer((GLuint)i, (GLint)pcd->m_Count, t[pcd->m_Type], false, (GLsizei)vsz, (void *)vo);
				break;
			}

			case VertexBuffer::ComponentDescription::VCT_U8:
			case VertexBuffer::ComponentDescription::VCT_S8:
			case VertexBuffer::ComponentDescription::VCT_U32:
			{
				if (is_color)
					m_Rend->gl.VertexAttribPointer((GLuint)i, (GLint)pcd->m_Count, t[pcd->m_Type], true, (GLsizei)vsz, (void *)vo);
				else
					m_Rend->gl.VertexAttribIPointer((GLuint)i, (GLint)pcd->m_Count, t[pcd->m_Type], (GLsizei)vsz, (void *)vo);
				break;
			}
		}

		m_Rend->gl.EnableVertexAttribArray((GLuint)i);

		vo += pcd->size();
	}
}
