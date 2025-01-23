// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


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



bool ReadPositionsFromXAF(AnimTrack *ptrack, tinyxml2::XMLElement *psamples, float time_scale)
{
	if (!psamples)
		return false;

	tinyxml2::XMLElement *elpval = psamples->FirstChildElement("PVal");
	while (elpval)
	{
		const tinyxml2::XMLAttribute *attime = elpval->FindAttribute("t");
		const tinyxml2::XMLAttribute *atv = elpval->FindAttribute("v");
		if (attime && atv)
		{
			float t = attime->FloatValue() * time_scale;

			glm::fvec3 p;
			sscanf_s(atv->Value(), "%f %f %f", &p.x, &p.y, &p.z);

			ptrack->AddPosKey(t, p);
		}

		elpval = elpval->NextSiblingElement("PVal");
	}

	return true;
}


bool ReadRotationsFromXAF(AnimTrack *ptrack, tinyxml2::XMLElement *psamples, float time_scale)
{
	if (!psamples)
		return false;

	tinyxml2::XMLElement *elrval = psamples->FirstChildElement("RVal");
	while (elrval)
	{
		const tinyxml2::XMLAttribute *attime = elrval->FindAttribute("t");
		const tinyxml2::XMLAttribute *atv = elrval->FindAttribute("v");
		if (attime && atv)
		{
			float t = attime->FloatValue() * time_scale;

			glm::fquat q;
			sscanf_s(atv->Value(), "%f %f %f %f", &q.x, &q.y, &q.z, &q.w);

			ptrack->AddOriKey(t, q);
		}

		elrval = elrval->NextSiblingElement("RVal");
	}

	return true;
}


bool ReadScalesFromXAF(AnimTrack *ptrack, tinyxml2::XMLElement *psamples, float time_scale)
{
	if (!psamples)
		return false;

	tinyxml2::XMLElement *elsval = psamples->FirstChildElement("SVal");
	while (elsval)
	{
		const tinyxml2::XMLAttribute *attime = elsval->FindAttribute("t");
		const tinyxml2::XMLAttribute *atv = elsval->FindAttribute("v");
		if (attime && atv)
		{
			float t = attime->FloatValue() * time_scale;

			glm::fvec3 s;
			sscanf_s(atv->Value(), "%f %f %f", &s.x, &s.y, &s.z);

			ptrack->AddSclKey(t, s);
		}

		elsval = elsval->NextSiblingElement("SVal");
	}

	return true;
}


bool ReadAnimFromXAF(Animation *panim, tinyxml2::XMLElement *xafdom)
{
	if (!xafdom)
		return false;

	tinyxml2::XMLElement *elroot = _stricmp(xafdom->Name(), "MaxAnimation") ? xafdom->FirstChildElement("MaxAnimation") : xafdom;

	if (elroot)
	{
		float startTick = 0.0f;
		float endTick = 0.0f;
		float fps = 30.0f;
		float tpf = 1.0f;

		tinyxml2::XMLElement *sinode = xafdom->FirstChildElement("SceneInfo");
		if (sinode)
		{
			const tinyxml2::XMLAttribute *atstart = sinode->FindAttribute("startTick");
			if (atstart)
				startTick = atstart->FloatValue();

			const tinyxml2::XMLAttribute *atend = sinode->FindAttribute("endTick");
			if (atend)
				endTick = atend->FloatValue();

			const tinyxml2::XMLAttribute *atfps = sinode->FindAttribute("frameRate");
			if (atfps)
				fps = atfps->FloatValue();

			const tinyxml2::XMLAttribute *attpf = sinode->FindAttribute("ticksPerFrame");
			if (attpf)
				tpf = attpf->FloatValue();
		}

		tinyxml2::XMLElement *elnode = xafdom->FirstChildElement("Node");
		while (elnode)
		{
			const tinyxml2::XMLAttribute *atname = elnode->FindAttribute("name");
			const tinyxml2::XMLAttribute *atpar = elnode->FindAttribute("parentNode");
			if (atname && atpar)
			{
				TCHAR *nodename, *parentname;
				CONVERT_MBCS2TCS(atname->Value(), nodename);
				CONVERT_MBCS2TCS(atpar->Value(), parentname);

				Animation::TrackIndex ti = panim->AddNewTrack(nodename, panim->FindTrackByName(parentname));
				AnimTrack *pt = panim->GetTrack(ti);
				if (pt)
				{
					tinyxml2::XMLElement *elctl = elnode->FirstChildElement("Controller");
					while (elctl)
					{
						const tinyxml2::XMLAttribute *atfilt = elctl->FindAttribute("filterType");
						if (atfilt)
						{
							tinyxml2::XMLElement *elsamples = elctl->FirstChildElement("Samples");

							if (!_stricmp(atfilt->Value(), "pos"))
							{
								ReadPositionsFromXAF(pt, elsamples, 1.0f / fps / tpf);
							}
							else if (!_stricmp(atfilt->Value(), "rot"))
							{
								ReadRotationsFromXAF(pt, elsamples, 1.0f / fps / tpf);
							}
							else if (!_stricmp(atfilt->Value(), "scl"))
							{
								ReadScalesFromXAF(pt, elsamples, 1.0f / fps / tpf);
							}
						}

						elctl = elctl->NextSiblingElement("Controller");
					}
				}
			}

			elnode = elnode->NextSiblingElement("Node");
		}

		return true;
	}

	return false;
}

c3::ResourceType::LoadResult RESOURCETYPENAME(Animation)::ReadFromFile(c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const
{
	c3::ResourceType::LoadResult ret = c3::ResourceType::LR_ERROR;

	if (returned_data)
	{
		*returned_data = nullptr;

		const TCHAR *ext = PathFindExtension(filename);
		if (!_tcsicmp(ext, _T(".xaf")))
		{
			char *s;
			CONVERT_TCS2MBCS(filename, s);

			tinyxml2::XMLDocument xafdoc;
			if (xafdoc.LoadFile(s) == tinyxml2::XMLError::XML_SUCCESS)
			{
				Animation *panim = Animation::Create();
				*returned_data = (void *)panim;

				if (ReadAnimFromXAF(panim, xafdoc.RootElement()))
				{
					panim->SetName(filename);
					ret = c3::ResourceType::LoadResult::LR_SUCCESS;
				}
			}
		}
	}

	return ret;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(Animation)::ReadFromMemory(c3::System *psys, const TCHAR *contextname, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
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
