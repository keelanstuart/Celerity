// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"

#include "C3AnimTrackImpl.h"

using namespace c3;


AnimTrackImpl::AnimTrackImpl()
{
	m_PosKeys.reserve(1024);
	m_OriKeys.reserve(1024);
	m_SclKeys.reserve(16);
	m_NoteKeys.reserve(4);
}


AnimTrackImpl::~AnimTrackImpl()
{
}


void AnimTrackImpl::SetName(const TCHAR *name)
{
	m_Name = name ? name : _T("");
}


const TCHAR *AnimTrackImpl::GetName() const
{
	return m_Name.c_str();
}


void AnimTrackImpl::SetParentName(const TCHAR *name)
{
	if (name)
		*m_ParentName = name;
	else
		m_ParentName.reset();
}


const TCHAR *AnimTrackImpl::GetParentName()
{
	if (m_ParentName.has_value())
		return m_ParentName->c_str();

	return nullptr;
}


float AnimTrackImpl::GetLength() const
{
	float poslen = m_PosKeys.empty() ? 0 : m_PosKeys.back().first;
	float orilen = m_OriKeys.empty() ? 0 : m_OriKeys.back().first;
	float scllen = m_SclKeys.empty() ? 0 : m_SclKeys.back().first;
	float notelen = m_NoteKeys.empty() ? 0 : m_NoteKeys.back().first;

	return std::max<float>(std::max<float>(poslen, orilen), std::max<float>(scllen, notelen));
}


void AnimTrackImpl::SortKeys()
{
	std::sort(m_PosKeys.begin(), m_PosKeys.end(), [](const PosKey &a, const PosKey &b) { return a.first < b.first; });
	std::sort(m_OriKeys.begin(), m_OriKeys.end(), [](const OriKey &a, const OriKey &b) { return a.first < b.first; });
	std::sort(m_SclKeys.begin(), m_SclKeys.end(), [](const SclKey &a, const SclKey &b) { return a.first < b.first; });
	std::sort(m_NoteKeys.begin(), m_NoteKeys.end(), [](const NoteKey &a, const NoteKey &b) { return a.first < b.first; });
}


size_t AnimTrackImpl::GetPosKeyCount() const
{
	return m_PosKeys.size();
}


AnimTrack::KeyIndex AnimTrackImpl::AddPosKey(float time, glm::fvec3 &pos)
{
	PosKeyArray::iterator it = std::lower_bound(m_PosKeys.begin(), m_PosKeys.end(), PosKey(time, glm::fvec3()), [](const PosKey &a, const PosKey &b)
	{
		return a.first < b.first;
	});

	if ((it != m_PosKeys.end()) && (it->first == time))
		return AnimTrack::KEYINDEX_INVALID;

	m_PosKeys.emplace_back();
	m_PosKeys.back().first = time;
	m_PosKeys.back().second = pos;

	return m_PosKeys.size() - 1;
}


bool AnimTrackImpl::GetPosKey(KeyIndex idx, float &time, glm::fvec3 &pos) const
{
	if (idx >= m_PosKeys.size())
		return false;

	time = m_PosKeys[idx].first;
	pos = m_PosKeys[idx].second;

	return true;
}


void AnimTrackImpl::DelPosKey(KeyIndex idx)
{
	if (idx < m_PosKeys.size())
		m_PosKeys.erase(m_PosKeys.begin() + idx);
}


glm::fvec3 AnimTrackImpl::GetPos(float time, KeyIndex &idx)
{
	if (m_PosKeys.empty())
		return glm::fvec3(0, 0, 0);

	PosKeyArray::iterator it, itn;

	if ((size_t)idx > m_PosKeys.size())
	{
		itn = std::upper_bound(m_PosKeys.begin(), m_PosKeys.end(), PosKey(time, glm::fvec3()), [](const PosKey &a, const PosKey &b)
		{
			return a.first < b.first;
		});

		idx = std::distance(m_PosKeys.begin(), itn);

		if (itn == m_PosKeys.end())
		{
			itn = m_PosKeys.begin() + idx - 1;
		}
	}
	else
	{
		while (((size_t)idx < m_PosKeys.size()) && (m_PosKeys[idx].first < time)) idx++;
		itn = m_PosKeys.begin() + idx;
	}

	it = itn;
	if (itn != m_PosKeys.begin())
		it--;
	if (itn == m_PosKeys.end())
		itn--;

	if (it == itn)
		return it->second;

	float pct = (time - it->first) / (itn->first - it->first);

	glm::fvec3 ret = it->second + ((itn->second - it->second) * pct);
	return ret;
}


size_t AnimTrackImpl::GetOriKeyCount() const
{
	return m_OriKeys.size();
}


AnimTrack::KeyIndex AnimTrackImpl::AddOriKey(float time, glm::fquat &ori)
{
	OriKeyArray::iterator it = std::lower_bound(m_OriKeys.begin(), m_OriKeys.end(), OriKey(time, glm::fquat()), [](const OriKey &a, const OriKey &b)
	{
		return a.first < b.first;
	});

	if (it != m_OriKeys.end() && it->first == time)
		return AnimTrack::KEYINDEX_INVALID;

	m_OriKeys.emplace_back();
	m_OriKeys.back().first = time;
	m_OriKeys.back().second = ori;

	return m_OriKeys.size() - 1;
}


bool AnimTrackImpl::GetOriKey(KeyIndex idx, float &time, glm::fquat &ori) const
{
	if (idx >= m_OriKeys.size())
		return false;

	time = m_OriKeys[idx].first;
	ori = m_OriKeys[idx].second;

	return true;
}


void AnimTrackImpl::DelOriKey(KeyIndex idx)
{
	if (idx < m_OriKeys.size())
		m_OriKeys.erase(m_OriKeys.begin() + idx);
}


glm::fquat AnimTrackImpl::GetOri(float time, KeyIndex &idx)
{
	if (m_OriKeys.empty())
		return glm::fquat(1, 0, 0, 0);

	OriKeyArray::iterator it, itn;

	if ((size_t)idx > m_OriKeys.size())
	{
		itn = std::upper_bound(m_OriKeys.begin(), m_OriKeys.end(), OriKey(time, glm::fvec3()), [](const OriKey &a, const OriKey &b)
		{
			return a.first < b.first;
		});

		idx = std::distance(m_OriKeys.begin(), itn);

		if (itn == m_OriKeys.end())
		{
			itn = m_OriKeys.begin() + idx - 1;
		}
	}
	else
	{
		while (((size_t)idx < m_OriKeys.size()) && (m_OriKeys[idx].first < time)) idx++;
		itn = m_OriKeys.begin() + idx;
	}

	it = itn;
	if (itn != m_OriKeys.begin())
		it--;
	if (itn == m_OriKeys.end())
		itn--;

	if (it == itn)
		return it->second;

	float pct = (time - it->first) / (itn->first - it->first);

#if 1
	return glm::slerp(it->second, itn->second, pct);
#else
	glm::fquat ret = it->second + ((itn->second - it->second) * pct);
	return ret;
#endif
}


size_t AnimTrackImpl::GetSclKeyCount() const
{
	return m_SclKeys.size();
}


AnimTrack::KeyIndex AnimTrackImpl::AddSclKey(float time, glm::fvec3 &scl)
{
	SclKeyArray::iterator it = std::lower_bound(m_SclKeys.begin(), m_SclKeys.end(), SclKey(time, glm::fvec3()), [](const SclKey &a, const SclKey &b)
	{
		return a.first < b.first;
	});

	if (it != m_SclKeys.end() && it->first == time)
		return AnimTrack::KEYINDEX_INVALID;

	m_SclKeys.emplace_back();
	m_SclKeys.back().first = time;
	m_SclKeys.back().second = scl;

	return m_SclKeys.size() - 1;
}


bool AnimTrackImpl::GetSclKey(KeyIndex idx, float &time, glm::fvec3 &scl) const
{
	if (idx >= m_PosKeys.size())
		return false;

	time = m_SclKeys[idx].first;
	scl = m_SclKeys[idx].second;

	return true;
}


void AnimTrackImpl::DelSclKey(KeyIndex idx)
{
	if (idx < m_SclKeys.size())
		m_SclKeys.erase(m_SclKeys.begin() + idx);
}


glm::fvec3 AnimTrackImpl::GetScl(float time, KeyIndex &idx)
{
	if (m_SclKeys.empty())
		return glm::fvec3(1, 1, 1);

	SclKeyArray::iterator it, itn;

	if ((size_t)idx > m_SclKeys.size())
	{
		itn = std::upper_bound(m_SclKeys.begin(), m_SclKeys.end(), SclKey(time, glm::fvec3()), [](const SclKey &a, const SclKey &b)
		{
			return a.first < b.first;
		});

		idx = std::distance(m_SclKeys.begin(), itn);

		if (itn == m_SclKeys.end())
		{
			itn = m_SclKeys.begin() + idx - 1;
		}
	}
	else
	{
		while (((size_t)idx < m_SclKeys.size()) && (m_SclKeys[idx].first < time)) idx++;
		itn = m_SclKeys.begin() + idx;
	}

	it = itn;
	if (itn != m_SclKeys.begin())
		it--;
	if (itn == m_SclKeys.end())
		itn--;

	if (it == itn)
		return it->second;

	float pct = (time - it->first) / (itn->first - it->first);

	glm::fvec3 ret = it->second + ((itn->second - it->second) * pct);
	return ret;
}


size_t AnimTrackImpl::GetNoteKeyCount() const
{
	return m_NoteKeys.size();
}


AnimTrack::KeyIndex AnimTrackImpl::AddNoteKey(float time, const TCHAR *note)
{
	m_NoteKeys.emplace_back();
	m_NoteKeys.back().first = time;
	m_NoteKeys.back().second = note;

	return m_NoteKeys.size() - 1;
}


const TCHAR *AnimTrackImpl::GetNoteKey(KeyIndex idx, float &time) const
{
	if (idx >= m_NoteKeys.size())
		return nullptr;

	time = m_NoteKeys[idx].first;

	return  m_NoteKeys[idx].second.c_str();
}


void AnimTrackImpl::DelNoteKey(KeyIndex idx)
{
	if (idx < m_NoteKeys.size())
		m_NoteKeys.erase(m_NoteKeys.begin() + idx);
}


const TCHAR *AnimTrackImpl::GetNote(float prev_time, float time)
{
	for (auto n : m_NoteKeys)
	{
		if (n.first > time)
			return nullptr;

		if ((n.first > prev_time) && (n.first <= time))
		{
			return n.second.c_str();
		}
	}

	return nullptr;
}
