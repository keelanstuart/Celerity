// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


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
}


c3::PublisherImpl::~PublisherImpl()
{
	for (auto &it : m_Subs)
	{
		delete (c3::SubscriptionImpl *)it;
	};

	m_Subs.clear();
}


void c3::PublisherImpl::Release()
{
	delete this;
}


c3::Subscription *c3::PublisherImpl::Subscribe()
{
	std::lock_guard<std::mutex> lock(m_mxSubs);

	c3::Subscription *ret = new c3::SubscriptionImpl(this);

	m_Subs.push_back(ret);
	((c3::SubscriptionImpl *)ret)->m_bReady = true;

	return ret;
}


void c3::PublisherImpl::Unsubscribe(c3::Subscription **sub)
{
	if (sub)
	{
		{
			std::lock_guard<std::mutex> lock(m_mxSubs);

			auto &it = std::find(m_Subs.begin(), m_Subs.end(), *sub);
			if (it != m_Subs.end())
				m_Subs.erase(it);
		}

		if (*sub)
		{
			delete (c3::SubscriptionImpl *)*sub;
			*sub = nullptr;
		}
	}
}


void c3::PublisherImpl::Deliver()
{
	std::lock_guard<std::mutex> lock(m_mxSubs);

	for (auto &it : m_Subs)
	{
		((c3::SubscriptionImpl *)it)->m_bReady;
	};
}
