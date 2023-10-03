// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3Texture.h>
#include <C3RendererImpl.h>
#include <C3Resource.h>

namespace c3
{

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

	DEFINE_RESOURCETYPE(Texture2D, RTFLAG_RUNBYRENDERER, GUID({0xd9afb0b6, 0x8eea, 0x4bdc, { 0x97, 0x68, 0x4f, 0x3d, 0x55, 0x69, 0x35, 0x4c }}), "Texture2D", "Two-Dimensional Textures", "png;jpg;tga;bmp;gif;psd;hdr;pic;pnm;dds", "tga");

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

	DEFINE_RESOURCETYPE(TextureCube, RTFLAG_RUNBYRENDERER, GUID({0xf46f85a2, 0x45da, 0x4fc1, {0x81, 0x4a, 0xee, 0x9a, 0x67, 0xc4, 0x3e, 0x9f}}), "TextureCube", "Cube Textures", "dds", "dds");

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

	DEFINE_RESOURCETYPE(Texture3D, RTFLAG_RUNBYRENDERER, GUID({0x93927dc6, 0xe335, 0x4728, {0x87, 0x55, 0x36, 0x7c, 0xc9, 0xc, 0x34, 0x3d}}), "Texture3D", "Volumetric Textures", "dds", "dds");

};