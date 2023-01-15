// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class Subscription
	{

	public:

		// Returns true if the delivery has been received, false otherwise
		static bool HasArrived(const Subscription *self);

		// Call to reset, once the delivery has been read
		static void Reset(Subscription *self);

	};

};
