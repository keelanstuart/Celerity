/*
 * TinyJS
 *
 * A single-file Javascript-alike engine
 *
 * - Useful language functions
 *
 * Authored By Gordon Williams <gw@pur3.co.uk>
 *
 * Copyright (C) 2009 Pur3 Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "pch.h"
#include "TinyJS_Functions.h"
#include <math.h>
#include <cstdlib>
#include <sstream>

using namespace std;
// ----------------------------------------------- Actual Functions
void scTrace(CScriptVar *c, void *userdata)
{
	CTinyJS *js = (CTinyJS *)userdata;
	js->m_Root->Trace();
}

void scObjectDump(CScriptVar *c, void *)
{
	c->GetParameter(_T("this"))->Trace(_T("> "));
}

void scObjectClone(CScriptVar *c, void *)
{
	CScriptVar *obj = c->GetParameter(_T("this"));

	c->GetReturnVar()->CopyValue(obj);
}

void scMathRand(CScriptVar *c, void *)
{
	c->GetReturnVar()->SetFloat((float)rand() / RAND_MAX);
}

void scMathRandInt(CScriptVar *c, void *)
{
	int64_t min = c->GetParameter(_T("min"))->GetInt();
	int64_t max = c->GetParameter(_T("max"))->GetInt();
	int64_t val = min + (int)(rand() % (1 + max - min));

	c->GetReturnVar()->SetInt(val);
}

void scCharToInt(CScriptVar *c, void *)
{
	tstring str = c->GetParameter(_T("ch"))->GetString();;
	int64_t val = 0;

	if (str.length() > 0)
		val = (int)str.c_str()[0];

	c->GetReturnVar()->SetInt(val);
}

void scStringIndexOf(CScriptVar *c, void *)
{
	tstring str = c->GetParameter(_T("this"))->GetString();
	tstring search = c->GetParameter(_T("search"))->GetString();
	size_t p = str.find(search);

	int64_t val = (p == tstring::npos) ? -1 : p;

	c->GetReturnVar()->SetInt(val);
}

void scStringSubstring(CScriptVar *c, void *)
{
	tstring str = c->GetParameter(_T("this"))->GetString();
	int64_t lo = c->GetParameter(_T("lo"))->GetInt();
	int64_t hi = c->GetParameter(_T("hi"))->GetInt();

	int64_t l = hi - lo;
	if ((l > 0) && (lo >= 0) && (lo + l <= (int)str.length()))
		c->GetReturnVar()->SetString(str.substr(lo, l).c_str());
	else
		c->GetReturnVar()->SetString(_T(""));
}

void scStringCharAt(CScriptVar *c, void *)
{
	tstring str = c->GetParameter(_T("this"))->GetString();

	int64_t p = c->GetParameter(_T("pos"))->GetInt();

	if (p >= 0 && p < (int)str.length())
		c->GetReturnVar()->SetString(str.substr(p, 1).c_str());
	else
		c->GetReturnVar()->SetString(_T(""));
}

void scStringCharCodeAt(CScriptVar *c, void *)
{
	tstring str = c->GetParameter(_T("this"))->GetString();
	int64_t p = c->GetParameter(_T("pos"))->GetInt();

	if (p >= 0 && p < (int)str.length())
		c->GetReturnVar()->SetInt(str.at(p));
	else
		c->GetReturnVar()->SetInt(0);
}

void scStringSplit(CScriptVar *c, void *)
{
	tstring str = c->GetParameter(_T("this"))->GetString();
	tstring sep = c->GetParameter(_T("separator"))->GetString();

	CScriptVar *result = c->GetReturnVar();
	result->SetArray();
	int64_t length = 0;

	size_t pos = str.find(sep);
	while (pos != tstring::npos)
	{
		result->SetArrayIndex(length++, new CScriptVar(str.substr(0, pos).c_str()));

		str = str.substr(pos + 1);
		pos = str.find(sep);
	}

	if (str.size() > 0)
		result->SetArrayIndex(length++, new CScriptVar(str.c_str()));
}

void scStringFromCharCode(CScriptVar *c, void *)
{
	TCHAR str[2];

	str[0] = (TCHAR)(c->GetParameter(_T("char"))->GetInt());
	str[1] = 0;

	c->GetReturnVar()->SetString(str);
}

void scStringIncludes(CScriptVar *c, void *)
{
	tstring t = c->GetParameter(_T("this"))->GetString();
	tstring s = c->GetParameter(_T("str"))->GetString();

	// optionally check 
	bool case_sensitive = true;
	CScriptVar *csv = c->GetParameter(_T("sensitive"));
	if (csv)
	{
		case_sensitive = csv->GetBool();
	}

	if (!case_sensitive)
	{
		std::transform(t.begin(), t.end(), t.end(), _tolower);
		std::transform(s.begin(), s.end(), s.end(), _tolower);
	}

	bool ret = false;

	if (t.find(s) < t.length())
		ret = true;

	c->GetReturnVar()->SetInt(ret ? 1 : 0);
}

void scIntegerParseInt(CScriptVar *c, void *)
{
	tstring str = c->GetParameter(_T("str"))->GetString();
	int64_t val = _tcstol(str.c_str(), 0, 0);
	c->GetReturnVar()->SetInt(val);
}

void scIntegerValueOf(CScriptVar *c, void *)
{
	tstring str = c->GetParameter(_T("str"))->GetString();

	int64_t val = 0;
	if (str.length() == 1)
		val = str[0];

	c->GetReturnVar()->SetInt(val);
}

void scExec(CScriptVar *c, void *data)
{
	CTinyJS *tinyJS = (CTinyJS *)data;
	tstring str = c->GetParameter(_T("jsCode"))->GetString();

	tinyJS->Execute(str.c_str());
}

void scArrayContains(CScriptVar *c, void *data)
{
	CScriptVar *obj = c->GetParameter(_T("obj"));
	CScriptVarLink *v = c->GetParameter(_T("this"))->m_Child.first;

	bool contains = false;
	while (v)
	{
		if (v->m_Var->Equals(obj))
		{
			contains = true;
			break;
		}

		v = v->m_Sibling.next;
	}

	c->GetReturnVar()->SetInt(contains ? 1 : 0);
}

void scArrayRemove(CScriptVar *c, void *data)
{
	CScriptVar *obj = c->GetParameter(_T("obj"));
	vector<int64_t> removedIndices;

	CScriptVarLink *v;

	// remove
	v = c->GetParameter(_T("this"))->m_Child.first;
	while (v)
	{
		if (v->m_Var->Equals(obj))
		{
			removedIndices.push_back(v->GetIntName());
		}

		v = v->m_Sibling.next;
	}

	// renumber
	v = c->GetParameter(_T("this"))->m_Child.first;
	while (v)
	{
		int64_t n = v->GetIntName();
		int64_t newn = n;

		for (size_t i = 0; i < removedIndices.size(); i++)
		{
			if (n >= removedIndices[i])
				newn--;
		}

		if (newn != n)
		{
			v->SetIntName(newn);
		}

		v = v->m_Sibling.next;
	}
}

void scArrayJoin(CScriptVar *c, void *data)
{
	tstring sep = c->GetParameter(_T("separator"))->GetString();
	CScriptVar *arr = c->GetParameter(_T("this"));

	tostringstream sstr;

	int64_t l = arr->GetArrayLength();
	for (int64_t i = 0; i < l; i++)
	{
		if (i > 0)
			sstr << sep;

		sstr << arr->GetArrayIndex(i)->GetString();
	}

	c->GetReturnVar()->SetString(sstr.str().c_str());
}

// ----------------------------------------------- Register Functions
void registerFunctions(CTinyJS *tinyJS)
{
	tinyJS->AddNative(_T("function Array.contains(obj)"), scArrayContains, 0);
	tinyJS->AddNative(_T("function Array.join(separator)"), scArrayJoin, 0);
	tinyJS->AddNative(_T("function Array.remove(obj)"), scArrayRemove, 0);
	tinyJS->AddNative(_T("function charToInt(ch)"), scCharToInt, 0); //  convert a character to an int - get its value
	tinyJS->AddNative(_T("function exec(jsCode)"), scExec, tinyJS); // execute the given code
	tinyJS->AddNative(_T("function Integer.parseInt(str)"), scIntegerParseInt, 0); // string to int
	tinyJS->AddNative(_T("function Integer.valueOf(str)"), scIntegerValueOf, 0); // value of a single character
	tinyJS->AddNative(_T("function Math.rand()"), scMathRand, 0);
	tinyJS->AddNative(_T("function Math.randInt(min, max)"), scMathRandInt, 0);
	tinyJS->AddNative(_T("function Object.clone()"), scObjectClone, 0);
	tinyJS->AddNative(_T("function Object.dump()"), scObjectDump, 0);
	tinyJS->AddNative(_T("function String.charAt(pos)"), scStringCharAt, 0);
	tinyJS->AddNative(_T("function String.charCodeAt(pos)"), scStringCharCodeAt, 0);
	tinyJS->AddNative(_T("function String.fromCharCode(char)"), scStringFromCharCode, 0);
	tinyJS->AddNative(_T("function String.includes(str, sensitive)"), scStringIncludes, 0);
	tinyJS->AddNative(_T("function String.indexOf(search)"), scStringIndexOf, 0); // find the position of a string in a string, -1 if not
	tinyJS->AddNative(_T("function String.split(separator)"), scStringSplit, 0);
	tinyJS->AddNative(_T("function String.substring(lo,hi)"), scStringSubstring, 0);
	tinyJS->AddNative(_T("function trace()"), scTrace, tinyJS);
}

