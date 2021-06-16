// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3PublisherImpl.h>
#include <C3SubscriptionImpl.h>
#include <algorithm>


c3::Publisher *c3::Publisher::Create(const TCHAR *name)
{
	return new PublisherImpl(name);
}


c3::PublisherImpl::PublisherImpl(const TCHAR *name)
{
	m_Name = name ? name : _T("<anonymous>");

	InitializeCriticalSection(&m_csSubs);
}


c3::PublisherImpl::~PublisherImpl()
{
	DeleteCriticalSection(&m_csSubs);

	std::for_each(m_Subs.begin(), m_Subs.end(), [&](TSubscriptionArray::value_type &sub)
	{
		delete (c3::SubscriptionImpl *)sub;
	});

	m_Subs.clear();
}


void c3::PublisherImpl::Release()
{
	delete this;
}


c3::Subscription *c3::PublisherImpl::Subscribe()
{
	EnterCriticalSection(&m_csSubs);

	c3::Subscription *ret = new c3::SubscriptionImpl(this);

	m_Subs.push_back(ret);
	((c3::SubscriptionImpl *)ret)->m_bReady = true;

	LeaveCriticalSection(&m_csSubs);

	return ret;
}


void c3::PublisherImpl::Unsubscribe(c3::Subscription **sub)
{
	if (sub)
	{
		EnterCriticalSection(&m_csSubs);

		TSubscriptionArray::iterator it = std::find(m_Subs.begin(), m_Subs.end(), *sub);
		if (it != m_Subs.end())
			m_Subs.erase(it);

		LeaveCriticalSection(&m_csSubs);

		if (*sub)
		{
			delete (c3::SubscriptionImpl *)*sub;
			*sub = nullptr;
		}
	}
}


void c3::PublisherImpl::Deliver()
{
	EnterCriticalSection(&m_csSubs);

	std::for_each(m_Subs.begin(), m_Subs.end(), [&](TSubscriptionArray::value_type &sub)
	{
		((c3::SubscriptionImpl *)sub)->m_bReady;
	});

	LeaveCriticalSection(&m_csSubs);
}
