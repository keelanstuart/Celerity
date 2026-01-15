// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	using Subscription = bool;

	class Publisher
	{

	public:

		static C3_API Publisher *Create(const TCHAR *name);

		virtual void Release() = NULL;

		/// Starts subscribing to this Publisher -- under the hood, a Subscription is a bool
		virtual void Subscribe(Subscription *sub) = NULL;

		/// Will make all Subscriptions indicate that their delivery has arrived
		virtual void Unsubscribe(Subscription *sub) = NULL;

		/// Will make all Subscriptions indicate that their delivery has arrived
		virtual void Deliver() = NULL;
		 
	};

};