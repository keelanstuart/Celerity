// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3ScriptableImpl.h>
#include <PowerProps.h>
#include <TinyJS_MathFunctions.h>
#include <TinyJS_Functions.h>
#include <C3GlobalObjectRegistry.h>
#include <C3Math.h>
#include <C3ColorDefs.h>

using namespace c3;


DECLARE_RESOURCETYPE(Script);

c3::ResourceType::LoadResult RESOURCETYPENAME(Script)::ReadFromFile(c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const
{
	if (returned_data)
	{
		FILE *f = nullptr;
		if ((_tfopen_s(&f, filename, _T("rt, ccs=UTF-8")) == EINVAL) || !f)
			return ResourceType::LoadResult::LR_NOTFOUND;

		fseek(f, 0, SEEK_END);
		size_t sz = ftell(f);
		fseek(f, 0, SEEK_SET);

		TCHAR *code = (TCHAR *)calloc(sz + 1, sizeof(TCHAR));

		if (sz)
			fread(code, sizeof(TCHAR), sz, f);

		fclose(f);

		Script *ps = new Script;
		ps->m_Code = code;
		*returned_data = ps;
		free(code);
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(Script)::ReadFromMemory(c3::System *psys, const TCHAR *contextname, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
{
	if (returned_data && buffer && buffer_length)
	{
		Script *ps = new Script;
		ps->m_Code = (const TCHAR *)buffer;
		*returned_data = ps;
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


bool RESOURCETYPENAME(Script)::WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const
{
	return false;
}


void RESOURCETYPENAME(Script)::Unload(void *data) const
{
	delete (Script *)data;
}


DECLARE_COMPONENTTYPE(Scriptable, ScriptableImpl);


ScriptableImpl::ScriptableImpl()
{
	m_JS = nullptr;
	m_Continue = true;
	m_bHasUpdate = false;
}


ScriptableImpl::~ScriptableImpl()
{

}


void ScriptableImpl::Release()
{
	if (m_JS)
	{
		if (FunctionExists(_T("free")))
			Execute(_T("free();"));

		delete m_JS;
		m_JS = nullptr;
	}
}


props::TFlags64 ScriptableImpl::Flags() const
{
	return 0;
}


void jcGetNumChildren(CScriptVar *c, void *userdata);
void jcGetChild(CScriptVar *c, void *userdata);
void jcFindObjByGUID(CScriptVar *c, void *userdata);
void jcFindFirstObjByName(CScriptVar *c, void *userdata);
void jcLog(CScriptVar *c, void *userdata);
void jcExit(CScriptVar *c, void *userdata);
void jcExecute(CScriptVar *c, void *userdata);
void jcCreateProperty(CScriptVar* c, void* userdata);
void jcDeleteProperty(CScriptVar* c, void* userdata);
void jcFindProperty(CScriptVar *c, void *userdata);
void jcGetPropertyValue(CScriptVar *c, void *userdata);
void jcSetPropertyValue(CScriptVar *c, void *userdata);
void jcCreateObject(CScriptVar *c, void *userdata);
void jcDeleteObject(CScriptVar *c, void *userdata);
void jcGetParent(CScriptVar *c, void *userdata);
void jcSetParent(CScriptVar *c, void *userdata);
void jcGetObjectName(CScriptVar *c, void *userdata);
void jcSetObjectName(CScriptVar *c, void *userdata);
void jcLoadPrototypes(CScriptVar *c, void *userdata);
void jcLoadObject(CScriptVar *c, void *userdata);
void jcSaveObject(CScriptVar *c, void *userdata);
void jcGetRegisteredObject(CScriptVar *c, void *userdata);
void jcRegisterObject(CScriptVar *c, void *userdata);
void jcIsObjFlagSet(CScriptVar *c, void *userdata);
void jcSetObjFlag(CScriptVar *c, void *userdata);
void jcAdjustQuat(CScriptVar *c, void *userdata);
void jcQuatToEuler(CScriptVar *c, void *userdata);
void jcEulerToQuat(CScriptVar *c, void *userdata);
void jcPlaySound(CScriptVar *c, void *userdata);
void jcStopSound(CScriptVar *c, void *userdata);
void jcPauseSound(CScriptVar *c, void *userdata);
void jcResumeSound(CScriptVar *c, void *userdata);
void jcSetSoundVolume(CScriptVar *c, void *userdata);
void jcSetSoundPitchMod(CScriptVar *c, void *userdata);
void jcSetSoundPosition(CScriptVar *c, void *userdata);
void jcGetModelNodeIndex(CScriptVar *c, void *userdata);
void jcGetModelNodeCount(CScriptVar *c, void *userdata);
void jcGetModelInstNodePos(CScriptVar *c, void *userdata);
void jcGetModelInstNodeOri(CScriptVar *c, void *userdata);
void jcGetModelInstNodeScl(CScriptVar *c, void *userdata);
void jcSetModelInstNodePos(CScriptVar *c, void *userdata);
void jcSetModelInstNodeOri(CScriptVar *c, void *userdata);
void jcSetModelInstNodeScl(CScriptVar *c, void *userdata);
void jcGetModelNodeName(CScriptVar *c, void *userdata);
void jcSetModelNodeVisibility(CScriptVar *c, void *userdata);
void jcSetModelNodeCollisions(CScriptVar *c, void *userdata);
void jcCreateCollisionResults(CScriptVar *c, void *userdata);
void jcFreeCollisionResults(CScriptVar *c, void *userdata);
void jcCheckCollisions(CScriptVar *c, void *userdata);
void jcHandleFPSMovement(CScriptVar *c, void *userdata);
void jcGetGravity(CScriptVar *c, void *userdata);
void jcPackColorFromIntVec(CScriptVar *c, void *userdata);
void jcPackColorFromFloatVec(CScriptVar *c, void *userdata);
void jcUnpackColorToIntVec(CScriptVar *c, void *userdata);
void jcUnpackColorToFloatVec(CScriptVar *c, void *userdata);
void jcLoadPackfile(CScriptVar *c, void *userdata);
void jcUnloadPackfile(CScriptVar *c, void *userdata);
void jcLoadPlugin(CScriptVar *c, void *userdata);
void jcUnloadPlugin(CScriptVar *c, void *userdata);
void jcMessageBox(CScriptVar *c, void *userdata);
void jcRegisterResourcePath(CScriptVar *c, void *userdata);
void jcRegisterInputAction(CScriptVar *c, void *userdata);
void jcUnregisterInputAction(CScriptVar *c, void *userdata);
void jcFindInputAction(CScriptVar *c, void *userdata);
void jcLinkInputToAction(CScriptVar *c, void *userdata);

void jcSetMousePos(CScriptVar *c, void *userdata);
void jcGetMousePos(CScriptVar *c, void *userdata);
void jcEnableMouse(CScriptVar *c, void *userdata);
void jcMouseEnabled(CScriptVar *c, void *userdata);
void jcEnableSystemMouse(CScriptVar *c, void *userdata);
void jcSystemMouseEnabled(CScriptVar *c, void *userdata);
void jcCaptureMouse(CScriptVar *c, void *userdata);
void jcMouseCaptured(CScriptVar *c, void *userdata);
void jcRegisterCursor(CScriptVar *c, void *userdata);
void jcUnregisterCursor(CScriptVar *c, void *userdata);
void jcGetNumCursors(CScriptVar *c, void *userdata);
void jcGetCursorName(CScriptVar *c, void *userdata);
void jcSetCursor(CScriptVar *c, void *userdata);
void jcGetCursor(CScriptVar *c, void *userdata);
void jcSetCursorTransform(CScriptVar *c, void *userdata);

void jcPauseGame(CScriptVar *c, void *userdata);
void jcGamePaused(CScriptVar *c, void *userdata);


void ScriptableImpl::ResetJS()
{
	Release();

	m_JS = new CTinyJS();

	System *psys = m_pOwner->GetSystem();
	m_JS->m_pSys = psys;
	m_JS->m_pObj = m_pOwner;

	registerFunctions(m_JS);
	registerMathFunctions(m_JS);

	m_JS->AddNative(_T("function GetNumChildren(hrootobj)"),							jcGetNumChildren, psys);
	m_JS->AddNative(_T("function GetChild(hrootobj, idx)"),								jcGetChild, psys);
	m_JS->AddNative(_T("function FindObjByGUID(hrootobj, guid, recursive)"),			jcFindObjByGUID, psys);
	m_JS->AddNative(_T("function FindFirstObjByName(hrootobj, name, recursive)"),		jcFindFirstObjByName, psys);

	m_JS->AddNative(_T("function Log(text)"),											jcLog, psys);

	m_JS->AddNative(_T("function Exit()"),												jcExit, psys);

	m_JS->AddNative(_T("function Execute(hobj, cmd)"),									jcExecute, psys);

	m_JS->AddNative(_T("function CreateProperty(hobj, name, fcc)"),							jcCreateProperty, psys);
	m_JS->AddNative(_T("function DeleteProperty(hprop)"),								jcDeleteProperty, psys);
	m_JS->AddNative(_T("function FindProperty(hobj, name)"),							jcFindProperty, psys);
	m_JS->AddNative(_T("function GetPropertyValue(hprop)"),								jcGetPropertyValue, psys);
	m_JS->AddNative(_T("function SetPropertyValue(hprop, val)"),						jcSetPropertyValue, psys);

	m_JS->AddNative(_T("function CreateObject(protoname, hparentobj)"),					jcCreateObject, psys);
	m_JS->AddNative(_T("function DeleteObject(hobj)"),									jcDeleteObject, psys);

	m_JS->AddNative(_T("function GetParent(hobj)"),										jcGetParent, psys);
	m_JS->AddNative(_T("function SetParent(hobj, hnewparentobj)"),						jcSetParent, psys);

	m_JS->AddNative(_T("function GetObjectName(hobj)"),									jcGetObjectName, psys);
	m_JS->AddNative(_T("function SetObjectName(hobj, newname)"),						jcSetObjectName, psys);

	m_JS->AddNative(_T("function LoadPrototypes(filename)"),							jcLoadPrototypes, psys);

	m_JS->AddNative(_T("function LoadObject(hobj, filename)"),							jcLoadObject, psys);
	m_JS->AddNative(_T("function SaveObject(hobj, filename)"),							jcSaveObject, psys);

	m_JS->AddNative(_T("function GetRegisteredObject(designation)"),					jcGetRegisteredObject, psys);
	m_JS->AddNative(_T("function RegisterObject(designation, hobj)"),					jcRegisterObject, psys);

	m_JS->AddNative(_T("function IsObjFlagSet(hobj, flagname)"),						jcIsObjFlagSet, psys);
	m_JS->AddNative(_T("function SetObjFlag(hobj, flagname, val)"),						jcSetObjFlag, psys);

	m_JS->AddNative(_T("function AdjustQuat(quat, axis, angle)"),						jcAdjustQuat, psys);
	m_JS->AddNative(_T("function EulerToQuat(euler)"),									jcEulerToQuat, psys);
	m_JS->AddNative(_T("function QuatToEuler(quat)"),									jcQuatToEuler, psys);

	m_JS->AddNative(_T("function PlaySound(filename, volmod, pitchmod, loop, pos)"),	jcPlaySound, psys);
	m_JS->AddNative(_T("function StopSound(hsound)"),									jcStopSound, psys);
	m_JS->AddNative(_T("function PauseSound(hsound)"),									jcPauseSound, psys);
	m_JS->AddNative(_T("function PauseSound(hsound)"),									jcResumeSound, psys);
	m_JS->AddNative(_T("function SetSoundVolume(hsound, vol)"),							jcSetSoundVolume, psys);
	m_JS->AddNative(_T("function SetSoundPitchMod(hsound, pitchmod)"),					jcSetSoundPitchMod, psys);
	m_JS->AddNative(_T("function SetSoundPosition(hsound, pos)"),						jcSetSoundPosition, psys);

	m_JS->AddNative(_T("function GetModelNodeIndex(hobj, nodename)"),					jcGetModelNodeIndex, psys);
	m_JS->AddNative(_T("function GetModelNodeCount(hobj)"),								jcGetModelNodeCount, psys);
	m_JS->AddNative(_T("function GetModelInstNodePos(hobj, nodeidx)"),					jcGetModelInstNodePos, psys);
	m_JS->AddNative(_T("function GetModelInstNodeOri(hobj, nodeidx)"),					jcGetModelInstNodeOri, psys);
	m_JS->AddNative(_T("function GetModelInstNodeScl(hobj, nodeidx)"),					jcGetModelInstNodeScl, psys);
	m_JS->AddNative(_T("function SetModelInstNodePos(hobj, nodeidx, pos)"),				jcSetModelInstNodePos, psys);
	m_JS->AddNative(_T("function SetModelInstNodeOri(hobj, nodeidx, ori)"),				jcSetModelInstNodeOri, psys);
	m_JS->AddNative(_T("function SetModelInstNodeScl(hobj, nodeidx, scl)"),				jcSetModelInstNodeScl, psys);
	m_JS->AddNative(_T("function GetModelNodeName(hobj, nodeidx)"),						jcGetModelNodeName, psys);
	m_JS->AddNative(_T("function SetModelNodeVisibility(hobj, nodeidx, b)"),			jcSetModelNodeVisibility, psys);
	m_JS->AddNative(_T("function SetModelNodeCollisions(hobj, nodeidx, b)"),			jcSetModelNodeCollisions, psys);

	m_JS->AddNative(_T("function CreateCollisionResults()"),							jcCreateCollisionResults, psys);
	m_JS->AddNative(_T("function FreeCollisionResults(colres)"),						jcFreeCollisionResults, psys);
	m_JS->AddNative(_T("function CheckCollisions(hrootobj, raypos, raydir, results)"),	jcCheckCollisions, psys);
	m_JS->AddNative(_T("function HandleFPSMovement(hrootobj, pos, user_vel, user_acc, elapsed_time, low_height, high_height)"), jcHandleFPSMovement, psys);
	m_JS->AddNative(_T("function GetGravity()"),										jcGetGravity, psys);

	m_JS->AddNative(_T("function PackColorFromIntVec(coloriv)"),						jcPackColorFromIntVec, psys);
	m_JS->AddNative(_T("function PackColorFromFloatVec(colorfv)"),						jcPackColorFromFloatVec, psys);
	m_JS->AddNative(_T("function UnpackColorToIntVec(color)"),							jcUnpackColorToIntVec, psys);
	m_JS->AddNative(_T("function UnpackColorToFloatVec(color)"),						jcUnpackColorToFloatVec, psys);

	m_JS->AddNative(_T("function LoadPackfile(filename)"),								jcLoadPackfile, psys);
	m_JS->AddNative(_T("function UnloadPackfile(filename)"),							jcUnloadPackfile, psys);

	m_JS->AddNative(_T("function LoadPlugin(filename)"),								jcLoadPlugin, psys);
	m_JS->AddNative(_T("function UnloadPlugin(filename)"),								jcUnloadPlugin, psys);

	m_JS->AddNative(_T("function MessageBox(caption, message, modestr)"),				jcMessageBox, psys);

	m_JS->AddNative(_T("function RegisterResourcePath(ext, path)"),						jcRegisterResourcePath, psys);

	m_JS->AddNative(_T("function RegisterInputAction(action_name, press_mode, delay)"),	jcRegisterInputAction, psys);
	m_JS->AddNative(_T("function UnregisterInputAction(action_name)"),					jcUnregisterInputAction, psys);
	m_JS->AddNative(_T("function FindInputAction(action_name)"),						jcFindInputAction, psys);
	m_JS->AddNative(_T("function LinkInputToAction(action_name, device, input_name)"),	jcLinkInputToAction, psys);

	m_JS->AddNative(_T("function SetMousePos(pos)"),									jcSetMousePos, psys);
	m_JS->AddNative(_T("function GetMousePos()"),										jcGetMousePos, psys);
	m_JS->AddNative(_T("function EnableMouse(enabled)"),									jcEnableMouse, psys);
	m_JS->AddNative(_T("function MouseEnabled()"),										jcMouseEnabled, psys);
	m_JS->AddNative(_T("function EnableSystemMouse(enabled)"),							jcEnableSystemMouse, psys);
	m_JS->AddNative(_T("function SystemMouseEnabled()"),								jcSystemMouseEnabled, psys);
	m_JS->AddNative(_T("function CaptureMouse(capture)"),								jcCaptureMouse, psys);
	m_JS->AddNative(_T("function MouseCaptured()"),										jcMouseCaptured, psys);
	m_JS->AddNative(_T("function RegisterCursor(filename, hotspot, cursorname)"),		jcRegisterCursor, psys);
	m_JS->AddNative(_T("function UnregisterCursor(cursorid)"),							jcUnregisterCursor, psys);
	m_JS->AddNative(_T("function GetNumCursors()"),										jcGetNumCursors, psys);
	m_JS->AddNative(_T("function GetCursorName(cursorid)"),								jcGetCursorName, psys);
	m_JS->AddNative(_T("function SetCursor(cursorid)"),									jcSetCursor, psys);
	m_JS->AddNative(_T("function GetCursor()"),											jcGetCursor, psys);
	m_JS->AddNative(_T("function SetCursorTransform(pos, rot, scl)"),					jcSetCursorTransform, psys);

	m_JS->AddNative(_T("function PauseGame(paused)"),									jcPauseGame, psys);
	m_JS->AddNative(_T("function GamePaused()"),										jcGamePaused, psys);

	TCHAR make_self_cmd[64];
	_stprintf_s(make_self_cmd, _T("var self = %lld;"), (int64_t)m_pOwner);
	m_JS->Execute(make_self_cmd);
}

bool ScriptableImpl::Initialize(Object *pobject)
{
	if (!pobject)
		return false;

	m_pOwner = pobject;

	ResetJS();

	props::IPropertySet *propset = m_pOwner->GetProperties();
	if (!propset)
		return false;

	props::IProperty *pudr = propset->GetPropertyById('SUDR');
	if (!pudr)
	{
		pudr = propset->CreateProperty(_T("ScriptUpdateRate"), 'SUDR');
		if (pudr)
			pudr->SetFloat(m_UpdateRate);
	}

	props::IProperty *psrc = propset->GetPropertyById('SRCF');
	if (!psrc)
	{
		psrc = propset->CreateProperty(_T("SourceFile"), 'SRCF');
		if (psrc)
		{
			psrc->SetString(_T(""));
			psrc->SetAspect(props::IProperty::PA_FILENAME);
			psrc->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
		}
	}

	return true;
}


void ScriptableImpl::Update(float elapsed_time)
{
	if (!m_Continue)
		return;

	if (m_bHasUpdate)
	{
		m_UpdateTime -= elapsed_time;

		if (m_UpdateTime <= 0)
		{
			m_UpdateTime += m_UpdateRate;

			if (m_UpdateTime < 0)
				m_UpdateTime = m_UpdateRate;
			Execute(_T("update(%0.3f);"), elapsed_time);
		}
	}
}


bool ScriptableImpl::Prerender(Object::RenderFlags flags, int draworder)
{
	return false;
}


void ScriptableImpl::Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat)
{

}


void ScriptableImpl::Preprocess(const TCHAR *options)
{
#define KILL_LINE	-1
	std::function<void(size_t, size_t)> KillCodeBlock = [&](size_t sc, size_t ec)
	{
		if (ec == KILL_LINE)
			ec = m_Code.find(_T('\n'), sc);
		m_Code.erase(m_Code.begin() + sc, m_Code.begin() + ec);
	};

	std::function<size_t(size_t)> SkipWhitespace = [&](size_t sc) -> size_t
	{
		size_t ret = m_Code.find_first_not_of(_T(" \t"), sc);
		return ret;
	};

	const tstring incdir = _T("#include");
	std::vector<tstring> already_included;

	size_t start_pos;
	while ((start_pos = m_Code.find(incdir)) != tstring::npos)
	{
		size_t qpos = SkipWhitespace(start_pos);
		if (qpos != tstring::npos)
		{
			if (m_Code[qpos] != _T('\"'))
			{
				m_pOwner->GetSystem()->GetLog()->Print(_T("#include directive missing include file\n"));
				KillCodeBlock(start_pos, KILL_LINE);
				continue;
			}

			size_t _qpos = m_Code.find_first_of(_T("\"\n"), qpos);
			if (m_Code[_qpos] != _T('\"'))
			{
				m_pOwner->GetSystem()->GetLog()->Print(_T("#include directive missing closing \"\n"));
				KillCodeBlock(start_pos, KILL_LINE);
				continue;
			}

			tstring filename = m_Code.substr(qpos, _qpos - qpos - 1);
			Resource *pres = m_pOwner->GetSystem()->GetResourceManager()->GetResource(filename.c_str(), RESF_DEMANDLOAD);
			if (pres && (pres->GetStatus() == Resource::RS_LOADED))
			{
				// no recursive inclusions
				if (std::find(already_included.cbegin(), already_included.cend(), filename) != already_included.cend())
				{
					KillCodeBlock(start_pos, KILL_LINE);
					continue;
				}

				// insert the included code
				m_Code.replace(m_Code.begin() + start_pos, m_Code.begin() + _qpos, ((Script *)(pres->GetData()))->m_Code);
			}
		}
	}
}


void ScriptableImpl::PropertyChanged(const props::IProperty *pprop)
{
	if (!pprop)
		return;

	bool needs_init = false;

	switch (pprop->GetID())
	{
		case 'SRCF':
		{
			Resource *pres = m_pOwner->GetSystem()->GetResourceManager()->GetResource(pprop->AsString(), RESF_DEMANDLOAD);
			if (pres && (pres->GetStatus() == Resource::RS_LOADED))
			{
				if (!m_JS->ExecutingNow() && (m_Code != ((Script *)(pres->GetData()))->m_Code))
				{
					ResetJS();

					m_Code = ((Script *)(pres->GetData()))->m_Code;
					Preprocess();

					m_Continue = m_JS->Execute(m_Code.c_str());
					needs_init = true;
				}
			}
			break;
		}

		case 'SRC':
			if (!m_JS->ExecutingNow() && (m_Code != pprop->AsString()))
			{
				ResetJS();

				m_Code = pprop->AsString();
				Preprocess();

				m_Continue = m_JS->Execute(m_Code.c_str());
				needs_init = true;
			}
			break;

		case 'SUDR':
			m_UpdateRate = pprop->AsFloat();
			m_UpdateTime = 0.0f;
			break;
	}

	if (needs_init)
	{
		if (FunctionExists(_T("init")))
			Execute(_T("init();"));

		m_bHasUpdate = FunctionExists(_T("update"));
	}
}


bool ScriptableImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance, glm::fvec3 *pNormal, bool force) const
{
	return false;
}


void ScriptableImpl::Execute(const TCHAR *pcmd, ...)
{
	// if we receive a "continue" command from the editor, say,
	// then retry running the script if it had stopped...
	if (!m_Continue)
	{
		static const TCHAR *contstr = _T("continue");
		if (_tcsstr(contstr, pcmd) != contstr)
			return;

		m_Continue = true;
	}

	static tstring execbuf;

	va_list marker;
	va_start(marker, pcmd);

	int sz = _vsctprintf(pcmd, marker);
	if (sz > (int)execbuf.capacity())
	{
		execbuf.reserve((sz * 2) + 1);
	}
	execbuf.resize(sz + 1);

	_vsntprintf_s((TCHAR *)(execbuf.data()), execbuf.capacity(), execbuf.capacity(), pcmd, marker);

	bool cont = m_Continue;
	m_Continue = m_JS->Execute(execbuf.c_str());
	if (cont != m_Continue)
	{
		if (!m_Continue)
			m_pOwner->GetSystem()->GetLog()->Print(_T("\"%s\" no longer running scripts after command: \"%s\"\n"), m_pOwner->GetName(), execbuf.c_str());
	}
}

bool ScriptableImpl::FunctionExists(const TCHAR *funcname)
{
	CScriptVar *v = m_JS->GetScriptVariable(funcname);
	if (v && v->IsFunction())
		return true;

	return false;
}

void jcGetNumChildren(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	ret->SetInt(0);

	int64_t hrootobj = c->GetParameter(_T("hrootobj"))->GetInt();

	Object *rootobj = dynamic_cast<Object *>((Object *)hrootobj);
	if (rootobj)
		ret->SetInt(rootobj->GetNumChildren());
}


void jcGetChild(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	ret->SetInt(0);

	int64_t hrootobj = c->GetParameter(_T("hrootobj"))->GetInt();
	int64_t idx = c->GetParameter(_T("idx"))->GetInt();

	Object *rootobj = dynamic_cast<Object *>((Object *)hrootobj);
	if (rootobj)
		ret->SetInt((int64_t)(rootobj->GetChild(idx)));
}


void jcFindObjByGUID(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	ret->SetInt(0);

	int64_t hrootobj = c->GetParameter(_T("hrootobj"))->GetInt();
	tstring guids = c->GetParameter(_T("guid"))->GetString();
	bool recurse = c->GetParameter(_T("recursive"))->GetBool();

	std::function<Object *(Object *, GUID &, bool)> findObjByGuid = [&findObjByGuid] (Object *proot, GUID &sg, bool r) -> Object *
	{
		if (!proot)
			return nullptr;

		if (proot->GetGuid() == sg)
			return proot;

		Object *pc = nullptr;
		for (size_t i = 0, maxi = proot->GetNumChildren(); i < maxi; i++)
		{
			pc = findObjByGuid(proot->GetChild(i), sg, r);
			if (pc)
				break;
		}

		return pc;
	};

	Object *rootobj = dynamic_cast<Object *>((Object *)hrootobj);
	if (rootobj)
	{
		int d[11];
		_sntscanf_s(guids.c_str(), guids.length() * sizeof(TCHAR), _T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
					&d[0], &d[1], &d[2], &d[3], &d[4], &d[5], &d[6], &d[7], &d[8], &d[9], &d[10]);

		GUID g;
		g.Data1 = d[0];
		g.Data2 = d[1];
		g.Data3 = d[2];
		g.Data4[0] = d[3];
		g.Data4[1] = d[4];
		g.Data4[2] = d[5];
		g.Data4[3] = d[6];
		g.Data4[4] = d[7];
		g.Data4[5] = d[8];
		g.Data4[6] = d[9];
		g.Data4[7] = d[10];

		Object *pretobj = findObjByGuid(rootobj, g, recurse);
		ret->SetInt((int64_t)pretobj);
	}
}


void jcFindFirstObjByName(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	ret->SetInt(0);

	int64_t hrootobj = c->GetParameter(_T("hrootobj"))->GetInt();
	tstring names = c->GetParameter(_T("name"))->GetString();
	bool recurse = c->GetParameter(_T("recursive"))->GetBool();

	std::function<Object *(Object *, const TCHAR *, bool)> findObjByName = [&findObjByName] (Object *proot, const TCHAR *n, bool r) -> Object *
	{
		if (!proot)
			return nullptr;

		if (!_tcsicmp(proot->GetName(), n))
			return proot;

		Object *pc = nullptr;
		for (size_t i = 0, maxi = proot->GetNumChildren(); i < maxi; i++)
		{
			pc = findObjByName(proot->GetChild(i), n, r);
			if (pc)
				break;
		}

		return pc;
	};

	Object *rootobj = dynamic_cast<Object *>((Object *)hrootobj);
	if (rootobj)
	{
		Object *pretobj = findObjByName(rootobj, names.c_str(), recurse);
		ret->SetInt((int64_t)pretobj);
	}
}


void jcLog(CScriptVar *c, void *userdata)
{
	tstring text = c->GetParameter(_T("text"))->GetString();
	System *psys = (System *)userdata;
	assert(psys);

	psys->GetLog()->Print(text.c_str());
}


void jcExit(CScriptVar *c, void *userdata)
{
	exit(0);
}


void jcExecute(CScriptVar *c, void *userdata)
{
	Object *pobj = (Object *)(c->GetParameter(_T("hobj"))->GetInt());
	if (!pobj)
		return;

	Scriptable *pscr = (Scriptable *)pobj->FindComponent(Scriptable::Type());
	if (!pscr)
		return;

	pscr->Execute(c->GetParameter(_T("cmd"))->GetString());
}

void jcCreateProperty(CScriptVar* c, void* userdata)
{
	CScriptVar* ret = c->GetReturnVar();
	if (!ret)
		return;

	ret->SetInt(0);

	Object* pobj = (Object*)(c->GetParameter(_T("hobj"))->GetInt());
	if (!pobj)
		return;

	tstring name = c->GetParameter(_T("name"))->GetString();

	props::IProperty* p = pobj->GetProperties()->GetPropertyByName(name.c_str());
	ret->SetInt((int64_t)p);
}


void jcDeleteProperty(CScriptVar* c, void* userdata)
{
	CScriptVar* ret = c->GetReturnVar();
	if (!ret)
		return;

	ret->SetInt(0);

	Object* pobj = (Object*)(c->GetParameter(_T("hobj"))->GetInt());
	if (!pobj)
		return;

	tstring name = c->GetParameter(_T("name"))->GetString();

	props::IProperty* p = pobj->GetProperties()->GetPropertyByName(name.c_str());
	ret->SetInt((int64_t)p);
}


void jcFindProperty(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	ret->SetInt(0);

	Object *pobj = (Object *)(c->GetParameter(_T("hobj"))->GetInt());
	if (!pobj)
		return;

	tstring name = c->GetParameter(_T("name"))->GetString();

	props::IProperty *p = pobj->GetProperties()->GetPropertyByName(name.c_str());
	ret->SetInt((int64_t)p);
}


void jcGetPropertyValue(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	//ret->removeAllChildren();
	//ret->setUndefined();

	props::IProperty *pprop = dynamic_cast<props::IProperty *>((props::IProperty *)(c->GetParameter(_T("hprop"))->GetInt()));
	if (!pprop)
		return;

	CScriptVar *px = nullptr, *py = nullptr, *pz = nullptr, *pw = nullptr;
	CScriptVarLink *psvl;

	switch (pprop->GetType())
	{
		case props::IProperty::PROPERTY_TYPE::PT_BOOLEAN:
		case props::IProperty::PROPERTY_TYPE::PT_INT:
		case props::IProperty::PROPERTY_TYPE::PT_ENUM:
			ret->SetInt(pprop->AsInt());
			break;

		case props::IProperty::PROPERTY_TYPE::PT_FLOAT:
			ret->SetFloat(pprop->AsFloat());
			break;

		case props::IProperty::PROPERTY_TYPE::PT_STRING:
		case props::IProperty::PROPERTY_TYPE::PT_GUID:
			ret->SetString(pprop->AsString());
			break;

		case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4:
			psvl = ret->FindChildOrCreate(_T("x"));
			psvl->m_Owned = true;
			px = psvl->m_Var;
			px->SetFloat(pprop->AsVec2F()->x);

			psvl = ret->FindChildOrCreate(_T("y"));
			psvl->m_Owned = true;
			py = psvl->m_Var;
			py->SetFloat(pprop->AsVec2F()->y);

			psvl = ret->FindChildOrCreate(_T("z"));
			psvl->m_Owned = true;
			pz = psvl->m_Var;
			pz->SetFloat(pprop->AsVec3F()->z);

			psvl = ret->FindChildOrCreate(_T("w"));
			psvl->m_Owned = true;
			pw = psvl->m_Var;
			pw->SetFloat(pprop->AsVec4F()->w);
			break;

		case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3:
			psvl = ret->FindChildOrCreate(_T("x"));
			psvl->m_Owned = true;
			px = psvl->m_Var;
			px->SetFloat(pprop->AsVec2F()->x);

			psvl = ret->FindChildOrCreate(_T("y"));
			psvl->m_Owned = true;
			py = psvl->m_Var;
			py->SetFloat(pprop->AsVec2F()->y);

			psvl = ret->FindChildOrCreate(_T("z"));
			psvl->m_Owned = true;
			pz = psvl->m_Var;
			pz->SetFloat(pprop->AsVec3F()->z);
			break;

		case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2:
			psvl = ret->FindChildOrCreate(_T("x"));
			psvl->m_Owned = true;
			px = psvl->m_Var;
			px->SetFloat(pprop->AsVec2F()->x);

			psvl = ret->FindChildOrCreate(_T("y"));
			psvl->m_Owned = true;
			py = psvl->m_Var;
			py->SetFloat(pprop->AsVec2F()->y);
			break;

		case props::IProperty::PROPERTY_TYPE::PT_INT_V4:
			psvl = ret->FindChildOrCreate(_T("x"));
			psvl->m_Owned = true;
			px = psvl->m_Var;
			px->SetInt(pprop->AsVec2I()->x);

			psvl = ret->FindChildOrCreate(_T("y"));
			psvl->m_Owned = true;
			py = psvl->m_Var;
			py->SetInt(pprop->AsVec2I()->y);

			psvl = ret->FindChildOrCreate(_T("z"));
			psvl->m_Owned = true;
			pz = psvl->m_Var;
			pz->SetInt(pprop->AsVec3I()->z);

			psvl = ret->FindChildOrCreate(_T("w"));
			psvl->m_Owned = true;
			pw = psvl->m_Var;
			pw->SetInt(pprop->AsVec4I()->w);
			break;

		case props::IProperty::PROPERTY_TYPE::PT_INT_V3:
			psvl = ret->FindChildOrCreate(_T("x"));
			psvl->m_Owned = true;
			px = psvl->m_Var;
			px->SetInt(pprop->AsVec2I()->x);

			psvl = ret->FindChildOrCreate(_T("y"));
			psvl->m_Owned = true;
			py = psvl->m_Var;
			py->SetInt(pprop->AsVec2I()->y);

			psvl = ret->FindChildOrCreate(_T("z"));
			psvl->m_Owned = true;
			pz = psvl->m_Var;
			pz->SetInt(pprop->AsVec3I()->z);
			break;

		case props::IProperty::PROPERTY_TYPE::PT_INT_V2:
			psvl = ret->FindChildOrCreate(_T("x"));
			psvl->m_Owned = true;
			px = psvl->m_Var;
			px->SetInt(pprop->AsVec2I()->x);

			psvl = ret->FindChildOrCreate(_T("y"));
			psvl->m_Owned = true;
			py = psvl->m_Var;
			py->SetInt(pprop->AsVec2I()->y);
			break;
	}
}


void jcSetPropertyValue(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	props::IProperty *pprop = (props::IProperty *)(c->GetParameter(_T("hprop"))->GetInt());
	if (!pprop)
		return;

	CScriptVar *pval = c->GetParameter(_T("val"));
	if (!pprop)
		return;

	if (pval->IsInt())
	{
		switch (pprop->GetType())
		{
			case props::IProperty::PROPERTY_TYPE::PT_BOOLEAN:
				pprop->SetBool(pval->GetInt() ? true : false);
				break;

			case props::IProperty::PROPERTY_TYPE::PT_ENUM:
				pprop->SetEnumVal(pval->GetInt());
				break;

			case props::IProperty::PROPERTY_TYPE::PT_FLOAT:
				pprop->SetFloat((float)pval->GetInt());
				break;

			default:
				pprop->SetInt(pval->GetInt());
				break;
		}
	}
	else if (pval->IsFloat())
	{
		pprop->SetFloat((float)pval->GetFloat());
	}
	else if (pval->IsString())
	{
		pprop->SetString(pval->GetString());
	}
	else if (pval->GetNumChildren())
	{
		CScriptVarLink *px = pval->FindChild(_T("x")),
			*py = pval->FindChild(_T("y")),
			*pz = pval->FindChild(_T("z")),
			*pw = pval->FindChild(_T("w"));

		size_t ct = 0;
		if (px) ct++;
		if (py) ct++;
		if (pz) ct++;
		if (pw) ct++;

		switch (pprop->GetType())
		{
			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4:
			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3:
			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2:
			{
				switch (ct)
				{
					default:
						break;
					case 2:
					{
						pprop->SetVec2F(props::SVec2<float>(
							px ? px->m_Var->GetFloat() : 0,
							py ? py->m_Var->GetFloat() : 0));
						break;
					}
					case 3:
					{
						pprop->SetVec3F(props::SVec3<float>(
							px ? px->m_Var->GetFloat() : 0,
							py ? py->m_Var->GetFloat() : 0,
							pz ? pz->m_Var->GetFloat() : 0));
						break;
					}
					case 4:
					{
						pprop->SetVec4F(props::SVec4<float>(
							px ? px->m_Var->GetFloat() : 0,
							py ? py->m_Var->GetFloat() : 0,
							pz ? pz->m_Var->GetFloat() : 0,
							pw ? pw->m_Var->GetFloat() : 0));
						break;
					}
				}
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_INT_V4:
			case props::IProperty::PROPERTY_TYPE::PT_INT_V3:
			case props::IProperty::PROPERTY_TYPE::PT_INT_V2:
			{
				switch (ct)
				{
					default:
						break;
					case 2:
					{
						pprop->SetVec2I(props::SVec2<int64_t>(
							px ? px->m_Var->GetInt() : 0,
							py ? py->m_Var->GetInt() : 0));
						break;
					}
					case 3:
					{
						pprop->SetVec3I(props::SVec3<int64_t>(
							px ? px->m_Var->GetInt() : 0,
							py ? py->m_Var->GetInt() : 0,
							pz ? pz->m_Var->GetInt() : 0));
						break;
					}
					case 4:
					{
						pprop->SetVec4I(props::SVec4<int64_t>(
							px ? px->m_Var->GetInt() : 0,
							py ? py->m_Var->GetInt() : 0,
							pz ? pz->m_Var->GetInt() : 0,
							pw ? pw->m_Var->GetInt() : 0));
						break;
					}
				}
				break;
			}
		}
	}
}


//m_JS->AddNative(_T("function CreateObject(protoname, hparentobj)"),				jcCreateObject, psys);
void jcCreateObject(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	System *psys = (System *)userdata;
	assert(psys);

	ret->SetInt(0);

	tstring protoname = c->GetParameter(_T("protoname"))->GetString();
	int64_t hparentobj = c->GetParameter(_T("hparentobj"))->GetInt();

	Object *pretobj = nullptr;

	Prototype *pproto = psys->GetFactory()->FindPrototype(protoname.c_str(), false);
	// if a particular prototype was specified and not found, inform the user
	if (!protoname.empty() && !pproto)
		psys->GetLog()->Print(_T("CreateObject could not find prototype: \"%s\"\n"), protoname.c_str());

	Object *pparentobj = dynamic_cast<Object *>((Object *)hparentobj);

	pretobj = psys->GetFactory()->Build(pproto, nullptr, pparentobj);

	ret->SetInt((int64_t)pretobj);
}


//m_JS->AddNative(_T("function DeleteObject(hobj)"),								jcDeleteObject, psys);
void jcDeleteObject(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		pobj->Release();
	}
}


//m_JS->AddNative(_T("function GetParent(hobj)"),									jcGetParent, psys);
void jcGetParent(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	ret->SetInt(0);

	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();

	Object *pretobj = nullptr;

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		pretobj = pobj->GetParent();
	}

	ret->SetInt((int64_t)pretobj);
}


//m_JS->AddNative(_T("function SetParent(hobj, hnewparentobj)"),					jcSetParent, psys);
void jcSetParent(CScriptVar *c, void *userdata)
{
	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	int64_t hnewparentobj = c->GetParameter(_T("hnewparentobj"))->GetInt();

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	Object *pnewparentobj = dynamic_cast<Object *>((Object *)hnewparentobj);
	if (pnewparentobj && pobj)
	{
		pobj->SetParent(pnewparentobj);
	}
}


//m_JS->AddNative(_T("function GetObjectName(hobj)"),						jcSetObjectName, psys);
void jcGetObjectName(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		ret->SetString(pobj->GetName());
		return;
	}

	ret->SetString(_T("[unnamed object]"));
}


//m_JS->AddNative(_T("function SetObjectName(hobj, newname)"),						jcSetObjectName, psys);
void jcSetObjectName(CScriptVar *c, void *userdata)
{
	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	tstring newname = c->GetParameter(_T("newname"))->GetString();

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		pobj->SetName(newname.c_str());
	}
}


//m_JS->AddNative(_T("function LoadPrototypes(filename)"),				jcLoadPrototypes, psys);
void jcLoadPrototypes(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();

	if (ret)
		ret->SetInt(0);

	System *psys = (System *)userdata;
	assert(psys);

	tstring filename = c->GetParameter(_T("filename"))->GetString();
	TCHAR fullpath[MAX_PATH];

	if (psys->GetFileMapper()->FindFile(filename.c_str(), fullpath, MAX_PATH))
	{
		if (psys->GetFactory()->LoadPrototypes((const tinyxml2::XMLNode *)nullptr, fullpath))
			ret->SetInt(1);
	}
}


//m_JS->AddNative(_T("function LoadObject(hobj, filename)"),				jcLoadObject, psys);
void jcLoadObject(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();

	if (ret)
		ret->SetInt(0);

	System *psys = (System *)userdata;
	assert(psys);

	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	tstring filename = c->GetParameter(_T("filename"))->GetString();
	TCHAR fullpath[MAX_PATH];

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);

	if (pobj && psys->GetFileMapper()->FindFile(filename.c_str(), fullpath, MAX_PATH))
	{
		genio::IInputStream *is = genio::IInputStream::Create();
		if (is)
		{
			if (is->Assign(fullpath) && is->Open())
			{
				CScriptVarLink *psvl;

				c3::Object::MetadataLoadFunc loadmd = [&](const tstring &name, const tstring &description, const tstring &author, const tstring &website, const tstring &copyright)
				{
					if (ret)
					{
						if (psvl = ret->FindChildOrCreate(_T("name")))
							psvl->m_Var->SetString(name.c_str());

						if (psvl = ret->FindChildOrCreate(_T("description")))
							psvl->m_Var->SetString(description.c_str());

						if (psvl = ret->FindChildOrCreate(_T("author")))
							psvl->m_Var->SetString(author.c_str());

						if (psvl = ret->FindChildOrCreate(_T("website")))
							psvl->m_Var->SetString(website.c_str());

						if (psvl = ret->FindChildOrCreate(_T("copyright")))
							psvl->m_Var->SetString(copyright.c_str());
					}
				};

				c3::Object::CameraLoadFunc loadcam = [&](c3::Object *camera, float yaw, float pitch)
				{
					if (psvl = ret->FindChildOrCreate(_T("camera")))
						psvl->m_Var->SetInt((int64_t)camera);

					if (psvl = ret->FindChildOrCreate(_T("yaw")))
						psvl->m_Var->SetFloat(yaw);

					if (psvl = ret->FindChildOrCreate(_T("pitch")))
						psvl->m_Var->SetFloat(pitch);
				};

				c3::Object::EnvironmentLoadFunc loadenv = [&](const glm::fvec4 &clearcolor, const glm::fvec4 &shadowcolor, const glm::fvec4 &fogcolor, const float &fogdensity)
				{
					Color::SRGBAColor c;

					c = Color::ConvertVecToInt((props::TVec4F &)clearcolor);
					if (psvl = ret->FindChildOrCreate(_T("clearcolor")))
						psvl->m_Var->SetInt((int64_t)c.i);

					c = Color::ConvertVecToInt((props::TVec4F &)shadowcolor);
					if (psvl = ret->FindChildOrCreate(_T("shadowcolor")))
						psvl->m_Var->SetInt((int64_t)c.i);

					c = Color::ConvertVecToInt((props::TVec4F &)fogcolor);
					if (psvl = ret->FindChildOrCreate(_T("fogcolor")))
						psvl->m_Var->SetInt((int64_t)c.i);

					if (psvl = ret->FindChildOrCreate(_T("fogdensity")))
						psvl->m_Var->SetFloat(fogdensity);
				};

				bool r = pobj->Load(is, pobj->GetParent(), loadmd, loadcam, loadenv);

				if (ret && (psvl = ret->FindChildOrCreate(_T("success"))))
					psvl->m_Var->SetInt(r ? 1 : 0);
			}

			is->Release();
		}
	}
}


//m_JS->AddNative(_T("function SaveObject(hobj, filename)"),				jcSaveObject, psys);
void jcSaveObject(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();

	if (ret)
		ret->SetInt(0);

	System *psys = (System *)userdata;
	assert(psys);

	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	tstring filename = c->GetParameter(_T("filename"))->GetString();
	TCHAR fullpath[MAX_PATH];

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);

	genio::IOutputStream *os = genio::IOutputStream::Create();
	if (os)
	{
		if (os->Assign(fullpath) && os->Open())
		{
			bool r = pobj->Save(os, 0);

			if (ret)
				ret->SetInt(r ? 1 : 0);
		}

		os->Release();
	}
}


const TCHAR *gGlobalDesignations[GlobalObjectRegistry::OD_NUMDESIGNATIONS] =
{
	_T("worldroot"),
	_T("skyboxroot"),
	_T("camera.root"),
	_T("camera.arm"),
	_T("camera"),
	_T("gui.root"),
	_T("gui.camera"),
	_T("player"),
};

//m_JS->AddNative(_T("function jcGetRegisteredObject(designation)"),					jcGetRegisteredObject, psys);
void jcGetRegisteredObject(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	System *psys = (System *)userdata;
	assert(psys);

	tstring designation = c->GetParameter(_T("designation"))->GetString();

	Object *pretobj = nullptr;

	for (size_t i = 0; i < GlobalObjectRegistry::OD_NUMDESIGNATIONS; i++)
	{
		if (!_tcsicmp(designation.c_str(), gGlobalDesignations[i]))
		{
			pretobj = psys->GetGlobalObjectRegistry()->GetRegisteredObject((GlobalObjectRegistry::ObjectDesignation)i);
			break;
		}
	}

	ret->SetInt((int64_t)pretobj);
}


//m_JS->AddNative(_T("function jcRegisterObject(designation, hobj)"),			jcRegisterObject, psys);
void jcRegisterObject(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	tstring designation = c->GetParameter(_T("designation"))->GetString();
	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();

	for (size_t i = 0; i < GlobalObjectRegistry::OD_NUMDESIGNATIONS; i++)
	{
		if (!_tcsicmp(designation.c_str(), gGlobalDesignations[i]))
		{
			psys->GetGlobalObjectRegistry()->RegisterObject((GlobalObjectRegistry::ObjectDesignation)i, (Object *)hobj);
			return;
		}
	}
}


// corresponds to the flags defined in Object
std::vector<const TCHAR *> gObjFlagName =
{
	_T("UPDATE"),
	_T("DRAW"),		
	_T("DRAWINEDITOR"),
	_T("POSCHANGED"),
	_T("ORICHANGED"),		
	_T("SCLCHANGED"),
	_T("KILL"),	
	_T("TEMPORARY"),
	_T("CHECKCOLLISIONS"),
	_T("TRACKCAMX"),
	_T("TRACKCAMY"),
	_T("TRACKCAMZ"),
	_T("TRACKCAMLITERAL"),
	_T("BILLBOARD"),
	_T("CHILDRENDIRTY"),
	_T("PARENTDIRTY"),
	_T("LIGHT"),
	_T("CASTSHADOW"),
	_T("NOMODELSCALE")
};


//m_JS->AddNative(_T("function IsObjFlagSet(hobj, flagname)"),			jcIsObjFlagSet, psys);
void jcIsObjFlagSet(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	CScriptVar *phobj = c->GetParameter(_T("hobj"));
	CScriptVar *pflagname = c->GetParameter(_T("flagname"));
	if (!phobj || !pflagname)
		return;

	Object *pobj = dynamic_cast<Object *>((Object *)phobj->GetInt());
	if (pobj)
	{
		uint64_t flag = 1;
		for (auto s : gObjFlagName)
		{
			if (!_tcsicmp(pflagname->GetString(), s))
				break;

			flag <<= 1;
		}

		ret->SetInt(pobj->Flags().IsSet(flag) ? 1 : 0);
		return;
	}

	ret->SetInt(0);
}


//m_JS->AddNative(_T("function SetObjFlag(hobj, flagname, val)"),			jcSetObjFlag, psys);
void jcSetObjFlag(CScriptVar *c, void *userdata)
{
	CScriptVar *phobj = c->GetParameter(_T("hobj"));
	CScriptVar *pflagname = c->GetParameter(_T("flagname"));
	CScriptVar *pval = c->GetParameter(_T("val"));
	if (!phobj || !pflagname || !pval)
		return;

	Object *pobj = dynamic_cast<Object *>((Object *)phobj->GetInt());
	if (pobj)
	{
		uint64_t flag = 1;
		for (auto s : gObjFlagName)
		{
			if (!_tcsicmp(pflagname->GetString(), s))
			{
				if (!(pval->GetInt()))
					pobj->Flags().Clear(flag);
				else
					pobj->Flags().Set(flag);

				break;
			}

			flag <<= 1;
		}
	}
}


//m_JS->AddNative(_T("function AdjustQuat(quat, axis, angle)"),			jcAdjustQuat, psys);
void jcAdjustQuat(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	CScriptVar *paxis = c->GetParameter(_T("axis"));
	CScriptVar *pquat = c->GetParameter(_T("quat"));
	CScriptVar *pangle = c->GetParameter(_T("angle"));
	if (!paxis || !pquat || !pangle)
		return;

	CScriptVarLink *pax = paxis->FindChild(_T("x"));
	CScriptVarLink *pay = paxis->FindChild(_T("y"));
	CScriptVarLink *paz = paxis->FindChild(_T("z"));
	if (!(pax && pay && paz)) // || (paxis->GetArrayLength() != 3))
		return;

	glm::fvec3 axis;
	axis.x = (float)(pax ? pax->m_Var->GetFloat() : paxis->GetArrayIndex(0)->GetFloat());
	axis.y = (float)(pay ? pay->m_Var->GetFloat() : paxis->GetArrayIndex(1)->GetFloat());
	axis.z = (float)(paz ? paz->m_Var->GetFloat() : paxis->GetArrayIndex(2)->GetFloat());


	CScriptVarLink *pqx = pquat->FindChild(_T("x"));
	CScriptVarLink *pqy = pquat->FindChild(_T("y"));
	CScriptVarLink *pqz = pquat->FindChild(_T("z"));
	CScriptVarLink *pqw = pquat->FindChild(_T("w"));
	if (!(pqx && pqy && pqz && pqw)) // || (pquat->GetArrayLength() != 4))
		return;

	glm::fquat q;
	q.x = (float)(pqx ? pqx->m_Var->GetFloat() : pquat->GetArrayIndex(0)->GetFloat());
	q.y = (float)(pqy ? pqy->m_Var->GetFloat() : pquat->GetArrayIndex(1)->GetFloat());
	q.z = (float)(pqz ? pqz->m_Var->GetFloat() : pquat->GetArrayIndex(2)->GetFloat());
	q.w = (float)(pqw ? pqw->m_Var->GetFloat() : pquat->GetArrayIndex(3)->GetFloat());

	glm::fquat r;
	float angle = (float)pangle->GetFloat();
	r = glm::angleAxis(angle, axis);

	q = r * q;

	CScriptVarLink *psvl;

	psvl = ret->FindChildOrCreate(_T("x"));
	psvl->m_Owned = true;
	CScriptVar *prx = psvl->m_Var;

	psvl = ret->FindChildOrCreate(_T("y"));
	psvl->m_Owned = true;
	CScriptVar *pry = psvl->m_Var;

	psvl = ret->FindChildOrCreate(_T("z"));
	psvl->m_Owned = true;
	CScriptVar *prz = psvl->m_Var;

	psvl = ret->FindChildOrCreate(_T("w"));
	psvl->m_Owned = true;
	CScriptVar *prw = psvl->m_Var;

	prx->SetFloat(q.x);
	pry->SetFloat(q.y);
	prz->SetFloat(q.z);
	prw->SetFloat(q.w);
}


//m_JS->AddNative(_T("function EulerToQuat(euler)"),			jcEulerToQuat, psys);
void jcEulerToQuat(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	CScriptVar *peuler = c->GetParameter(_T("euler"));
	if (!peuler)
		return;

	CScriptVarLink *px = peuler->FindChild(_T("x"));
	CScriptVarLink *py = peuler->FindChild(_T("y"));
	CScriptVarLink *pz = peuler->FindChild(_T("z"));
	if (!(px && py && pz))
		return;

	glm::fquat q = glm::identity<glm::fquat>();

	float roll = glm::radians(py->m_Var->GetFloat());
	q = glm::normalize(glm::angleAxis(roll, glm::fvec3(0, 1, 0)) * q);

	float pitch = glm::radians(px->m_Var->GetFloat());
	q = glm::normalize(glm::angleAxis(pitch, glm::fvec3(1, 0, 0)) * q);

	float yaw = glm::radians(pz->m_Var->GetFloat());
	q = glm::normalize(glm::angleAxis(yaw, glm::fvec3(0, 0, 1)) * q);

	CScriptVarLink *psvl;

	psvl = ret->FindChildOrCreate(_T("x"));
	psvl->m_Owned = true;
	CScriptVar *prx = psvl->m_Var;

	psvl = ret->FindChildOrCreate(_T("y"));
	psvl->m_Owned = true;
	CScriptVar *pry = psvl->m_Var;

	psvl = ret->FindChildOrCreate(_T("z"));
	psvl->m_Owned = true;
	CScriptVar *prz = psvl->m_Var;

	psvl = ret->FindChildOrCreate(_T("w"));
	psvl->m_Owned = true;
	CScriptVar *prw = psvl->m_Var;

	prx->SetFloat(q.x);
	pry->SetFloat(q.y);
	prz->SetFloat(q.z);
	prw->SetFloat(q.w);
}


//m_JS->AddNative(_T("function QuatToEuler(quat)"),			jcQuatToEuler, psys);
void jcQuatToEuler(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	CScriptVar *pquat = c->GetParameter(_T("quat"));
	if (!pquat)
		return;

	CScriptVarLink *pqx = pquat->FindChild(_T("x"));
	CScriptVarLink *pqy = pquat->FindChild(_T("y"));
	CScriptVarLink *pqz = pquat->FindChild(_T("z"));
	CScriptVarLink *pqw = pquat->FindChild(_T("w"));
	if (!(pqx && pqy && pqz && pqw))
		return;

	glm::fquat q;
	q.x = (float)(pqx ? pqx->m_Var->GetFloat() : pquat->GetArrayIndex(0)->GetFloat());
	q.y = (float)(pqy ? pqy->m_Var->GetFloat() : pquat->GetArrayIndex(1)->GetFloat());
	q.z = (float)(pqz ? pqz->m_Var->GetFloat() : pquat->GetArrayIndex(2)->GetFloat());
	q.w = (float)(pqw ? pqw->m_Var->GetFloat() : pquat->GetArrayIndex(3)->GetFloat());

	CScriptVarLink *psvl;

	psvl = ret->FindChildOrCreate(_T("x"));
	psvl->m_Owned = true;
	CScriptVar *prx = psvl->m_Var;

	psvl = ret->FindChildOrCreate(_T("y"));
	psvl->m_Owned = true;
	CScriptVar *pry = psvl->m_Var;

	psvl = ret->FindChildOrCreate(_T("z"));
	psvl->m_Owned = true;
	CScriptVar *prz = psvl->m_Var;

	float pitch = glm::degrees(c3::math::GetPitch(&q));
	float yaw = glm::degrees(c3::math::GetRoll(&q));
	float roll = glm::degrees(c3::math::GetYaw(&q));
	prx->SetFloat(pitch);
	pry->SetFloat(roll);
	prz->SetFloat(yaw);
}


//m_JS->AddNative(_T("function PlaySound(filename, volmod, pitchmod, loop, pos)"),			jcPlaySound, psys);
void jcPlaySound(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	CScriptVar *pfilename = c->GetParameter(_T("filename"));
	if (!pfilename)
		return;

	CScriptVar *pvolmod = c->GetParameter(_T("volmod"));
	CScriptVar *ppitchmod = c->GetParameter(_T("pitchmod"));
	CScriptVar *ploop = c->GetParameter(_T("loop"));

	glm::fvec3 pos(0, 0, 0);
	CScriptVar *ppos = c->GetParameter(_T("pos"));
	if (ppos)
	{
		CScriptVarLink *px = ppos->FindChild(_T("x"));
		if (px)	pos.x = px->m_Var->GetFloat();

		CScriptVarLink *py = ppos->FindChild(_T("y"));
		if (py)	pos.y = py->m_Var->GetFloat();

		CScriptVarLink *pz = ppos->FindChild(_T("z"));
		if (pz)	pos.z = pz->m_Var->GetFloat();
	}

	Resource *pres = psys->GetResourceManager()->GetResource(pfilename->GetString(), RESF_DEMANDLOAD);

	ret->SetInt((int64_t)(psys->GetSoundPlayer()->Play(pres, SoundPlayer::SOUND_TYPE::ST_SFX, pvolmod ? pvolmod->GetFloat() : 0, ppitchmod ? ppitchmod->GetFloat() : 0, ploop ? ploop->GetInt() : 1, &pos)));
}


//m_JS->AddNative(_T("function StopSound(hsound)"),			jcStopSound, psys);
void jcStopSound(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	CScriptVar *phsound = c->GetParameter(_T("hsound"));

	psys->GetSoundPlayer()->Stop(phsound->GetInt());
}


//m_JS->AddNative(_T("function PauseSound(hsound)"),			jcPauseSound, psys);
void jcPauseSound(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	CScriptVar *phsound = c->GetParameter(_T("hsound"));

	psys->GetSoundPlayer()->Pause(phsound->GetInt());
}


//m_JS->AddNative(_T("function ResumeSound(hsound)"),			jcResumeSound, psys);
void jcResumeSound(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	CScriptVar *phsound = c->GetParameter(_T("hsound"));

	psys->GetSoundPlayer()->Resume(phsound->GetInt());
}


//m_JS->AddNative(_T("function SetSoundVolume(hsound, vol)"),			jcSetSoundVolume, psys);
void jcSetSoundVolume(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	CScriptVar *phsound = c->GetParameter(_T("hsound"));
	CScriptVar *pvol = c->GetParameter(_T("vol"));

	float vol = pvol->GetFloat();
	if (vol < 0)
		vol = 0.0f;

	psys->GetSoundPlayer()->SetChannelVolume(phsound->GetInt(), vol);
}


//m_JS->AddNative(_T("function SetSoundPitchMod(hsound, pitchmod)"),			jcSetSoundPitchMod, psys);
void jcSetSoundPitchMod(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	CScriptVar *phsound = c->GetParameter(_T("hsound"));
	CScriptVar *ppitchmod = c->GetParameter(_T("pitchmod"));

	psys->GetSoundPlayer()->SetChannelPitchMod(phsound->GetInt(), ppitchmod->GetFloat());
}

//m_JS->AddNative(_T("function SetSoundPosition(hsound, pos)"),			jcSetSoundPosition, psys);
void jcSetSoundPosition(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	CScriptVar *phsound = c->GetParameter(_T("hsound"));

	glm::fvec3 pos(0, 0, 0);
	CScriptVar *ppos = c->GetParameter(_T("pos"));
	if (ppos)
	{
		CScriptVarLink *px = ppos->FindChild(_T("x"));
		if (px)	pos.x = px->m_Var->GetFloat();

		CScriptVarLink *py = ppos->FindChild(_T("y"));
		if (py)	pos.y = py->m_Var->GetFloat();

		CScriptVarLink *pz = ppos->FindChild(_T("z"));
		if (pz)	pos.z = pz->m_Var->GetFloat();
	}

	psys->GetSoundPlayer()->SetChannelPos(phsound->GetInt(), &pos);
}


//m_JS->AddNative(_T("function GetModelNodeIndex(hobj, nodename)"),						jcGetModelNodeIndex, psys);
void jcGetModelNodeIndex(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	Model::NodeIndex ni = Model::INVALID_INDEX;

	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	tstring nodename = c->GetParameter(_T("nodename"))->GetString();

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(pobj->FindComponent(ModelRenderer::Type()));
		if (pmr)
		{
			const Model *pm = pmr->GetModel();
			if (pm)
				pm->FindNode(nodename.c_str(), &ni);
		}
	}

	ret->SetInt(ni);
}


//m_JS->AddNative(_T("function GetModelNodeCount(hobj)"),							jcGetModelNodeCount, psys);
void jcGetModelNodeCount(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(pobj->FindComponent(ModelRenderer::Type()));
		if (pmr)
		{
			const Model *pm = pmr->GetModel();
			if (pm)
			{
				ret->SetInt(pm->GetNodeCount());
				return;
			}
		}
	}

	ret->SetInt(0);
}


//m_JS->AddNative(_T("function GetModelInstNodePos(hobj, nodeidx)"),						jcGetModelInstNodePos, psys);
void jcGetModelInstNodePos(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	Model::NodeIndex ni = c->GetParameter(_T("nodeidx"))->GetInt();

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(pobj->FindComponent(ModelRenderer::Type()));
		if (pmr)
		{
			glm::fmat4x4 t;
			if (pmr->GetModelInstanceData()->GetTransform(ni, t))
			{
				glm::fvec3 scl;
				glm::fquat ori;
				glm::fvec3 pos;
				glm::fvec3 skew;
				glm::fvec4 perspective;
				glm::decompose(t, scl, ori, pos, skew, perspective);

				CScriptVarLink *psvl;

				psvl = ret->FindChildOrCreate(_T("x"));
				psvl->m_Owned = true;
				CScriptVar *prx = psvl->m_Var;

				psvl = ret->FindChildOrCreate(_T("y"));
				psvl->m_Owned = true;
				CScriptVar *pry = psvl->m_Var;

				psvl = ret->FindChildOrCreate(_T("z"));
				psvl->m_Owned = true;
				CScriptVar *prz = psvl->m_Var;

				prx->SetFloat(pos.x);
				pry->SetFloat(pos.y);
				prz->SetFloat(pos.z);
			}
		}
	}
}


//m_JS->AddNative(_T("function GetModelInstNodeOri(hobj, nodeidx)"),						jcGetModelInstNodeOri, psys);
void jcGetModelInstNodeOri(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	Model::NodeIndex ni = c->GetParameter(_T("nodeidx"))->GetInt();

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(pobj->FindComponent(ModelRenderer::Type()));
		Model::InstanceData* pmid = pmr ? pmr->GetModelInstanceData() : nullptr;
		if (pmid)
		{
			glm::fmat4x4 t;
			if (pmid->GetTransform(ni, t))
			{
				glm::fvec3 scl;
				glm::fquat ori;
				glm::fvec3 pos;
				glm::fvec3 skew;
				glm::fvec4 perspective;
				glm::decompose(t, scl, ori, pos, skew, perspective);

				CScriptVarLink *psvl;

				psvl = ret->FindChildOrCreate(_T("x"));
				psvl->m_Owned = true;
				CScriptVar *prx = psvl->m_Var;

				psvl = ret->FindChildOrCreate(_T("y"));
				psvl->m_Owned = true;
				CScriptVar *pry = psvl->m_Var;

				psvl = ret->FindChildOrCreate(_T("z"));
				psvl->m_Owned = true;
				CScriptVar *prz = psvl->m_Var;

				psvl = ret->FindChildOrCreate(_T("w"));
				psvl->m_Owned = true;
				CScriptVar *prw = psvl->m_Var;

				prx->SetFloat(ori.x);
				pry->SetFloat(ori.y);
				prz->SetFloat(ori.z);
				prw->SetFloat(ori.w);
			}
		}
	}
}


//m_JS->AddNative(_T("function GetModelInstNodeScl(hobj, nodeidx)"),						jcGetModelInstNodeScl, psys);
void jcGetModelInstNodeScl(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	Model::NodeIndex ni = c->GetParameter(_T("nodeidx"))->GetInt();

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(pobj->FindComponent(ModelRenderer::Type()));
		Model::InstanceData* pmid = pmr ? pmr->GetModelInstanceData() : nullptr;
		if (pmid)
		{
			glm::fmat4x4 t;
			if (pmid->GetTransform(ni, t))
			{
				glm::fvec3 scl;
				glm::fquat ori;
				glm::fvec3 pos;
				glm::fvec3 skew;
				glm::fvec4 perspective;
				glm::decompose(t, scl, ori, pos, skew, perspective);

				CScriptVarLink *psvl;

				psvl = ret->FindChildOrCreate(_T("x"));
				psvl->m_Owned = true;
				CScriptVar *prx = psvl->m_Var;

				psvl = ret->FindChildOrCreate(_T("y"));
				psvl->m_Owned = true;
				CScriptVar *pry = psvl->m_Var;

				psvl = ret->FindChildOrCreate(_T("z"));
				psvl->m_Owned = true;
				CScriptVar *prz = psvl->m_Var;

				prx->SetFloat(scl.x);
				pry->SetFloat(scl.y);
				prz->SetFloat(scl.z);
			}
		}
	}
}


//m_JS->AddNative(_T("function SetModelInstNodePos(hobj, nodeidx, pos)"),				jcSetModelInstNodePos, psys);
void jcSetModelInstNodePos(CScriptVar *c, void *userdata)
{
	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	Model::NodeIndex ni = c->GetParameter(_T("nodeidx"))->GetInt();
	CScriptVar *ppos = c->GetParameter(_T("pos"));
	if (!ppos)
		return;

	CScriptVarLink *px = ppos->FindChild(_T("x"));
	CScriptVarLink *py = ppos->FindChild(_T("y"));
	CScriptVarLink *pz = ppos->FindChild(_T("z"));
	if (!(px && py && pz))
		return;

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(pobj->FindComponent(ModelRenderer::Type()));
		if (pmr)
		{
			glm::fmat4x4 t;
			if (pmr->GetModelInstanceData()->GetTransform(ni, t))
			{
				glm::fvec3 scl;
				glm::fquat ori;
				glm::fvec3 pos;
				glm::fvec3 skew;
				glm::fvec4 perspective;
				glm::decompose(t, scl, ori, pos, skew, perspective);

				pos.x = px->m_Var->GetFloat();
				pos.y = py->m_Var->GetFloat();
				pos.z = pz->m_Var->GetFloat();

				t = glm::scale(glm::identity<glm::fmat4x4>(), scl) * (glm::fmat4x4)(ori);

				// Then translate last... 
				t = glm::translate(glm::identity<glm::fmat4x4>(), pos) * t;

				pmr->GetModelInstanceData()->SetTransform(ni, t);
			}
		}
	}
}


//m_JS->AddNative(_T("function SetModelInstNodeOri(hobj, nodeidx, Ori)"),				jcSetModelInstNodeOri, psys);
void jcSetModelInstNodeOri(CScriptVar *c, void *userdata)
{
	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	Model::NodeIndex ni = c->GetParameter(_T("nodeidx"))->GetInt();
	CScriptVar *pori = c->GetParameter(_T("ori"));
	if (!pori)
		return;

	CScriptVarLink *px = pori->FindChild(_T("x"));
	CScriptVarLink *py = pori->FindChild(_T("y"));
	CScriptVarLink *pz = pori->FindChild(_T("z"));
	CScriptVarLink *pw = pori->FindChild(_T("w"));
	if (!(px && py && pz && pw))
		return;

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(pobj->FindComponent(ModelRenderer::Type()));
		if (pmr)
		{
			glm::fmat4x4 t;
			if (pmr->GetModelInstanceData()->GetTransform(ni, t))
			{
				glm::fvec3 scl;
				glm::fquat ori;
				glm::fvec3 pos;
				glm::fvec3 skew;
				glm::fvec4 perspective;
				glm::decompose(t, scl, ori, pos, skew, perspective);

				ori.x = px->m_Var->GetFloat();
				ori.y = py->m_Var->GetFloat();
				ori.z = pz->m_Var->GetFloat();
				ori.w = pw->m_Var->GetFloat();

				t = glm::scale(glm::identity<glm::fmat4x4>(), scl) * (glm::fmat4x4)(ori);

				// Then translate last... 
				t = glm::translate(glm::identity<glm::fmat4x4>(), pos) * t;

				pmr->GetModelInstanceData()->SetTransform(ni, t);
			}
		}
	}
}


//m_JS->AddNative(_T("function SetModelInstNodeScl(hobj, nodeidx, scl)"),				jcSetModelInstNodeScl, psys);
void jcSetModelInstNodeScl(CScriptVar *c, void *userdata)
{
	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	Model::NodeIndex ni = c->GetParameter(_T("nodeidx"))->GetInt();
	CScriptVar *pscl = c->GetParameter(_T("scl"));
	if (!pscl)
		return;

	CScriptVarLink *px = pscl->FindChild(_T("x"));
	CScriptVarLink *py = pscl->FindChild(_T("y"));
	CScriptVarLink *pz = pscl->FindChild(_T("z"));
	if (!(px && py && pz))
		return;

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(pobj->FindComponent(ModelRenderer::Type()));
		if (pmr)
		{
			glm::fmat4x4 t;
			if (pmr->GetModelInstanceData()->GetTransform(ni, t))
			{
				glm::fvec3 scl;
				glm::fquat ori;
				glm::fvec3 pos;
				glm::fvec3 skew;
				glm::fvec4 perspective;
				glm::decompose(t, scl, ori, pos, skew, perspective);

				scl.x = px->m_Var->GetFloat();
				scl.y = py->m_Var->GetFloat();
				scl.z = pz->m_Var->GetFloat();

				t = glm::scale(glm::identity<glm::fmat4x4>(), scl) * (glm::fmat4x4)(ori);

				// Then translate last... 
				t = glm::translate(glm::identity<glm::fmat4x4>(), pos) * t;

				pmr->GetModelInstanceData()->SetTransform(ni, t);
			}
		}
	}
}


// m_JS->AddNative(_T("function GetModelNodeName(hobj, nodeidx)"), jcGetModelNodeName, psys);
void jcGetModelNodeName(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	Model::NodeIndex ni = c->GetParameter(_T("nodeidx"))->GetInt();

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(pobj->FindComponent(ModelRenderer::Type()));
		const Model *pm = pmr->GetModel();

		if (pm)
		{
			const TCHAR *s = pm->GetNodeName(ni);
			if (s)
			{
				ret->SetString(s);
				return;
			}
		}
	}

	ret->SetUndefined();
}


// m_JS->AddNative(_T("function SetModelNodeVisibility(hobj, nodeidx, b)"), jcSetModelNodeVisibility, psys);
void jcSetModelNodeVisibility(CScriptVar *c, void *userdata)
{
	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	Model::NodeIndex ni = c->GetParameter(_T("nodeidx"))->GetInt();
	bool b = c->GetParameter(_T("b"))->GetBool();

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(pobj->FindComponent(ModelRenderer::Type()));
		Model *pm = (Model *)pmr->GetModel();

		if (pm)
			pm->NodeVisibility(ni, std::make_optional<bool>(b));
	}
}


// m_JS->AddNative(_T("function SetModelNodeCollisions(hobj, nodeidx, b)"), jcSetModelNodeCollisions, psys);
void jcSetModelNodeCollisions(CScriptVar *c, void *userdata)
{
	int64_t hobj = c->GetParameter(_T("hobj"))->GetInt();
	Model::NodeIndex ni = c->GetParameter(_T("nodeidx"))->GetInt();
	bool b = c->GetParameter(_T("b"))->GetBool();

	Object *pobj = dynamic_cast<Object *>((Object *)hobj);
	if (pobj)
	{
		ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(pobj->FindComponent(ModelRenderer::Type()));
		Model *pm = (Model *)pmr->GetModel();

		if (pm)
			pm->NodeCollidability(ni, std::make_optional<bool>(b));
	}
}


// m_JS->AddNative(_T("function SetModelNodeCollisions()"), jcCreateCollisionResults, psys);
void jcCreateCollisionResults(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	CScriptVarLink *pf;
	pf = ret->FindChildOrCreate(_T("distance"));
	pf->m_Owned = true;
	CScriptVar *pdist = pf->m_Var;
	pdist->SetFloat(0.0f);

	pf = ret->FindChildOrCreate(_T("hobj"));
	pf->m_Owned = true;
	CScriptVar *phobj = pf->m_Var;
	phobj->SetInt(0);

	pf = ret->FindChildOrCreate(_T("found"));
	pf->m_Owned = true;
	CScriptVar *pfound = pf->m_Var;
	pfound->SetInt(0);

	pf = ret->FindChildOrCreate(_T("normal"));
	pf->m_Owned = true;
	CScriptVar *pnorm = pf->m_Var;
	CScriptVarLink *px = pnorm->FindChildOrCreate(_T("x"));
	CScriptVarLink *py = pnorm->FindChildOrCreate(_T("y"));
	CScriptVarLink *pz = pnorm->FindChildOrCreate(_T("z"));
	px->m_Var->SetFloat(0.0f);
	py->m_Var->SetFloat(0.0f);
	pz->m_Var->SetFloat(1.0f);
}


// m_JS->AddNative(_T("function FreeCollisionResults(collision_results)"), jcFreeCollisionResults, psys);
void jcFreeCollisionResults(CScriptVar *c, void *userdata)
{
	CScriptVar *pres = c->GetParameter(_T("collision_results"));
}


// m_JS->AddNative(_T("function CheckCollisions(hrootobj, raypos, raydir, results)"), jcCheckCollisions, psys);
void jcCheckCollisions(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	CScriptVar *phrootobj = c->GetParameter(_T("hrootobj"));
	CScriptVar *praypos = c->GetParameter(_T("raypos"));
	CScriptVar *praydir = c->GetParameter(_T("raydir"));
	CScriptVar *ret = c->GetParameter(_T("results"));
	if (c->GetReturnVar())
		c->GetReturnVar()->SetInt(0);

	if (!(phrootobj && praypos && praydir))
		return;

	CScriptVarLink *ret_found = ret->FindChild(_T("found"));
	if (!ret_found)
		return;

	glm::fvec3 raypos, raydir;

	{
		CScriptVarLink *px = praydir->FindChild(_T("x"));
		CScriptVarLink *py = praydir->FindChild(_T("y"));
		CScriptVarLink *pz = praydir->FindChild(_T("z"));
		if (!(px && py && pz))
			return;

		raydir.x = px->m_Var->GetFloat();
		raydir.y = py->m_Var->GetFloat();
		raydir.z = pz->m_Var->GetFloat();
	}

	{
		CScriptVarLink *px = praypos->FindChild(_T("x"));
		CScriptVarLink *py = praypos->FindChild(_T("y"));
		CScriptVarLink *pz = praypos->FindChild(_T("z"));
		if (!(px && py && pz))
			return;

		raypos.x = px->m_Var->GetFloat();
		raypos.y = py->m_Var->GetFloat();
		raypos.z = pz->m_Var->GetFloat();
		if (glm::length(raydir) < 0.0001)
		{
			ret_found->m_Var->SetInt(0);
			return;
		}
	}

	CScriptVarLink *ret_dist = ret->FindChild(_T("distance"));
	CScriptVarLink *ret_hobj = ret->FindChild(_T("hobj"));
	CScriptVarLink *ret_norm = ret->FindChild(_T("normal"));
	if (!(ret_dist && ret_hobj))
	{
		static bool errshown = false;
		if (!errshown)
		{
			psys->GetLog()->Print(_T("JS ERROR: you must call CreateCollisionResults before calling CheckCollisions!\n"));
			errshown = true;
		}

		return;
	}

	ret_found->m_Var->SetInt(0);

	int64_t hrootobj = c->GetParameter(_T("hrootobj"))->GetInt();
	Object *prootobj = dynamic_cast<Object *>((Object *)hrootobj);

	if (prootobj)
	{
		Object *obj = nullptr;
		float dist = FLT_MAX;
		glm::fvec3 norm(0, 0, 0);
		float len = glm::length(raydir);
		raydir = glm::normalize(raydir);
		if (prootobj->Intersect(&raypos, &raydir, nullptr, &dist, &norm, &obj, OF_CHECKCOLLISIONS, -1) && (dist < len))
		{
			ret_found->m_Var->SetInt(1);
			ret_hobj->m_Var->SetInt((int64_t)obj);
			ret_dist->m_Var->SetFloat(dist);

			CScriptVarLink *px = ret_norm->m_Var->FindChild(_T("x"));
			CScriptVarLink *py = ret_norm->m_Var->FindChild(_T("y"));
			CScriptVarLink *pz = ret_norm->m_Var->FindChild(_T("z"));

			px->m_Var->SetFloat(norm.x);
			py->m_Var->SetFloat(norm.y);
			pz->m_Var->SetFloat(norm.z);

			if (c->GetReturnVar())
				c->GetReturnVar()->SetInt(1);
		}
	}
}


// m_JS->AddNative(_T("function HandleFPSMovement(hrootobj, pos, user_vel, user_acc, elapsed_time, low_height, high_height)"), jcHandleFPSMovement, psys);
void jcHandleFPSMovement(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	CScriptVar *phrootobj = c->GetParameter(_T("hrootobj"));
	CScriptVar *ppos      = c->GetParameter(_T("pos"));
	CScriptVar *puservel  = c->GetParameter(_T("user_vel"));
	CScriptVar *puseracc  = c->GetParameter(_T("user_acc"));
	CScriptVar *peltime   = c->GetParameter(_T("elapsed_time"));
	CScriptVar *plowh     = c->GetParameter(_T("low_height"));
	CScriptVar *phighh    = c->GetParameter(_T("high_height"));

	if (c->GetReturnVar())
		c->GetReturnVar()->SetInt(0);
	if (!(phrootobj && ppos && puservel && puseracc && peltime && plowh && phighh))
		return;

	const float dt = peltime->GetFloat();
	if (dt == 0.0f)
		return;

	int64_t hrootobj = c->GetParameter(_T("hrootobj"))->GetInt();
	Object *prootobj = dynamic_cast<Object *>((Object *)hrootobj);
	if (!prootobj)
		return;

	// --- Fetch pos/vel/acc from script ---
	glm::fvec3 pos, vel, acc;
	{
		CScriptVarLink *pposx = ppos->FindChild(_T("x"));
		CScriptVarLink *pposy = ppos->FindChild(_T("y"));
		CScriptVarLink *pposz = ppos->FindChild(_T("z"));
		if (!(pposx && pposy && pposz))
			return;

		pos = { pposx->m_Var->GetFloat(), pposy->m_Var->GetFloat(), pposz->m_Var->GetFloat() };

		CScriptVarLink *pvelx = puservel->FindChild(_T("x"));
		CScriptVarLink *pvely = puservel->FindChild(_T("y"));
		CScriptVarLink *pvelz = puservel->FindChild(_T("z"));
		if (!(pvelx && pvely && pvelz))
			return;

		vel = { pvelx->m_Var->GetFloat(), pvely->m_Var->GetFloat(), pvelz->m_Var->GetFloat() };

		CScriptVarLink *paccx = puseracc->FindChild(_T("x"));
		CScriptVarLink *paccy = puseracc->FindChild(_T("y"));
		CScriptVarLink *paccz = puseracc->FindChild(_T("z"));
		if (!(paccx && paccy && paccz))
			return;

		acc = { paccx->m_Var->GetFloat(), paccy->m_Var->GetFloat(), paccz->m_Var->GetFloat() };
	}

	// helpers
	auto safeNorm = [](const glm::fvec3 &v)
	{
		float L = glm::length(v);
		return (L > 0.0f) ? (v / L) : glm::fvec3(0);
	};

	auto proj = [](const glm::fvec3 &v, const glm::fvec3 &nUnit)
	{
		return glm::dot(v, nUnit) * nUnit; // nUnit must be normalized
	};

	auto reject = [&](const glm::fvec3 &v, const glm::fvec3 &nUnit)
	{
		return v - proj(v, nUnit);
	};

	// Tunables
	constexpr float kSkin     = 0.5f;   // small clearance from ground/walls
	constexpr float kSnapDist = 4.0f;   // snap-to-ground tolerance
	constexpr float kMinVVel  = 1e-3f;  // dead-zone for vertical velocity

	const float lowtest  = plowh->GetFloat();   // lower ray height (from your script)
	const float hightest = phighh->GetFloat();  // upper ray height (capsule-ish)

	// Gravity
	glm::fvec3 grav;
	psys->GetEnvironment()->GetGravity(&grav);
	glm::fvec3 gravdir = safeNorm(grav);
	if (gravdir == glm::fvec3(0))
		gravdir = glm::fvec3(0, 0, -1); // fallback

	// Integrate velocity
	vel += (acc + grav) * dt;

	// Proposed displacement this frame:
	glm::fvec3 move = vel * dt;

	// Horizontal pass (orthogonal to gravity) with slide/capsule-ish rays
	{
		glm::fvec3 moveH = reject(move, gravdir);
		float lenH = glm::length(moveH);
		if (lenH > 0.0f)
		{
			glm::fvec3 dirH = moveH / lenH;

			float dist = FLT_MAX;
			glm::fvec3 norm(0);
			bool horzHit = false;

			// Ray from "feet" height
			glm::fvec3 poslow  = pos - (gravdir * lowtest);
			horzHit = prootobj->Intersect(&poslow, &dirH, nullptr, &dist, &norm, nullptr,
				OF_CHECKCOLLISIONS, -1) && (dist < lenH);

			// Ray from "head/waist" height if feet ray didn't hit
			if (!horzHit) {
				dist = FLT_MAX;
				glm::fvec3 poshigh = pos - (gravdir * hightest);
				horzHit = prootobj->Intersect(&poshigh, &dirH, nullptr, &dist, &norm, nullptr,
					OF_CHECKCOLLISIONS, -1) && (dist < lenH);
			}

			if (horzHit)
			{
				// stop short of the hit by a tiny skin
				float allowed = std::max(0.0f, dist - kSkin);
				glm::fvec3 hitpos = pos + dirH * allowed;

				float remaining = std::max(0.0f, lenH - allowed);

				// slide direction: horizontalize normal and slide along it
				glm::fvec3 norm_no_g = safeNorm(reject(norm, gravdir));
				glm::fvec3 slide_dir = safeNorm(reject(dirH, norm_no_g));

				pos = hitpos + slide_dir * remaining;

				// after sliding, zero horizontal velocity (keep only vertical)
				vel = proj(vel, gravdir);
			}
			else
			{
				pos += moveH; // no hit: apply full horizontal displacement
			}
		}
	}

	// Vertical resolve
	bool grounded = false;
	{
		float dist = FLT_MAX;
		glm::fvec3 norm(0);

		if (prootobj->Intersect(&pos, &gravdir, nullptr, &dist, &norm, nullptr, OF_CHECKCOLLISIONS, -1))
		{
			// distance along gravity from current pos to ground
			// consider close-enough to ground if within clearance + snap
			if (dist <= hightest + kSnapDist)
			{
				grounded = true;

				// current vertical velocity sign (>0 = down; <0 = up/jumping)
				float vdotg = glm::dot(vel, gravdir);

				// Only perform push/snap when not moving upward (avoid killing jump)
				if (vdotg >= -kMinVVel)
				{
					// push up if too close (maintain clearance minus skin)
					if (dist < hightest - kSkin)
					{
						pos -= gravdir * (hightest - kSkin - dist);
					}
					// snap down gently if slightly above target clearance
					else if (dist > hightest + kSkin && dist <= hightest + kSnapDist)
					{
						pos += gravdir * (dist - (hightest + kSkin)); // gravdir points "down"
					}
				}

				// Kill downward velocity only (leave upward jump intact)
				if (vdotg > kMinVVel)
				{
					vel -= gravdir * vdotg;
				}
				// If we’re clearly moving up, consider airborne for this frame
				else if (vdotg < -kMinVVel)
				{
					grounded = false;
				}

				if (c->GetReturnVar())
					c->GetReturnVar()->SetInt(grounded ? 1 : 0);
			}
			else
			{
				if (c->GetReturnVar())
					c->GetReturnVar()->SetInt(0);
			}
		}
		else
		{
			if (c->GetReturnVar())
				c->GetReturnVar()->SetInt(0);
		}
	}

	// If not grounded, apply vertical displacement from this frame
	if (!grounded)
	{
		glm::fvec3 moveV = proj(move, gravdir);
		pos += moveV;
	}

	// Write back to script vars
	{
		CScriptVarLink *pposx = ppos->FindChild(_T("x"));
		CScriptVarLink *pposy = ppos->FindChild(_T("y"));
		CScriptVarLink *pposz = ppos->FindChild(_T("z"));

		CScriptVarLink *pvelx = puservel->FindChild(_T("x"));
		CScriptVarLink *pvely = puservel->FindChild(_T("y"));
		CScriptVarLink *pvelz = puservel->FindChild(_T("z"));

		pposx->m_Var->SetFloat(pos.x);
		pposy->m_Var->SetFloat(pos.y);
		pposz->m_Var->SetFloat(pos.z);

		pvelx->m_Var->SetFloat(vel.x);
		pvely->m_Var->SetFloat(vel.y);
		pvelz->m_Var->SetFloat(vel.z);
	}
}


void jcGetGravity(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	System *psys = (System *)userdata;
	assert(psys);

	CScriptVarLink *psvl;

	psvl = ret->FindChildOrCreate(_T("x"));
	psvl->m_Owned = true;
	CScriptVar *prx = psvl->m_Var;

	psvl = ret->FindChildOrCreate(_T("y"));
	psvl->m_Owned = true;
	CScriptVar *pry = psvl->m_Var;

	psvl = ret->FindChildOrCreate(_T("z"));
	psvl->m_Owned = true;
	CScriptVar *prz = psvl->m_Var;

	glm::fvec3 grav;
	psys->GetEnvironment()->GetGravity(&grav);

	prx->SetFloat(grav.x);
	pry->SetFloat(grav.y);
	prz->SetFloat(grav.z);
}


static const TCHAR *elnames[4] ={_T("r"), _T("g"), _T("b"), _T("a")};

//PackColorFromIntVec(coloriv) - Takes a color vector of ints in the range of [0..255] (3 or 4 channel) and packs it into an int
void jcPackColorFromIntVec(CScriptVar *c, void *userdata)
{
	CScriptVar *pc = c->GetParameter(_T("coloriv"));
	CScriptVar *pr = c->GetReturnVar();
	size_t elct = pc->GetNumChildren();

	if ((elct < 1) || (elct > 4) || !pr || !pc)
		return;

	uint32_t r = 0;

	for (size_t i = 0; i < elct; i++)
	{
		CScriptVarLink *pccomp = pc->GetChild(i);

		uint32_t cc = std::min<int>(std::max<int>(0, (int)(pccomp->m_Var->GetInt())), 255);
		cc <<= (i * 8);
		r |= cc;
	}

	pr->SetInt(r);
}


//PackColorFromFloatVec(colorfv) - Takes a color vector of floats in the range of [0..1] (3 or 4 channel) and packs it into an int
void jcPackColorFromFloatVec(CScriptVar *c, void *userdata)
{
	CScriptVar *pc = c->GetParameter(_T("colorfv"));
	CScriptVar *pr = c->GetReturnVar();
	size_t elct = pc->GetNumChildren();

	if ((elct < 1) || (elct > 4) || !pr || !pc)
		return;

	uint32_t r = 0;

	for (size_t i = 0; i < elct; i++)
	{
		CScriptVarLink *pccomp = pc->GetChild(i);

		uint32_t cc = (uint32_t)(std::min<float>(std::max<float>(0.0f, pccomp->m_Var->GetFloat()), 1.0f) * 255.0f);
		cc <<= (i * 8);
		r |= cc;
	}

	pr->SetInt(r);
}


//UnpackColorToIntVec(color) - Takes a packed color and unpacks it into an int vector in the range of [0..255] (3 or 4 channel)
void jcUnpackColorToIntVec(CScriptVar *c, void *userdata)
{
	CScriptVar *pc = c->GetParameter(_T("color"));
	CScriptVar *pr = c->GetReturnVar();

	if (!pr || !pc)
		return;

	size_t elct = pr->GetNumChildren();
	if (!elct)
		elct = 4;

	uint32_t r = (uint32_t)pc->GetInt();

	for (size_t i = 0; i < elct; i++)
	{
		CScriptVarLink *prcomp = pr->FindChildOrCreate(elnames[i]);
		prcomp->m_Var->SetInt(r & 0xff);
		r >>= 8;
	}
}


//UnpackColorToFloatVec(color) - Takes a packed color and unpacks it into a float vector in the range of [0..1] (3 or 4 channel)
void jcUnpackColorToFloatVec(CScriptVar *c, void *userdata)
{
	CScriptVar *pc = c->GetParameter(_T("color"));
	CScriptVar *pr = c->GetReturnVar();

	if (!pr || !pc)
		return;

	size_t elct = pr->GetNumChildren();
	if (!elct)
		elct = 4;

	uint32_t r = (uint32_t)pc->GetInt();

	for (size_t i = 0; i < elct; i++)
	{
		CScriptVarLink *prcomp = pr->FindChildOrCreate(elnames[i]);
		prcomp->m_Var->SetFloat((float)(r & 0xff) / 255.0f);
		r >>= 8;
	}
}


//m_JS->AddNative(_T("function LoadPackfile(filename)"),				jcLoadPackfile, psys);
void jcLoadPackfile(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	bool success = false;

	System *psys = (System *)userdata;
	assert(psys);

	tstring filename = c->GetParameter(_T("filename"))->GetString();
	TCHAR fullpath[MAX_PATH];

	if (psys->GetFileMapper()->FindFile(filename.c_str(), fullpath, MAX_PATH))
	{
		if (psys->GetResourceManager()->RegisterZipArchive(filename.c_str()))
		{
			success = true;
		}
	}

	if (ret)
		ret->SetInt(success ? 1 : 0);
}


//m_JS->AddNative(_T("function UnloadPackfile(filename)"),				jcUnloadPackfile, psys);
void jcUnloadPackfile(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	tstring filename = c->GetParameter(_T("filename"))->GetString();

	psys->GetResourceManager()->UnregisterZipArchive(filename.c_str());
}


//m_JS->AddNative(_T("function LoadPlugin(filename)"),				jcLoadPlugin, psys);
void jcLoadPlugin(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	bool success = false;

	System *psys = (System *)userdata;
	assert(psys);

	tstring filename = c->GetParameter(_T("filename"))->GetString();

	PluginManager *plugman = psys->GetPluginManager();

	if (!plugman->PluginIsLoaded(filename.c_str()))
	{
		success = (plugman->LoadPlugin(filename.c_str(), true) == c3::PluginManager::RET_OK);
	}

	if (ret)
		ret->SetInt(success ? 1 : 0);
}


//m_JS->AddNative(_T("function UnloadPlugin(filename)"),				jcUnloadPlugin, psys);
void jcUnloadPlugin(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	tstring filename = c->GetParameter(_T("filename"))->GetString();

	PluginManager *plugman = psys->GetPluginManager();

	size_t idx;
	if (plugman->PluginIsLoaded(filename.c_str(), &idx))
	{
		plugman->UnloadPlugin(plugman->GetPlugin(idx));
	}
}


//m_JS->AddNative(_T("function MessageBox(caption, message, modestr)"),				jcUnloadPlugin, psys);
void jcMessageBox(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();

	System *psys = (System *)userdata;
	assert(psys);

	tstring caption = c->GetParameter(_T("caption"))->GetString();
	tstring message = c->GetParameter(_T("message"))->GetString();
	tstring modestr = c->GetParameter(_T("modestr"))->GetString();

	PluginManager *plugman = psys->GetPluginManager();

	if (!_tcsicmp(modestr.c_str(), _T("ok")))
	{
		::MessageBox(NULL, message.c_str(), caption.c_str(), MB_OK);
		ret->SetInt(1);
	}
	else if (!_tcsicmp(modestr.c_str(), _T("okcancel")))
	{
		ret->SetInt((::MessageBox(NULL, message.c_str(), caption.c_str(), MB_OKCANCEL) == IDOK) ? 1 : 0);
	}
	if (!_tcsicmp(modestr.c_str(), _T("yesno")))
	{
		ret->SetInt((::MessageBox(NULL, message.c_str(), caption.c_str(), MB_YESNO) == IDYES) ? 1 : 0);
	}
}


//m_JS->AddNative(_T("function RegisterResourcePath(ext, path)"),				jcRegisterResourcePath, psys);
void jcRegisterResourcePath(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	tstring ext = c->GetParameter(_T("ext"))->GetString();
	tstring path = c->GetParameter(_T("path"))->GetString();

	FileMapper *pfm = psys->GetFileMapper();
	pfm->AddMapping(ext.c_str(), path.c_str());
}



void jcRegisterInputAction(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();

	System *psys = (System *)userdata;
	assert(psys);

	tstring action_name = c->GetParameter(_T("action_name"))->GetString();
	tstring press_mode = c->GetParameter(_T("press_mode"))->GetString();
	float delay = c->GetParameter(_T("delay"))->GetFloat();

	// add more trigger types later? For specificity? But "" is continuous down, "down" is delta down, "up" delta up
	ActionMapper::ETriggerType tt = ActionMapper::ETriggerType::DOWN_CONTINUOUS;
	if (!_tcsicmp(press_mode.c_str(), _T("up")))
	{
		tt = ActionMapper::UP_DELTA;
	}
	else if (!_tcsicmp(press_mode.c_str(), _T("down")))
	{
		tt = ActionMapper::DOWN_DELTA;
	}
	else if (!_tcsicmp(press_mode.c_str(), _T("delta")))
	{
		tt = ActionMapper::ANY_DELTA;
	}

	c3::ActionMapper *pam = psys->GetActionMapper();

	size_t ai = pam->RegisterAction(action_name.c_str(), tt, delay, [](const TCHAR *action_name, c3::InputDevice *from_device, size_t user, c3::InputDevice::VirtualButton button, float value, void *userdata)
	{
		System *psys = from_device->GetSystem();
		assert(psys);

		c3::Object *inputobj = psys->GetGlobalObjectRegistry()->GetRegisteredObject(c3::GlobalObjectRegistry::OD_PLAYER);

		if (!inputobj)
			inputobj = psys->GetGlobalObjectRegistry()->GetRegisteredObject(c3::GlobalObjectRegistry::OD_WORLDROOT);

		if (inputobj)
		{
			c3::Scriptable *pscr = dynamic_cast<c3::Scriptable *>(inputobj->FindComponent(c3::Scriptable::Type()));
			if (pscr)
			{
				if (pscr->FunctionExists(_T("handle_input")))
					pscr->Execute(_T("handle_input(\"%s\", %0.5f);"), action_name, value);
				return true;
			}
		}
		return false;
	}, nullptr);

	ret->SetInt(ai);
}


void jcUnregisterInputAction(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	c3::ActionMapper *pam = psys->GetActionMapper();

	CScriptVar *pact = c->GetParameter(_T("action_name"));
	size_t actidx;
	if (pact->IsString())
		actidx = pam->FindActionIndex(pact->GetString());
	else
		actidx = pact->GetInt();

	if (actidx != -1)
		pam->UnregisterAction(actidx);
}


void jcFindInputAction(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();

	System *psys = (System *)userdata;
	assert(psys);

	tstring action_name = c->GetParameter(_T("action_name"))->GetString();

	c3::ActionMapper *pam = psys->GetActionMapper();

	size_t idx = pam->FindActionIndex(action_name.c_str());

	ret->SetInt(idx);
}


void jcLinkInputToAction(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();

	System *psys = (System *)userdata;
	assert(psys);

	c3::ActionMapper *pam = psys->GetActionMapper();
	c3::InputManager *pim = psys->GetInputManager();

	CScriptVar *pact = c->GetParameter(_T("action_name"));
	size_t actidx;
	if (pact->IsString())
		actidx = pam->FindActionIndex(pact->GetString());
	else
		actidx = pact->GetInt();

	CScriptVar *pdev = c->GetParameter(_T("device"));
	size_t devidx;
	if (pdev->IsString())
		pim->FindDevice(pdev->GetString(), devidx);
	else
		devidx = pdev->GetInt();
	InputDevice *pid = pim->GetDevice(devidx);

	if (pid && (actidx != -1))
	{
		CScriptVar *pinp = c->GetParameter(_T("input_name"));
		InputDevice::VirtualButton vb;
		if (InputDevice::GetButtonCodeByName(pinp->GetString(), vb))
		{
			pam->MakeAssociation(actidx, pid->GetUID(), vb);
		}
	}

	ret->SetInt((actidx == -1) ? 0 : 1);
}


void jcSetMousePos(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	CScriptVar *ppos = c->GetParameter(_T("pos"));

	CScriptVarLink *pposx = ppos->FindChild(_T("x"));
	CScriptVarLink *pposy = ppos->FindChild(_T("y"));
	if (!(pposx && pposy))
		return;

	pim->SetMousePos((int32_t)(pposx->m_Var->GetInt()), (int32_t)(pposy->m_Var->GetInt()));
}


void jcGetMousePos(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();

	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	CScriptVarLink *pposx = ret->FindChild(_T("x"));
	CScriptVarLink *pposy = ret->FindChild(_T("y"));
	if (!(pposx && pposy))
		return;

	glm::ivec2 p;
	pim->GetMousePos(p.x, p.y);

	pposx->m_Var->SetInt(p.x);
	pposy->m_Var->SetInt(p.y);
}


void jcEnableMouse(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	CScriptVar *pen = c->GetParameter(_T("enabled"));
	if (!pen)
		return;

	pim->EnableMouse((pen->GetInt() == 0) ? false : true);
}


void jcMouseEnabled(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();

	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	ret->SetInt(pim->MouseEnabled() ? 1 : 0);
}


void jcEnableSystemMouse(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	CScriptVar *pen = c->GetParameter(_T("enabled"));
	if (!pen)
		return;

	ShowCursor((pen->GetInt() == 0) ? FALSE : TRUE);
}


void jcSystemMouseEnabled(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();

	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	CURSORINFO cinfo;
	cinfo.cbSize = sizeof(CURSORINFO);
	GetCursorInfo(&cinfo);

	ret->SetInt(!cinfo.flags ? 0 : 1);
}


void jcCaptureMouse(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	CScriptVar *pcap = c->GetParameter(_T("capture"));
	if (!pcap)
		return;

	pim->CaptureMouse((pcap->GetInt() == 0) ? false : true);
}


void jcMouseCaptured(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();

	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	ret->SetInt(pim->MouseCaptured() ? 1 : 0);
}


void jcRegisterCursor(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();

	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	CScriptVar *pfilename = c->GetParameter(_T("filename"));
	CScriptVar *photspot = c->GetParameter(_T("hotspot"));
	CScriptVar *pname = c->GetParameter(_T("cursorname"));
	if (!(pfilename && photspot && pname))
		return;

	CScriptVarLink *pposx = photspot->FindChild(_T("x"));
	CScriptVarLink *pposy = photspot->FindChild(_T("y"));
	if (!(pposx && pposy))
		return;

	InputManager::CursorID cid = pim->RegisterCursor(pfilename->GetString(),
		glm::ivec2(pposx->m_Var->GetInt(), pposy->m_Var->GetInt()), pname->GetString());

	if (ret)
		ret->SetInt(cid);
}


void jcUnregisterCursor(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	CScriptVar *pid = c->GetParameter(_T("cursorid"));
	if (!pid)
		return;

	pim->UnregisterCursor((InputManager::CursorID)(pid->GetInt()));
}


void jcGetNumCursors(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();
	ret->SetInt(pim->GetNumCursors());
}


void jcGetCursorName(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	CScriptVar *pid = c->GetParameter(_T("cursorid"));
	if (!pid)
		return;

	const TCHAR *pname = pim->GetCursorName((InputManager::CursorID)(pid->GetInt()));
	ret->SetString(pname ? pname : _T(""));
}


void jcSetCursor(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	CScriptVar *pid = c->GetParameter(_T("cursorid"));
	if (!pid)
		return;

	pim->SetCursor((InputManager::CursorID)(pid->GetInt()));
}


void jcGetCursor(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	ret->SetInt(pim->GetCursor());
}


void jcSetCursorTransform(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	c3::InputManager *pim = psys->GetInputManager();

	CScriptVar *ppos = c->GetParameter(_T("pos"));
	CScriptVar *prot = c->GetParameter(_T("rot"));
	CScriptVar *pscl = c->GetParameter(_T("scl"));
	if (!(ppos && prot && pscl))
		return;

	glm::fvec2 pos;
	CScriptVarLink *pposx = ppos->FindChild(_T("x"));
	CScriptVarLink *pposy = ppos->FindChild(_T("y"));
	if (pposx && pposy)
	{
		pos.x = pposx->m_Var->GetFloat();
		pos.y = pposy->m_Var->GetFloat();
	}
	else
	{
		pos.x = pos.y = ppos->GetFloat();
	}

	float rot = prot->GetFloat();

	glm::fvec2 scl;
	CScriptVarLink *psclx = pscl->FindChild(_T("x"));
	CScriptVarLink *pscly = pscl->FindChild(_T("y"));
	if (psclx && pscly)
	{
		scl.x = psclx->m_Var->GetFloat();
		scl.y = pscly->m_Var->GetFloat();
	}
	else
	{
		scl.x = scl.y = pscl->GetFloat();
	}

	glm::fmat4x4 mat = glm::translate(glm::fvec3(pos, 1.0f)) * (glm::rotate(glm::radians(rot), glm::fvec3(0, 0, 1.0f)) * glm::scale(glm::fvec3(scl, 1.0f)));
	pim->SetCursorTransform(mat);
}


void jcPauseGame(CScriptVar *c, void *userdata)
{
	System *psys = (System *)userdata;
	assert(psys);

	CScriptVar *pp = c->GetParameter(_T("paused"));
	if (!pp)
		return;
}


void jcGamePaused(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	System *psys = (System *)userdata;
	assert(psys);

	ret->SetInt(0);
}
