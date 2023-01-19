// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3ScriptableImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(Scriptable, ScriptableImpl);


ScriptableImpl::ScriptableImpl()
{

}


ScriptableImpl::~ScriptableImpl()
{

}


void ScriptableImpl::Release()
{

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


bool ScriptableImpl::Initialize(Object *pobject)
{
	if (!pobject)
		return false;

	m_JS.addNative(_T("GetNumChildren(hrootobj)"),							jcGetNumChildren, m_pSys);
	m_JS.addNative(_T("GetChild(hrootobj, idx)"),							jcGetChild, m_pSys);
	m_JS.addNative(_T("FindObjByGUID(hrootobj, guid, recursive)"),			jcFindObjByGUID, m_pSys);
	m_JS.addNative(_T("FindFirstObjByName(hrootobj, name, recursive)"),		jcFindFirstObjByName, m_pSys);
	m_JS.addNative(_T("Log(text)"),											jcLog, m_pSys);

	m_pSys = pobject->GetSystem();

	props::IPropertySet *propset = pobject->GetProperties();
	if (!propset)
		return false;

	props::IProperty *psrc = propset->CreateProperty(_T("SourceFile"), 'SRCF');
	if (psrc)
	{
		psrc->SetString(_T("my_script.js"));
		psrc->SetAspect(props::IProperty::PA_FILENAME);
	}

	props::IProperty *pscl = propset->CreateReferenceProperty(_T("ScriptUpdateRateS"), 'SUDR', &m_UpdateRate, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pscl)
	{
		pscl->SetFloat(1.0f);
	}

	return true;
}


void ScriptableImpl::Update(Object *pobject, float elapsed_time)
{
	m_UpdateTime -= elapsed_time;
	if (m_UpdateTime <= 0)
	{
		m_UpdateTime = m_UpdateRate + m_UpdateTime;
		if (m_UpdateTime < 0)
			m_UpdateTime = m_UpdateRate;

		// TODO: run update script
	}
}


bool ScriptableImpl::Prerender(Object *pobject, Object::RenderFlags flags)
{
	return true;
}


void ScriptableImpl::Render(Object *pobject, Object::RenderFlags flags)
{

}


void ScriptableImpl::PropertyChanged(const props::IProperty *pprop)
{
	if (!pprop)
		return;

	switch (pprop->GetID())
	{
		case 'SRCF':
			m_pSys->GetResourceManager()->GetResource(pprop->AsString());
			break;

		case 'SRC':
			m_Code = pprop->AsString();
			break;

		case 'SUDR':
			m_UpdateRate = pprop->AsFloat();
			m_UpdateTime = 0.0f;
			break;
	}
}


bool ScriptableImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance) const
{
	return false;
}


void jcGetNumChildren(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->getReturnVar();
	if (!ret)
		return;

	ret->setInt(0);

	int64_t hrootobj = c->getParameter(_T("hrootobj"))->getInt();

	Object *rootobj = dynamic_cast<Object *>((Object *)hrootobj);
	if (rootobj)
		ret->setInt(rootobj->GetNumChildren());
}


void jcGetChild(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->getReturnVar();
	if (!ret)
		return;

	ret->setInt(0);

	int64_t hrootobj = c->getParameter(_T("hrootobj"))->getInt();
	int64_t idx = c->getParameter(_T("idx"))->getInt();

	Object *rootobj = dynamic_cast<Object *>((Object *)hrootobj);
	if (rootobj)
		ret->setInt((int64_t)(rootobj->GetChild(idx)));
}


void jcFindObjByGUID(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->getReturnVar();
	if (!ret)
		return;

	ret->setInt(0);

	int64_t hrootobj = c->getParameter(_T("hrootobj"))->getInt();
	tstring guids = c->getParameter(_T("guid"))->getString();
	bool recurse = c->getParameter(_T("recursive"))->getBool();

	std::function<Object *(Object *, GUID, bool)> findObjByGuid = [&findObjByGuid] (Object *proot, GUID &sg, bool r) -> Object *
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
		ret->setInt((int64_t)pretobj);
	}
}


void jcFindFirstObjByName(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->getReturnVar();
	if (!ret)
		return;

	ret->setInt(0);

	int64_t hrootobj = c->getParameter(_T("hrootobj"))->getInt();
	tstring names = c->getParameter(_T("name"))->getString();
	bool recurse = c->getParameter(_T("recursive"))->getBool();

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
		ret->setInt((int64_t)pretobj);
	}
}


void jcLog(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->getReturnVar();
	if (!ret)
		return;

	tstring text = c->getParameter(_T("text"))->getString();
	System *psys = (System *)userdata;
	assert(psys);

	psys->GetLog()->Print(text.c_str());
}
