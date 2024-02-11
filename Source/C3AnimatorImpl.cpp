// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include "C3AnimatorImpl.h"
#include "C3AnimTrackImpl.h"
#include "C3AnimationImpl.h"

using namespace c3;


DECLARE_COMPONENTTYPE(Animator, AnimatorImpl);


#define DEFAULT_ANIMSTATE	_T("Default")


AnimatorImpl::AnimatorImpl()
{
	m_Owner = nullptr;

	m_CurState = m_LastState = m_StateMap.end();
	m_CurAnimTime = m_LastAnimTime = 0.0f;
	m_CurAnim = nullptr;

	m_Flags = AF_FORCENEXT;

	m_MatStack = MatrixStack::Create();
	m_StateProp = nullptr;
}


AnimatorImpl::~AnimatorImpl()
{
	C3_SAFERELEASE(m_MatStack);

	ResetStates();
}


void AnimatorImpl::Release()
{
	if (m_StateProp)
		m_StateProp->SetString(m_StateProp->AsString());

	delete this;
}


props::TFlags64 AnimatorImpl::Flags() const
{
	return m_Flags;
}


size_t AnimatorImpl::GetNumValues(const props::IProperty *pprop) const
{
	assert(pprop);

	switch (pprop->GetID())
	{
		case 'ST8':
		{
			size_t ret = m_StateMap.size();
			return std::max<size_t>(1, ret);
			break;
		}

		default:
			break;
	}

	return 1;
}


const TCHAR *AnimatorImpl::GetValue(const props::IProperty *pprop, size_t ordinal, TCHAR *buf, size_t bufsize) const
{
	assert(pprop);

	const TCHAR *ret = nullptr;

	switch (pprop->GetID())
	{
		case 'ST8':
		{
			if (!m_StateMap.empty())
			{
				auto s = m_StateMap.begin();
				while (ordinal--)
					s++;

				ret = s->first.c_str();
			}
			else
				ret = DEFAULT_ANIMSTATE;

			break;
		}

		default:
			break;
	}

	if (ret && buf && bufsize)
		_tcscpy_s(buf, bufsize, ret);

	return ret;
}


bool AnimatorImpl::Initialize(Object *pobject)
{
	m_Owner = pobject;

	props::IPropertySet *props = m_Owner->GetProperties();
	if (!props)
		return false;

	props::IProperty *pp = props->GetPropertyById('ST8F');
	if (!pp)
	{
		pp = props->CreateProperty(_T("StateDefinitionsFile"), 'ST8F');
		pp->SetString(_T(""));
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FILENAME);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}
	PropertyChanged(pp);

	if (m_StateProp = props->CreateProperty(_T("State"), 'ST8'))
	{
		const TCHAR *_s = m_StateProp->AsString();
		tstring s = _s ? _s : DEFAULT_ANIMSTATE;
		m_StateProp->SetEnumProvider(this);
		// maybe this property already existed on the object - if so, get the state index to restore later
		m_StateProp->SetEnumValByString(s.c_str());
	}

	return true;
}


void AnimatorImpl::GenerateNodeToTrackMapping()
{
	if (!m_CurAnim)
		return;

	ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(m_Owner->FindComponent(ModelRenderer::Type()));
	if (!pmr)
		return;

	const Model *pm = pmr->GetModel();
	if (!pm)
		return;

	Model::InstanceData *pmid = pmr->GetModelInstanceData();

	size_t nc = pm->GetNodeCount();
	if (m_NodeToTrack.size() < nc)
		m_NodeToTrack.resize(nc);

	if (m_KeyIndices.size() < nc)
		m_KeyIndices.resize(nc);

	for (size_t n = 0; n < nc; n++)
	{
		const TCHAR *name = pm->GetNodeName(n);

		Animation::TrackIndex ti = m_CurAnim->FindTrackByName(name);

		m_NodeToTrack[n] = ti;

		m_KeyIndices[n].m_Pos = AnimTrack::KEYINDEX_INVALID;
		m_KeyIndices[n].m_Ori = AnimTrack::KEYINDEX_INVALID;
		m_KeyIndices[n].m_Scl = AnimTrack::KEYINDEX_INVALID;
	}
}


void AnimatorImpl::SelectAnimation()
{
	auto oldanim = m_CurAnim;

	if (m_CurState != m_StateMap.end())
	{
		// Firgure out which random animation to play

		// get the total weight of all animations in the state, then iteratively decerement that by the weights of individual animations until it's less than one of them
		size_t animchoice = rand() % m_CurState->second->m_TotalWeight;

		for (size_t i = 0, maxi = m_CurState->second->m_WeightedAnims.size(); i < maxi; i++)
		{
			size_t w = m_CurState->second->m_WeightedAnims[i].m_Weight;
			if (animchoice < w)
			{
				m_CurAnim = m_CurState->second->m_WeightedAnims[i].m_Anim;
				break;
			}

			animchoice -= w;
		}
	}

	if (!m_CurAnim)
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(m_Owner->FindComponent(ModelRenderer::Type()));
		if (pmr)
		{
			const Model *pm = pmr->GetModel();
			if (pm)
				m_CurAnim = pm->GetDefaultAnim();
		}
	}

	if (m_CurAnim != oldanim)
		GenerateNodeToTrackMapping();
}


void AnimatorImpl::AdvanceState()
{
	// save the last state and set the current one based on the "goto" state
	m_LastState = m_CurState;

	if (m_LastState != m_StateMap.end())
	{
		m_CurState = m_StateMap.find(m_LastState->second->m_GotoName);

		SelectAnimation();
	}

	m_CurAnimTime = 0;
}


void AnimatorImpl::Update(float elapsed_time)
{
	auto oldanim = m_CurAnim;

	// store tha last animation time
	m_LastAnimTime = m_CurAnimTime;

	// advance the animation time by the time that's passed...
	m_CurAnimTime += elapsed_time;

	if (!m_CurAnim || (m_CurAnimTime > GetCurAnimLength()))
	{
		AdvanceState();

		if (!m_CurAnim)
			return;
	}

	if (m_CurAnim && ((m_CurAnimTime != m_LastAnimTime) || (m_CurAnim != oldanim)))
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(m_Owner->FindComponent(ModelRenderer::Type()));
		const Model *pm = pmr->GetModel();
		Model::InstanceData *pmid = pmr->GetModelInstanceData();

		if (pm && pmid)
		{
			if (m_NodeToTrack.size() != pm->GetNodeCount())
				GenerateNodeToTrackMapping();

			glm::fmat4x4 m, ident = glm::identity<glm::fmat4x4>();

			for (size_t n = 0, maxn = pm->GetNodeCount(); n < maxn; n++)
			{
				Animation::TrackIndex ti = m_NodeToTrack[n];

				if (ti != Animation::TRACKINDEX_INVALID)
				{
					AnimTrack *pat = m_CurAnim->GetTrack(ti);

					// if there's a note on the current key and we just now reached it (last time was before this key) then process it
					// TODO: evaluate for correctness - this might be better in Update than Prerender. <shrug>
					if (const TCHAR *pnote = pat->GetNote(m_LastAnimTime, m_CurAnimTime))
						ProcessNote(pnote);

					AnimTrack::KeyIndex kip = AnimTrack::KEYINDEX_INVALID, kio = kip, kis = kio;
					glm::fvec3 apos = pat->GetPos(m_CurAnimTime, kip);//m_KeyIndices[ti].m_Pos);
					glm::fquat aori = pat->GetOri(m_CurAnimTime, kio);//m_KeyIndices[ti].m_Ori);
					glm::fvec3 ascl = pat->GetScl(m_CurAnimTime, kis);//m_KeyIndices[ti].m_Scl);

					m = glm::scale(glm::identity<glm::fmat4x4>(), ascl) * (glm::fmat4x4)(aori);

					// Then translate last... 
					m = glm::translate(glm::identity<glm::fmat4x4>(), apos) * m;

					pmid->SetTransform(n, m);
				}
				else
				{
					pmid->SetTransform(n, ident);
				}
			}
		}
	}
}


bool AnimatorImpl::Prerender(Object::RenderFlags flags, int draworder)
{
	return false;
}


void AnimatorImpl::Render(Object::RenderFlags flags)
{

}


void AnimatorImpl::ResetStates()
{
	for (auto it : m_StateMap)
		delete it.second;

	m_StateMap.clear();

	m_CurState = m_StateMap.end();
	m_LastState = m_StateMap.end();
}


void AnimatorImpl::PropertyChanged(const props::IProperty *pprop)
{
	assert(pprop);

	switch (pprop->GetID())
	{
		case 'ST8F':
		{
			System *psys = m_Owner->GetSystem();
			ResourceManager *prm = psys->GetResourceManager();

			ResetStates();

			TCHAR filename[1024];
			pprop->AsString(filename, 1023);

			Resource *pr = prm->GetResource(filename, RESF_DEMANDLOAD);
			if (pr && (pr->GetStatus() == Resource::Status::RS_LOADED))
			{
				tinyxml2::XMLDocument *pd = (tinyxml2::XMLDocument *)pr->GetData();

				tinyxml2::XMLElement *pstates = pd->FirstChildElement("states");
				if (pstates)
				{
					const tinyxml2::XMLAttribute *pstartstate = pstates->FindAttribute("startstate");
					if (pstartstate)
					{
						TCHAR *_startstate;
						CONVERT_MBCS2TCS(pstartstate->Value(), _startstate);
						m_StartState = _startstate;
					}

					tinyxml2::XMLElement *pstate = pstates->FirstChildElement("state");
					while (pstate)
					{
						const tinyxml2::XMLAttribute *pname = pstate->FindAttribute("name");
						if (pname)
						{
							TCHAR *_name;
							CONVERT_MBCS2TCS(pname->Value(), _name);

							std::pair<AnimStateMap::iterator, bool> emres = m_StateMap.insert(AnimStateMap::value_type(_name, new AnimStateInfo()));

							if (!emres.second)
							{
								m_Owner->GetSystem()->GetLog()->Print(_T("\"%s\" contains duplicate state: \"%s\"\n"), filename, _name);
							}
							else
							{
								emres.first->second->m_Name = _name;

								const tinyxml2::XMLAttribute *pgoto = pstate->FindAttribute("goto");
								if (pgoto)
								{
									TCHAR *_goto;
									CONVERT_MBCS2TCS(pgoto->Value(), _goto);
									emres.first->second->m_GotoName = _goto;
								}
								else
								{
									emres.first->second->m_GotoName = _name;
								}

								tinyxml2::XMLElement *panim = pstate->FirstChildElement("animation");
								while (panim)
								{
									const tinyxml2::XMLAttribute *pfilename = panim->FindAttribute("filename");
									if (pfilename)
									{
										TCHAR *_filename;
										CONVERT_MBCS2TCS(pfilename->Value(), _filename);

										Resource *animres = prm->GetResource(_filename, RESF_DEMANDLOAD);
										if (animres && animres->GetData())
										{
											emres.first->second->m_WeightedAnims.emplace_back();
											emres.first->second->m_WeightedAnims.back().m_Anim = (Animation *)animres->GetData();

											const tinyxml2::XMLAttribute *pweight = panim->FindAttribute("weight");
											size_t wt = pweight ? pweight->IntValue() : 1;
											emres.first->second->m_WeightedAnims.back().m_Weight = wt;
											emres.first->second->m_TotalWeight += wt;
										}
									}

									panim = panim->NextSiblingElement("animation");
								}
							}
						}

						pstate = pstate->NextSiblingElement("state");
					}
				}

				SetCurrentState(m_StartState.c_str());
			}
			// load states
			break;
		}

		case 'MODF':
		{
			// remap nodes
			break;
		}

		default:
			break;
	}
}


bool AnimatorImpl::ProcessNote(const TCHAR *note)
{
	if (m_Owner)
	{
		Scriptable *ps = (Scriptable *)(m_Owner->FindComponent(Scriptable::Type()));
		if (ps)
		{
			ps->Execute(note);
			return true;
		}
	}

	return false;;
}


bool AnimatorImpl::HasState(const TCHAR *name) const
{
	if (!name)
		return false;

	size_t namelen = _tcslen(name) + 1;
	TCHAR *_name = (TCHAR *)_alloca(sizeof(TCHAR) * namelen);

	for (size_t i = 0; i < namelen; i++)
		_name[i] = std::tolower(name[i], std::locale());

	return (m_StateMap.find(name) != m_StateMap.end());
}


const TCHAR *AnimatorImpl::GetCurrentState() const
{
	if (m_CurState != m_StateMap.end())
		return m_CurState->first.c_str();

	return nullptr;
}


void AnimatorImpl::SetCurrentState(const TCHAR *name)
{
	if (!name)
		return;

	size_t namelen = _tcslen(name) + 1;
	TCHAR *_name = (TCHAR *)_alloca(sizeof(TCHAR) * namelen);

	for (size_t i = 0; i < namelen; i++)
		_name[i] = std::tolower(name[i], std::locale());

	m_CurState = m_StateMap.find(_name);

	m_Flags.Set(AF_FORCENEXT);
}


void AnimatorImpl::ForceStateChange()
{
	m_Flags.Set(AF_FORCENEXT);
}


float AnimatorImpl::GetCurAnimLength() const
{
	return m_CurAnim ? m_CurAnim->GetLength() : 0;
}


float AnimatorImpl::GetCurAnimTime() const
{
	return m_CurAnimTime;
}


bool AnimatorImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats, float *pDistance) const
{
	return false;
}


DECLARE_RESOURCETYPE(AnimStatesDesc);


c3::ResourceType::LoadResult RESOURCETYPENAME(AnimStatesDesc)::ReadFromFile(c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const
{
	c3::ResourceType::LoadResult ret = c3::ResourceType::LR_ERROR;

	if (returned_data)
	{
		*returned_data = new tinyxml2::XMLDocument();

		FILE *f;
		if (!_tfopen_s(&f, filename, _T("rb, ccs=UTF-8")))
		{
			if (!((tinyxml2::XMLDocument *)*returned_data)->LoadFile(f))
				ret = c3::ResourceType::LR_SUCCESS;

			fclose(f);
		}
	}

	return ret;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(AnimStatesDesc)::ReadFromMemory(c3::System *psys, const TCHAR *contextname, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
{
	c3::ResourceType::LoadResult ret = c3::ResourceType::LR_ERROR;

	if (returned_data)
	{
		*returned_data = new tinyxml2::XMLDocument();

		if (!((tinyxml2::XMLDocument *)*returned_data)->Parse((const char *)buffer, buffer_length))
			ret = c3::ResourceType::LR_SUCCESS;
	}

	return ret;
}


bool RESOURCETYPENAME(AnimStatesDesc)::WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const
{
	return false;
}


void RESOURCETYPENAME(AnimStatesDesc)::Unload(void *data) const
{
	if (data)
	{
		delete ((AnimStateDesc *)data);
	}
}
