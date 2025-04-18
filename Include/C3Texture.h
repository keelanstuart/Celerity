// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Renderer.h>

namespace c3
{

	class Texture
	{

	public:

		using RETURNCODE = enum
		{
			RET_OK = 0,
			RET_ALREADY_LOCKED,
			RET_NOT_LOCKED,
			RET_NULL_BUFFER,
			RET_GENBUFFER_FAILED,
			RET_MAPBUFFER_FAILED,
			RET_UPDATENOW_NEEDS_USERBUFFER,
		};

		virtual void Release() = NULL;

		/// Sets the name of the Texture - this is done automatically for resources, but can be done manually in other cases
		virtual void SetName(const TCHAR *name) = NULL;

		/// Gets the name of the Texture
		virtual const TCHAR *GetName() const = NULL;

		/// Describes the type and arrangement of the texels
		virtual Renderer::ETextureType Format() = NULL;

		/// The number of mip levels this texture has
		virtual size_t MipCount() = NULL;

		/// The X length of the texture
		virtual size_t Width() = NULL;

		/// The Y length of the texture
		virtual size_t Height() = NULL;

		/// Attaches the texture to the render context
		virtual void Bind() = NULL;

		virtual void Unbind() = NULL;

		/// Call to end updates to the texture and update video memory. This mechanism is common to all texture types.
		virtual RETURNCODE Unlock() = NULL;
	};

	#define TEXCREATEFLAG_RENDERTARGET			0x00000001

	#define TEXLOCKFLAG_READ					0x00000001
	#define TEXLOCKFLAG_WRITE					0x00000002
	#define TEXLOCKFLAG_GENMIPS					0x00000004
	#define TEXLOCKFLAG_UPDATENOW				0x00000008
	#define TEXLOCKFLAG_USERBUFFER				0x00000010
	#define TEXLOCKFLAG_CACHE					0x00000020

	// combine these flags...
	// TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPNEAREST = GL_LINEAR_MIPMAP_NEAREST
	// TEXFLAG_MINFILTER_MIPLINEAR = GL_NEAREST_MIPMAP_LINEAR
	#define TEXFLAG_WRAP_U						0x00000001
	#define TEXFLAG_MIRROR_U					0x00000002
	#define TEXFLAG_WRAP_V						0x00000004
	#define TEXFLAG_MIRROR_V					0x00000008
	#define TEXFLAG_WRAP_W						0x00000010
	#define TEXFLAG_MIRROR_W					0x00000020
	#define TEXFLAG_MAGFILTER_LINEAR			0x00000040
	#define TEXFLAG_MINFILTER_LINEAR			0x00000080
	#define TEXFLAG_MINFILTER_MIPLINEAR			0x00000100
	#define TEXFLAG_RESOURCE					0x00000200

	class Texture2D : public Texture
	{

	public:

		typedef struct sLockInfo
		{
			size_t width;
			size_t height;
			size_t stride;		// The number of bytes, not pixels, between lines

		} SLockInfo;

		/// Call to begin updating texture; fill out the returned buffer and call unlock to finalize the update
		virtual Texture::RETURNCODE Lock(void **buffer, SLockInfo &lockinfo, size_t mip = 0, props::TFlags64 flags = 0) = NULL;

	};


	class TextureCube : public Texture
	{

	public:

		typedef struct sLockInfo
		{
			size_t width;
			size_t height;
			size_t stride;		// The number of bytes, not pixels, between lines

		} SLockInfo;

		typedef enum ECubeFace
		{
			POS_X = 0,
			NEG_X,
			POS_Y,
			NEG_Y,
			POS_Z,
			NEG_Z,

			FACE_COUNT

		} CubeFace;

		// The Z length of the texture
		virtual size_t Depth() = NULL;

		/// Call to begin updating texture; fill out the returned buffer and call unlock to finalize the update
		virtual Texture::RETURNCODE Lock(void **buffer, CubeFace face, SLockInfo &lockinfo, size_t mip = 0, props::TFlags64 flags = 0) = NULL;

	};


	class Texture3D : public Texture
	{

	public:

		typedef struct sLockInfo
		{
			size_t width;
			size_t height;
			size_t depth;
			size_t stride_w;		// The number of bytes, not pixels, between lines
			size_t stride_h;		// The number of bytes, not pixels, between slices

		} SLockInfo;

		// The Z length of the texture
		virtual size_t Depth() = NULL;

		/// Call to begin updating texture; fill out the returned buffer and call unlock to finalize the update
		virtual Texture::RETURNCODE Lock(void **buffer, SLockInfo &lockinfo, size_t mip = 0, props::TFlags64 flags = 0) = NULL;

	};

};