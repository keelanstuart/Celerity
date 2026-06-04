// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3Texture.h>
#include <C3RendererImpl.h>
#include <C3Resource.h>

namespace c3
{

	DEFINE_RESOURCETYPE(Texture, RTFLAG_RUNBYRENDERER,
		GUID({ 0xd9afb0b6, 0x8eea, 0x4bdc, { 0x97, 0x68, 0x4f, 0x3d, 0x55, 0x69, 0x35, 0x4c } }),
		"Texture", "Textures");
	
	DEFINE_RESOURCECODEC(Texture2D, Texture, RTFLAG_RUNBYRENDERER,
		GUID({ 0xd1d7344f, 0x2857, 0x441d, { 0xbd, 0x77, 0x7d, 0x94, 0x71, 0x7c, 0xc0, 0x8e } }),
		"Texture2D", "2D Textures",
		"png;jpg;tga;bmp;gif;psd;hdr;pic;pnm;dds;tif;tiff", "tga", 0);

	DEFINE_RESOURCECODEC(TextureCube, Texture, RTFLAG_RUNBYRENDERER,
		GUID({ 0x5644209e, 0x72da, 0x46b4, { 0x93, 0xad, 0x60, 0xa0, 0xbc, 0x60, 0x25, 0x7b } }),
		"TextureCube", "Cube Textures",
		"dds", "dds", 1);

	DEFINE_RESOURCECODEC(Texture3D, Texture, RTFLAG_RUNBYRENDERER,
		GUID({ 0x5f040bd, 0x361c, 0x47ee, { 0x96, 0xc8, 0x44, 0xa6, 0xcf, 0x41, 0xdb, 0xf } }),
		"Texture3D", "3D Textures",
		"dds", "dds", 2);


	// PRIVATE don't use in UseTexture's texflags
	#define TEXFLAG_GENMIPS			0x80000000


	class Texture2DImpl : public Texture2D
	{

	protected:
		tstring m_Name;
		Renderer::ETextureType m_Type;
		size_t m_Width, m_Height;
		props::TFlags64 m_Flags;
		size_t m_MipCount;
		int64_t m_SamplerID;
		GLuint m_glID;
		void *m_Buffer;
		std::vector<uint8_t> m_Cache;
		size_t m_LockMip;
		RendererImpl *m_Rend;

	public:
		Texture2DImpl(RendererImpl *prend, size_t width, size_t height, Renderer::ETextureType type, size_t mipcount, props::TFlags64 flags);
		virtual ~Texture2DImpl();

		virtual void Release();

		virtual void SetName(const TCHAR *name);

		virtual const TCHAR *GetName() const;

		virtual Renderer::ETextureType Format();

		virtual size_t MipCount();

		virtual size_t Width();

		virtual size_t Height();

		virtual void Bind();

		virtual void Unbind();

		virtual Texture::RETURNCODE Lock(void **buffer, Texture2D::SLockInfo &lockinfo, size_t mip, props::TFlags64 flags);

		virtual Texture::RETURNCODE Unlock();

		operator GLuint() const { return m_glID; }

	};


	class TextureCubeImpl : public TextureCube
	{

	protected:
		tstring m_Name;
		Renderer::ETextureType m_Type;
		size_t m_Width, m_Height, m_Depth;
		props::TFlags64 m_Flags;
		size_t m_MipCount;
		GLuint m_glID;
		int64_t m_SamplerID;
		void *m_Buffer;
		size_t m_LockMip;
		TextureCube::CubeFace m_LockFace;
		size_t m_LockWidth;
		size_t m_LockHeight;
		RendererImpl *m_Rend;

	public:
		TextureCubeImpl(RendererImpl *prend, size_t width, size_t height, size_t depth, Renderer::ETextureType type, size_t mipcount, props::TFlags64 flags);
		virtual ~TextureCubeImpl();

		virtual void Release();

		virtual void SetName(const TCHAR *name);

		virtual const TCHAR *GetName() const;

		virtual Renderer::ETextureType Format();

		virtual size_t MipCount();

		virtual size_t Width();

		virtual size_t Height();

		virtual size_t Depth();

		virtual void Bind();

		virtual void Unbind();

		virtual Texture::RETURNCODE Lock(void **buffer, CubeFace face, TextureCube::SLockInfo &lockinfo, size_t mip, props::TFlags64 flags);

		virtual Texture::RETURNCODE Unlock();

		operator GLuint() const { return m_glID; }

	};


	class Texture3DImpl : public Texture3D
	{

	protected:
		tstring m_Name;
		Renderer::ETextureType m_Type;
		size_t m_Width, m_Height, m_Depth;
		props::TFlags64 m_Flags;
		size_t m_MipCount;
		int64_t m_SamplerID;
		GLuint m_glID;
		void *m_Buffer;
		size_t m_LockMip;
		RendererImpl *m_Rend;

	public:
		Texture3DImpl(RendererImpl *prend, size_t width, size_t height, size_t depth, Renderer::ETextureType type, size_t mipcount, props::TFlags64 flags);
		virtual ~Texture3DImpl();

		virtual void Release();

		virtual void SetName(const TCHAR *name);

		virtual const TCHAR *GetName() const;

		virtual Renderer::ETextureType Format();

		virtual size_t MipCount();

		virtual size_t Width();

		virtual size_t Height();

		virtual size_t Depth();

		virtual void Bind();

		virtual void Unbind();

		virtual Texture::RETURNCODE Lock(void **buffer, Texture3D::SLockInfo &lockinfo, size_t mip, props::TFlags64 flags);

		virtual Texture::RETURNCODE Unlock();

		operator GLuint() const { return m_glID; }

	};

};