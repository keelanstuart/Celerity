// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3ActionMapper.h>
#include <map>


namespace c3
{

	class ActionMapperImpl : public ActionMapper
	{

	public:

		ActionMapperImpl(System *psys);

		virtual ~ActionMapperImpl();

		virtual size_t RegisterAction(const TCHAR *name, TriggerType trigger, float delay, ACTION_CALLBACK_FUNC func, void *userdata);

		virtual bool UnregisterAction(size_t index);

		virtual size_t GetNumActions() const;

		virtual size_t FindActionIndex(const TCHAR *name) const;

		virtual const TCHAR *GetActionName(size_t index) const;

		virtual bool MakeAssociation(size_t actionidx, uint32_t devid, InputDevice::VirtualButton button);

		virtual bool BreakAssociation(size_t actionidx, uint32_t devid, InputDevice::VirtualButton button);

		virtual void Update();

		virtual bool Save() const;

		virtual bool Load();

	protected:

		System *m_pSys;

		typedef struct sAction
		{
			tstring name;
			TriggerType trigger;
			float delay;
			float timeout;
			typedef std::multimap<uint32_t, InputDevice::VirtualButton> TDevToTrigger;
			TDevToTrigger trigmap;
			ACTION_CALLBACK_FUNC func;
			void *userdata;
		} SAction;

		typedef std::deque<SAction> TActionsArray;
		TActionsArray m_Actions;

	};

};