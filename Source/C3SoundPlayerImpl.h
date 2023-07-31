// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3SoundPlayer.h>

#include <miniaudio.h>
#include <mmsystem.h>
#include <vfw.h>
#include <mmreg.h>
#include <msacm.h>
#include <deque>
#include <optional>
#include <array>

namespace c3
{
	class SoundPlayerImpl : public SoundPlayer
	{

		friend class RESOURCETYPENAME(Sound);

	public:
		SoundPlayerImpl(System *system);
		virtual ~SoundPlayerImpl();

		virtual size_t GetDeviceCount() const;

		virtual const TCHAR *GetDeviceName(size_t devidx) const;

		virtual bool Initialize(size_t devidx = DEFAULT_DEVICE);

		virtual void Shutdown();

		virtual void SetListenerPos(const glm::fvec3 *lpos);

		virtual const glm::fvec3 *GetListenerPos(glm::fvec3 *lpos) const;

		virtual void SetListenerDir(const glm::fvec3 *ldir);

		virtual const glm::fvec3 *GetListenerDir(glm::fvec3 *ldir) const;

		virtual void SetListenerRadius(float min_rad, float max_rad);

		virtual void Update(float elapsed_seconds = 0.0f);

		virtual HCHANNEL Play(HSAMPLE hs, float volume = 1.0f, float pitchmult = 1.0f, size_t loopcount = 1, const glm::fvec3 *pos = nullptr);

		virtual void Stop(HCHANNEL hc = SOUND_ALL);

		virtual void Pause(HCHANNEL hc = SOUND_ALL);

		virtual void Resume(HCHANNEL hc = SOUND_ALL);

		virtual PLAY_STATUS Status(HCHANNEL hc) const ;

		virtual float GetVolume(SOUND_TYPE type);

		virtual void SetVolume(SOUND_TYPE type, float volume);

		virtual void SetChannelVolume(HCHANNEL hc, float volume);

		virtual void SetChannelPos(HCHANNEL hc, const glm::fvec3 *pos = nullptr);

		virtual ma_engine *GetSfxEngine() { return &m_Engine[ST_SFX]; }

	protected:

		struct SCDTrackInfo
		{
			uint32_t startms;
			uint32_t endms;
		};

		typedef class std::deque<SCDTrackInfo> TCDTrackInfoArray;

		System *m_pSys;

		bool m_bInitialized;
		size_t m_DefaultDevice;

		ma_resource_manager_config m_ResManConfig;
		ma_resource_manager m_ResMan;
		ma_context m_Context;
		ma_device_config m_DeviceConfig;
		ma_device m_Device;
		ma_device_info* m_pDeviceInfo;
		std::vector<tstring> m_DeviceName;
		ma_uint32 m_DeviceCount;
		ma_engine m_Engine[SOUND_TYPES];

		typedef std::array<std::pair<size_t, ma_sound>, MA_MAX_CHANNELS> TChannelArray;
		TChannelArray m_Channels;

		float m_Volume[SOUND_TYPES];

		glm::fvec3 m_ListenerPos;
		glm::fvec3 m_ListenerDir;
		float m_ListenerMinRadius;
		float m_ListenerMaxRadius;

		int32_t m_CD_DevId;
		TCDTrackInfoArray m_CD_TrackInfo;
		PLAY_STATUS m_CD_Status;
		int32_t m_CD_PausePos;
		bool m_CD_Active;

	};

	DEFINE_RESOURCETYPE(Sound, 0, GUID({0x9178daa1, 0xd5f5, 0x40ef, { 0xa0, 0x5d, 0x3a, 0xa, 0x98, 0xec, 0xf4, 0x81 }}), "Sounds", "Sound Files", "wav;mp3;flac", "wav");

};
