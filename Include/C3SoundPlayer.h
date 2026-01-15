// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class SoundPlayer
	{

	public:

		enum SOUND_TYPE
		{
			ST_SFX = 0,
			ST_MUSIC,
			ST_DIALOG,
			ST_CD,

			SOUND_TYPES
		};

		enum PLAY_STATUS
		{
			PS_STOPPED = 0,
			PS_PLAYING,
			PS_PAUSED,
			PS_ERROR
		};

		typedef size_t HSAMPLE;
		typedef size_t HCHANNEL;

		enum { INVALID_HSAMPLE = -1 };
		enum { INVALID_HCHANNEL = -2 };
		enum { SOUND_ALL = -3 };
		enum { DEFAULT_DEVICE = (size_t)-1 };
		enum { LOOP_INFINITE = (size_t)-1 };


		virtual size_t GetDeviceCount() const = NULL;

		virtual const TCHAR *GetDeviceName(size_t devidx) const = NULL;

		virtual bool Initialize(size_t devidx = (size_t)DEFAULT_DEVICE) = NULL;

		virtual bool Initialized() = NULL;

		virtual void Shutdown() = NULL;

		virtual void SetListenerPos(const glm::fvec3 *lpos) = NULL;

		virtual const glm::fvec3 *GetListenerPos(glm::fvec3 *lpos) const = NULL;

		virtual void SetListenerDir(const glm::fvec3 *ldir) = NULL;

		virtual const glm::fvec3 *GetListenerDir(glm::fvec3 *ldir) const = NULL;

		virtual void SetListenerRadius(float min_rad, float max_rad) = NULL;

		virtual void Update(float elapsed_seconds = 0.0f) = NULL;

		virtual HCHANNEL Play(Resource *pres, SOUND_TYPE sndtype = SOUND_TYPE::ST_SFX, float volume = 1.0f, float pitchmult = 1.0f, size_t loopcount = 1, const glm::fvec3 *pos = nullptr) = NULL;

		virtual void Stop(HCHANNEL hc = SOUND_ALL) = NULL;

		virtual void Pause(HCHANNEL hc = SOUND_ALL) = NULL;

		virtual void Resume(HCHANNEL hc = SOUND_ALL) = NULL;

		virtual PLAY_STATUS Status(HCHANNEL hc) const = NULL;

		virtual float GetVolume(SOUND_TYPE type) = NULL;

		virtual void SetVolume(SOUND_TYPE type, float volume) = NULL;

		virtual void SetChannelVolume(HCHANNEL chidx, float volume) = NULL;

		virtual void SetChannelPitchMod(HCHANNEL hc, float pitchmult) = NULL;

		virtual void SetChannelPos(HCHANNEL hc, const glm::fvec3 *pos = nullptr) = NULL;

	};

};