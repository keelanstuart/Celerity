// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include "C3AnimTrackImpl.h"
#include "C3AnimationImpl.h"

using namespace c3;


Animation *Animation::Create()
{
	return new AnimationImpl();
}

AnimationImpl::AnimationImpl()
{

}


AnimationImpl::~AnimationImpl()
{

}


void AnimationImpl::Release()
{
	delete this;
}


void AnimationImpl::SetName(const TCHAR *name)
{
	m_Name = name;
}


const TCHAR *AnimationImpl::GetName() const
{
	return m_Name.c_str();
}


float AnimationImpl::GetLength() const
{
	float ret = 0;

	for (auto t : m_Tracks)
	{
		ret = std::max<float>(ret, t.GetLength());
	}

	return ret;
}


size_t AnimationImpl::GetNumTracks() const
{
	return m_Tracks.size();
}


Animation::TrackIndex AnimationImpl::AddNewTrack(const TCHAR *trackname, TrackIndex parent)
{
	for (auto t : m_Tracks)
	{
		if (!_tcsicmp(t.GetName(), trackname))
			return Animation::TRACKINDEX_FAILURE;
	}

	TrackIndex ret = (TrackIndex)m_Tracks.size();

	m_Tracks.emplace_back();
	m_Tracks[ret].SetName(trackname);

	return ret;
}


Animation::TrackIndex AnimationImpl::FindTrackByName(const TCHAR *trackname) const
{
	Animation::TrackIndex ret = 0;

	for (auto t : m_Tracks)
	{
		if (!_tcsicmp(t.GetName(), trackname))
			return ret;

		ret++;
	}

	return Animation::TRACKINDEX_INVALID;
}


AnimTrack *AnimationImpl::GetTrack(Animation::TrackIndex index) const
{
	if (index >= m_Tracks.size())
		return nullptr;

	return (AnimTrack *)(&m_Tracks[index]);
}


bool AnimationImpl::DeleteTrack(Animation::TrackIndex index)
{
	if (index >= m_Tracks.size())
		return false;

	m_Tracks.erase(m_Tracks.begin() + index);
	return true;
}


void AnimationImpl::BuildNodeHierarchy()
{
	m_TrackParent.clear();

	TrackIndex idx = 0;
	for (auto t : m_Tracks)
	{
		TrackIndex pidx = FindTrackByName(t.GetParentName());

		m_TrackParent.push_back(pidx);
	}
}


DECLARE_RESOURCETYPE(Animation);


c3::ResourceType::LoadResult RESOURCETYPENAME(Animation)::ReadFromFile(c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const
{
	c3::ResourceType::LoadResult ret = c3::ResourceType::LR_ERROR;

	if (returned_data)
	{
		*returned_data = nullptr;
	}

	return ret;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(Animation)::ReadFromMemory(c3::System *psys, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
{
	c3::ResourceType::LoadResult ret = c3::ResourceType::LR_ERROR;

	if (returned_data)
	{
		*returned_data = nullptr;
	}

	return ret;
}


bool RESOURCETYPENAME(Animation)::WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const
{
	return false;
}


void RESOURCETYPENAME(Animation)::Unload(void *data) const
{
	((Animation *)data)->Release();
}
