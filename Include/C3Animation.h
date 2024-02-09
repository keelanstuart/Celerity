// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3AnimTrack.h>


namespace c3
{

	// An Animation is a set of AnimTracks that corelate to a Model's Nodes, each containing interpolable keyframes to transform those Nodes over time

	class Animation
	{

	public:

		using TrackIndex = int;

		enum { TRACKINDEX_INVALID = -1 };
		enum { TRACKINDEX_FAILURE = -2 };

		static Animation *Create();

		virtual void Release() = NULL;

		virtual void SetName(const TCHAR *name) = NULL;

		virtual const TCHAR *GetName() const = NULL;

		virtual float GetLength() const = NULL;

		virtual size_t GetNumTracks() const = NULL;

		virtual TrackIndex AddNewTrack(const TCHAR *trackname, TrackIndex parent = TRACKINDEX_INVALID) = NULL;

		virtual TrackIndex FindTrackByName(const TCHAR *trackname) const = NULL;

		virtual AnimTrack *GetTrack(TrackIndex index) const = NULL;

		virtual bool DeleteTrack(TrackIndex index) = NULL;

	};

};
