// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Animation.h>
#include <C3Animator.h>
#include <vector>


#define ANIM_RECYCLESTATE		-1


namespace c3
{

	using AnimStateDesc = tinyxml2::XMLDocument;

	class AnimatorImpl : public Animator, props::IProperty::IEnumProvider
	{

#define AF_FORCENEXT		0x00000001

	public:

		AnimatorImpl();

		virtual ~AnimatorImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags);

		virtual void Render(Object::RenderFlags flags);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool ProcessNote(const TCHAR *note);

		virtual bool HasState(const TCHAR *name) const;

		virtual const TCHAR *GetCurrentState() const;

		virtual void SetCurrentState(const TCHAR *name);

		virtual void ForceStateChange();

		virtual float GetCurAnimLength() const;

		virtual float GetCurAnimTime() const;

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats, float *pDistance) const;

		virtual size_t GetNumValues(const props::IProperty *pprop) const;

		virtual const TCHAR *GetValue(const props::IProperty *pprop, size_t ordinal, TCHAR *buf = nullptr, size_t bufsize = 0) const;

	private:

		void GenerateNodeToTrackMapping();

		void SelectAnimation();

		void AdvanceState();

	protected:

		Object *m_Owner;

		const Animation *m_CurAnim;

		// m_CurAnimTime is the current time point in the active animation
		float m_CurAnimTime;

		// m_LastAnimTime is the last m_CurAnimTime. This can be used to get the time offset into the next animation
		float m_LastAnimTime;

		struct AnimStateInfo
		{
			tstring m_Name;
			tstring m_GotoName;

			struct WeightedAnim
			{
				Animation *m_Anim;
				size_t m_Weight;
			};

			using WeightedAnimArray = std::vector<WeightedAnim>;
			WeightedAnimArray m_WeightedAnims;

			size_t m_TotalWeight;
		};

		using AnimStateMap = std::map<tstring, AnimStateInfo *>;
		AnimStateMap m_StateMap;

		void ResetStates();

		props::IProperty *m_StateProp;
		tstring m_StartState;
		AnimStateMap::iterator m_CurState;
		AnimStateMap::iterator m_LastState;

		struct KeyIndices
		{
			AnimTrack::KeyIndex m_Pos, m_Ori, m_Scl;
		};
		std::vector<KeyIndices> m_KeyIndices;

		props::TFlags64 m_Flags;

		std::vector<Animation::TrackIndex> m_NodeToTrack;

		MatrixStack *m_MatStack;

	};

	DEFINE_COMPONENTTYPE(Animator, AnimatorImpl, GUID({ 0x37c5c9f5, 0xd98b, 0x4d3c, { 0xac, 0xe0, 0x54, 0xc3, 0xf6, 0xf1, 0x1e, 0xb9 }}), "Animator", "Animator maintains and applies animation states to Models", 0);

	DEFINE_RESOURCETYPE(AnimStatesDesc, 0, GUID({0x26193eee, 0x518e, 0x4ecd, { 0xb1, 0xcd, 0x80, 0x74, 0xd6, 0xa0, 0x3d, 0x33 }}), "AnimStateDefs", "Animation State Definitions", "c3states", "c3states");

};
