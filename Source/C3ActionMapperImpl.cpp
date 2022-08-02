#include "pch.h"
#include <C3ActionMapperImpl.h>

using namespace c3;

ActionMapperImpl::ActionMapperImpl(System *psys)
{
	m_pSys = psys;
}

ActionMapperImpl::~ActionMapperImpl()
{

}

size_t ActionMapperImpl::RegisterAction(const TCHAR *name, TriggerType trigger, ACTION_CALLBACK_FUNC func, void *userdata)
{
	size_t idx = FindActionIndex(name);
	if (idx < m_Actions.size())
	{
		m_Actions[idx].func = func;
		return idx;
	}

	m_Actions.push_back(TActionsArray::value_type());
	auto &a = m_Actions.back();
	a.name = name;
	a.trigger = trigger;
	a.func = func;
	a.userdata = userdata;

	return m_Actions.size() - 1;
}


bool ActionMapperImpl::UnregisterAction(size_t index)
{
	if (index < m_Actions.size())
	{
		m_Actions.erase(m_Actions.begin() + index);
		return true;
	}

	return false;
}


size_t ActionMapperImpl::GetNumActions() const
{
	return m_Actions.size();
}


size_t ActionMapperImpl::FindActionIndex(const TCHAR *name) const
{
	for (size_t i = 0; i < m_Actions.size(); i++)
	{
		if (!_tcsicmp(name, m_Actions[i].name.c_str()))
		{
			return i;
		}
	}

	return -1;
}

const TCHAR *ActionMapperImpl::GetActionName(size_t index) const
{
	if (index < m_Actions.size())
		return m_Actions[index].name.c_str();

	return nullptr;
}


void ActionMapperImpl::Update()
{
	InputManager *pim = m_pSys->GetInputManager();
	if (!pim)
		return;

	for (auto a : m_Actions)
	{
		for (size_t id = 0, maxid = pim->GetNumDevices(); id < maxid; id++)
		{
			InputDevice *pid = pim->GetDevice(id);
			if (!pid->IsActive())
				continue;

			uint32_t devid = pid->GetUID();
			for (sAction::TDevToTrigger::iterator it = a.trigmap.lower_bound(devid), lastit = a.trigmap.upper_bound(devid); it != lastit; it++)
			{
				if (it->first == devid)
				{
					bool triggered = false;
					switch (a.trigger)
					{
						case TriggerType::DOWN_CONTINUOUS:
							triggered = pid->ButtonPressed(it->second);
							break;

						case TriggerType::DOWN_DELTA:
							triggered = (pid->ButtonChange(it->second) > 0) ? true : false;
							break;

						case TriggerType::UP_DELTA:
							triggered = pid->ButtonReleased(it->second);
							break;
					}

					if (triggered)
					{
						size_t user;
						pim->GetAssignedUser(pid, user);
						a.func(pid, user, it->second, pid->ButtonPressedProportional(it->second), a.userdata);
						break;
					}
					break;
				}
			}
		}
	}
}


bool ActionMapperImpl::Save() const
{
	return false;
}


bool ActionMapperImpl::Load()
{
	return false;
}

bool ActionMapperImpl::MakeAssociation(size_t actionidx, uint32_t devid, InputDevice::VirtualButton button)
{
	if (actionidx > m_Actions.size())
		return false;

	SAction &a = m_Actions[actionidx];

	for (sAction::TDevToTrigger::iterator it = a.trigmap.lower_bound(devid), lastit = a.trigmap.upper_bound(devid); it != lastit; it++)
	{
		if ((it->first == devid) && (it->second == button))
			return true;
	}

	a.trigmap.insert(sAction::TDevToTrigger::value_type(devid, button));

	return true;
}

bool ActionMapperImpl::BreakAssociation(size_t actionidx, uint32_t devid, InputDevice::VirtualButton button)
{
	if (actionidx > m_Actions.size())
		return false;

	SAction &a = m_Actions[actionidx];

	for (sAction::TDevToTrigger::iterator it = a.trigmap.lower_bound(devid), lastit = a.trigmap.upper_bound(devid); it != lastit; it++)
	{
		if ((it->first == devid) && (it->second == button))
		{
			a.trigmap.erase(it);
			return true;
		}
	}

	return false;
}

