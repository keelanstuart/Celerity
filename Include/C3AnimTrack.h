// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	// An AnimTrack is a sorted collection of AnimKeys intended to provide transformations

	class AnimTrack
	{

	public:

		using KeyIndex = size_t;

		enum { KEYINDEX_INVALID = -1 };

		// Sets the name of the AnimTrack
		virtual void SetName(const TCHAR *name) = NULL;

		// Returns the name of the AnimTrack
		virtual const TCHAR *GetName() const = NULL;

		// Sets the name of the track whose transforms this track will add to
		virtual void SetParentName(const TCHAR *name) = NULL;

		// Returns the name of the track whose transforms this track will add to
		virtual const TCHAR *GetParentName() = NULL;

		// Returns the length of the track in seconds
		virtual float GetLength() const = NULL;

		// Sorts the AnimKeys by time
		virtual void SortKeys() = NULL;


		// *********************************
		// Position Keys

		// Returns the number of position keys
		virtual size_t GetPosKeyCount() const = NULL;

		// Creates a new positional animation key at the given time and adds it to this AnimTrack
		virtual KeyIndex AddPosKey(float time, glm::fvec3 &pos) = NULL;

		// Given an index, returns true if it was a valid key, and fills out time and pos in that case
		virtual bool GetPosKey(KeyIndex idx, float &time, glm::fvec3 &pos) const = NULL;

		// Deletes the animation key at the given index
		virtual void DelPosKey(KeyIndex idx) = NULL;

		// Gets an interpolated position at the given time
		virtual glm::fvec3 GetPos(float time, KeyIndex &idx) = NULL;


		// *********************************
		// Orientation Keys

		// Returns the number of orientation keys
		virtual size_t GetOriKeyCount() const = NULL;

		// Creates a new positional animation key at the given time and adds it to this AnimTrack
		virtual KeyIndex AddOriKey(float time, glm::fquat &ori) = NULL;

		// Given an index, returns true if it was a valid key, and fills out time and ori in that case
		virtual bool GetOriKey(KeyIndex idx, float &time, glm::fquat &ori) const = NULL;

		// Deletes the animation key at the given index
		virtual void DelOriKey(KeyIndex idx) = NULL;

		// Gets an interpolated position at the given time
		virtual glm::fquat GetOri(float time, KeyIndex &idx) = NULL;


		// *********************************
		// Scale Keys

		// Returns the number of scale keys
		virtual size_t GetSclKeyCount() const = NULL;

		// Creates a new Sclitional animation key at the given time and adds it to this AnimTrack
		virtual KeyIndex AddSclKey(float time, glm::fvec3 &scl) = NULL;

		// Given an index, returns true if it was a valid key, and fills out time and scl in that case
		virtual bool GetSclKey(KeyIndex idx, float &time, glm::fvec3 &scl) const = NULL;

		// Deletes the animation key at the given index
		virtual void DelSclKey(KeyIndex idx) = NULL;

		// Gets an interpolated position at the given time
		virtual glm::fvec3 GetScl(float time, KeyIndex &idx) = NULL;


		// *********************************
		// Note Keys

		// Returns the number of note keys
		virtual size_t GetNoteKeyCount() const = NULL;

		// Creates a new positional animation key at the given time and adds it to this AnimTrack
		virtual KeyIndex AddNoteKey(float time, const TCHAR *note) = NULL;

		// Given an index, returns the note string. If the index is valid, fills out time, otherwise, the return is nullptr
		virtual const TCHAR *GetNoteKey(KeyIndex idx, float &time) const = NULL;

		// Deletes the animation key at the given index
		virtual void DelNoteKey(KeyIndex idx) = NULL;

		// Gets the last note between the given previous and current times
		virtual const TCHAR *GetNote(float prev_time, float time) = NULL;

	};

};
