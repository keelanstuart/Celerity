// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"

#include <C3ScreenManagerImpl.h>
#include <C3ScreenImpl.h>
#include <C3Interactable.h>
#include <C3BlobImpl.h>

using namespace c3;


ScreenManagerImpl::ScreenManagerImpl(System *psys)
{
	m_pSys = psys;
}


ScreenManagerImpl::~ScreenManagerImpl()
{
	for (TScreenStack::iterator it = m_ScreenStack.begin(); it != m_ScreenStack.end(); it++)
		it->first->Release();

	m_ScreenStack.clear();
}


void ScreenManagerImpl::Update(float elapsed_time)
{
	for (size_t i = m_StartRenderIndex; i < m_ScreenStack.size(); i++)
	{
		m_ScreenStack[i].first->Update(elapsed_time);
	}

	if (!m_Cleanup.empty())
	{
		for (auto po : m_Cleanup)
			po->Release();

		m_Cleanup.clear();
	}
}


void ScreenManagerImpl::Render(ScreenFunc render_func)
{
	for (size_t i = m_StartRenderIndex; i < m_ScreenStack.size(); i++)
	{
		Screen *pscr = dynamic_cast<Screen *>((Screen *)m_ScreenStack[i].first->FindComponent(Screen::Type()));

		render_func(pscr->GetObjectRegistry());
	}
}


void ScreenManagerImpl::ComputeStartIndices()
{
	if (m_ScreenStack.empty())
		return;

	m_StartRenderIndex = m_ScreenStack.size() - 1;
	while (m_StartRenderIndex && m_ScreenStack[m_StartRenderIndex].second.IsSet(SCRFLAG_DRAWOVER))
		m_StartRenderIndex--;

	m_StartUpdateIndex = m_ScreenStack.size() - 1;
	while (m_StartUpdateIndex && m_ScreenStack[m_StartUpdateIndex].second.IsSet(SCRFLAG_UPDATEOVER))
		m_StartUpdateIndex--;
}


bool ScreenManagerImpl::PushScreen(const TCHAR *screen_name, const TCHAR *script_filename, TScreenFlags flags)
{
	Factory *pfac = m_pSys->GetFactory();

	Object *po_world = pfac->Build((Prototype *)nullptr, nullptr, nullptr);
	if (!po_world)
		return false;

	m_pSys->GetLog()->Print(_T("Pushing Screen: %s\n"), screen_name);
	po_world->SetName(screen_name);

	Screen *pc_scr = dynamic_cast<Screen *>(po_world->AddComponent(Screen::Type()));
	GlobalObjectRegistry *objreg = pc_scr->GetObjectRegistry();

	po_world->AddComponent(c3::Positionable::Type());
	po_world->AddComponent(c3::Scriptable::Type());
	po_world->Flags().Set(OF_LIGHT | OF_CASTSHADOW);

	objreg->RegisterObject(c3::GlobalObjectRegistry::OD_WORLDROOT, po_world);
	m_pSys->GetLog()->Print(_T("World root created and registered\n"));

	Object *po_gui = pfac->Build();
	po_gui->SetName(_T("GUI ROOT"));
	po_gui->AddComponent(c3::Positionable::Type());
	po_gui->AddComponent(c3::Scriptable::Type());

	objreg->RegisterObject(c3::GlobalObjectRegistry::OD_GUI_ROOT, po_gui);
	m_pSys->GetLog()->Print(_T("GUI root created and registered\n"));

	if (!objreg->GetRegisteredObject(c3::GlobalObjectRegistry::OD_CAMERA))
	{
		Object *po_camroot = pfac->Build();
		po_camroot->SetName(_T("CameraRoot"));
		po_camroot->AddComponent(c3::Positionable::Type());

		Object *po_camarm = pfac->Build();
		po_camarm->SetName(_T("CameraArm"));
		po_camarm->AddComponent(c3::Positionable::Type());
		po_camroot->AddChild(po_camarm);
		c3::Positionable *pc_armpos = dynamic_cast<c3::Positionable *>(po_camarm->FindComponent(c3::Positionable::Type()));
		if (pc_armpos)
		{
			pc_armpos->AdjustPitch(glm::radians(0.01f));
		}

		Object *po_cam = pfac->Build();
		po_cam->SetName(_T("Camera"));
		po_camarm->AddChild(po_cam);
		c3::Positionable *pcampos = dynamic_cast<c3::Positionable *>(po_cam->AddComponent(c3::Positionable::Type()));
		c3::Camera *pc_cam = dynamic_cast<c3::Camera *>(po_cam->AddComponent(c3::Camera::Type()));

		if (pc_cam)
		{
			pc_cam->SetFOV(65.0f);
			pc_cam->SetPolarDistance(0.01f);
		}

		objreg->RegisterObject(c3::GlobalObjectRegistry::OD_CAMERA_ROOT, po_camroot);
		objreg->RegisterObject(c3::GlobalObjectRegistry::OD_CAMERA_ARM, po_camarm);
		objreg->RegisterObject(c3::GlobalObjectRegistry::OD_CAMERA, po_cam);

		m_pSys->GetLog()->Print(_T("Camera hierarchy created and registered\n"));
	}

	Object *po_uicam = pfac->Build();
	po_uicam->SetName(_T("GUI Camera"));

	c3::Positionable *pc_uicampos = dynamic_cast<c3::Positionable *>(po_uicam->AddComponent(c3::Positionable::Type()));
	if (pc_uicampos)
	{
		pc_uicampos->AdjustPos(0, 0, 10.0f);
		pc_uicampos->SetYawPitchRoll(0, glm::radians(-90.0f), 0);
	}

	c3::Camera *pc_uicam = dynamic_cast<c3::Camera *>(po_uicam->AddComponent(c3::Camera::Type()));
	if (pc_uicam)
	{
		pc_uicam->SetProjectionMode(c3::Camera::EProjectionMode::PM_ORTHOGRAPHIC);
		pc_uicam->SetOrthoDimensions(320.0f, 240.0f);// (float)r.Width(), (float)r.Height());
		pc_uicam->SetPolarDistance(10.0f);
	}

	objreg->RegisterObject(c3::GlobalObjectRegistry::OD_GUI_CAMERA, po_uicam);

	m_ScreenStack.push_back(TScreenStack::value_type(po_world, flags));

	ComputeStartIndices();

	if (script_filename)
	{
		const TCHAR *ext = PathFindExtension(script_filename);

		if (!_tcsicmp(ext, _T(".c3js")))
		{
			props::IPropertySet *pps = po_world->GetProperties();
			if (props::IProperty *pprop = pps->GetPropertyById('SRCF'))
				pprop->SetString(script_filename);
		}
		else if (!_tcsicmp(ext, _T(".c3o")))
		{
			Resource *hr = m_pSys->GetResourceManager()->GetResource(script_filename, RESF_DEMANDLOAD, RESOURCETYPE(Blob));

			if (hr && (hr->GetStatus() == Resource::RS_LOADED))
			{
				c3::Blob *pblob = dynamic_cast<Blob *>((Blob *)hr->GetData());

				genio::IInputStream *is = genio::IMemoryInputStream::Create(pblob->Data(), pblob->Size());
				if (is)
				{
					// use the camera from the editor to start
					c3::Object::CameraLoadFunc loadcam = [&](c3::Object *camera, float yaw, float pitch)
					{
						// use the one we loaded
						objreg->RegisterObject(c3::GlobalObjectRegistry::OD_CAMERA, camera);
					};

					po_world->Load(is, po_world, nullptr, loadcam);
					is->Close();
					

					is->Release();
				}
			}
		}
	}

	return true;
}


bool ScreenManagerImpl::PopScreen()
{
	if (!m_ScreenStack.empty())
	{
		Object *ps = m_ScreenStack.back().first;
		m_ScreenStack.pop_back();

		if (ps)
		{
			m_pSys->GetLog()->Print(_T("Popping Screen: %s\n"), ps->GetName());

			// can't delete things yet...
			m_Cleanup.push_back(ps);
		}

		ComputeStartIndices();
	}

	return !m_ScreenStack.empty();
}


Object *ScreenManagerImpl::GetActiveScreen(size_t offset) const
{
	Object *ret = nullptr;

	if (offset >= m_ScreenStack.size())
		return nullptr;

	TScreenStack::const_reverse_iterator rit = m_ScreenStack.rbegin() + offset;
	ret = rit->first;

	return ret;
}
