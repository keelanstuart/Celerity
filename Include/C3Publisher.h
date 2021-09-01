// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Subscription.h>

namespace c3
{

	class Publisher
	{

	public:

		static C3_API Publisher *Create(const TCHAR *name);

		virtual void Release() = NULL;

		///  Returns a Subscription that you can query to see if something new has been published
		virtual Subscription *Subscribe() = NULL;

		/// Will make all Subscriptions indicate that their delivery has arrived
		virtual void Unsubscribe(Subscription **sub) = NULL;

		/// Will make all Subscriptions indicate that their delivery has arrived
		virtual void Deliver() = NULL;
		 
	};

};