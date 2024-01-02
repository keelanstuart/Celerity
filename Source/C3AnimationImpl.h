// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3AnimTrackImpl.h>
#include <C3Animation.h>
#include <deque>


namespace c3
{

	class AnimationImpl : public Animation
	{

	public:

		AnimationImpl();

		virtual ~AnimationImpl();

		virtual void Release();

		virtual void SetName(const TCHAR *name);

		virtual const TCHAR *GetName() const;

		virtual float GetLength() const;

		virtual size_t GetNumTracks() const;

		virtual TrackIndex AddNewTrack(const TCHAR *trackname, TrackIndex parent = TRACKINDEX_INVALID);

		virtual TrackIndex FindTrackByName(const TCHAR *trackname) const;

		virtual AnimTrack *GetTrack(TrackIndex index) const;

		virtual bool DeleteTrack(TrackIndex index);

		void BuildNodeHierarchy();

	protected:

		tstring m_Name;

		using TrackArray = std::deque<AnimTrackImpl>;
		TrackArray m_Tracks;

		using TrackIndexArray = std::vector<TrackIndex>;
		TrackIndexArray m_TrackParent;

	};

	DEFINE_RESOURCETYPE(Animation, 0, GUID({ 0x3dd51727, 0xe26e, 0x4f9c, { 0xbd, 0xe6, 0x44, 0x6c, 0xa1, 0xcc, 0xb1, 0x1f }}), "Animation", "3D Animation Data", "c3anim", "c3anim");

};
