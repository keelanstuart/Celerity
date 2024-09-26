// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3DepthBuffer.h>
#include <C3Publisher.h>


namespace c3
{

	class FrameBuffer
	{

	public:

		enum RETURNCODE
		{
			RET_OK = 0,

			RET_NOTINITIALIZED,
			RET_NULLTARGET,
			RET_WRONGRES,
			RET_INCOMPLETE,
			RET_BADTYPE,
			RET_TOOMANYTARGETS,
			RET_NOTARGETS,
			RET_UNKNOWN,

			RET_NUMERRORS
		};

		// The practical limit is 4, but...
		enum { MAX_COLORTARGETS = 32 };

		typedef struct TargetDesc
		{
			const TCHAR* name;
			Renderer::ETextureType type;
			uint64_t flags;
		} TargetDesc;

		/// Releases the resources owned by the framebuffer (note: does not release any attached surfaces)
		virtual void Release() = NULL;

		/// Returns the name that the FrameBuffer was given when it was created
		virtual const TCHAR *GetName() const = NULL;

		/// Fills out a complete FrameBuffer based on the descriptions of a target
		virtual RETURNCODE Setup(size_t numtargs, const TargetDesc *ptargdescs, DepthBuffer *pdb, RECT &r) = NULL;

		/// Tears down the FrameBuffer if it has been Setup
		virtual RETURNCODE Teardown() = NULL;

		/// Attaches a texture target to the given position
		virtual RETURNCODE AttachColorTarget(Texture2D *target, size_t position) = NULL;

		/// Returns the number of color targets currently attached to the frame buffer
		virtual size_t GetNumColorTargets() = NULL;

		/// Returns the target at the given position
		virtual Texture2D *GetColorTarget(size_t position) = NULL;

		/// Returns the target with the given name
		virtual Texture2D *GetColorTargetByName(const TCHAR *name) = NULL;

		/// Attaches a depth target
		virtual RETURNCODE AttachDepthTarget(DepthBuffer *pdepth) = NULL;

		/// Returns the current depth target
		virtual DepthBuffer *GetDepthTarget() = NULL;

		/// Finalizes the creation of the frame buffer and indicates whether it is complete
		virtual RETURNCODE Seal() = NULL;

		/// Sets the clear color for an individual target at the given position
		virtual void SetClearColor(size_t position, glm::fvec4 color) = NULL;

		/// Gets the clear color for an individual target at the given position
		virtual glm::fvec4 GetClearColor(size_t position) const = NULL;

		/// Sets the clear depth
		virtual void SetClearDepth(float depth = 1.0f) = NULL;

		/// Gets the clear depth
		virtual float GetClearDepth() const = NULL;

		/// Sets the clear stencil value
		virtual void SetClearStencil(int8_t stencil = 0) = NULL;

		/// Gets the clear depth
		virtual int8_t GetClearStencil() const = NULL;

		/// Clears the FrameBuffer with the values given to the SetClearColor method
		/// NOTE: Uses the same flags as Renderer::UserFrameBuffer for selecting what to clear
		virtual void Clear(props::TFlags64 flags, int target = -1) = NULL;

		/// Sets the blend mode for this render target (or a specific attachment)
		virtual void SetBlendMode(Renderer::BlendMode mode, int target = -1) = NULL;

		/// Gets the blend mode for this render target (or a specific attachment)
		virtual Renderer::BlendMode GetBlendMode(int target = -1) const = NULL;

		/// Sets the blend equation for this render target
		virtual void SetBlendEquation(Renderer::BlendEquation eq, int target = -1) = NULL;

		/// Gets the blend equation for this render target
		virtual Renderer::BlendEquation GetBlendEquation(int target = -1) const = NULL;

		/// Sets the channel write mask for this render target (or a specific attachment)
		virtual void SetChannelWriteMask(Renderer::ChannelMask mask, int target = -1) = NULL;

		/// Gets the channel write mask for this render target (or a specific attachment)
		virtual Renderer::ChannelMask GetChannelWriteMask(int target = -1) const = NULL;

	};

};