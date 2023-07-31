// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3SoundPlayerImpl.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>


#define SOUND_RATE_DEFAULT			44100	
#define SOUND_CHANNELS_DEFAULT		1					// single channel, because we're spatialized!
#define SOUND_FORMAT_DEFAULT		ma_format_s16		// signed 16-bit audio


using namespace c3;



SoundPlayerImpl::SoundPlayerImpl(System *system)
{
	m_bInitialized = false;
	m_pSys = system;

	m_CD_DevId = 0;
	m_CD_PausePos = 0;

	m_Volume[ST_SFX] = 1;
	m_Volume[ST_MUSIC] = 1;
	m_Volume[ST_CD] = 1;

	m_ListenerPos.x = 0;
	m_ListenerPos.y = 0;
	m_ListenerPos.z = 0;

	m_ListenerDir.x = 0;
	m_ListenerDir.y = 1;
	m_ListenerDir.z = 0;

	m_ListenerMinRadius = 10.0f;
	m_ListenerMaxRadius = 100.0f;

	m_ResManConfig = ma_resource_manager_config_init();
	m_ResManConfig.decodedFormat = SOUND_FORMAT_DEFAULT;
	m_ResManConfig.decodedChannels = SOUND_CHANNELS_DEFAULT;				
	m_ResManConfig.decodedSampleRate = SOUND_RATE_DEFAULT;

	ma_resource_manager_init(&m_ResManConfig, &m_ResMan);

	ma_context_init(nullptr, 0, nullptr, &m_Context);

	m_pDeviceInfo = nullptr;
	m_DeviceCount = 0;

	ma_context_get_devices(&m_Context, &m_pDeviceInfo, &m_DeviceCount, nullptr, nullptr);
	m_DeviceName.resize(m_DeviceCount);
	m_DefaultDevice = 0;
	for (size_t i = 0; i < m_DeviceCount; i++)
	{
		TCHAR *name;
		CONVERT_MBCS2TCS(m_pDeviceInfo[i].name, name);
		m_DeviceName[i] = name;

		if (m_pDeviceInfo[i].isDefault)
			m_DefaultDevice = i;
	}
}


SoundPlayerImpl::~SoundPlayerImpl()
{
	Shutdown();

	ma_context_uninit(&m_Context);

	ma_resource_manager_uninit(&m_ResMan);
}


size_t SoundPlayerImpl::GetDeviceCount() const
{
	return m_DeviceCount;
}


const TCHAR *SoundPlayerImpl::GetDeviceName(size_t devidx) const
{
	if (devidx < m_DeviceCount)
		return m_DeviceName[devidx].c_str();

	return nullptr;
}


bool SoundPlayerImpl::Initialize(size_t devidx)
{
	Shutdown();

	if (devidx > m_DeviceCount)
		devidx = m_DefaultDevice;

	m_DeviceConfig = ma_device_config_init(ma_device_type_playback);
	m_DeviceConfig.playback.format   = SOUND_FORMAT_DEFAULT;
	m_DeviceConfig.playback.channels = SOUND_CHANNELS_DEFAULT;
	m_DeviceConfig.sampleRate        = SOUND_RATE_DEFAULT;
	m_DeviceConfig.dataCallback      = nullptr;
	m_DeviceConfig.pUserData         = this;

	m_pSys->GetLog()->Print(_T("Initializing SoundPlayer... "));

	ma_result ir = ma_device_init(NULL, &m_DeviceConfig, &m_Device);
	m_bInitialized = (ir == MA_SUCCESS);

	switch (ir)
	{
		case MA_SUCCESS:
			m_pSys->GetLog()->Print(_T("ok\n"));
			break;

		default:
			m_pSys->GetLog()->Print(_T("failed\n"));
			break;
	}

	if (m_bInitialized)
	{
		ma_device_start(&m_Device);

		ma_engine_config ecfg;
		ecfg = ma_engine_config_init();
		ecfg.pDevice = &m_Device;
		ecfg.listenerCount = 1;

		for (size_t i = 0; i < SOUND_TYPES; i++)
		{
			ma_engine_init(nullptr, &m_Engine[i]);
		}

		ma_engine_listener_set_world_up(&m_Engine[ST_SFX], 0, 0, 0, 1);
		ma_engine_listener_set_cone(&m_Engine[ST_SFX], 0, glm::radians(80.0f), glm::radians(120.f), 0.75f);

		return true;
	}

	return false;
}
	
	
void SoundPlayerImpl::Shutdown()
{
	if (m_bInitialized)
	{
		for (size_t i = 0; i < SOUND_TYPES; i++)
			ma_engine_uninit(&m_Engine[i]);

		ma_device_uninit(&m_Device);

		m_bInitialized = false;
	}
}


void SoundPlayerImpl::SetListenerPos(const glm::fvec3 *lpos)
{
	if (lpos)
	{
		m_ListenerPos = *lpos;
	}
	else
	{
		m_ListenerPos.x = 0;
		m_ListenerPos.y = 0;
		m_ListenerPos.z = 0;
	}

	ma_engine_listener_set_position(&m_Engine[ST_SFX], 0, m_ListenerPos.x, m_ListenerPos.y, m_ListenerPos.z);
}


const glm::fvec3 *SoundPlayerImpl::GetListenerPos(glm::fvec3 *lpos) const
{
	if (lpos)
	{
		*lpos = m_ListenerPos;

		return lpos;
	}

	return &m_ListenerPos;
}


void SoundPlayerImpl::SetListenerDir(const glm::fvec3 *ldir)
{
	if (ldir)
	{
		m_ListenerDir = *ldir;
	}
	else
	{
		m_ListenerDir.x = 0;
		m_ListenerDir.y = 1;
	}

	m_ListenerDir.y += 0.001f;	// no zero-len
	m_ListenerDir.z = 0;

	m_ListenerDir = glm::normalize(m_ListenerDir);

	ma_engine_listener_set_direction(&m_Engine[ST_SFX], 0, m_ListenerDir.x, m_ListenerDir.y, m_ListenerDir.z);
}


const glm::fvec3 *SoundPlayerImpl::GetListenerDir(glm::fvec3 *ldir) const
{
	if (ldir)
	{
		*ldir = m_ListenerDir;

		return ldir;
	}

	return &m_ListenerDir;
}


void SoundPlayerImpl::SetListenerRadius(float min_rad, float max_rad)
{
	if (!m_bInitialized)
		return;
}


void SoundPlayerImpl::Update(float elapsed_seconds)
{
	if (!m_bInitialized)
		return;

	ma_engine_listener_set_position(&m_Engine[ST_SFX], 0, m_ListenerPos.x, m_ListenerPos.y, m_ListenerPos.z);
	ma_engine_listener_set_direction(&m_Engine[ST_SFX], 0, m_ListenerDir.x, m_ListenerDir.y, m_ListenerDir.z);
}


SoundPlayer::HCHANNEL SoundPlayerImpl::Play(HSAMPLE hs, float volume, float pitchmult, size_t loopcount, const glm::fvec3 *pos)
{
	if (!m_bInitialized)
		return INVALID_HCHANNEL;

	TChannelArray::iterator chit = m_Channels.begin();
	for (; chit != m_Channels.end(); chit++)
	{
		if (!chit->first)
		{
			ma_sound tmp;
			ma_sound_init_copy(&m_Engine[ST_SFX], (const ma_sound *)hs, 0, nullptr, &tmp);
			chit->second = tmp;
			chit->first = loopcount;
			break;
		}
	}

	if (chit == m_Channels.end())
		return INVALID_HCHANNEL;

	HCHANNEL hc = (HCHANNEL)std::distance(m_Channels.begin(), chit);
	ma_sound *ps = &chit->second;

	ma_sound_set_volume(ps, volume * m_Volume[ST_SFX]);
	ma_sound_set_looping(ps, (loopcount == LOOP_INFINITE) ? 1 : 0);
	ma_sound_set_position(ps, pos ? pos->x : 0, pos ? pos->y : 0, pos ? pos->z : 0);
	ma_sound_set_pitch(ps, pitchmult);
	ps->pEndCallbackUserData = &chit->first;
	ps->endCallback = [](void *userdata, ma_sound *psound)
	{
		size_t *count = (size_t *)userdata;

		if (*count > 0)
		{
			*count--;

			if (*count)
				ma_sound_start(psound);
		}
	};

	ma_sound_start(ps);

	return hc;
}


void SoundPlayerImpl::Stop(HCHANNEL hc)
{
	if (!m_bInitialized)
		return;

	if (hc == SOUND_ALL)
	{
		for (size_t i = 0; i < m_Channels.size(); i++)
			Stop((HCHANNEL)i);

		return;
	}

	if (hc >= m_Channels.size())
		return;

	TChannelArray::iterator chit = m_Channels.begin() + hc;
	ma_sound *ps = &chit->second;

	ma_sound_stop(ps);

	chit->first = 0;
}


void SoundPlayerImpl::Pause(HCHANNEL hc)
{
	if (!m_bInitialized)
		return;

	if (hc == SOUND_ALL)
	{
		for (size_t i = 0; i < m_Channels.size(); i++)
			Pause((HCHANNEL)i);

		return;
	}

	if (hc >= m_Channels.size())
		return;

	TChannelArray::iterator chit = m_Channels.begin() + hc;
	ma_sound *ps = &chit->second;

	ma_sound_stop(ps);
}


void SoundPlayerImpl::Resume(HCHANNEL hc)
{
	if (!m_bInitialized)
		return;

	if (hc == SOUND_ALL)
	{
		return;
	}

	if (hc >= m_Channels.size())
		return;

	TChannelArray::iterator chit = m_Channels.begin() + hc;
	ma_sound *ps = &chit->second;

	if (chit->first)
		ma_sound_start(ps);
}


SoundPlayer::PLAY_STATUS SoundPlayerImpl::Status(HCHANNEL hc) const
{
	if (!m_bInitialized)
		return SoundPlayer::PLAY_STATUS::PS_ERROR;

	if (hc >= m_Channels.size())
		return SoundPlayer::PLAY_STATUS::PS_ERROR;

	TChannelArray::const_iterator chit = m_Channels.cbegin() + hc;
	const ma_sound *ps = &chit->second;

	if (!chit->first || ma_sound_at_end(ps))
		return SoundPlayer::PLAY_STATUS::PS_STOPPED;

	return SoundPlayer::PLAY_STATUS::PS_PLAYING;
}


float SoundPlayerImpl::GetVolume(SOUND_TYPE type)
{
#if 0
	if (type == SOUND_CD)
	{
		uint32_t cdlevel;
		cdlevel = MCIWndGetVolume(m_hWnd);
		cdlevel = (uint32_t) (((float)cdlevel / 1000.0f) * (float)(SOUND_MAXVOLUME - SOUND_MINVOLUME));

		vol[SOUND_CD] = cdlevel;
	}
#endif

	return m_Volume[type];
}


void SoundPlayerImpl::SetVolume(SOUND_TYPE type, float volume)
{
	m_Volume[type] = std::min<float>(std::max<float>(0, volume), 1);

	if (m_bInitialized)
	{
		switch (type)
		{
			case ST_SFX:
				break;

			case ST_MUSIC:
				break;

			case ST_CD:
			{
#if 0
				uint32_t tmplevel = (uint32_t)(((float)(vol[SOUND_CD] - SOUND_MINVOLUME) / (float)(SOUND_MAXVOLUME - SOUND_MINVOLUME)) * 1000.0f);

				MCIWndSetVolume(m_hWnd, tmplevel);
#endif

				break;
			}
		}
	}
}


void SoundPlayerImpl::SetChannelVolume(HCHANNEL hc, float volume)
{
	if (!m_bInitialized)
		return;

	if (hc >= m_Channels.size())
		return;

	float tmpvol = std::min<float>(std::max<float>(0, volume), 1);

	TChannelArray::iterator chit = m_Channels.begin() + hc;
	ma_sound *ps = &chit->second;

	ma_sound_set_volume(ps, tmpvol * m_Volume[ST_SFX]);
}


void SoundPlayerImpl::SetChannelPos(HCHANNEL hc, const glm::fvec3 *pos)
{
	if (!m_bInitialized)
		return;

	if (hc >= m_Channels.size())
		return;

	TChannelArray::iterator chit = m_Channels.begin() + hc;
	ma_sound *ps = &chit->second;

	ma_sound_set_position(ps, pos ? pos->x : 0, pos ? pos->y : 0, pos ? pos->z : 0);
}


#if 0

C2_ERR SoundPlayerImpl::CDPlay(uint32_t tracknumber)
{
	if (cd_devid < 0)
		return C2_AUD_BADCDDEV;

	if (cd_trackinfo && (tracknumber < cd_trackinfo->size()))
	{
		MCI_PLAY_PARMS mciplay;

		mciplay.dwFrom = cd_trackinfo->at(tracknumber).startms;
		mciplay.dwTo = cd_trackinfo->at(tracknumber).endms;

		mciSendCommand(cd_devid, MCI_PLAY, MCI_FROM | MCI_TO, (DWORD)(LPVOID)&mciplay);

		CDUpdateStatus();

		return C2_OK;
	}

	return C2_AUD_INVALIDCDTRACK;
}


C2_ERR SoundPlayerImpl::CDStop()
{
	if (cd_devid < 0)
		return C2_AUD_BADCDDEV;

	if (cd_status != PLAYSTATUS_PLAYING)
		return C2_OK;

	mciSendCommand(cd_devid, MCI_STOP, MCI_WAIT, NULL);

	CDUpdateStatus();

	return C2_OK;
}


C2_ERR SoundPlayerImpl::CDPause()
{
	if (cd_devid < 0)
		return C2_AUD_BADCDDEV;

	if (cd_status != PLAYSTATUS_PLAYING)
		return C2_OK;

	cd_pausepos = CDPosition();
	CDStop();

	return C2_OK;
}


C2_ERR SoundPlayerImpl::CDResume()
{
	if (cd_devid < 0)
		return C2_AUD_BADCDDEV;

	if ((cd_status != PLAYSTATUS_PAUSED) || (cd_pausepos == 0))
		return C2_AUD_CDNOTPAUSED;

	MCI_PLAY_PARMS mciplay;

	mciplay.dwFrom = cd_pausepos;
	mciplay.dwTo = cd_trackinfo->at(CDFindTrackByStart(cd_pausepos)).endms;

	mciSendCommand(cd_devid, MCI_PLAY, MCI_FROM | MCI_TO, (DWORD)(LPVOID)&mciplay);

	cd_pausepos = 0;

	CDUpdateStatus();

	return C2_OK;
}


PLAYSTATUS SoundPlayerImpl::CDUpdateStatus()
{
	if (cd_devid < 0)
		return PLAYSTATUS_ERROR;

	MCI_STATUS_PARMS mcistatus;

	mcistatus.dwItem = MCI_STATUS_MODE;

	mciSendCommand(cd_devid, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&mcistatus);

	switch (mcistatus.dwReturn)
	{
		case MCI_MODE_PLAY:
			cd_status = PLAYSTATUS_PLAYING;
			break;

		case MCI_MODE_STOP:
			if (!cd_pausepos)
				cd_status = PLAYSTATUS_STOPPED;
			else
				cd_status = PLAYSTATUS_PAUSED;
			break;
	}

	return cd_status;
}


uint32_t SoundPlayerImpl::CDPosition()
{
	if (cd_devid < 0)
		return 0;

	MCI_STATUS_PARMS mcistatus;

	mcistatus.dwItem = MCI_STATUS_POSITION;

	mciSendCommand(cd_devid, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&mcistatus);

	return mcistatus.dwReturn;
}


uint32_t SoundPlayerImpl::CDNumTracks()
{
	if (cd_devid < 0)
		return 0;

	MCI_STATUS_PARMS mcistatus;

	mcistatus.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;

	mciSendCommand(cd_devid, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD)(LPVOID)&mcistatus);

	if (mcistatus.dwReturn > 256)
		mcistatus.dwReturn = 0;

	return mcistatus.dwReturn;
}


C2_ERR SoundPlayerImpl::CDAcquireTrackInfo()
{
	if (!CDNumTracks())
		return C2_OK;

	if (!cd_trackinfo)
		cd_trackinfo = new TCDTrackInfoArray();
	else
		cd_trackinfo->clear();

	if (!cd_trackinfo)
		return C2_OUTOFMEM;

	// MCI tracks are 1-based, not 0-based... so start at 1, not 0 when acquiring track information
	for (uint32_t i = 1; i <= CDNumTracks(); i++)
	{
		MCI_STATUS_PARMS mcistatus;

		SCDTrackInfo ti;

		mcistatus.dwItem = MCI_STATUS_POSITION;
		mcistatus.dwTrack = i;

		mciSendCommand(cd_devid, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, (DWORD)(LPVOID)&mcistatus);

		ti.startms = mcistatus.dwReturn;

		mcistatus.dwItem = MCI_STATUS_LENGTH;

		mciSendCommand(cd_devid, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, (DWORD)(LPVOID)&mcistatus);

		if (i == CDNumTracks())
			mcistatus.dwReturn-=15;

		ti.endms = ti.startms + mcistatus.dwReturn;

		cd_trackinfo->push_back(ti);
	}

	return C2_OK;
}


int32_t SoundPlayerImpl::CDFindTrackByStart(uint32_t startms)
{
	for (uint32_t i = 0; i < cd_trackinfo->size(); i++)
	{
		if ((startms >= cd_trackinfo->at(i).startms) && (startms <= cd_trackinfo->at(i).endms))
			return i;
	}

	return -1;
}

void SoundPlayerImpl::EnableCDAudio()
{
	// Open our MCI device for playing redbook audio...
	{
		MCI_OPEN_PARMS mciopen;

		mciopen.lpstrDeviceType = (LPCTSTR)MCI_DEVTYPE_CD_AUDIO;

		if (mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID, (DWORD)(LPVOID)&mciopen))
			mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE, (DWORD)(LPVOID)&mciopen);

		cd_devid = mciopen.wDeviceID;
	}

	// If an MCI device was successfully opened, then do the following...
	if (cd_devid >= 0)
	{
		// Set the time format to milliseconds...
		MCI_SET_PARMS mciset;
		mciset.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
		mciSendCommand(cd_devid, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&mciset);

		CDUpdateStatus();

		CDAcquireTrackInfo();

		m_pSys->Log()->Log(_T("SoundPlayerImpl: Initialized CD/Redbook Audio Device!\n"));
	}

	cd_active = true;
}


void SoundPlayerImpl::DisableCDAudio()
{
	if (cd_active)
	{
		CDStop();

		if (cd_devid >= 0)
			mciSendCommand(cd_devid, MCI_CLOSE, MCI_WAIT, 0);

		cd_active = false;
	}

	if (cd_trackinfo)
	{
		delete cd_trackinfo;
		cd_trackinfo = NULL;
	}
}

#endif


DECLARE_RESOURCETYPE(Sound);

c3::ResourceType::LoadResult RESOURCETYPENAME(Sound)::ReadFromFile(c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const
{
	if (returned_data)
	{
		SoundPlayerImpl *sp = (SoundPlayerImpl *)psys->GetSoundPlayer();

		ma_sound *sound = (ma_sound *)malloc(sizeof(ma_sound));
#if UNICODE
		if (ma_sound_init_from_file_w(sp->GetSfxEngine(), filename, 0, nullptr, nullptr, sound) != MA_SUCCESS)
#else
		if (ma_sound_init_from_file(sp->GetSfxEngine(), filename, 0, nullptr, nullptr, sound) != MA_SUCCESS)
#endif
		{
			free(sound);
			sound = nullptr;
		}

		*returned_data = sound;
		if (!*returned_data)
			return ResourceType::LoadResult::LR_ERROR;
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(Sound)::ReadFromMemory(c3::System *psys, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
{
	SoundPlayerImpl *sp = (SoundPlayerImpl *)psys->GetSoundPlayer();

	ma_sound *sound = (ma_sound *)malloc(sizeof(ma_sound));
	if (ma_sound_init_from_data_source(sp->GetSfxEngine(), (ma_data_source *)buffer, 0, nullptr, sound) != MA_SUCCESS)
	{
		free(sound);
		sound = nullptr;
	}
	
	*returned_data = sound;
	if (!*returned_data)
		return ResourceType::LoadResult::LR_ERROR;

	return ResourceType::LoadResult::LR_SUCCESS;
}


bool RESOURCETYPENAME(Sound)::WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const
{
	return false;
}


void RESOURCETYPENAME(Sound)::Unload(void *data) const
{
	ma_sound_uninit((ma_sound *)data);
	free(data);
	data = nullptr;
}
