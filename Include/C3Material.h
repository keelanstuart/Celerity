// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Texture.h>
#include <C3Resource.h>

namespace c3
{

	class Texture;

	class C3_API Material
	{

	public:

		typedef enum EColorComponentType
		{
			CCT_DIFFUSE = 0,		// diffuse lighting modulator, RGBA maps literally (alpha is opacity)
			CCT_SPECULAR,			// specular lighting modulator, RGB maps literally, A maps to specular strength
			CCT_EMISSIVE,			// emissive lighting modulator, RGB maps literally, A ignored

			NUM_COLORTYPES
		} ColorComponentType;

		typedef enum ETextureComponentType
		{
			TCT_DIFFUSE = 0,		// diffuse map, RGBA maps literally (alpha is opacity)
			TCT_NORMAL,				// normal map, RGB maps to XYZ
			TCT_SURFACEDESC,		// metalness/roughness material map and ambient occlusion
			TCT_EMISSIVE,			// emissive map, RGB maps literally, A ignored
			TCT_POSITIONDEPTH,		// position/depth map RGBA (final deferred lighting pass)

			NUM_TEXTURETYPES
		} TextureComponentType;

		typedef enum ERenderModeFlag
		{
			RMF_WIREFRAME = 0,			// solid / wireframe rendering; default: off
			RMF_FLAT,					// gouraud / flat shading; default: off
			RMF_RENDERFRONT,			// render front-facing polygons; default: on
			RMF_RENDERBACK,				// render back-facing polygons; default: off
			RMF_WRITEDEPTH,				// write the fragment depth to the depth buffer
			RMF_READDEPTH,				// read from the depth buffer
			RMF_STENCIL,				// do stencil things
		} RenderModeFlag;

		static constexpr uint64_t RENDERMODEFLAG(RenderModeFlag f) { return (1LL << (f)); }

		virtual void Release() = NULL;

		/// Sets the given material component color
		virtual void SetColor(ColorComponentType comptype, const glm::fvec4 *pcolor) = NULL;

		/// GetColor returns the given component color
		virtual const glm::fvec4 *GetColor(ColorComponentType comptype, glm::fvec4 *pcolor = nullptr) const = NULL;

		/// Sets the texture for the given type
		virtual void SetTexture(TextureComponentType comptype, Texture *ptex) = NULL;

		/// Sets a texture resource for the given type
		virtual void SetTexture(TextureComponentType comptype, Resource *ptexres) = NULL;

		/// Gets the texture for the given type
		virtual Texture *GetTexture(TextureComponentType comptype) const = NULL;

		/// Construct the flags using RENDERMODEFLAGS
		virtual props::TFlags64 &RenderModeFlags() = NULL;

		/// Determines whether a triangle's front face is clockwise or counter-clockwise
		virtual void SetWindingOrder(Renderer::WindingOrder mode) = NULL;

		/// Get whether a triangle's front face is clockwise or counter-clockwise
		virtual Renderer::WindingOrder GetWindingOrder() = NULL;

		/// Sets the depth test type
		virtual void SetDepthTest(Renderer::Test test) = NULL;

		/// Gets the depth test type
		virtual Renderer::Test GetDepthTest() const = NULL;

		/// Enables or Disables stencil operations
		virtual void SetStencilEnabled(bool en) = NULL;

		/// Returns whether stencil operations are enabled
		virtual bool GetStencilEnabled() const = NULL;

		/// Sets the stencil modes
		/// stfail is what to do to the stencil buffer if the stencil test fails
		/// dfail is what to do to the stencil buffer if the stencil test passes but the depth test fails
		/// dfail is what to do to the stencil buffer if the stencil and depth tests pass
		virtual void SetStencilOperation(Renderer::StencilOperation stencil_fail, Renderer::StencilOperation zfail, Renderer::StencilOperation zpass) = NULL;

		/// Gets the stencil Operations
		virtual void GetStencilOperation(Renderer::StencilOperation &stencil_fail, Renderer::StencilOperation &zfail, Renderer::StencilOperation &zpass) const = NULL;

		/// Sets the stencil test mode
		virtual void SetStencilTest(Renderer::Test test, uint8_t ref = 0, uint8_t mask = 0xff) = NULL;

		/// Gets the stencil test mode and optionally the reference and mask values
		virtual Renderer::Test GetStencilTest(uint8_t *ref = nullptr, uint8_t *mask = nullptr) const = NULL;

		/// Applies this Materials settings to the Renderer and the given shader (READ: set shader FIRST for proper operation)
		virtual bool Apply(ShaderProgram *shader) const = NULL;

		/// User-supplied callback intended to provide the filenames for textures types that were not given
		/// when a material is created as a result of a Model being loaded (via ResourceManager).
		/// This is primarily a workaround for FBX-format models where only diffuse textures are available
		/// in the data.
		typedef bool (__cdecl *MTL_ALT_TEXNAME_FUNC)(const TCHAR *diffuse_texname, TextureComponentType typeneeded, TCHAR *needed_texnamebuf, size_t texnamebuf_len);

		static void SetAlternateTextureFilenameFunc(MTL_ALT_TEXNAME_FUNC func);

	};

};
