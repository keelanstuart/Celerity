// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Renderer.h>

namespace c3
{

	class Texture
	{

	public:

		enum RETURNCODE
		{
			RET_OK = 0,
			RET_ALREADY_LOCKED,
			RET_NOT_LOCKED,
			RET_NULL_BUFFER,
			RET_GENBUFFER_FAILED,
			RET_MAPBUFFER_FAILED,
		};

		virtual void Release() = NULL;

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

	#define TEXLOCKFLAG_READ		0x0001
	#define TEXLOCKFLAG_WRITE		0x0002
	#define TEXLOCKFLAG_GENMIPS		0x0004

	class Texture2D : public Texture
	{

	public:

		typedef struct sLockInfo
		{
			size_t width;
			size_t height;
			size_t stride;

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
			size_t stride;

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
			size_t stride_w;
			size_t stride_h;

		} SLockInfo;

		// The Z length of the texture
		virtual size_t Depth() = NULL;

		/// Call to begin updating texture; fill out the returned buffer and call unlock to finalize the update
		virtual Texture::RETURNCODE Lock(void **buffer, SLockInfo &lockinfo, size_t mip = 0, props::TFlags64 flags = 0) = NULL;
	};

};