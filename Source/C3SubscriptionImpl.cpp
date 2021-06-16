// **************************************************************
// C2 Game / Visualization Engine Source File
//
// Copyright © 2001-2017 Keelan Stuart


#include "pch.h"

#include <C3SubscriptionImpl.h>


c3::SubscriptionImpl::SubscriptionImpl(const c3::PublisherImpl *pub)
{
	m_bReady = false;
	m_Pub = pub;
}


bool c3::Subscription::HasArrived(const c3::Subscription *self)
{
	return ((self != nullptr) && ((c3::SubscriptionImpl *)self)->m_bReady);
}


void c3::Subscription::Reset(c3::Subscription *self)
{
	if (self)
		((c3::SubscriptionImpl *)self)->m_bReady = false;
}

