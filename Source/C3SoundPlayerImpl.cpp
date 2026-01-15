// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"

#include <C3SoundPlayerImpl.h>
#include <C3Log.h>

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

	for (size_t i = 0; i < SOUND_TYPE::SOUND_TYPES; i++)
		m_Volume[i] = 1;

	m_ListenerPos.x = 0;
	m_ListenerPos.y = 0;
	m_ListenerPos.z = 0;

	m_ListenerDir.x = 0;
	m_ListenerDir.y = 1;
	m_ListenerDir.z = 0;

	m_ListenerMinRadius = 10.0f;
	m_ListenerMaxRadius = 100.0f;

	// create and configure miniaudio's resource manager
	m_ResManConfig = ma_resource_manager_config_init();

	m_ResManConfig.decodedFormat = SOUND_FORMAT_DEFAULT;
	m_ResManConfig.decodedChannels = SOUND_CHANNELS_DEFAULT;				
	m_ResManConfig.decodedSampleRate = SOUND_RATE_DEFAULT;

	// no threading - Celerity's own resource manager is already multi-threaded
	m_ResManConfig.flags = MA_RESOURCE_MANAGER_FLAG_NO_THREADING;

	ma_resource_manager_init(&m_ResManConfig, &m_ResMan);

	ma_context_config ccfg = ma_context_config_init();
	ma_context_init(nullptr, 0, &ccfg, &m_Context);

	m_pDeviceInfo = nullptr;
	m_DeviceCount = 0;

	// get information about all the audio devices available
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

	// initialize all the playback channels
	for (size_t i = 0; i < m_Channels.size(); i++)
	{
		m_Channels[i].state = ChannelState::inactive;
		m_Channels[i].loop_count = 0;
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

	// initialize the given device
	ma_engine_config ecfg;
	ecfg = ma_engine_config_init();
	ecfg.pResourceManager = &m_ResMan;
	ecfg.listenerCount = 1;
	ecfg.noAutoStart = true;

	ma_result ir = ma_engine_init(&ecfg, &m_Engine);
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

	// "up" is +z
	ma_engine_listener_set_world_up(&m_Engine, 0, 0, 0, 1);

	// default cone
	ma_engine_listener_set_cone(&m_Engine, 0, glm::radians(80.0f), glm::radians(120.f), 0.75f);

	ma_sound_group_config grpcfg[SOUND_TYPE::SOUND_TYPES];
	for (size_t i = 0; i < SOUND_TYPE::SOUND_TYPES; i++)
		grpcfg[i] = ma_sound_group_config_init();

	for (size_t i = 0; i < SOUND_TYPE::SOUND_TYPES; i++)
	{
		ma_sound_group_init(&m_Engine, 0, nullptr, &m_Group[i]);

		// disable spatialization for everything except sound effects...
		// ...music uses its own stereo separation or is mono
		ma_sound_group_set_spatialization_enabled(&m_Group[i], (i == SOUND_TYPE::ST_SFX) ? true : false);
	}

	ma_engine_start(&m_Engine);

	return m_bInitialized;
}


bool SoundPlayerImpl::Initialized()
{
	return m_bInitialized;
}

	
void SoundPlayerImpl::Shutdown()
{
	if (m_bInitialized)
	{
		Stop();

		ma_engine_stop(&m_Engine);

		ma_engine_uninit(&m_Engine);

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

	// called intermittently to update the listener's position and orientation

	ma_engine_listener_set_position(&m_Engine, 0, m_ListenerPos.x, m_ListenerPos.y, m_ListenerPos.z);

	ma_engine_listener_set_direction(&m_Engine, 0, m_ListenerDir.x, m_ListenerDir.y, m_ListenerDir.z);
}


SoundPlayer::HCHANNEL SoundPlayerImpl::Play(Resource *pres, SOUND_TYPE sndtype, float volume, float pitchmult, size_t loopcount, const glm::fvec3 *pos)
{
	if (!m_bInitialized)
		return INVALID_HCHANNEL;

	// make sure that the resource given is a sound - and loaded
	if (!pres || (pres->GetType() != &SoundResourceType::self) || (pres->GetStatus() != Resource::RS_LOADED))
		return INVALID_HSAMPLE;

	const ma_sound *hs = (const ma_sound *)pres->GetData();
	if (!hs)
		return INVALID_HSAMPLE;

	// find an available channel for playback

	size_t chidx = 0;
	TChannel *pch = m_Channels.data();
	for (; chidx < MA_MAX_CHANNELS; chidx++, pch++)
	{
		// any sound that has finished playing must be uninitialized, I think...
		if (pch->state == ChannelState::finished)
		{
			ma_sound_uninit(&(pch->sound));

			pch->state = ChannelState::inactive;
		}

		// any sound that is no loger queued for play can be replaced
		if (pch->state == ChannelState::inactive)
		{
			ma_sound_init_copy(&m_Engine, hs, 0, &m_Group[sndtype], &pch->sound);

			pch->sound.ownsDataSource = false;	// this is key; we are playing a copy of a sound we loaded elsewhere
			pch->loop_count = loopcount;
			pch->state = ChannelState::active;
			break;
		}
	}

	if (chidx >= MA_MAX_CHANNELS)
		return INVALID_HCHANNEL;

	ma_sound *ps = &(pch->sound);

	// configure the sound

	ma_sound_set_volume(ps, volume);

	ma_sound_set_looping(ps, (loopcount == LOOP_INFINITE) ? 1 : 0);

	ma_sound_set_position(ps, pos ? pos->x : 0, pos ? pos->y : 0, pos ? pos->z : 0);

	ma_sound_set_pitch(ps, pitchmult);

	ps->pEndCallbackUserData = pch;

	// this is called by miniaudio when the sample is finished playing back...
	// ...it handles looping for a finite count
	ps->endCallback = [](void *userdata, ma_sound *psound)
	{
		TChannel *pch = (TChannel *)userdata;

		// if we're looping this sound, then check the loop count and restart play if it's greater than 0
		if (pch->loop_count > 0)
		{
			if (pch->loop_count != LOOP_INFINITE)
				pch->loop_count--;

			if (pch->loop_count)
			{
				ma_sound_start(&(pch->sound));

				ma_sound_seek_to_pcm_frame(&(pch->sound), 0);

				ma_sound_set_at_end(&(pch->sound), false);
			}
		}

		// if the playback counter is 0, stop playback
		if (!pch->loop_count)
		{
			ma_sound_stop(&(pch->sound));

			ma_sound_seek_to_pcm_frame(&(pch->sound), 0);

			ma_sound_set_at_end(&(pch->sound), false);

			pch->state = ChannelState::finished;
		}
	};

	ma_sound_start(ps);

	return chidx;
}


void SoundPlayerImpl::Stop(HCHANNEL hc)
{
	if (!m_bInitialized)
		return;

	if (hc >= m_Channels.size())
		return;

	if (hc == SOUND_ALL)
	{
		for (size_t i = 0; i < m_Channels.size(); i++)
			Stop((HCHANNEL)i);

		return;
	}

	TChannelArray::iterator chit = m_Channels.begin() + hc;

	if (chit->state == ChannelState::active)
	{
		ma_sound_stop(&chit->sound);

		chit->state = ChannelState::inactive;
	}
}


void SoundPlayerImpl::Pause(HCHANNEL hc)
{
	if (!m_bInitialized)
		return;

	if (hc >= m_Channels.size())
		return;

	if (hc == SOUND_ALL)
	{
		for (size_t i = 0; i < m_Channels.size(); i++)
			Pause((HCHANNEL)i);

		return;
	}

	TChannelArray::iterator chit = m_Channels.begin() + hc;

	if (chit->state == ChannelState::active)
	{
		ma_sound *ps = &chit->sound;

		ma_sound_stop(ps);
	}
}


void SoundPlayerImpl::Resume(HCHANNEL hc)
{
	if (!m_bInitialized)
		return;

	if (hc >= m_Channels.size())
		return;

	if (hc == SOUND_ALL)
	{
		for (size_t i = 0; i < m_Channels.size(); i++)
			Resume((HCHANNEL)i);

		return;
	}

	TChannelArray::iterator chit = m_Channels.begin() + hc;

	if (chit->state == ChannelState::active)
	{
		ma_sound_start(&chit->sound);
	}
}


SoundPlayer::PLAY_STATUS SoundPlayerImpl::Status(HCHANNEL hc) const
{
	if (!m_bInitialized)
		return SoundPlayer::PLAY_STATUS::PS_ERROR;

	if (hc >= m_Channels.size())
		return SoundPlayer::PLAY_STATUS::PS_ERROR;

	TChannelArray::const_iterator chit = m_Channels.cbegin() + hc;

	if (chit->state != ChannelState::active)
		return SoundPlayer::PLAY_STATUS::PS_STOPPED;

	if (ma_sound_at_end(&chit->sound))
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
			case ST_MUSIC:
			case ST_DIALOG:
				ma_sound_group_set_volume(&m_Group[type], m_Volume[type]);
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

	TChannelArray::iterator chit = m_Channels.begin() + hc;
	if (chit->state != ChannelState::active)
		return;

	float tmpvol = std::min<float>(std::max<float>(0, volume), 1);

	ma_sound_set_volume(&chit->sound, tmpvol * m_Volume[ST_SFX]);
}


void SoundPlayerImpl::SetChannelPitchMod(HCHANNEL hc, float pitchmult)
{
	if (!m_bInitialized)
		return;

	if (hc >= m_Channels.size())
		return;

	TChannelArray::iterator chit = m_Channels.begin() + hc;
	if (chit->state != ChannelState::active)
		return;

	if (pitchmult < 0.1f)
		pitchmult = 0.1f;

	ma_sound_set_pitch(&chit->sound, pitchmult);
}


void SoundPlayerImpl::SetChannelPos(HCHANNEL hc, const glm::fvec3 *pos)
{
	if (!m_bInitialized)
		return;

	if (hc >= m_Channels.size())
		return;

	TChannelArray::iterator chit = m_Channels.begin() + hc;

	if (chit->state != ChannelState::active)
		return;

	ma_sound_set_position(&chit->sound, pos ? pos->x : 0, pos ? pos->y : 0, pos ? pos->z : 0);
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

// this function is called back by the ResourceManager to load a sound resource from a file
c3::ResourceType::LoadResult RESOURCETYPENAME(Sound)::ReadFromFile(c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const
{
	if (returned_data)
	{
		SoundPlayerImpl *sp = (SoundPlayerImpl *)psys->GetSoundPlayer();

		ma_sound *psound = (ma_sound *)malloc(sizeof(ma_sound));

#if UNICODE
		if (ma_sound_init_from_file_w(&(sp->m_Engine), filename, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT, nullptr, nullptr, psound) != MA_SUCCESS)
#else
		if (ma_sound_init_from_file(&(sp->m_Engine), filename, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT, nullptr, nullptr, psound) != MA_SUCCESS)
#endif
		{
			free(psound);
			psound = nullptr;
		}

		*returned_data = psound;
		if (!*returned_data)
			return ResourceType::LoadResult::LR_ERROR;
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


// this function is called back by the ResourceManager to load a sound resource from a location in memory
c3::ResourceType::LoadResult RESOURCETYPENAME(Sound)::ReadFromMemory(c3::System *psys, const TCHAR *contextname, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
{
	SoundPlayerImpl *sp = (SoundPlayerImpl *)psys->GetSoundPlayer();

	ma_sound *psound = (ma_sound *)malloc(sizeof(ma_sound));

	uint32_t flags = MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT | MA_SOUND_FLAG_DECODE;
	//if (ma_sound_init_ex(sp->GetSfxEngine(), ) != MS_SUCCESS)
	if (ma_sound_init_from_data_source(&(sp->m_Engine), (ma_data_source *)buffer, flags, nullptr, psound) != MA_SUCCESS)
	{
		free(psound);
		psound = nullptr;
	}

	*returned_data = psound;
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
