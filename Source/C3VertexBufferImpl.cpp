// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


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
	m_Configured = false;
}


VertexBufferImpl::~VertexBufferImpl()
{
	if (m_Buffer)
		Unlock();

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
	if (!buffer)
		return RET_NULL_BUFFER;

	// if we want read-only access, then use the cache if one is available. this avoids a map/unmap
	if (flags.IsSet(VBLOCKFLAG_READ) && !flags.IsSet(VBLOCKFLAG_WRITE) && !m_Cache.empty())
	{
		*buffer = m_Cache.data();
		return RET_OK;
	}

	if (m_Buffer)
		return RET_ALREADY_LOCKED;

	bool init = (m_NumVerts != numverts);
	m_NumVerts = numverts;

	bool update_now = flags.IsSet(VBLOCKFLAG_UPDATENOW);
	bool user_buffer = flags.IsSet(VBLOCKFLAG_USERBUFFER);
	if (update_now && !user_buffer)
		return RET_UPDATENOW_NEEDS_USERBUFFER;

	if (flags.IsSet(VBLOCKFLAG_WRITE))
	{
		if (!components)
			return RET_NULL_VERTEX_DESCRIPTION;

		if (!numverts)
			return RET_ZERO_ELEMENTS;

		if (!IdenticalComponents(components, m_Components))
		{
			m_Configured = false;

			size_t sz = 0;
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

			init |= (sz != m_VertSize);
			m_VertSize = sz;
		}
	}

	if (m_VAOglID == NULL)
	{
		m_Rend->gl.GenVertexArrays(1, &m_VAOglID);
	}

	if (m_VBglID == NULL)
	{
		m_Rend->gl.GenBuffers(1, &m_VBglID);
		init = true;
	}

	if (m_VBglID == NULL)
		return RET_GENBUFFER_FAILED;

	GLbitfield mode = 0;
	if (flags.IsSet(VBLOCKFLAG_READ) || flags.IsSet(VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE))
		mode |= GL_MAP_READ_BIT;
	if (flags.IsSet(VBLOCKFLAG_WRITE))
		mode |= GL_MAP_WRITE_BIT;

	size_t bufsize = m_VertSize * m_NumVerts;
	if (flags.IsSet(VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE))
	{
		if (bufsize > m_Cache.size())
			m_Cache.resize(bufsize);
	}

	// bind the buffer
	m_Rend->UseVertexBuffer(this);

	if (init || update_now)
	{
		// make sure that it is allocated
		m_Rend->gl.BufferData(GL_ARRAY_BUFFER, bufsize, user_buffer ? *buffer : nullptr, flags.IsSet(VBLOCKFLAG_DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		if (update_now)
		{
			if (flags.IsSet(VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE))
				memcpy(m_Cache.data(), *buffer, bufsize);

			ConfigureAttributes();

			return RET_OK;
		}
	}

	m_Buffer = m_Rend->gl.MapBufferRange(GL_ARRAY_BUFFER, 0, bufsize, mode);
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
		if (m_Buffer != m_Cache.data() && !m_Cache.empty())
		{
			// if there IS a cache, then copy our new buffer contents to it
			memcpy(m_Cache.data(), m_Buffer, m_VertSize * m_NumVerts);
		}

		GLboolean b = m_Rend->gl.UnmapBuffer(GL_ARRAY_BUFFER);
		if (!b)
			m_Rend->GetSystem()->GetLog()->Print(_T("WTF!!"));

		ConfigureAttributes();

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

	m_Configured = true;
}
