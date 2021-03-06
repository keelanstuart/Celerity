// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2020, Keelan Stuart


#pragma once

#include <C3Texture.h>
#include <C3RendererImpl.h>
#include <C3Resource.h>

namespace c3
{

	#define TEXFLAG_WRAP_U			0x00000001
	#define TEXFLAG_MIRROR_U		0x00000002
	#define TEXFLAG_WRAP_V			0x00000100
	#define TEXFLAG_MIRROR_V		0x00000200
	#define TEXFLAG_WRAP_W			0x00010000
	#define TEXFLAG_MIRROR_W		0x00020000

	#define TEXFLAG_GENMIPS			0x00100000

	#define TEXLOCKFLAG_UPDATENOW	0x0001
	#define TEXLOCKFLAG_USERBUFFER	0x0002

	class Texture2DImpl : public Texture2D
	{

	protected:
		RendererImpl *m_Rend;
		GLuint m_glID;
		size_t m_Width, m_Height;
		size_t m_MipCount;
		Renderer::ETextureType m_Type;
		void *m_Buffer;
		size_t m_LockMip;
		props::TFlags64 m_Flags;

	public:
		Texture2DImpl(RendererImpl *prend, size_t width, size_t height, Renderer::ETextureType type, size_t mipcount, props::TFlags64 flags);
		virtual ~Texture2DImpl();

		virtual void Release();

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

	DEFINE_RESOURCETYPE(Texture2D, RTFLAG_RUNBYRENDERER, GUID({0xd9afb0b6, 0x8eea, 0x4bdc, { 0x97, 0x68, 0x4f, 0x3d, 0x55, 0x69, 0x35, 0x4c }}), "Texture2D", "Two-Dimensional Textures", "png;jpg;tga;bmp;gif;psd;hdr;pic;pnm", "tga");

	class TextureCubeImpl : public TextureCube
	{

	protected:
		RendererImpl *m_Rend;
		GLuint m_glID;
		size_t m_Width, m_Height, m_Depth;
		size_t m_MipCount;
		Renderer::ETextureType m_Type;
		void *m_Buffer;
		size_t m_LockMip;
		TextureCube::CubeFace m_LockFace;
		size_t m_LockWidth;
		size_t m_LockHeight;
		props::TFlags64 m_Flags;

	public:
		TextureCubeImpl(RendererImpl *prend, size_t width, size_t height, size_t depth, Renderer::ETextureType type, size_t mipcount, props::TFlags64 flags);
		virtual ~TextureCubeImpl();

		virtual void Release();

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
		RendererImpl *m_Rend;
		GLuint m_glID;
		size_t m_Width, m_Height, m_Depth;
		size_t m_MipCount;
		Renderer::ETextureType m_Type;
		void *m_Buffer;
		size_t m_LockMip;
		props::TFlags64 m_Flags;

	public:
		Texture3DImpl(RendererImpl *prend, size_t width, size_t height, size_t depth, Renderer::ETextureType type, size_t mipcount, props::TFlags64 flags);
		virtual ~Texture3DImpl();

		virtual void Release();

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