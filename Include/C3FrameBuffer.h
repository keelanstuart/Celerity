// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3DepthBuffer.h>


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


		/// Releases the resources owned by the framebuffer (note: does not release any attached surfaces)
		virtual void Release() = NULL;

		/// Attaches a texture target to the given position
		virtual RETURNCODE AttachColorTarget(Texture2D *target, size_t position) = NULL;

		/// Returns the number of color targets currently attached to the frame buffer
		virtual size_t GetNumColorTargets() = NULL;

		/// Returns the target at the given position
		virtual Texture2D *GetColorTarget(size_t position) = NULL;

		/// Attaches a depth target
		virtual RETURNCODE AttachDepthTarget(DepthBuffer *pdepth) = NULL;

		/// Returns the current depth target
		virtual DepthBuffer *GetDepthTarget() = NULL;

		/// Finalizes the creation of the frame buffer and indicates whether it is complete
		virtual RETURNCODE Seal() = NULL;

	};

};