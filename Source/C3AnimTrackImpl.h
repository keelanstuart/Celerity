// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3AnimTrack.h>
#include <deque>
#include <optional>


namespace c3
{

	class AnimTrackImpl : public AnimTrack
	{

	public:

		AnimTrackImpl();

		virtual ~AnimTrackImpl();

		// Sets the name of the AnimTrack
		virtual void SetName(const TCHAR *name);

		// Returns the name of the AnimTrack
		virtual const TCHAR *GetName() const;

		// Sets the name of the track whose transforms this track will add to
		virtual void SetParentName(const TCHAR *name);

		// Returns the name of the track whose transforms this track will add to
		virtual const TCHAR *GetParentName();

		// Returns the length of the track in seconds
		virtual float GetLength() const;

		// Sorts the AnimKeys by time
		virtual void SortKeys();


		// *********************************
		// Position Keys
		
		virtual size_t GetPosKeyCount() const;
		
		virtual KeyIndex AddPosKey(float time, glm::fvec3 &pos);

		virtual bool GetPosKey(KeyIndex idx, float &time, glm::fvec3 &pos) const;

		virtual void DelPosKey(KeyIndex idx);

		virtual glm::fvec3 GetPos(float time, KeyIndex &idx);


		// *********************************
		// Orientation Keys

		virtual size_t GetOriKeyCount() const;

		virtual KeyIndex AddOriKey(float time, glm::fquat &ori);

		virtual bool GetOriKey(KeyIndex idx, float &time, glm::fquat &ori) const;

		virtual void DelOriKey(KeyIndex idx);

		virtual glm::fquat GetOri(float time, KeyIndex &idx);


		// *********************************
		// Scale Keys

		virtual size_t GetSclKeyCount() const;

		virtual KeyIndex AddSclKey(float time, glm::fvec3 &scl);

		virtual bool GetSclKey(KeyIndex idx, float &time, glm::fvec3 &scl) const;

		virtual void DelSclKey(KeyIndex idx);

		virtual glm::fvec3 GetScl(float time, KeyIndex &idx);


		// *********************************
		// Note Keys

		virtual size_t GetNoteKeyCount() const;

		virtual KeyIndex AddNoteKey(float time, const TCHAR *note);

		virtual const TCHAR *GetNoteKey(KeyIndex idx, float &time) const;

		virtual void DelNoteKey(KeyIndex idx);

		virtual const TCHAR *GetNote(float prev_time, float time);


	protected:

		tstring m_Name;
		std::optional<tstring> m_ParentName;

		// Key types
		using PosKey = std::pair<float, glm::fvec3>;
		using OriKey = std::pair<float, glm::fquat>;
		using SclKey = std::pair<float, glm::fvec3>;
		using NoteKey = std::pair<float, tstring>;

		// Arrays of Key
		using PosKeyArray = std::vector<PosKey>;
		using OriKeyArray = std::vector<OriKey>;
		using SclKeyArray = std::vector<SclKey>;
		using NoteKeyArray = std::vector<NoteKey>;

		PosKeyArray m_PosKeys;
		OriKeyArray m_OriKeys;
		SclKeyArray m_SclKeys;
		NoteKeyArray m_NoteKeys;

	};

};
