// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Component.h>


namespace c3
{

	class C3_API OmniLight : public Component
	{

	public:

		static const ComponentType *Type();

		virtual void SetSourceFrameBuffer(FrameBuffer *psource) = NULL;

	};

};
