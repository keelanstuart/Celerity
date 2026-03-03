// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"

#include "C3AnimatorImpl.h"
#include "C3AnimTrackImpl.h"
#include "C3AnimationImpl.h"
#include "C3ModelRendererImpl.h"

using namespace c3;


DECLARE_COMPONENTTYPE(Animator, AnimatorImpl);


#define DEFAULT_ANIMSTATE	_T("Default")


/*

Since people are starting to look at this file,
I thought I would explain how the animation system works in detail.

First, an Animator is a Component that loads c3states files which are xml content, resembling this:

<states startstate="idle">

	<state name="idle" goto="idle">
		<animation filename="idle.anim" weight="50" />
		<animation filename="shift_weight.anim" weight="10" />
		<animation filename="scratch_butt.anim" weight="2" />
		<animation filename="pick_nose.anim" weight="1" />
	</state>

	<state name="walk" goto="idle">
		<animation filename="walk.anim" weight="1" />
	</state>

	<state name="dying" goto="dead">
		<animation filename="death1.anim" weight="1" />
		<animation filename="death2.anim" weight="1" />
	</state>

	<state name="dead" goto="dead">
		<animation filename="dead.anim" weight="1" />
	</state>

</states>

The idea is that states, when their animation is finished playing, transition to another state.
When that happens, an animation for the new state is chosen... and this is done randomly, given
weights for each actual animation. In the example above, the "idle" state has 4 possible animations
it could play, with a total, combined weight of 63 (50 + 10 + 2 + 1).

So, a random number is generated between 0-62 and if it is less than 50, the first animation
(idle.anim) is chosen. If it is 50-59, "shift_weight.anim" is chosen. In the unlikely event
that the number is above that, the character will scratch their butt or pick their nose. The key
is that the weights are relative... thus, if you wanted to decrease the likelihood of a nose pick,
you would increase the weights on other animations, not necessarily decrease the weight of
"pick_nose.anim" (impossible here, since it is already only 1).

The other important things to note here:

- Animation channels are independent; you can have different numbers of keys for translation,
  rotation, and scale (or not at all for any of those)\
- There is the concept of a "note" track; notes are script code embedded in the animation...
  you might use them to place footfall sounds at the exact moment of ground impact or
  spawn an effect / missile during a spell. Let your imagination go wild.
  The bad news about notes: currently there is no format that supports them, even though you can
  create them in 3DStudio Max. I may write a note-tracker application for Celerity 3.0 (one existed for 1.0).

Individual animations are collections of tracks, containing key frames... see C3Animation*.* and
C3AnimTrack*.* for more information.

*/

AnimatorImpl::AnimatorImpl()
{
	m_pOwner = nullptr;
	m_pLastModel = nullptr;

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

	ResetStates(false);
}


void AnimatorImpl::Release()
{
	if (m_StateProp)
		m_StateProp->SetString(m_StateProp->AsString());

	delete this;
}


props::TFlags64 &AnimatorImpl::Flags()
{
	return m_Flags;
}


// This is required by props::IProperty::IEnumProvider... it's not related to "animation" --
// it returns the animation states current availble in a way that makes them discoverable [in the editor]
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


// This is required by props::IProperty::IEnumProvider... it's not related to "animation" --
// it returns the animation states current availble in a way that makes them discoverable [in the editor]
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
	m_pOwner = pobject;

	props::IPropertySet *props = m_pOwner->GetProperties();
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


// maps nodes to tracks based on their names
void AnimatorImpl::GenerateNodeToTrackMapping()
{
	if (!m_CurAnim)
		return;

	ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(m_pOwner->FindComponent(ModelRenderer::Type()));
	if (!pmr)
		return;

	const Model *pm = pmr->GetModel();
	if (!pm)
		return;

	size_t nc = pm->GetNodeCount();
	if (m_NodeToTrack.size() < nc)
		m_NodeToTrack.resize(nc);

	if (m_KeyIndices.size() < nc)
		m_KeyIndices.resize(nc);

	for (size_t n = 0; n < nc; n++)
	{
		const TCHAR *name = pm->GetNodeName(n);

		Animation::TrackIndex ti = m_CurAnim->FindTrackByName(name);

		// store the track index in the map
		m_NodeToTrack[n] = ti;

		// reset our key cache
		m_KeyIndices[n].m_Pos = AnimTrack::KEYINDEX_INVALID;
		m_KeyIndices[n].m_Ori = AnimTrack::KEYINDEX_INVALID;
		m_KeyIndices[n].m_Scl = AnimTrack::KEYINDEX_INVALID;
	}
}


void AnimatorImpl::SelectAnimation()
{
	auto oldanim = m_CurAnim;

	if (m_CurState == m_StateMap.end())
		m_CurState = m_StateMap.find(DEFAULT_ANIMSTATE);

	if (m_CurState != m_StateMap.end())
	{
		// Firgure out which random animation to play

		size_t w = m_CurState->second->m_TotalWeight;
		if (w)
		{
			// get the total weight of all animations in the state, then iteratively decerement that by
			// the weights of individual animations until it's less than one of them
			size_t animchoice = rand() % w;

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
		else
			m_CurAnim = nullptr; // no animations in this state?
	}

	if (!m_CurAnim)
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(m_pOwner->FindComponent(ModelRenderer::Type()));
		if (pmr)
		{
			const Model *pm = pmr->GetModel();
			if (pm)
				m_CurAnim = pm->GetDefaultAnim();
		}
	}

	// if a new animation was chosen, then re-map the tracks to nodes (they could be different)
	if (m_CurAnim != oldanim)
		GenerateNodeToTrackMapping();
}


void AnimatorImpl::AdvanceState()
{
	// save the last state and set the current one based on the "goto" state
	m_LastState = m_CurState;

	if (m_LastState != m_StateMap.end())
		m_CurState = m_StateMap.find(m_LastState->second->m_GotoName);

	if (m_CurState == m_StateMap.end())
		m_CurState = m_StateMap.find(m_StartState.empty() ? DEFAULT_ANIMSTATE : m_StartState);

	// now we can choose a new animation
	SelectAnimation();

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

	// if we have an animation, and either time changed or the animation changed, we need to do something...
	if (m_CurAnim && ((m_CurAnimTime != m_LastAnimTime) || (m_CurAnim != oldanim)))
	{
		// Maybe the model changed!
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(m_pOwner->FindComponent(ModelRenderer::Type()));
		const Model *pm = pmr->GetModel();
		if (pm != m_pLastModel)
		{
			// synthesize a state file property change to force the re-mapping of our
			// nodes to tracks (since the nodes are likely different)
			const props::IProperty *psf = m_pOwner->GetProperties()->GetPropertyById('ST8F');
			if (psf)
				PropertyChanged(psf);
		}

		// the ModelInstanceData is what stores the individual node transforms
		// (including bones for weighted / skin setups)
		Model::InstanceData *pmid = pmr->GetModelInstanceData();

		if (pm && pmid)
		{
			// first time through? initalize the mapping
			if (m_NodeToTrack.size() != pm->GetNodeCount())
				GenerateNodeToTrackMapping();

			glm::fmat4x4 m, om, ident = glm::identity<glm::fmat4x4>();

			std::function<void(size_t)> ComputeTransforms = [&](size_t child_of)
			{
				for (size_t n = 0, maxn = pm->GetNodeCount(); n < maxn; n++)
				{
					if (pm->GetParentNode(n) == child_of)
					{
						Animation::TrackIndex ti = m_NodeToTrack[n];

						if (ti != Animation::TRACKINDEX_INVALID)
						{
							AnimTrack *pat = m_CurAnim->GetTrack(ti);

							// if there's a note on the current key and we just now reached it (last time was before this key) then process it
							// TODO: evaluate for correctness - this might be better in Update than Prerender. <shrug>
							if (const TCHAR *pnote = pat->GetNote(m_LastAnimTime, m_CurAnimTime))
								ProcessNote(pnote);

							// get the transform and key index from the animation track for this node
							// based on the current time
							AnimTrack::KeyIndex kip = AnimTrack::KEYINDEX_INVALID, kio = kip, kis = kio;
							glm::fvec3 apos = pat->GetPos(m_CurAnimTime, kip);
							glm::fquat aori = pat->GetOri(m_CurAnimTime, kio);
							glm::fvec3 ascl = pat->GetScl(m_CurAnimTime, kis);

							// build the affine transform... start with scale, then rotation...
							m = glm::scale(glm::identity<glm::fmat4x4>(), ascl) * (glm::fmat4x4)(aori);

							// translate last... 
							m = glm::translate(glm::identity<glm::fmat4x4>(), apos) * m;

							// push the matrix
							m_MatStack->Push(&m);
						}
						else
						{
							// in case there was no mapping track for the node,
							// we want to make sure that we're applying identity to the node...
							// this way we can have incomplete animations that still "work" for
							// models (that is, a more detailed model can have an animation
							// applied to it that is less detailed)
							m_MatStack->Push(&ident);
						}

						// get the concatenated transform from the stack
						m_MatStack->Top(&m);

						// apply it to the model instance data
						pmid->SetTransform(n, m);

						// recurse into the node hierarchy
						ComputeTransforms(n);

						// pop the stack
						m_MatStack->Pop();
					}
				}
			};

			ComputeTransforms(Model::NO_PARENT);
		}
	}
}


bool AnimatorImpl::Prerender(RenderFlags flags, int draworder)
{
	return false;
}


void AnimatorImpl::Render(RenderFlags flags, const glm::fmat4x4 *pmat)
{

}


void AnimatorImpl::ResetStates(bool add_default)
{
	for (auto it : m_StateMap)
		delete it.second;

	m_StateMap.clear();

	if (add_default)
	{
		std::pair<AnimStateMap::iterator, bool> emres = m_StateMap.insert(AnimStateMap::value_type(DEFAULT_ANIMSTATE, new AnimStateInfo()));
		emres.first->second->m_GotoName = DEFAULT_ANIMSTATE;
		emres.first->second->m_Name = DEFAULT_ANIMSTATE;
		emres.first->second->m_TotalWeight = 1;
		emres.first->second->m_WeightedAnims.emplace_back();
		emres.first->second->m_WeightedAnims.back().m_Anim = nullptr;
		emres.first->second->m_WeightedAnims.back().m_Weight = 1;
	}

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
			System *psys = m_pOwner->GetSystem();
			ResourceManager *prm = psys->GetResourceManager();

			const ModelRendererImpl *pmr = dynamic_cast<ModelRendererImpl *>(m_pOwner->FindComponent(ModelRenderer::Type()));
			const Model *pm = pmr ? pmr->GetModel() : nullptr;
			m_pLastModel = pm;

			ResetStates(true);

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
								m_pOwner->GetSystem()->GetLog()->Print(_T("\"%s\" contains duplicate state: \"%s\"\n"), filename, _name);
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
										const Animation *pa = (const Animation *)(animres ? animres->GetData() : nullptr);

										emres.first->second->m_WeightedAnims.emplace_back();
										emres.first->second->m_WeightedAnims.back().m_Anim = pa;

										const tinyxml2::XMLAttribute *pweight = panim->FindAttribute("weight");
										size_t wt = pweight ? pweight->IntValue() : 1;
										emres.first->second->m_WeightedAnims.back().m_Weight = wt;
										emres.first->second->m_TotalWeight += wt;
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
	if (m_pOwner)
	{
		Scriptable *ps = (Scriptable *)(m_pOwner->FindComponent(Scriptable::Type()));
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


bool AnimatorImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance, glm::fvec3 *pNormal, bool force) const
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
