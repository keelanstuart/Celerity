// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3Publisher.h>

namespace c3
{

	class PublisherImpl : public Publisher
	{

	public:

		PublisherImpl(const TCHAR *name);

		virtual ~PublisherImpl();

		virtual void Release();

		virtual Subscription *Subscribe();

		virtual void Unsubscribe(Subscription **sub);

		virtual void Deliver();

	protected:

		typedef std::deque<c3::Subscription *> TSubscriptionArray;

		TSubscriptionArray m_Subs;
		std::mutex m_mxSubs;

		tstring m_Name;

	};

};