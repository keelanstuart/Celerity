// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3PublisherImpl.h>
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
}


void c3::PublisherImpl::Release()
{
	delete this;
}


void c3::PublisherImpl::Subscribe(c3::Subscription *sub)
{
	std::lock_guard<std::mutex> lock(m_mxSubs);

	m_Subs.push_back(sub);
	*sub = true;
}


void c3::PublisherImpl::Unsubscribe(c3::Subscription *sub)
{
	if (sub)
	{
		std::lock_guard<std::mutex> lock(m_mxSubs);

		TSubscriptionArray::iterator it = std::find(m_Subs.begin(), m_Subs.end(), sub);
		if (it != m_Subs.end())
			m_Subs.erase(it);
	}
}


void c3::PublisherImpl::Deliver()
{
	std::lock_guard<std::mutex> lock(m_mxSubs);

	for (TSubscriptionArray::iterator it = m_Subs.begin(), lit = m_Subs.end(); it != lit; it++)
	{
		*(*it) = true;
	};
}
