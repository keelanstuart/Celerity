// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Subscription.h>
#include <C3PublisherImpl.h>

namespace c3
{

	class SubscriptionImpl : public Subscription
	{

	public:

		friend class PublisherImpl;

		bool m_bReady;				// Indicates that a publisher has a delivery for us

		const c3::PublisherImpl *m_Pub;	// The currently subscribed publisher

	protected:

		SubscriptionImpl(const PublisherImpl *pub);

	};

};
