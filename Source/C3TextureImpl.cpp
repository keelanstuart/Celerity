// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3TextureImpl.h>
#include <C3Resource.h>

#include <stb_image.h>
#include <Shlwapi.h>
#include <ddraw.h>
#include <tiffio.h>


using namespace c3;


Texture2DImpl::Texture2DImpl(RendererImpl *prend, size_t width, size_t height, Renderer::ETextureType type, size_t mipcount, props::TFlags64 flags)
{
	assert(prend);

	m_Rend = prend;

	m_Width = std::max(width, size_t(1));
	m_Height = std::max(height, size_t(1));

	if (!flags.IsSet(TEXCREATEFLAG_RENDERTARGET) && (mipcount < 1))
	{
		mipcount = 0;
		size_t mr = std::min(width, height);
		while (mr)
		{
			mr >>= 1;
			mipcount++;
		}
	}

	m_MipCount = flags.IsSet(TEXCREATEFLAG_RENDERTARGET) ? 1 : mipcount;
	m_Type = type;
	m_Buffer = nullptr;
	m_glID = NULL;
	m_LockMip = 0;
	m_Flags = 0;

	if (flags.IsSet(TEXCREATEFLAG_RENDERTARGET))
	{
		m_Rend->gl.GenTextures(1, &m_glID);

		m_Rend->gl.BindTexture(GL_TEXTURE_2D, m_glID);

		m_Rend->gl.TexStorage2D(GL_TEXTURE_2D, (GLsizei)m_MipCount, m_Rend->GLInternalFormat(m_Type), (GLsizei)m_Width, (GLsizei)m_Height);

		m_Rend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		m_Rend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		m_Rend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		m_Rend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		m_Rend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	m_Rend->gl.BindTexture(GL_TEXTURE_2D, 0);
}


Texture2DImpl::~Texture2DImpl()
{
	assert(m_Rend);

	if (m_Buffer)
	{
		free(m_Buffer);
		m_Buffer = nullptr;
	}

	if (m_glID != NULL)
	{
		m_Rend->gl.DeleteTextures(1, &m_glID);
	}
}


void Texture2DImpl::Release()
{
	delete this;
}


void Texture2DImpl::SetName(const TCHAR *name)
{
	m_Name = name ? name : _T("");
}


const TCHAR *Texture2DImpl::GetName() const
{
	return m_Name.c_str();
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

	if (m_glID != NULL)
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
	if (!buffer)
		return RET_NULL_BUFFER;

	assert(m_Rend);

	if (flags.IsSet(IBLOCKFLAG_READ) && !flags.IsSet(IBLOCKFLAG_WRITE) && !m_Cache.empty())
	{
		*buffer = m_Cache.data();
		return RET_OK;
	}
		
	if (m_Buffer)
		return RET_ALREADY_LOCKED;

	bool init = false;
	if (m_glID == NULL)
	{
		m_Rend->gl.CreateTextures(GL_TEXTURE_2D, 1, &m_glID);

		// if we can't get a gl 4.5 context, then we may have to use gen instead of create
		if (m_glID == NULL)
		{
			m_Rend->gl.GenTextures(1, &m_glID);
		}

		init = true;
	}

	if (m_glID == NULL)
		return RET_GENBUFFER_FAILED;

	bool update_now = flags.IsSet(TEXLOCKFLAG_UPDATENOW);
	bool user_buffer = flags.IsSet(TEXLOCKFLAG_USERBUFFER);
	if (update_now && !user_buffer)
		return RET_UPDATENOW_NEEDS_USERBUFFER;

	// bind the buffer
	Bind();

	if (init)
	{
		// make sure that it is allocated
		m_Rend->gl.TexStorage2D(GL_TEXTURE_2D, (GLsizei)m_MipCount, m_Rend->GLInternalFormat(m_Type), (GLsizei)m_Width, (GLsizei)m_Height);

		GLenum wrapmode_u = GL_CLAMP_TO_EDGE;
		if (m_Flags.IsSet(TEXFLAG_WRAP_U))
			wrapmode_u = GL_REPEAT;
		else if (m_Flags.IsSet(TEXFLAG_MIRROR_U))
			wrapmode_u = GL_MIRROR_CLAMP_TO_EDGE;
		else if (m_Flags.IsSet(TEXFLAG_WRAP_U | TEXFLAG_MIRROR_U))
			wrapmode_u = GL_MIRRORED_REPEAT;

		GLenum wrapmode_v = GL_CLAMP_TO_EDGE;
		if (m_Flags.IsSet(TEXFLAG_WRAP_V))
			wrapmode_v = GL_REPEAT;
		else if (m_Flags.IsSet(TEXFLAG_MIRROR_V))
			wrapmode_v = GL_MIRROR_CLAMP_TO_EDGE;
		else if (m_Flags.IsSet(TEXFLAG_WRAP_V | TEXFLAG_MIRROR_V))
			wrapmode_v = GL_MIRRORED_REPEAT;

		m_Rend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapmode_u);
		m_Rend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapmode_v);

		m_Rend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (m_MipCount > 0) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
		m_Rend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	lockinfo.width = m_Width >> mip;
	lockinfo.height = m_Height >> mip;
	lockinfo.stride = lockinfo.width * m_Rend->PixelSize(m_Type);

	if (flags.IsSet(TEXLOCKFLAG_CACHE))
	{
		m_Cache.resize(lockinfo.height * lockinfo.stride);
		m_Buffer = m_Cache.data();
	}
	else
	{
		if (!m_Cache.empty())
			m_Cache.clear();

		m_Buffer = malloc(lockinfo.height * lockinfo.stride);
	}

	if (!m_Buffer)
		return RET_MAPBUFFER_FAILED;

	m_LockMip = mip;

	if (flags.IsSet(TEXLOCKFLAG_READ))
	{
		if (m_Type < Renderer::TextureType::DXT1)
			m_Rend->gl.GetTexImage(GL_TEXTURE_2D, (GLsizei)mip, m_Rend->GLFormat(m_Type), m_Rend->GLType(m_Type), m_Buffer);
		else
			m_Rend->gl.GetCompressedTexImage(GL_TEXTURE_2D, (GLsizei)mip, m_Buffer);
	}

	if (flags.IsSet(TEXLOCKFLAG_GENMIPS))
	{
		m_Flags.Set(TEXFLAG_GENMIPS);
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
	if (m_Type < Renderer::TextureType::DXT1)
	{
		GLenum fmt = m_Rend->GLFormat(m_Type);
		GLenum type = m_Rend->GLType(m_Type);
		m_Rend->gl.TexSubImage2D(GL_TEXTURE_2D, (GLint)m_LockMip, 0, 0, GLsizei(m_Width >> m_LockMip), GLsizei(m_Height >> m_LockMip), fmt, type, m_Buffer);
	}
	else
	{
		m_Rend->gl.CompressedTexSubImage2D(GL_TEXTURE_2D, (GLint)m_LockMip, 0, 0, GLsizei(m_Width >> m_LockMip), GLsizei(m_Height >> m_LockMip), m_Rend->GLFormat(m_Type), m_Rend->GLType(m_Type), m_Buffer);
	}

	if (m_Cache.data() != m_Buffer)
		free(m_Buffer);

	m_Buffer = nullptr;

	if (m_Flags.IsSet(TEXFLAG_GENMIPS))
	{
		m_Rend->gl.GenerateMipmap(GL_TEXTURE_2D);
		m_Flags.Clear(TEXFLAG_GENMIPS);
	}

	return RET_OK;
}


DECLARE_RESOURCETYPE(DefaultTexture2D);


c3::ResourceType::LoadResult RESOURCETYPENAME(DefaultTexture2D)::ReadFromFile(c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const
{
	c3::ResourceType::LoadResult ret = c3::ResourceType::LR_ERROR;

	if (returned_data)
	{
		*returned_data = nullptr;

		int width = 0;
		int height = 0;
		int numchannels = 0;
		int bpp = 1;

		const TCHAR* ext = PathFindExtension(filename);

		if (!_tcsicmp(ext, _T(".tif")) || !_tcsicmp(ext, _T(".tiff")))
		{
#ifdef UNICODE
			TIFF* tif = TIFFOpenW(filename, "r");
#else
			char* _filename;
			CONVERT_TCS2MBCS(filename, _filename);

			TIFF* tif = TIFFOpen(_filename, "r");
#endif
			if (tif)
			{
				TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
				TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
				TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &numchannels);
				TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bpp);
				bpp >>= 3;

				int photomode;
				TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photomode);

				Renderer::TextureType tt = Renderer::TextureType(Renderer::TextureType::U8_1CH + numchannels - 1);
				Texture2D* ptex = nullptr;
				uint8_t* dst = nullptr;
				Texture2D::SLockInfo li;

				c3::RendererImpl* pr = (c3::RendererImpl*)psys->GetRenderer();
				ptex = pr->CreateTexture2D(width, height, tt, 0, TEXFLAG_WRAP_U | TEXFLAG_WRAP_V | TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPLINEAR);

				if (ptex)
				{
					ptex->Lock((void**)&dst, li, 0, TEXLOCKFLAG_WRITE | TEXLOCKFLAG_GENMIPS);

					if (dst)
					{
						if ((bpp == 1) && (numchannels == 4))
						{
							TIFFReadRGBAImage(tif, width, height, (uint32_t *)dst);
						}
						else
						{
							unsigned char* dst_row = dst;
							int ofs = width * numchannels * bpp;

							for (uint32_t y = 0; y < (unsigned int)height; y++)
							{
								TIFFReadScanline(tif, dst_row, y);
								dst_row += ofs;
							}
						}

						ptex->Unlock();

						ptex->SetName(filename);

						ret = ResourceType::LoadResult::LR_SUCCESS;
					}
				}

				TIFFClose(tif);

				*returned_data = ptex;
			}
		}
		else if (!_tcsicmp(ext, _T(".dds")))
		{
			// load dds
		}
		else // all other formats are read by STB
		{
			char *_filename;
			CONVERT_TCS2MBCS(filename, _filename);

			if (stbi_info(_filename, &width, &height, &numchannels))
			{
				Renderer::TextureType tt = Renderer::TextureType(Renderer::TextureType::U8_1CH + numchannels - 1);
				Texture2D *ptex = nullptr;
				uint8_t *dst = nullptr;
				Texture2D::SLockInfo li;

				c3::RendererImpl *pr = (c3::RendererImpl *)psys->GetRenderer();
				ptex = pr->CreateTexture2D(width, height, tt, 0, TEXFLAG_WRAP_U | TEXFLAG_WRAP_V | TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPLINEAR);

				if (ptex)
				{
					ptex->Lock((void **)&dst, li, 0, TEXLOCKFLAG_WRITE | TEXLOCKFLAG_GENMIPS);

					if (dst)
					{
						stbi_uc *src = stbi_load(_filename, &width, &height, &numchannels, 0);
						if (src)
						{
							unsigned char *dst_row = dst;
							int ofs = width * numchannels * bpp;
							unsigned char *src_row = src + (width * height * numchannels) - ofs;

							for (size_t y = 0; y < height; y++)
							{
								memcpy(dst_row, src_row, width * numchannels);
								src_row -= ofs;
								dst_row += ofs;
							}

							ptex->Unlock();

							free(src);
						}

						ptex->SetName(filename);

						ret = ResourceType::LoadResult::LR_SUCCESS;
					}
				}

				*returned_data = ptex;
			}
		}
	}

	return ret;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(DefaultTexture2D)::ReadFromMemory(c3::System *psys, const TCHAR *contextname, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
{
	c3::ResourceType::LoadResult ret = c3::ResourceType::LR_ERROR;

	if (returned_data)
	{
		*returned_data = nullptr;

		int width = 0;
		int height = 0;
		int numchannels = 0;

		if (stbi_info_from_memory((const stbi_uc *)buffer, (int)buffer_length, &width, &height, &numchannels))
		{
#if defined(TEX2D_REQUIRE_POW2) && TEX2D_REQUIRE_POW2
			// only allow power of 2 dimensions......?
			if (((width & (width - 1)) == 0) && ((height & (height - 1)) == 0))
#endif
			{
				Renderer::TextureType tt = Renderer::TextureType(Renderer::TextureType::U8_1CH + numchannels - 1);
				Texture2D *ptex = nullptr;
				uint8_t *dst = nullptr;
				Texture2D::SLockInfo li;

				c3::RendererImpl *pr = (c3::RendererImpl *)psys->GetRenderer();
				ptex = pr->CreateTexture2D(width, height, tt, 0, TEXFLAG_WRAP_U | TEXFLAG_WRAP_V | TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPLINEAR);

				if (ptex)
				{
					ptex->SetName(contextname);

					ptex->Lock((void **)&dst, li, 0, TEXLOCKFLAG_WRITE | TEXLOCKFLAG_GENMIPS);

					if (dst)
					{
						stbi_uc *src = stbi_load_from_memory((const stbi_uc *)buffer, (int)buffer_length, &width, &height, &numchannels, 0);
						if (src)
						{
							unsigned char *dst_row = dst;
							int src_ofs = width * numchannels;
							unsigned char *src_row = src + (width * height * numchannels) - src_ofs;

							for (size_t y = 0; y < height; y++)
							{
								memcpy(dst_row, src_row, width * numchannels);
								src_row -= src_ofs;
								dst_row += src_ofs;
							}

							ptex->Unlock();

							free(src);
						}

						ret = ResourceType::LoadResult::LR_SUCCESS;
					}
				}

				*returned_data = ptex;
			}
#if defined(TEX2D_REQUIRE_POW2) && TEX2D_REQUIRE_POW2
			else
			{
				psys->GetLog()->Print(_T("\nTexture2D: non-power-of-2 texture found ... not loading for safety reasons. (SIGH) \"%s\"\n"), filename);
			}
#endif
		}

	}

	return ret;
}


bool RESOURCETYPENAME(DefaultTexture2D)::WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const
{
	return false;
}


void RESOURCETYPENAME(DefaultTexture2D)::Unload(void *data) const
{
	((Texture2D *)data)->Release();
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
	m_glID = NULL;
	m_LockMip = 0;
	m_Flags = 0;
}


TextureCubeImpl::~TextureCubeImpl()
{
	assert(m_Rend);

	if (m_Buffer)
	{
		free(m_Buffer);
		m_Buffer = nullptr;
	}

	if (m_glID != NULL)
	{
		m_Rend->gl.DeleteTextures(1, &m_glID);
	}
}


void TextureCubeImpl::Release()
{
	delete this;
}


void TextureCubeImpl::SetName(const TCHAR *name)
{
	m_Name = name;
}


const TCHAR *TextureCubeImpl::GetName() const
{
	return m_Name.c_str();
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

	if (m_glID != NULL)
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
	if (m_glID == NULL)
	{
		m_Rend->gl.CreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_glID);

		// if we can't get a gl 4.5 context, then we may have to use gen instead of create
		if (m_glID == NULL)
		{
			m_Rend->gl.GenTextures(1, &m_glID);
		}

		init = true;
	}

	if (m_glID == NULL)
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

	if (flags.IsSet(TEXLOCKFLAG_GENMIPS))
	{
		m_Flags.Set(TEXFLAG_GENMIPS);
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

	if (m_Flags.IsSet(TEXFLAG_GENMIPS))
	{
		m_Rend->gl.GenerateMipmap(GL_TEXTURE_CUBE_MAP);
		m_Flags.Clear(TEXFLAG_GENMIPS);
	}

	return RET_OK;
}


DECLARE_RESOURCETYPE(DefaultTextureCube);

c3::ResourceType::LoadResult RESOURCETYPENAME(DefaultTextureCube)::ReadFromFile(c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const
{
	if (returned_data)
	{
		*returned_data = nullptr; //psys->GetRenderer()->CreateTextureCubeFromFile(filename, TEXFLAG_WRAP_U | TEXFLAG_WRAP_V | TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPLINEAR);
		if (!*returned_data)
			return ResourceType::LoadResult::LR_ERROR;
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(DefaultTextureCube)::ReadFromMemory(c3::System *psys, const TCHAR *contextname, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
{
	return ResourceType::LoadResult::LR_ERROR;
}


bool RESOURCETYPENAME(DefaultTextureCube)::WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const
{
	return false;
}


void RESOURCETYPENAME(DefaultTextureCube)::Unload(void *data) const
{
	((TextureCube *)data)->Release();
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
	m_glID = NULL;
	m_LockMip = 0;
	m_Flags = 0;
}


Texture3DImpl::~Texture3DImpl()
{
	assert(m_Rend);

	if (m_Buffer)
	{
		free(m_Buffer);
		m_Buffer = nullptr;
	}

	if (m_glID != NULL)
	{
		m_Rend->gl.DeleteTextures(1, &m_glID);
	}
}


void Texture3DImpl::Release()
{
	delete this;
}


void Texture3DImpl::SetName(const TCHAR *name)
{
	m_Name = name;
}


const TCHAR *Texture3DImpl::GetName() const
{
	return m_Name.c_str();
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

	if (m_glID != NULL)
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
	if (m_glID == NULL)
	{
		m_Rend->gl.CreateTextures(GL_TEXTURE_3D, 1, &m_glID);

		// if we can't get a gl 4.5 context, then we may have to use gen instead of create
		if (m_glID == NULL)
		{
			m_Rend->gl.GenTextures(1, &m_glID);
		}

		init = true;
	}

	if (m_glID == NULL)
		return RET_GENBUFFER_FAILED;

	// bind the buffer
	Bind();

	if (init)
	{
		// make sure that it is allocated
		m_Rend->gl.TexStorage3D(GL_TEXTURE_3D, (GLsizei)m_MipCount, m_Rend->GLInternalFormat(m_Type), (GLsizei)m_Width, (GLsizei)m_Height, (GLsizei)m_Depth);

		GLenum wrapmode_u = GL_CLAMP_TO_EDGE;
		if (m_Flags.IsSet(TEXFLAG_WRAP_U))
			wrapmode_u = GL_REPEAT;
		else if (m_Flags.IsSet(TEXFLAG_MIRROR_U))
			wrapmode_u = GL_MIRROR_CLAMP_TO_EDGE;
		else if (m_Flags.IsSet(TEXFLAG_WRAP_U | TEXFLAG_MIRROR_U))
			wrapmode_u = GL_MIRRORED_REPEAT;

		GLenum wrapmode_v = GL_CLAMP_TO_EDGE;
		if (m_Flags.IsSet(TEXFLAG_WRAP_V))
			wrapmode_v = GL_REPEAT;
		else if (m_Flags.IsSet(TEXFLAG_MIRROR_V))
			wrapmode_v = GL_MIRROR_CLAMP_TO_EDGE;
		else if (m_Flags.IsSet(TEXFLAG_WRAP_V | TEXFLAG_MIRROR_V))
			wrapmode_v = GL_MIRRORED_REPEAT;

		GLenum wrapmode_w = GL_CLAMP_TO_EDGE;
		if (m_Flags.IsSet(TEXFLAG_WRAP_W))
			wrapmode_w = GL_REPEAT;
		else if (m_Flags.IsSet(TEXFLAG_MIRROR_W))
			wrapmode_w = GL_MIRROR_CLAMP_TO_EDGE;
		else if (m_Flags.IsSet(TEXFLAG_WRAP_W | TEXFLAG_MIRROR_W))
			wrapmode_w = GL_MIRRORED_REPEAT;

		m_Rend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapmode_u);
		m_Rend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapmode_v);
		m_Rend->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrapmode_w);
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

	if (flags.IsSet(TEXLOCKFLAG_GENMIPS))
	{
		m_Flags.Set(TEXFLAG_GENMIPS);
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

	if (m_Flags.IsSet(TEXFLAG_GENMIPS))
	{
		m_Rend->gl.GenerateMipmap(GL_TEXTURE_3D);
		m_Flags.Clear(TEXFLAG_GENMIPS);
	}

	return RET_OK;
}


DECLARE_RESOURCETYPE(DefaultTexture3D);

c3::ResourceType::LoadResult RESOURCETYPENAME(DefaultTexture3D)::ReadFromFile(c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const
{
	if (returned_data)
	{
		*returned_data = nullptr; //psys->GetRenderer()->CreateTexture2DFromFile(filename, TEXFLAG_WRAP_U | TEXFLAG_WRAP_V | TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPLINEAR);
		if (!*returned_data)
			return ResourceType::LoadResult::LR_ERROR;
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(DefaultTexture3D)::ReadFromMemory(c3::System *psys, const TCHAR *contextname, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
{
	return ResourceType::LoadResult::LR_ERROR;
}


bool RESOURCETYPENAME(DefaultTexture3D)::WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const
{
	return false;
}


void RESOURCETYPENAME(DefaultTexture3D)::Unload(void *data) const
{
	((Texture3D *)data)->Release();
}
