// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3TextureImpl.h>


using namespace c3;


Texture2DImpl::Texture2DImpl(RendererImpl *prend, size_t width, size_t height, Renderer::ETextureType type, size_t mipcount, props::TFlags64 flags)
{
	assert(prend);

	m_Rend = prend;

	m_Width = std::max(width, size_t(1));
	m_Height = std::max(height, size_t(1));

	if (mipcount < 1)
	{
		mipcount = 0;
		size_t mr = std::min(width, height);
		while (mr)
		{
			mr >>= 1;
			mipcount++;
		}
	}

	m_MipCount = mipcount;
	m_Type = type;
	m_Buffer = nullptr;
	m_glID = GL_INVALID_VALUE;
	m_LockMip = 0;

	if (flags.IsSet(TEXCREATEFLAG_RENDERTARGET))
	{
		m_Rend->gl.CreateTextures(GL_TEXTURE_2D, 1, &m_glID);

		Bind();

		m_Rend->gl.TexStorage2D(GL_TEXTURE_2D, (GLsizei)m_MipCount, m_Rend->GLInternalFormat(m_Type), (GLsizei)m_Width, (GLsizei)m_Height);
	}
}


Texture2DImpl::~Texture2DImpl()
{
	assert(m_Rend);

	if (m_Buffer)
	{
		free(m_Buffer);
		m_Buffer = nullptr;
	}

	if (m_glID != GL_INVALID_VALUE)
	{
		m_Rend->gl.DeleteTextures(1, &m_glID);
	}
}


void Texture2DImpl::Release()
{
	delete this;
}


Renderer::ETextureType Texture2DImpl::Format()
{
	return m_Type;
}


size_t Texture2DImpl::MipCount()
{
	return m_MipCount;
}


size_t Texture2DImpl::Width()
{
	return m_Width;
}


size_t Texture2DImpl::Height()
{
	return m_Height;
}


void Texture2DImpl::Bind()
{
	assert(m_Rend);

	if (m_glID != GL_INVALID_VALUE)
	{
		m_Rend->gl.BindTexture(GL_TEXTURE_2D, m_glID);
	}
}


void Texture2DImpl::Unbind()
{
	assert(m_Rend);

	m_Rend->gl.BindTexture(GL_TEXTURE_2D, 0);
}


Texture::RETURNCODE Texture2DImpl::Lock(void **buffer, Texture2D::SLockInfo &lockinfo, size_t mip, props::TFlags64 flags)
{
	assert(m_Rend);

	if (m_Buffer)
		return RET_ALREADY_LOCKED;

	if (!buffer)
		return RET_NULL_BUFFER;

	bool init = false;
	if (m_glID == GL_INVALID_VALUE)
	{
		m_Rend->gl.CreateTextures(GL_TEXTURE_2D, 1, &m_glID);
		init = true;
	}

	if (m_glID == GL_INVALID_VALUE)
		return RET_GENBUFFER_FAILED;

	// bind the buffer
	Bind();

	if (init)
	{
		// make sure that it is allocated
		m_Rend->gl.TexStorage2D(GL_TEXTURE_2D, (GLsizei)m_MipCount, m_Rend->GLInternalFormat(m_Type), (GLsizei)m_Width, (GLsizei)m_Height);
	}

	lockinfo.width = m_Width >> mip;
	lockinfo.height = m_Height >> mip;
	lockinfo.stride = lockinfo.width * m_Rend->PixelSize(m_Type);

	m_Buffer = malloc(lockinfo.height * lockinfo.stride);
	if (!m_Buffer)
		return RET_MAPBUFFER_FAILED;

	m_LockMip = mip;

	if (flags.IsSet(TEXLOCKFLAG_READ))
	{
		m_Rend->gl.GetTexImage(GL_TEXTURE_2D, (GLsizei)mip, m_Rend->GLFormat(m_Type), m_Rend->GLType(m_Type), m_Buffer);
	}

	*buffer = m_Buffer;

	return RET_OK;
}	


Texture::RETURNCODE Texture2DImpl::Unlock()
{
	assert(m_Rend);

	if (!m_Buffer)
		return RET_NOT_LOCKED;

	Bind();

	m_Rend->gl.TexSubImage2D(GL_TEXTURE_2D, (GLint)m_LockMip, 0, 0, GLsizei(m_Width >> m_LockMip), GLsizei(m_Height >> m_LockMip), m_Rend->GLFormat(m_Type), m_Rend->GLType(m_Type), m_Buffer);

	free(m_Buffer);
	m_Buffer = nullptr;

	return RET_OK;
}


// *******************************************************************************
// *******************************************************************************


TextureCubeImpl::TextureCubeImpl(RendererImpl *prend, size_t width, size_t height, size_t depth, Renderer::ETextureType type, size_t mipcount, props::TFlags64 flags)
{
	assert(prend);

	m_Rend = prend;

	m_Width = std::max(width, size_t(1));
	m_Height = std::max(height, size_t(1));
	m_Depth = std::max(depth, size_t(1));

	if (mipcount < 1)
	{
		mipcount = 0;
		size_t mr = std::min(width, height);
		while (mr)
		{
			mr >>= 1;
			mipcount++;
		}
	}

	m_MipCount = mipcount;
	m_Type = type;
	m_Buffer = nullptr;
	m_glID = GL_INVALID_VALUE;
	m_LockMip = 0;
}


TextureCubeImpl::~TextureCubeImpl()
{
	assert(m_Rend);

	if (m_Buffer)
	{
		free(m_Buffer);
		m_Buffer = nullptr;
	}

	if (m_glID != GL_INVALID_VALUE)
	{
		m_Rend->gl.DeleteTextures(1, &m_glID);
	}
}


void TextureCubeImpl::Release()
{
	delete this;
}


Renderer::ETextureType TextureCubeImpl::Format()
{
	return m_Type;
}


size_t TextureCubeImpl::MipCount()
{
	return m_MipCount;
}


size_t TextureCubeImpl::Width()
{
	return m_Width;
}


size_t TextureCubeImpl::Height()
{
	return m_Height;
}


size_t TextureCubeImpl::Depth()
{
	return m_Depth;
}


void TextureCubeImpl::Bind()
{
	assert(m_Rend);

	if (m_glID != GL_INVALID_VALUE)
	{
		m_Rend->gl.BindTexture(GL_TEXTURE_CUBE_MAP, m_glID);
	}
}


void TextureCubeImpl::Unbind()
{
	assert(m_Rend);

	m_Rend->gl.BindTexture(GL_TEXTURE_CUBE_MAP, 0);
}


const static GLenum GLface[TextureCube::ECubeFace::FACE_COUNT] ={
	GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};


Texture::RETURNCODE TextureCubeImpl::Lock(void **buffer, CubeFace face, TextureCube::SLockInfo &lockinfo, size_t mip, props::TFlags64 flags)
{
	assert(m_Rend);

	if (m_Buffer)
		return RET_ALREADY_LOCKED;

	if (!buffer)
		return RET_NULL_BUFFER;

	bool init = false;
	if (m_glID == GL_INVALID_VALUE)
	{
		m_Rend->gl.CreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_glID);
		init = true;
	}

	if (m_glID == GL_INVALID_VALUE)
		return RET_GENBUFFER_FAILED;

	// bind the buffer
	Bind();

	if (init)
	{
		// make sure that it is allocated
		m_Rend->gl.TexStorage3D(GL_TEXTURE_CUBE_MAP, (GLsizei)m_MipCount, m_Rend->GLInternalFormat(m_Type), (GLsizei)m_Width, (GLsizei)m_Height, (GLsizei)m_Depth);
	}

	switch (face)
	{
		case ECubeFace::POS_X:
		case ECubeFace::NEG_X:
			lockinfo.width = m_Depth >> mip;
			lockinfo.height = m_Height >> mip;
			break;

		case ECubeFace::POS_Y:
		case ECubeFace::NEG_Y:
			lockinfo.width = m_Width >> mip;
			lockinfo.height = m_Depth >> mip;
			break;

		case ECubeFace::POS_Z:
		case ECubeFace::NEG_Z:
			lockinfo.width = m_Width >> mip;
			lockinfo.height = m_Height >> mip;
			break;
	}

	lockinfo.stride = lockinfo.width * m_Rend->PixelSize(m_Type);

	m_Buffer = malloc(lockinfo.height * lockinfo.stride);
	if (!m_Buffer)
		return RET_MAPBUFFER_FAILED;

	m_LockMip = mip;
	m_LockWidth = lockinfo.width;
	m_LockHeight = lockinfo.height;
	m_LockFace = face;

	if (flags.IsSet(TEXLOCKFLAG_READ))
	{
		m_Rend->gl.GetTexImage(GLface[m_LockFace], (GLsizei)mip, m_Rend->GLFormat(m_Type), m_Rend->GLType(m_Type), m_Buffer);
	}

	*buffer = m_Buffer;

	return RET_OK;
}


Texture::RETURNCODE TextureCubeImpl::Unlock()
{
	assert(m_Rend);

	if (!m_Buffer)
		return RET_NOT_LOCKED;

	Bind();

	m_Rend->gl.TexSubImage2D(GLface[m_LockFace], (GLint)m_LockMip, 0, 0, (GLsizei)m_LockWidth, (GLsizei)m_LockHeight, m_Rend->GLFormat(m_Type), m_Rend->GLType(m_Type), m_Buffer);

	free(m_Buffer);
	m_Buffer = nullptr;

	return RET_OK;
}


// *******************************************************************************
// *******************************************************************************


Texture3DImpl::Texture3DImpl(RendererImpl *prend, size_t width, size_t height, size_t depth, Renderer::ETextureType type, size_t mipcount, props::TFlags64 flags)
{
	assert(prend);

	m_Rend = prend;

	m_Width = std::max(width, size_t(1));
	m_Height = std::max(height, size_t(1));
	m_Depth = std::max(depth, size_t(1));

	if (mipcount < 1)
	{
		mipcount = 0;
		size_t mr = std::min(width, height);
		while (mr)
		{
			mr >>= 1;
			mipcount++;
		}
	}

	m_MipCount = mipcount;
	m_Type = type;
	m_Buffer = nullptr;
	m_glID = GL_INVALID_VALUE;
	m_LockMip = 0;
}


Texture3DImpl::~Texture3DImpl()
{
	assert(m_Rend);

	if (m_Buffer)
	{
		free(m_Buffer);
		m_Buffer = nullptr;
	}

	if (m_glID != GL_INVALID_VALUE)
	{
		m_Rend->gl.DeleteTextures(1, &m_glID);
	}
}


void Texture3DImpl::Release()
{
	delete this;
}


Renderer::ETextureType Texture3DImpl::Format()
{
	return m_Type;
}


size_t Texture3DImpl::MipCount()
{
	return m_MipCount;
}


size_t Texture3DImpl::Width()
{
	return m_Width;
}


size_t Texture3DImpl::Height()
{
	return m_Height;
}


size_t Texture3DImpl::Depth()
{
	return m_Depth;
}


void Texture3DImpl::Bind()
{
	assert(m_Rend);

	if (m_glID != GL_INVALID_VALUE)
	{
		m_Rend->gl.BindTexture(GL_TEXTURE_3D, m_glID);
	}
}


void Texture3DImpl::Unbind()
{
	assert(m_Rend);

	m_Rend->gl.BindTexture(GL_TEXTURE_3D, 0);
}


Texture::RETURNCODE Texture3DImpl::Lock(void **buffer, Texture3D::SLockInfo &lockinfo, size_t mip, props::TFlags64 flags)
{
	assert(m_Rend);

	if (m_Buffer)
		return RET_ALREADY_LOCKED;

	if (!buffer)
		return RET_NULL_BUFFER;

	bool init = false;
	if (m_glID == GL_INVALID_VALUE)
	{
		m_Rend->gl.CreateTextures(GL_TEXTURE_3D, 1, &m_glID);
		init = true;
	}

	if (m_glID == GL_INVALID_VALUE)
		return RET_GENBUFFER_FAILED;

	// bind the buffer
	Bind();

	if (init)
	{
		// make sure that it is allocated
		m_Rend->gl.TexStorage3D(GL_TEXTURE_3D, (GLsizei)m_MipCount, m_Rend->GLInternalFormat(m_Type), (GLsizei)m_Width, (GLsizei)m_Height, (GLsizei)m_Depth);
	}

	lockinfo.width = m_Width >> mip;
	lockinfo.height = m_Height >> mip;
	lockinfo.depth = m_Depth >> mip;
	lockinfo.stride_w = lockinfo.width;
	lockinfo.stride_h = lockinfo.height;

	m_Buffer = malloc(lockinfo.stride_w * lockinfo.stride_h * m_Depth * m_Rend->PixelSize(m_Type));
	if (!m_Buffer)
		return RET_MAPBUFFER_FAILED;

	m_LockMip = mip;

	if (flags.IsSet(TEXLOCKFLAG_READ))
	{
		m_Rend->gl.GetTexImage(GL_TEXTURE_3D, (GLsizei)mip, m_Rend->GLFormat(m_Type), m_Rend->GLType(m_Type), m_Buffer);
	}

	*buffer = m_Buffer;

	return RET_OK;
}


Texture::RETURNCODE Texture3DImpl::Unlock()
{
	assert(m_Rend);

	if (!m_Buffer)
		return RET_NOT_LOCKED;

	Bind();

	m_Rend->gl.TexSubImage3D(GL_TEXTURE_3D, (GLint)m_LockMip, 0, 0, 0, (GLsizei)m_Width, (GLsizei)m_Height, (GLsizei)m_Depth, m_Rend->GLFormat(m_Type), m_Rend->GLType(m_Type), m_Buffer);

	free(m_Buffer);
	m_Buffer = nullptr;

	return RET_OK;
}
