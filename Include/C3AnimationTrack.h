// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3AnimationKey.h>


namespace c3
{

	class AnimTrack
	{

	public:

		virtual void SetName(const TCHAR *name) = 0;

		virtual AnimKey *AddKey(float time) = 0;

	};

};