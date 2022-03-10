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
	js->root->trace();
}

void scObjectDump(CScriptVar *c, void *)
{
	c->getParameter(_T("this"))->trace(_T("> "));
}

void scObjectClone(CScriptVar *c, void *)
{
	CScriptVar *obj = c->getParameter(_T("this"));

	c->getReturnVar()->copyValue(obj);
}

void scMathRand(CScriptVar *c, void *)
{
	c->getReturnVar()->setDouble((double)rand() / RAND_MAX);
}

void scMathRandInt(CScriptVar *c, void *)
{
	int64_t min = c->getParameter(_T("min"))->getInt();
	int64_t max = c->getParameter(_T("max"))->getInt();
	int64_t val = min + (int)(rand() % (1 + max - min));

	c->getReturnVar()->setInt(val);
}

void scCharToInt(CScriptVar *c, void *)
{
	tstring str = c->getParameter(_T("ch"))->getString();;
	int64_t val = 0;

	if (str.length() > 0)
		val = (int)str.c_str()[0];

	c->getReturnVar()->setInt(val);
}

void scStringIndexOf(CScriptVar *c, void *)
{
	tstring str = c->getParameter(_T("this"))->getString();
	tstring search = c->getParameter(_T("search"))->getString();
	size_t p = str.find(search);

	int64_t val = (p == tstring::npos) ? -1 : p;

	c->getReturnVar()->setInt(val);
}

void scStringSubstring(CScriptVar *c, void *)
{
	tstring str = c->getParameter(_T("this"))->getString();
	int64_t lo = c->getParameter(_T("lo"))->getInt();
	int64_t hi = c->getParameter(_T("hi"))->getInt();

	int64_t l = hi - lo;
	if ((l > 0) && (lo >= 0) && (lo + l <= (int)str.length()))
		c->getReturnVar()->setString(str.substr(lo, l));
	else
		c->getReturnVar()->setString(_T(""));
}

void scStringCharAt(CScriptVar *c, void *)
{
	tstring str = c->getParameter(_T("this"))->getString();

	int64_t p = c->getParameter(_T("pos"))->getInt();

	if (p >= 0 && p < (int)str.length())
		c->getReturnVar()->setString(str.substr(p, 1));
	else
		c->getReturnVar()->setString(_T(""));
}

void scStringCharCodeAt(CScriptVar *c, void *)
{
	tstring str = c->getParameter(_T("this"))->getString();
	int64_t p = c->getParameter(_T("pos"))->getInt();

	if (p >= 0 && p < (int)str.length())
		c->getReturnVar()->setInt(str.at(p));
	else
		c->getReturnVar()->setInt(0);
}

void scStringSplit(CScriptVar *c, void *)
{
	tstring str = c->getParameter(_T("this"))->getString();
	tstring sep = c->getParameter(_T("separator"))->getString();

	CScriptVar *result = c->getReturnVar();
	result->setArray();
	int64_t length = 0;

	size_t pos = str.find(sep);
	while (pos != tstring::npos)
	{
		result->setArrayIndex(length++, new CScriptVar(str.substr(0, pos)));

		str = str.substr(pos + 1);
		pos = str.find(sep);
	}

	if (str.size() > 0)
		result->setArrayIndex(length++, new CScriptVar(str));
}

void scStringFromCharCode(CScriptVar *c, void *)
{
	TCHAR str[2];

	str[0] = (TCHAR)(c->getParameter(_T("char"))->getInt());
	str[1] = 0;

	c->getReturnVar()->setString(str);
}

void scStringIncludes(CScriptVar *c, void *)
{
	tstring t = c->getParameter(_T("this"))->getString();
	tstring s = c->getParameter(_T("str"))->getString();

	// optionally check 
	bool case_sensitive = true;
	CScriptVar *csv = c->getParameter(_T("sensitive"));
	if (csv)
	{
		case_sensitive = csv->getBool();
	}

	if (!case_sensitive)
	{
		std::transform(t.begin(), t.end(), t.end(), _tolower);
		std::transform(s.begin(), s.end(), s.end(), _tolower);
	}

	bool ret = false;

	if (t.find(s) < t.length())
		ret = true;

	c->getReturnVar()->setInt(ret ? 1 : 0);
}

void scIntegerParseInt(CScriptVar *c, void *)
{
	tstring str = c->getParameter(_T("str"))->getString();
	int64_t val = _tcstol(str.c_str(), 0, 0);
	c->getReturnVar()->setInt(val);
}

void scIntegerValueOf(CScriptVar *c, void *)
{
	tstring str = c->getParameter(_T("str"))->getString();

	int64_t val = 0;
	if (str.length() == 1)
		val = str[0];

	c->getReturnVar()->setInt(val);
}

void scJSONStringify(CScriptVar *c, void *)
{
	tostringstream result;

	c->getParameter(_T("obj"))->getJSON(result);
	c->getReturnVar()->setString(result.str());
}

void scExec(CScriptVar *c, void *data)
{
	CTinyJS *tinyJS = (CTinyJS *)data;
	tstring str = c->getParameter(_T("jsCode"))->getString();

	tinyJS->execute(str);
}

void scEval(CScriptVar *c, void *data)
{
	CTinyJS *tinyJS = (CTinyJS *)data;
	tstring str = c->getParameter(_T("jsCode"))->getString();

	c->setReturnVar(tinyJS->evaluateComplex(str).var);
}

void scArrayContains(CScriptVar *c, void *data)
{
	CScriptVar *obj = c->getParameter(_T("obj"));
	CScriptVarLink *v = c->getParameter(_T("this"))->firstChild;

	bool contains = false;
	while (v)
	{
		if (v->var->equals(obj))
		{
			contains = true;
			break;
		}

		v = v->nextSibling;
	}

	c->getReturnVar()->setInt(contains ? 1 : 0);
}

void scArrayRemove(CScriptVar *c, void *data)
{
	CScriptVar *obj = c->getParameter(_T("obj"));
	vector<int64_t> removedIndices;

	CScriptVarLink *v;

	// remove
	v = c->getParameter(_T("this"))->firstChild;
	while (v)
	{
		if (v->var->equals(obj))
		{
			removedIndices.push_back(v->getIntName());
		}

		v = v->nextSibling;
	}

	// renumber
	v = c->getParameter(_T("this"))->firstChild;
	while (v)
	{
		int64_t n = v->getIntName();
		int64_t newn = n;

		for (size_t i = 0; i < removedIndices.size(); i++)
		{
			if (n >= removedIndices[i])
				newn--;
		}

		if (newn != n)
		{
			v->setIntName(newn);
		}

		v = v->nextSibling;
	}
}

void scArrayJoin(CScriptVar *c, void *data)
{
	tstring sep = c->getParameter(_T("separator"))->getString();
	CScriptVar *arr = c->getParameter(_T("this"));

	tostringstream sstr;

	int64_t l = arr->getArrayLength();
	for (int64_t i = 0; i < l; i++)
	{
		if (i > 0)
			sstr << sep;

		sstr << arr->getArrayIndex(i)->getString();
	}

	c->getReturnVar()->setString(sstr.str());
}

// ----------------------------------------------- Register Functions
void registerFunctions(CTinyJS *tinyJS)
{
	tinyJS->addNative(_T("function exec(jsCode)"), scExec, tinyJS); // execute the given code
	tinyJS->addNative(_T("function eval(jsCode)"), scEval, tinyJS); // execute the given string (an expression) and return the result
	tinyJS->addNative(_T("function trace()"), scTrace, tinyJS);
	tinyJS->addNative(_T("function Object.dump()"), scObjectDump, 0);
	tinyJS->addNative(_T("function Object.clone()"), scObjectClone, 0);
	tinyJS->addNative(_T("function Math.rand()"), scMathRand, 0);
	tinyJS->addNative(_T("function Math.randInt(min, max)"), scMathRandInt, 0);
	tinyJS->addNative(_T("function charToInt(ch)"), scCharToInt, 0); //  convert a character to an int - get its value
	tinyJS->addNative(_T("function String.indexOf(search)"), scStringIndexOf, 0); // find the position of a string in a string, -1 if not
	tinyJS->addNative(_T("function String.substring(lo,hi)"), scStringSubstring, 0);
	tinyJS->addNative(_T("function String.charAt(pos)"), scStringCharAt, 0);
	tinyJS->addNative(_T("function String.charCodeAt(pos)"), scStringCharCodeAt, 0);
	tinyJS->addNative(_T("function String.fromCharCode(char)"), scStringFromCharCode, 0);
	tinyJS->addNative(_T("function String.split(separator)"), scStringSplit, 0);
	tinyJS->addNative(_T("function String.includes(str, sensitive)"), scStringIncludes, 0);
	tinyJS->addNative(_T("function Integer.parseInt(str)"), scIntegerParseInt, 0); // string to int
	tinyJS->addNative(_T("function Integer.valueOf(str)"), scIntegerValueOf, 0); // value of a single character
	tinyJS->addNative(_T("function JSON.stringify(obj, replacer)"), scJSONStringify, 0); // convert to JSON. replacer is ignored at the moment
	// JSON.parse is left out as you can (unsafely!) use eval instead
	tinyJS->addNative(_T("function Array.contains(obj)"), scArrayContains, 0);
	tinyJS->addNative(_T("function Array.remove(obj)"), scArrayRemove, 0);
	tinyJS->addNative(_T("function Array.join(separator)"), scArrayJoin, 0);
}

