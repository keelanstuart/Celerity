// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3ScriptableImpl.h>
#include <PowerProps.h>
#include <TinyJS_MathFunctions.h>
#include <TinyJS_Functions.h>
#include <C3GlobalObjectRegistry.h>

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
	m_JS = nullptr;
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
void jcCreateObject(CScriptVar *c, void *userdata);
void jcDeleteObject(CScriptVar *c, void *userdata);
void jcGetParent(CScriptVar *c, void *userdata);
void jcSetParent(CScriptVar *c, void *userdata);
void jcSetObjectName(CScriptVar *c, void *userdata);
void jcLoadObject(CScriptVar *c, void *userdata);
void jcGetRegisteredObject(CScriptVar *c, void *userdata);
void jcRegisterObject(CScriptVar *c, void *userdata);
void jcAdjustQuaternion(CScriptVar *c, void *userdata);

void ScriptableImpl::ResetJS()
{
	if (m_JS)
		delete m_JS;
	m_JS = new CTinyJS();

	System *psys = m_pOwner->GetSystem();
	m_JS->m_pSys = psys;
	m_JS->m_pObj = m_pOwner;

	registerFunctions(m_JS);
	registerMathFunctions(m_JS);

	m_JS->AddNative(_T("function GetNumChildren(hrootobj)"),						jcGetNumChildren, psys);
	m_JS->AddNative(_T("function GetChild(hrootobj, idx)"),							jcGetChild, psys);
	m_JS->AddNative(_T("function FindObjByGUID(hrootobj, guid, recursive)"),		jcFindObjByGUID, psys);
	m_JS->AddNative(_T("function FindFirstObjByName(hrootobj, name, recursive)"),	jcFindFirstObjByName, psys);
	m_JS->AddNative(_T("function Log(text)"),										jcLog, psys);
	m_JS->AddNative(_T("function FindProperty(hobj, name)"),						jcFindProperty, psys);
	m_JS->AddNative(_T("function GetPropertyValue(hprop)"),							jcGetPropertyValue, psys);
	m_JS->AddNative(_T("function SetPropertyValue(hprop, val)"),					jcSetPropertyValue, psys);
	m_JS->AddNative(_T("function CreateObject(protoname, hparentobj)"),				jcCreateObject, psys);
	m_JS->AddNative(_T("function DeleteObject(hobj)"),								jcDeleteObject, psys);
	m_JS->AddNative(_T("function GetParent(hobj)"),									jcGetParent, psys);
	m_JS->AddNative(_T("function SetParent(hobj, hnewparentobj)"),					jcSetParent, psys);
	m_JS->AddNative(_T("function SetObjectName(hobj, newname)"),					jcSetObjectName, psys);
	m_JS->AddNative(_T("function LoadObject(hobj, filename)"),						jcLoadObject, psys);
	m_JS->AddNative(_T("function GetRegisteredObject(designation)"),				jcGetRegisteredObject, psys);
	m_JS->AddNative(_T("function RegisterObject(designation, hobj)"),				jcRegisterObject, psys);
	m_JS->AddNative(_T("function AdjustQuaternion(quat, axis, angle)"),				jcAdjustQuaternion, psys);

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

	props::IProperty *pscl = propset->CreateReferenceProperty(_T("ScriptUpdateRate"), 'SUDR', &m_UpdateRate, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pscl)
		pscl->SetFloat(0.033f);

	props::IProperty *psrc = propset->GetPropertyById('SRCF');
	if (!psrc)
	{
		psrc = propset->CreateProperty(_T("SourceFile"), 'SRCF');
		psrc->SetAspect(props::IProperty::PA_FILENAME);
		psrc->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	return true;
}


void ScriptableImpl::Update(float elapsed_time)
{
	if (!m_Continue)
		return;

	m_UpdateTime -= elapsed_time;
	if (m_UpdateTime <= 0)
	{
		m_UpdateTime = m_UpdateRate + m_UpdateTime;
		if (m_UpdateTime < 0)
			m_UpdateTime = m_UpdateRate;

		Execute(_T("update(%0.3f);"), elapsed_time);
	}
}


bool ScriptableImpl::Prerender(Object::RenderFlags flags)
{
	return true;
}


void ScriptableImpl::Render(Object::RenderFlags flags)
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
			Resource *pres = m_pOwner->GetSystem()->GetResourceManager()->GetResource(pprop->AsString(), RESF_DEMANDLOAD);
			if (pres && (pres->GetStatus() == Resource::RS_LOADED))
			{
				if (m_Code != ((Script *)(pres->GetData()))->m_Code)
				{
					ResetJS();

					m_Code = ((Script *)(pres->GetData()))->m_Code;
					m_Continue = m_JS->Execute(m_Code.c_str());
					Execute(_T("init();"));
				}
			}
			break;
		}

		case 'SRC':
			if (m_Code != pprop->AsString())
			{
				ResetJS();

				m_Code = pprop->AsString();
				m_Continue = m_JS->Execute(m_Code.c_str());
				Execute(_T("init();"));
			}
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


#define CMD_BUFSIZE	1024

void ScriptableImpl::Execute(const TCHAR *pcmd, ...)
{
	va_list marker;
	va_start(marker, pcmd);

	if (!m_Continue)
		return;

	TCHAR buf[CMD_BUFSIZE];	// Temporary buffer for output
	_vsntprintf_s(buf, CMD_BUFSIZE - 1, pcmd, marker);

	m_Continue = m_JS->Execute(buf);
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
	CScriptVar *ret = c->GetReturnVar();
	if (!ret)
		return;

	tstring text = c->GetParameter(_T("text"))->GetString();
	System *psys = (System *)userdata;
	assert(psys);

	psys->GetLog()->Print(text.c_str());
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

	ret->SetInt((int64_t)(pobj->GetProperties()->GetPropertyByName(name.c_str())));
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
			psvl = ret->FindChildOrCreate(_T("w"));
			psvl->m_Owned = true;
			pw = psvl->m_Var;
			pw->SetFloat(pprop->AsVec4F()->w);

		case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3:
			psvl = ret->FindChildOrCreate(_T("z"));
			psvl->m_Owned = true;
			pz = psvl->m_Var;
			pz->SetFloat(pprop->AsVec3F()->z);

		case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2:
			psvl = ret->FindChildOrCreate(_T("y"));
			psvl->m_Owned = true;
			py = psvl->m_Var;
			py->SetFloat(pprop->AsVec2F()->y);

			psvl = ret->FindChildOrCreate(_T("x"));
			psvl->m_Owned = true;
			px = psvl->m_Var;
			px->SetFloat(pprop->AsVec2F()->x);
			break;

		case props::IProperty::PROPERTY_TYPE::PT_INT_V4:
			psvl = ret->FindChildOrCreate(_T("w"));
			psvl->m_Owned = true;
			pw = psvl->m_Var;
			pw->SetInt(pprop->AsVec4I()->w);

		case props::IProperty::PROPERTY_TYPE::PT_INT_V3:
			psvl = ret->FindChildOrCreate(_T("z"));
			psvl->m_Owned = true;
			pz = psvl->m_Var;
			pz->SetInt(pprop->AsVec3I()->z);

		case props::IProperty::PROPERTY_TYPE::PT_INT_V2:
			psvl = ret->FindChildOrCreate(_T("y"));
			psvl->m_Owned = true;
			py = psvl->m_Var;
			py->SetInt(pprop->AsVec2I()->y);

			psvl = ret->FindChildOrCreate(_T("x"));
			psvl->m_Owned = true;
			px = psvl->m_Var;
			px->SetInt(pprop->AsVec2I()->x);
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
	Object *pparentobj = dynamic_cast<Object *>((Object *)hparentobj);
	if (pproto)
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
				genio::FOURCHARCODE b = is->NextBlockId();
				if (b == 'CEL0')
				{
					if (is->BeginBlock(b))
					{
						uint16_t len;

						tstring name, description, author, website, copyright;

						is->ReadUINT16(len);
						name.resize(len);
						if (len)
							is->ReadString(name.data());

						is->ReadUINT16(len);
						description.resize(len);
						if (len)
							is->ReadString(description.data());

						is->ReadUINT16(len);
						author.resize(len);
						if (len)
							is->ReadString(author.data());

						is->ReadUINT16(len);
						website.resize(len);
						if (len)
							is->ReadString(website.data());

						is->ReadUINT16(len);
						copyright.resize(len);
						if (len)
							is->ReadString(copyright.data());

						if (is->BeginBlock('CAM0'))
						{
							is->EndBlock();
						}

						if (is->BeginBlock('ENV0'))
						{
							is->EndBlock();
						}

						if (pobj)
						{
							bool r = pobj->Load(is);
							if (ret)
								ret->SetInt(r ? 1 : 0);
						}

						is->EndBlock();
					}
				}
			}

			is->Release();
		}
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


//m_JS->AddNative(_T("function AdjustQuaternion(quat, axis, angle)"),			jcAdjustQuaternion, psys);
void jcAdjustQuaternion(CScriptVar *c, void *userdata)
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
	if (!(pax && pay && paz) || (paxis->GetArrayLength() != 3))
		return;

	glm::fvec3 axis;
	axis.x = (float)(pax ? pax->m_Var->GetFloat() : paxis->GetArrayIndex(0)->GetFloat());
	axis.y = (float)(pay ? pay->m_Var->GetFloat() : paxis->GetArrayIndex(1)->GetFloat());
	axis.z = (float)(paz ? paz->m_Var->GetFloat() : paxis->GetArrayIndex(2)->GetFloat());


	CScriptVarLink *pqx = pquat->FindChild(_T("x"));
	CScriptVarLink *pqy = pquat->FindChild(_T("y"));
	CScriptVarLink *pqz = pquat->FindChild(_T("z"));
	CScriptVarLink *pqw = pquat->FindChild(_T("w"));
	if (!(pqx && pqy && pqz && pqw) || (pquat->GetArrayLength() != 4))
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
