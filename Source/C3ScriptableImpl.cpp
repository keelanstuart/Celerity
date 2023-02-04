// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3ScriptableImpl.h>
#include <PowerProps.h>
#include <TinyJS_MathFunctions.h>
#include <TinyJS_Functions.h>

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

		if (sz)
		{
			TCHAR *code = (TCHAR *)calloc(sz + 1, sizeof(TCHAR));

			fread(code, sizeof(TCHAR), sz, f);
			fclose(f);

			Script *ps = new Script;
			ps->m_Code = code;
			*returned_data = ps;
			free(code);
		}
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(Script)::ReadFromMemory(c3::System *psys, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
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
void jcFindProperty(CScriptVar *c, void *userdata);
void jcGetPropertyValue(CScriptVar *c, void *userdata);
void jcSetPropertyValue(CScriptVar *c, void *userdata);

bool ScriptableImpl::Initialize(Object *pobject)
{
	if (!pobject)
		return false;

	registerFunctions(&m_JS);
	registerMathFunctions(&m_JS);

	m_JS.addNative(_T("function GetNumChildren(hrootobj)"),							jcGetNumChildren, m_pSys);
	m_JS.addNative(_T("function GetChild(hrootobj, idx)"),							jcGetChild, m_pSys);
	m_JS.addNative(_T("function FindObjByGUID(hrootobj, guid, recursive)"),			jcFindObjByGUID, m_pSys);
	m_JS.addNative(_T("function FindFirstObjByName(hrootobj, name, recursive)"),	jcFindFirstObjByName, m_pSys);
	m_JS.addNative(_T("function Log(text)"),										jcLog, m_pSys);
	m_JS.addNative(_T("function FindProperty(hobj, name)"),							jcFindProperty, m_pSys);
	m_JS.addNative(_T("function GetPropertyValue(hprop)"),							jcGetPropertyValue, m_pSys);
	m_JS.addNative(_T("function SetPropertyValue(hprop, val)"),						jcSetPropertyValue, m_pSys);

	TCHAR make_self_cmd[64];
	_stprintf_s(make_self_cmd, _T("var self = %lld;"), (int64_t)pobject);
	m_JS.execute(make_self_cmd);

	m_pSys = pobject->GetSystem();

	props::IPropertySet *propset = pobject->GetProperties();
	if (!propset)
		return false;

	props::IProperty *pscl = propset->CreateReferenceProperty(_T("ScriptUpdateRate"), 'SUDR', &m_UpdateRate, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pscl)
		pscl->SetFloat(1.0f);

	props::IProperty *psrc = propset->GetPropertyById('SRCF');
	if (!psrc)
	{
		psrc = propset->CreateProperty(_T("SourceFile"), 'SRCF');
		psrc->SetString(_T("my_script.c3js"));
		psrc->SetAspect(props::IProperty::PA_FILENAME);
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

		TCHAR update_cmd[64];
		_stprintf_s(update_cmd, _T("update(%0.3f);"), elapsed_time);
		m_JS.execute(update_cmd);
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
		{
			Resource *pres = m_pSys->GetResourceManager()->GetResource(pprop->AsString(), RESF_DEMANDLOAD);
			if (pres && (pres->GetStatus() == Resource::RS_LOADED))
			{
				m_Code = ((Script *)(pres->GetData()))->m_Code;
				m_JS.execute(m_Code);
			}
			break;
		}

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


void jcFindProperty(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->getReturnVar();
	if (!ret)
		return;

	Object *pobj = (Object *)(c->getParameter(_T("hobj"))->getInt());
	tstring name = c->getParameter(_T("name"))->getString();

	ret->setInt((int64_t)(pobj->GetProperties()->GetPropertyByName(name.c_str())));
}


void jcGetPropertyValue(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->getReturnVar();
	if (!ret)
		return;

	ret->removeAllChildren();
	ret->setUndefined();

	props::IProperty *pprop = dynamic_cast<props::IProperty *>((props::IProperty *)(c->getParameter(_T("hprop"))->getInt()));
	if (!pprop)
		return;

	CScriptVar *px = nullptr, *py = nullptr, *pz = nullptr, *pw = nullptr;

	switch (pprop->GetType())
	{
		case props::IProperty::PROPERTY_TYPE::PT_BOOLEAN:
		case props::IProperty::PROPERTY_TYPE::PT_INT:
		case props::IProperty::PROPERTY_TYPE::PT_ENUM:
			ret->setInt(pprop->AsInt());
			break;

		case props::IProperty::PROPERTY_TYPE::PT_FLOAT:
			ret->setDouble(pprop->AsFloat());
			break;

		case props::IProperty::PROPERTY_TYPE::PT_STRING:
		case props::IProperty::PROPERTY_TYPE::PT_GUID:
			ret->setString(pprop->AsString());
			break;

		case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4:
			pw = (ret->findChildOrCreate(_T("w")))->var;
			pw->setDouble(pprop->AsVec4F()->w);
		case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3:
			pz = (ret->findChildOrCreate(_T("z")))->var;
			pz->setDouble(pprop->AsVec3F()->z);
		case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2:
			py = (ret->findChildOrCreate(_T("y")))->var;
			py->setDouble(pprop->AsVec2F()->y);
			px = (ret->findChildOrCreate(_T("x")))->var;
			px->setDouble(pprop->AsVec2F()->x);
			break;

		case props::IProperty::PROPERTY_TYPE::PT_INT_V4:
			pw = (ret->findChildOrCreate(_T("w")))->var;
			pw->setInt(pprop->AsVec4I()->w);
		case props::IProperty::PROPERTY_TYPE::PT_INT_V3:
			pz = (ret->findChildOrCreate(_T("z")))->var;
			pz->setInt(pprop->AsVec3I()->z);
		case props::IProperty::PROPERTY_TYPE::PT_INT_V2:
			py = (ret->findChildOrCreate(_T("y")))->var;
			py->setInt(pprop->AsVec2I()->y);
			px = (ret->findChildOrCreate(_T("x")))->var;
			px->setInt(pprop->AsVec2I()->x);
			break;
	}
}


void jcSetPropertyValue(CScriptVar *c, void *userdata)
{
	CScriptVar *ret = c->getReturnVar();
	if (!ret)
		return;

	props::IProperty *pprop = (props::IProperty *)(c->getParameter(_T("hprop"))->getInt());
	if (!pprop)
		return;

	CScriptVar *pval = c->getParameter(_T("val"));
	if (!pprop)
		return;

	if (pval->isDouble())
	{
		pprop->SetFloat((float)pval->getDouble());
	}
	else if (pval->isInt())
	{
		switch (pprop->GetType())
		{
			case props::IProperty::PROPERTY_TYPE::PT_BOOLEAN:
				pprop->SetBool(pval->getInt() ? true : false);
				break;

			case props::IProperty::PROPERTY_TYPE::PT_ENUM:
				pprop->SetEnumVal(pval->getInt());
				break;

			case props::IProperty::PROPERTY_TYPE::PT_FLOAT:
				pprop->SetFloat((float)pval->getInt());
				break;

			default:
				pprop->SetInt(pval->getInt());
				break;
		}
	}
	else if (pval->isDouble())
	{
		pprop->SetFloat((float)pval->getDouble());
	}
	else if (pval->isString())
	{
		pprop->SetString(pval->getString().c_str());
	}
	else if (pval->getChildren())
	{
		CScriptVarLink *px = pval->findChild(_T("x")),
			*py = pval->findChild(_T("y")),
			*pz = pval->findChild(_T("z")),
			*pw = pval->findChild(_T("w"));

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
							px ? (float)px->var->getDouble() : 0,
							py ? (float)py->var->getDouble() : 0));
						break;
					}
					case 3:
					{
						pprop->SetVec3F(props::SVec3<float>(
							px ? (float)px->var->getDouble() : 0,
							py ? (float)py->var->getDouble() : 0,
							pz ? (float)pz->var->getDouble() : 0));
						break;
					}
					case 4:
					{
						pprop->SetVec4F(props::SVec4<float>(
							px ? (float)px->var->getDouble() : 0,
							py ? (float)py->var->getDouble() : 0,
							pz ? (float)pz->var->getDouble() : 0,
							pw ? (float)pw->var->getDouble() : 0));
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
							px ? px->var->getInt() : 0,
							py ? py->var->getInt() : 0));
						break;
					}
					case 3:
					{
						pprop->SetVec3I(props::SVec3<int64_t>(
							px ? px->var->getInt() : 0,
							py ? py->var->getInt() : 0,
							pz ? pz->var->getInt() : 0));
						break;
					}
					case 4:
					{
						pprop->SetVec4I(props::SVec4<int64_t>(
							px ? px->var->getInt() : 0,
							py ? py->var->getInt() : 0,
							pz ? pz->var->getInt() : 0,
							pw ? pw->var->getInt() : 0));
						break;
					}
				}
				break;
			}
		}
	}
}
