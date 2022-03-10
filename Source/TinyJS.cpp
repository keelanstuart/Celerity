/*
 * TinyJS
 *
 * A single-file Javascript-alike engine
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

/* Version 0.1  :  (gw) First published on Google Code
   Version 0.11 :  Making sure the 'root' variable never changes
				   'symbol_base' added for the current base of the sybmbol table
   Version 0.12 :  Added findChildOrCreate, changed string passing to use references
				   Fixed broken string encoding in getJSString()
				   Removed getInitCode and added getJSON instead
				   Added nil
				   Added rough JSON parsing
				   Improved example app
   Version 0.13 :  Added tokenEnd/tokenLastEnd to lexer to avoid parsing whitespace
				   Ability to define functions without names
				   Can now do "var mine = function(a,b) { ... };"
				   Slightly better 'trace' function
				   Added findChildOrCreateByPath function
				   Added simple test suite
				   Added skipping of blocks when not executing
   Version 0.14 :  Added parsing of more number types
				   Added parsing of string defined with '
				   Changed nil to null as per spec, added 'undefined'
				   Now set variables with the correct scope, and treat unknown
							  as 'undefined' rather than failing
				   Added proper (I hope) handling of null and undefined
				   Added === check
   Version 0.15 :  Fix for possible memory leaks
   Version 0.16 :  Removal of un-needed findRecursive calls
				   symbol_base removed and replaced with 'scopes' stack
				   Added reference counting a proper tree structure
					   (Allowing pass by reference)
				   Allowed JSON output to output IDs, not strings
				   Added get/set for array indices
				   Changed Callbacks to include user data pointer
				   Added some support for objects
				   Added more Java-esque builtin functions
   Version 0.17 :  Now we don't deepCopy the parent object of the class
				   Added JSON.stringify and eval()
				   Nicer JSON indenting
				   Fixed function output in JSON
				   Added evaluateComplex
				   Fixed some reentrancy issues with evaluate/execute
   Version 0.18 :  Fixed some issues with code being executed when it shouldn't
   Version 0.19 :  Added array.length
				   Changed '__parent' to 'prototype' to bring it more in line with javascript
   Version 0.20 :  Added '%' operator
   Version 0.21 :  Added array type
				   String.length() no more - now String.length
				   Added extra constructors to reduce confusion
				   Fixed checks against undefined
   Version 0.22 :  First part of ardi's changes:
					   sprintf -> sprintf_s
					   extra tokens parsed
					   array memory leak fixed
				   Fixed memory leak in evaluateComplex
				   Fixed memory leak in FOR loops
				   Fixed memory leak for unary minus
   Version 0.23 :  Allowed evaluate[Complex] to take in semi-colon separated
					 statements and then only return the value from the last one.
					 Also checks to make sure *everything* was parsed.
				   Ints + doubles are now stored in binary form (faster + more precise)
   Version 0.24 :  More useful error for maths ops
				   Don't dump everything on a match error.
   Version 0.25 :  Better string escaping
   Version 0.26 :  Add CScriptVar::equals
				   Add built-in array functions
   Version 0.27 :  Added OZLB's TinyJS.setVariable (with some tweaks)
				   Added OZLB's Maths Functions
   Version 0.28 :  Ternary operator
				   Rudimentary call stack on error
				   Added String Character functions
				   Added shift operators
   Version 0.29 :  Added new object via functions
				   Fixed getString() for double on some platforms
   Version 0.30 :  Rlyeh Mario's patch for Math Functions on VC++
   Version 0.31 :  Add exec() to TinyJS functions
				   Now print quoted JSON that can be read by PHP/Python parsers
				   Fixed postfix increment operator
   Version 0.32 :  Fixed Math.randInt on 32 bit PCs, where it was broken
   Version 0.33 :  Fixed Memory leak + brokenness on === comparison

	NOTE:
		  Constructing an array with an initial length 'Array(5)' doesn't work
		  Recursive loops of data such as a.foo = a; fail to be garbage collected
		  length variable cannot be set
		  The postfix increment operator returns the current value, not the previous as it should.
		  There is no prefix increment operator
		  Arrays are implemented as a linked list - hence a lookup time is O(n)

	TODO:
		  Utility va-args style function in TinyJS for executing a function directly
		  Merge the parsing of expressions/statements so eval("statement") works like we'd expect.
		  Move 'shift' implementation into mathsOp

 */

#include "pch.h"

#include "TinyJS.h"
#include <assert.h>
#include <inttypes.h>

 /* Frees the given link IF it isn't owned by anything else */
#define CLEAN(x) { CScriptVarLink *__v = x; if (__v && !__v->owned) { delete __v; } }

 /* Create a LINK to point to VAR and free the old link.
 * BUT this is more clever - it tries to keep the old link if it's not owned to save allocations */
#define CREATE_LINK(LINK, VAR) { if (!LINK || LINK->owned) LINK = new CScriptVarLink(VAR); else LINK->replaceWith(VAR); }

#include <string>
#include <string.h>
#include <sstream>
#include <cstdlib>
#include <stdio.h>

using namespace std;

#ifdef _WIN32
#ifdef _DEBUG
   #ifndef DBG_NEW
	  #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	  #define new DBG_NEW
   #endif
#endif
#endif

#ifdef __GNUC__
#define vsprintf_s vsnprintf
#define sprintf_s snprintf
#define _strdup strdup
#endif

// ----------------------------------------------------------------------------------- Memory Debug

#define DEBUG_MEMORY 0

#if DEBUG_MEMORY

vector<CScriptVar*> allocatedVars;
vector<CScriptVarLink*> allocatedLinks;

void mark_allocated(CScriptVar *v)
{
	allocatedVars.push_back(v);
}

void mark_deallocated(CScriptVar *v)
{
	for (size_t i=0; i < allocatedVars.size(); i++)
	{
		if (allocatedVars[i] == v)
		{
			allocatedVars.erase(allocatedVars.begin() + i);
			break;
		}
	}
}

void mark_allocated(CScriptVarLink *v)
{
	allocatedLinks.push_back(v);
}

void mark_deallocated(CScriptVarLink *v)
{
	for (size_t i=0;i<allocatedLinks.size();i++)
	{
		if (allocatedLinks[i] == v)
		{
			allocatedLinks.erase(allocatedLinks.begin()+i);
			break;
		}
	}
}

void show_allocated()
{
	for (size_t i = 0; i < allocatedVars.size(); i++)
	{
		_tprintf(_T("ALLOCATED, %d refs\n"), allocatedVars[i]->getRefs());
		allocatedVars[i]->trace(_T("  "));
	}

	for (size_t i = 0; i < allocatedLinks.size(); i++)
	{
		_tprintf(_T("ALLOCATED LINK %s, allocated[%d] to \n"), allocatedLinks[i]->name.c_str(), allocatedLinks[i]->var->getRefs());
		allocatedLinks[i]->var->trace(_T("  "));
	}

	allocatedVars.clear();
	allocatedLinks.clear();
}

#endif

// ----------------------------------------------------------------------------------- Utils
bool isWhitespace(TCHAR ch)
{
	return (ch == _T(' ')) || (ch == _T('\t')) || (ch == _T('\n')) || (ch == _T('\r'));
}

bool isNumeric(TCHAR ch)
{
	return (ch >= _T('0')) && (ch <= _T('9'));
}

bool isNumber(const tstring &str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isNumeric(str[i]))
			return false;
	}

	return true;
}

bool isHexadecimal(TCHAR ch)
{
	return ((ch >= _T('0')) && (ch <= _T('9'))) ||
		   ((ch >= _T('a')) && (ch <= _T('f'))) ||
		   ((ch >= _T('A')) && (ch <= _T('F')));
}

bool isAlpha(TCHAR ch)
{
	return ((ch >= _T('a')) && (ch <= _T('z'))) || ((ch >= _T('A')) && (ch <= _T('Z'))) || (ch == _T('_'));
}

bool isIDString(const TCHAR *s)
{
	if (!isAlpha(*s))
		return false;

	while (*s)
	{
		if (!(isAlpha(*s) || isNumeric(*s)))
			return false;

		s++;
	}

	return true;
}

void replace(tstring &str, TCHAR textFrom, const TCHAR *textTo)
{
	size_t sLen = _tcslen(textTo);
	size_t p = str.find(textFrom);

	while (p != string::npos)
	{
		str = str.substr(0, p) + textTo + str.substr(p + 1);
		p = str.find(textFrom, p + sLen);
	}
}

/// convert the given string into a quoted string suitable for javascript
tstring getJSString(const tstring &str)
{
	tstring nStr = str;

	for (size_t i=0; i < nStr.size(); i++)
	{
		const TCHAR *replaceWith = _T("");
		bool replace = true;

		switch (nStr[i])
		{
			case _T('\\'):
				replaceWith = _T("\\\\");
				break;

			case _T('\n'):
				replaceWith = _T("\\n");
				break;

			case _T('\r'):
				replaceWith = _T("\\r");
				break;

			case _T('\a'):
				replaceWith = _T("\\a");
				break;

			case _T('"'):
				replaceWith = _T("\\\"");
				break;

			default:
			{
				uint8_t nCh = ((int64_t)nStr[i]) & 0xFF;
				if ((nCh < 32) || (nCh > 127))
				{
					TCHAR buffer[5];
					_stprintf_s(buffer, 5, _T("\\x%02X"), nCh);

					replaceWith = buffer;
				}
				else
				{
					replace = false;
				}
			}
		}

		if (replace)
		{
			nStr = nStr.substr(0, i) + replaceWith + nStr.substr(i + 1);
			i += _tcslen(replaceWith) - 1;
		}
	}

	return _T("\"") + nStr + _T("\"");
}

/** Is the string alphanumeric */
bool isAlphaNum(const tstring &str)
{
	if (str.empty())
		return true;

	if (!isAlpha(str[0]))
		return false;

	for (size_t i = 0; i < str.size(); i++)
	{
		if (!(isAlpha(str[i]) || isNumeric(str[i])))
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------------------- CSCRIPTEXCEPTION

CScriptException::CScriptException(const tstring &exceptionText)
{
	text = exceptionText;
}

// ----------------------------------------------------------------------------------- CSCRIPTLEX

CScriptLex::CScriptLex(const tstring &input)
{
	data = _tcsdup(input.c_str());
	dataOwned = true;
	dataStart = 0;
	dataEnd = _tcslen(data);

	reset();
}

CScriptLex::CScriptLex(CScriptLex *owner, int64_t startChar, int64_t endChar)
{
	data = owner->data;
	dataOwned = false;
	dataStart = startChar;
	dataEnd = endChar;

	reset();
}

CScriptLex::~CScriptLex(void)
{
	if (dataOwned)
		free((void*)data);
}

void CScriptLex::reset()
{
	dataPos = dataStart;
	tokenStart = 0;
	tokenEnd = 0;
	tokenLastEnd = 0;
	tk = 0;
	tkStr = _T("");

	getNextCh();
	getNextCh();
	getNextToken();
}

void CScriptLex::match(int64_t expected_tk)
{
	if (tk != expected_tk)
	{
		tostringstream errorString;
		errorString << _T("Got ") << getTokenStr(tk) << _T(" expected ") << getTokenStr(expected_tk) << _T(" at ") << getPosition(tokenStart);

		throw new CScriptException(errorString.str());
	}

	getNextToken();
}

tstring CScriptLex::getTokenStr(int64_t token)
{
	if ((token > 32) && (token < 128))
	{
		TCHAR buf[4] = _T("' '");
		buf[1] = (char)token;
		return buf;
	}
	
	switch (token)
	{
		case LEX_EOF : return _T("EOF");
		case LEX_ID : return _T("ID");
		case LEX_INT : return _T("INT");
		case LEX_FLOAT : return _T("FLOAT");
		case LEX_STR : return _T("STRING");
		case LEX_EQUAL : return _T("==");
		case LEX_TYPEEQUAL : return _T("===");
		case LEX_NEQUAL : return _T("!=");
		case LEX_NTYPEEQUAL : return _T("!==");
		case LEX_LEQUAL : return _T("<=");
		case LEX_LSHIFT : return _T("<<");
		case LEX_LSHIFTEQUAL : return _T("<<=");
		case LEX_GEQUAL : return _T(">=");
		case LEX_RSHIFT : return _T(">>");
		case LEX_RSHIFTUNSIGNED : return _T(">>");
		case LEX_RSHIFTEQUAL : return _T(">>=");
		case LEX_PLUSEQUAL : return _T("+=");
		case LEX_MINUSEQUAL : return _T("-=");
		case LEX_PLUSPLUS : return _T("++");
		case LEX_MINUSMINUS : return _T("--");
		case LEX_ANDEQUAL : return _T("&=");
		case LEX_ANDAND : return _T("&&");
		case LEX_OREQUAL : return _T("|=");
		case LEX_OROR : return _T("||");
		case LEX_XOREQUAL : return _T("^=");
				// reserved words
		case LEX_R_IF : return _T("if");
		case LEX_R_ELSE : return _T("else");
		case LEX_R_DO : return _T("do");
		case LEX_R_WHILE : return _T("while");
		case LEX_R_FOR : return _T("for");
		case LEX_R_BREAK : return _T("break");
		case LEX_R_CONTINUE : return _T("continue");
		case LEX_R_FUNCTION : return _T("function");
		case LEX_R_RETURN : return _T("return");
		case LEX_R_VAR : return _T("var");
		case LEX_R_TRUE : return _T("true");
		case LEX_R_FALSE : return _T("false");
		case LEX_R_NULL : return _T("null");
		case LEX_R_UNDEFINED : return _T("undefined");
		case LEX_R_NEW : return _T("new");
	}
	
	tostringstream msg;
	msg << _T("?[") << token << _T("]");

	return msg.str();
}

void CScriptLex::getNextCh()
{
	currCh = nextCh;

	if (dataPos < dataEnd)
		nextCh = data[dataPos];
	else
		nextCh = 0;

	dataPos++;
}

void CScriptLex::getNextToken()
{
	tk = LEX_EOF;
	tkStr.clear();

	while (currCh && isWhitespace(currCh)) getNextCh();

	// newline comments
	if ((currCh == _T('/')) && (nextCh == _T('/')))
	{
		while (currCh && (currCh != _T('\n')))
			getNextCh();

		getNextCh();
		getNextToken();

		return;
	}

	// block comments
	if ((currCh == _T('/')) && (nextCh == _T('*')))
	{
		while (currCh && ((currCh != _T('*')) || (nextCh != _T('/')))) getNextCh();

		getNextCh();
		getNextCh();
		getNextToken();

		return;
	}

	// record beginning of this token
	tokenStart = dataPos - 2;

	// tokens
	if (isAlpha(currCh))
	{ //  IDs
		while (isAlpha(currCh) || isNumeric(currCh))
		{
			tkStr += currCh;
			getNextCh();
		}

		tk = LEX_ID;

		if (tkStr == _T("if")) tk = LEX_R_IF;
		else if (tkStr == _T("else")) tk = LEX_R_ELSE;
		else if (tkStr == _T("do")) tk = LEX_R_DO;
		else if (tkStr == _T("while")) tk = LEX_R_WHILE;
		else if (tkStr == _T("for")) tk = LEX_R_FOR;
		else if (tkStr == _T("break")) tk = LEX_R_BREAK;
		else if (tkStr == _T("continue")) tk = LEX_R_CONTINUE;
		else if (tkStr == _T("function")) tk = LEX_R_FUNCTION;
		else if (tkStr == _T("return")) tk = LEX_R_RETURN;
		else if (tkStr == _T("var")) tk = LEX_R_VAR;
		else if (tkStr == _T("true")) tk = LEX_R_TRUE;
		else if (tkStr == _T("false")) tk = LEX_R_FALSE;
		else if (tkStr == _T("null")) tk = LEX_R_NULL;
		else if (tkStr == _T("undefined")) tk = LEX_R_UNDEFINED;
		else if (tkStr == _T("new")) tk = LEX_R_NEW;
	}
	else if (isNumeric(currCh))
	{ // Numbers
		bool isHex = false;
		if (currCh == _T('0'))
		{
			tkStr += currCh; getNextCh();
		}

		if (currCh == _T('x'))
		{
		  isHex = true;
		  tkStr += currCh; getNextCh();
		}

		tk = LEX_INT;
		while (isNumeric(currCh) || (isHex && isHexadecimal(currCh)))
		{
			tkStr += currCh;
			getNextCh();
		}

		if (!isHex && (currCh == _T('.')))
		{
			tk = LEX_FLOAT;
			tkStr += _T('.');
			getNextCh();

			while (isNumeric(currCh))
			{
				tkStr += currCh;
				getNextCh();
			}
		}

		// do fancy e-style floating point
		if (!isHex && ((currCh == _T('e')) || (currCh == _T('E'))))
		{
			tk = LEX_FLOAT;
			tkStr += currCh;
			getNextCh();

			if (currCh == _T('-'))
			{
				tkStr += currCh;
				getNextCh();
			}

			while (isNumeric(currCh))
			{
			   tkStr += currCh; getNextCh();
			}
		}
	}
	else if (currCh == _T('"'))
	{
		// strings...
		getNextCh();
		while (currCh && (currCh != _T('"')))
		{
			if (currCh == _T('\\'))
			{
				getNextCh();
				switch (currCh)
				{
					case _T('n'): tkStr += _T('\n'); break;
					case _T('"'): tkStr += _T('"'); break;
					case _T('\\'): tkStr += _T('\\'); break;
					default: tkStr += currCh;
				}
			}
			else
			{
				tkStr += currCh;
			}

			getNextCh();
		}

		getNextCh();
		tk = LEX_STR;
	}
	else if (currCh == _T('\''))
	{
		// strings again...
		getNextCh();
		while (currCh && (currCh != _T('\'')))
		{
			if (currCh == _T('\\'))
			{
				getNextCh();
				switch (currCh)
				{
					case _T('n'): tkStr += _T('\n'); break;
					case _T('a'): tkStr += _T('\a'); break;
					case _T('r'): tkStr += _T('\r'); break;
					case _T('t'): tkStr += _T('\t'); break;
					case _T('b'): tkStr += _T('\b'); break;
					case _T('\''): tkStr += _T('\''); break;
					case _T('\\'): tkStr += _T('\\'); break;
					case _T('x'):
					{ // hex digits
						TCHAR buf[3] = _T("??");

						getNextCh();
						buf[0] = currCh;

						getNextCh();
						buf[1] = currCh;

						tkStr += (TCHAR)_tcstol(buf, 0, 16);
						break;
					}

					default:
					{
						if ((currCh >= _T('0')) && (currCh <= _T('7')))
						{
							// octal digits
							TCHAR buf[4] = _T("???");
							buf[0] = currCh;

							getNextCh();
							buf[1] = currCh;

							getNextCh();
							buf[2] = currCh;

							tkStr += (TCHAR)_tcstol(buf, 0, 8);
						}
						else
							tkStr += currCh;
					}
				}
			}
			else
			{
				tkStr += currCh;
			}

			getNextCh();
		}

		getNextCh();
		tk = LEX_STR;
	}
	else
	{
		// single chars
		tk = currCh;
		if (currCh)
			getNextCh();

		if ((tk == _T('=')) && (currCh == _T('='))) // ==
		{
			tk = LEX_EQUAL;
			getNextCh();
			if (currCh == _T('=')) // ===
			{
				tk = LEX_TYPEEQUAL;
				getNextCh();
			}
		}
		else if ((tk == _T('!')) && (currCh == _T('='))) // !=
		{
			tk = LEX_NEQUAL;
			getNextCh();

			if (currCh == _T('=')) // !==
			{
				tk = LEX_NTYPEEQUAL;
				getNextCh();
			}
		}
		else if ((tk == _T('<')) && (currCh == _T('=')))
		{
			tk = LEX_LEQUAL;
			getNextCh();
		}
		else if ((tk == _T('<')) && (currCh == _T('<')))
		{
			tk = LEX_LSHIFT;
			getNextCh();
			if (currCh == _T('=')) // <<=
			{
				tk = LEX_LSHIFTEQUAL;
				getNextCh();
			}
		}
		else if ((tk == _T('>')) && (currCh == _T('=')))
		{
			tk = LEX_GEQUAL;
			getNextCh();
		}
		else if ((tk == _T('>')) && (currCh == _T('>')))
		{
			tk = LEX_RSHIFT;
			getNextCh();

			if (currCh == _T('=')) // >>=
			{
				tk = LEX_RSHIFTEQUAL;
				getNextCh();
			}
			else if (currCh == _T('>')) // >>>
			{
				tk = LEX_RSHIFTUNSIGNED;
				getNextCh();
			}
		}
		else if ((tk == _T('+')) && (currCh == _T('=')))
		{
			tk = LEX_PLUSEQUAL;
			getNextCh();
		}
		else if ((tk == _T('-')) && (currCh == _T('=')))
		{
			tk = LEX_MINUSEQUAL;
			getNextCh();
		}
		else if ((tk == _T('+')) && (currCh == _T('+')))
		{
			tk = LEX_PLUSPLUS;
			getNextCh();
		}
		else if ((tk == _T('-')) && (currCh == _T('-')))
		{
			tk = LEX_MINUSMINUS;
			getNextCh();
		}
		else if ((tk == _T('&')) && (currCh == _T('=')))
		{
			tk = LEX_ANDEQUAL;
			getNextCh();
		}
		else if ((tk == _T('&')) && (currCh == _T('&')))
		{
			tk = LEX_ANDAND;
			getNextCh();
		}
		else if ((tk == _T('|')) && (currCh == _T('=')))
		{
			tk = LEX_OREQUAL;
			getNextCh();
		}
		else if ((tk == _T('|')) && (currCh == _T('|')))
		{
			tk = LEX_OROR;
			getNextCh();
		}
		else if ((tk == _T('^')) && (currCh == _T('=')))
		{
			tk = LEX_XOREQUAL;
			getNextCh();
		}
	}

	/* This isn't quite right yet */
	tokenLastEnd = tokenEnd;
	tokenEnd = dataPos - 3;
}

tstring CScriptLex::getSubString(int64_t lastPosition)
{
	int64_t lastCharIdx = tokenLastEnd + 1;

	if (lastCharIdx < dataEnd)
	{
		// save a memory alloc by using our data array to create the substring
		TCHAR old = data[lastCharIdx];
		data[lastCharIdx] = 0;

		tstring value = &data[lastPosition];
		data[lastCharIdx] = old;

		return value;
	}
	else
	{
		return &data[lastPosition];
	}
}


CScriptLex *CScriptLex::getSubLex(int64_t lastPosition)
{
	int64_t lastCharIdx = tokenLastEnd + 1;

	if (lastCharIdx < dataEnd)
		return new CScriptLex(this, lastPosition, lastCharIdx);
	else
		return new CScriptLex(this, lastPosition, dataEnd);
}

tstring CScriptLex::getPosition(int64_t pos)
{
	if (pos < 0)
		pos = tokenLastEnd;

	int64_t line = 1, col = 1;

	for (int64_t i=0; i < pos; i++)
	{
		TCHAR ch;
		if (i < dataEnd)
			ch = data[i];
		else
			ch = 0;

		col++;

		if (ch == _T('\n'))
		{
			line++;
			col = 0;
		}
	}

	TCHAR buf[256];
	_stprintf_s(buf, 256, _T("(line: %" PRId64 ", col: %" PRId64 ")"), line, col);

	return buf;
}

// ----------------------------------------------------------------------------------- CSCRIPTVARLINK

CScriptVarLink::CScriptVarLink(CScriptVar *var, const tstring &name)
{
#if DEBUG_MEMORY
	mark_allocated(this);
#endif

	this->name = name;
	this->nextSibling = nullptr;
	this->prevSibling = nullptr;
	this->var = var->ref();
	this->owned = false;
}

CScriptVarLink::CScriptVarLink(const CScriptVarLink &link)
{
	// Copy constructor
#if DEBUG_MEMORY
	mark_allocated(this);
#endif

	this->name = link.name;
	this->nextSibling = 0;
	this->prevSibling = 0;
	this->var = link.var->ref();
	this->owned = false;
}

CScriptVarLink::~CScriptVarLink()
{
#if DEBUG_MEMORY
	mark_deallocated(this);
#endif

	var->unref();
}

void CScriptVarLink::replaceWith(CScriptVar *newVar)
{
	CScriptVar *oldVar = var;
	var = newVar->ref();
	oldVar->unref();
}

void CScriptVarLink::replaceWith(CScriptVarLink *newVar)
{
	if (newVar)
		replaceWith(newVar->var);
	else
		replaceWith(new CScriptVar());
}

int64_t CScriptVarLink::getIntName()
{
	return _ttoi(name.c_str());
}

void CScriptVarLink::setIntName(int64_t n)
{
	TCHAR sIdx[64];
	_stprintf_s(sIdx, sizeof(sIdx), _T("%" PRId64), n);
	name = sIdx;
}

// ----------------------------------------------------------------------------------- CSCRIPTVAR

CScriptVar::CScriptVar()
{
	refs = 0;
#if DEBUG_MEMORY
	mark_allocated(this);
#endif
	init();
	flags = SCRIPTVAR_UNDEFINED;
}

CScriptVar::CScriptVar(const tstring &str)
{
	refs = 0;

#if DEBUG_MEMORY
	mark_allocated(this);
#endif

	init();
	flags = SCRIPTVAR_STRING;
	data = str;
}


CScriptVar::CScriptVar(const tstring &varData, int64_t varFlags)
{
	refs = 0;

#if DEBUG_MEMORY
	mark_allocated(this);
#endif

	init();
	flags = varFlags;

	if (varFlags & SCRIPTVAR_INTEGER)
	{
		intData = _tcstol(varData.c_str(), 0, 0);
	}
	else if (varFlags & SCRIPTVAR_DOUBLE)
	{
		doubleData = _tcstod(varData.c_str(), 0);
	}
	else
		data = varData;
}

CScriptVar::CScriptVar(double val)
{
	refs = 0;

#if DEBUG_MEMORY
	mark_allocated(this);
#endif

	init();
	setDouble(val);
}

CScriptVar::CScriptVar(int64_t val)
{
	refs = 0;

#if DEBUG_MEMORY
	mark_allocated(this);
#endif

	init();
	setInt(val);
}

CScriptVar::~CScriptVar(void)
{
#if DEBUG_MEMORY
	mark_deallocated(this);
#endif

	removeAllChildren();
}

void CScriptVar::init()
{
	firstChild = nullptr;
	lastChild = nullptr;
	flags = 0;
	jsCallback = nullptr;
	jsCallbackUserData = 0;
	data = TINYJS_BLANK_DATA;
	intData = 0;
	doubleData = 0;
}

CScriptVar *CScriptVar::getReturnVar()
{
	return getParameter(TINYJS_RETURN_VAR);
}

void CScriptVar::setReturnVar(CScriptVar *var)
{
	findChildOrCreate(TINYJS_RETURN_VAR)->replaceWith(var);
}


CScriptVar *CScriptVar::getParameter(const tstring &name)
{
	return findChildOrCreate(name)->var;
}

CScriptVarLink *CScriptVar::findChild(const tstring &childName)
{
	CScriptVarLink *v = firstChild;
	while (v)
	{
		if (v->name.compare(childName) == 0)
			return v;

		v = v->nextSibling;
	}
	return 0;
}

CScriptVarLink *CScriptVar::findChildOrCreate(const tstring &childName, int64_t varFlags)
{
	CScriptVarLink *l = findChild(childName);
	if (l)
		return l;

	return addChild(childName, new CScriptVar(TINYJS_BLANK_DATA, varFlags));
}

CScriptVarLink *CScriptVar::findChildOrCreateByPath(const tstring &path)
{
	size_t p = path.find(_T('.'));
	if (p == tstring::npos)
		return findChildOrCreate(path);

	return findChildOrCreate(path.substr(0, p), SCRIPTVAR_OBJECT)->var->findChildOrCreateByPath(path.substr(p + 1));
}

CScriptVarLink *CScriptVar::addChild(const tstring &childName, CScriptVar *child)
{
	if (isUndefined())
	{
	  flags = SCRIPTVAR_OBJECT;
	}

	// if no child supplied, create one
	if (!child)
	  child = new CScriptVar();

	CScriptVarLink *link = new CScriptVarLink(child, childName);
	link->owned = true;

	if (lastChild)
	{
		lastChild->nextSibling = link;
		link->prevSibling = lastChild;
		lastChild = link;
	}
	else
	{
		firstChild = link;
		lastChild = link;
	}

	return link;
}

CScriptVarLink *CScriptVar::addChildNoDup(const tstring &childName, CScriptVar *child)
{
	// if no child supplied, create one
	if (!child)
		child = new CScriptVar();
	
	CScriptVarLink *v = findChild(childName);
	if (v)
	{
		v->replaceWith(child);
	}
	else
	{
		v = addChild(childName, child);
	}
	
	return v;
}

void CScriptVar::removeChild(CScriptVar *child)
{
	CScriptVarLink *link = firstChild;
	
	while (link)
	{
		if (link->var == child)
			break;
	
		link = link->nextSibling;
	}
	
	removeLink(link);
}

void CScriptVar::removeLink(CScriptVarLink *link)
{
	if (!link)
		return;

	if (link->nextSibling)
		link->nextSibling->prevSibling = link->prevSibling;

	if (link->prevSibling)
		link->prevSibling->nextSibling = link->nextSibling;

	if (lastChild == link)
		lastChild = link->prevSibling;

	if (firstChild == link)
		firstChild = link->nextSibling;

	delete link;
}

void CScriptVar::removeAllChildren()
{
	CScriptVarLink *c = firstChild;
	while (c)
	{
		CScriptVarLink *t = c->nextSibling;
		delete c;
		c = t;
	}

	firstChild = 0;
	lastChild = 0;
}

CScriptVar *CScriptVar::getArrayIndex(int64_t idx)
{
	TCHAR sIdx[64];
	_stprintf_s(sIdx, sizeof(sIdx), _T("%" PRId64), idx);

	CScriptVarLink *link = findChild(sIdx);
	if (link)
		return link->var;
	else
		return new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_NULL); // undefined
}

void CScriptVar::setArrayIndex(int64_t idx, CScriptVar *value)
{
	TCHAR sIdx[64];
	_stprintf_s(sIdx, sizeof(sIdx), _T("%" PRId64), idx);
	CScriptVarLink *link = findChild(sIdx);
	
	if (link)
	{
		if (value->isUndefined())
			removeLink(link);
		else
			link->replaceWith(value);
	}
	else
	{
		if (!value->isUndefined())
			addChild(sIdx, value);
	}
}

int64_t CScriptVar::getArrayLength()
{
	int64_t highest = -1;
	if (!isArray())
		return 0;

	CScriptVarLink *link = firstChild;
	while (link)
	{
		if (isNumber(link->name))
		{
			int64_t val = _ttoi(link->name.c_str());
			if (val > highest)
				highest = val;
		}

		link = link->nextSibling;
	}

	return highest + 1;
}

int64_t CScriptVar::getChildren()
{
	int64_t n = 0;

	CScriptVarLink *link = firstChild;
	while (link)
	{
		n++;
		link = link->nextSibling;
	}

	return n;
}

int64_t CScriptVar::getInt()
{
	/* strtol understands about hex and octal */
	if (isInt())
		return intData;

	if (isNull())
		return 0;

	if (isUndefined())
		return 0;

	if (isDouble())
		return (int64_t)doubleData;

	return 0;
}

double CScriptVar::getDouble()
{
	if (isDouble())
		return doubleData;

	if (isInt())
		return (double)intData;

	if (isNull())
		return 0;

	if (isUndefined())
		return 0;

	return 0; /* or NaN? */
}

const tstring &CScriptVar::getString()
{
	/* Because we can't return a string that is generated on demand.
	 * I should really just use char* :) */
	static tstring s_null = _T("null");
	static tstring s_undefined = _T("undefined");

	tstringstream ss;

	if (isInt())
	{
		ss << intData;
		data = ss.str();
		return data;
	}
	
	if (isDouble())
	{
		ss << doubleData;
		data = ss.str();
		return data;
	}
	
	if (isNull())
		return s_null;
	
	if (isUndefined())
		return s_undefined;
	
	// are we just a string here?
	return data;
}

void CScriptVar::setInt(int64_t val)
{
	flags = (flags & ~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_INTEGER;
	intData = val;
	doubleData = 0;
	data = TINYJS_BLANK_DATA;
}

void CScriptVar::setDouble(double val)
{
	flags = (flags & ~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_DOUBLE;
	doubleData = val;
	intData = 0;
	data = TINYJS_BLANK_DATA;
}

void CScriptVar::setString(const tstring &str)
{
	// name sure it's not still a number or integer
	flags = (flags&~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_STRING;
	data = str;
	intData = 0;
	doubleData = 0;
}

void CScriptVar::setUndefined()
{
	// name sure it's not still a number or integer
	flags = (flags & ~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_UNDEFINED;
	data = TINYJS_BLANK_DATA;
	intData = 0;
	doubleData = 0;
	removeAllChildren();
}

void CScriptVar::setArray()
{
	// name sure it's not still a number or integer
	flags = (flags & ~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_ARRAY;
	data = TINYJS_BLANK_DATA;
	intData = 0;
	doubleData = 0;
	removeAllChildren();
}

bool CScriptVar::equals(CScriptVar *v)
{
	CScriptVar *resV = mathsOp(v, LEX_EQUAL);
	bool res = resV->getBool();
	delete resV;
	return res;
}

CScriptVar *CScriptVar::mathsOp(CScriptVar *b, int64_t op)
{
	CScriptVar *a = this;

	// Type equality check
	if ((op == LEX_TYPEEQUAL) || (op == LEX_NTYPEEQUAL))
	{
		// check type first, then call again to check data
		bool eql = ((a->flags & SCRIPTVAR_VARTYPEMASK) == (b->flags & SCRIPTVAR_VARTYPEMASK));

		if (eql)
		{
			CScriptVar *contents = a->mathsOp(b, LEX_EQUAL);

			if (!contents->getBool())
				eql = false;

			if (!contents->refs)
				delete contents;
		}

		if (op == LEX_TYPEEQUAL)
			return new CScriptVar(eql ? 1ll : 0ll);
		else
			return new CScriptVar(!eql ? 0ll : 1ll);
	}

	// do maths...
	if (a->isUndefined() && b->isUndefined())
	{
		if (op == LEX_EQUAL)
			return new CScriptVar(1ll);
		else if (op == LEX_NEQUAL)
			return new CScriptVar(0ll);
		else
			return new CScriptVar(); // undefined
	}
	else if ((a->isNumeric() || a->isUndefined()) && (b->isNumeric() || b->isUndefined()))
	{
		if (!a->isDouble() && !b->isDouble())
		{
			// use ints
			int64_t da = a->getInt();
			int64_t db = b->getInt();

			switch (op)
			{
				case _T('+'):		return new CScriptVar(da + db);
				case _T('-'):		return new CScriptVar(da - db);
				case _T('*'):		return new CScriptVar(da * db);
				case _T('/'):		return new CScriptVar(da / db);
				case _T('&'):		return new CScriptVar(da & db);
				case _T('|'):		return new CScriptVar(da | db);
				case _T('^'):		return new CScriptVar(da ^ db);
				case _T('%'):		return new CScriptVar(da % db);
				case LEX_EQUAL:		return new CScriptVar((da == db) ? 1ll : 0ll);
				case LEX_NEQUAL:	return new CScriptVar((da != db) ? 1ll : 0ll);
				case _T('<'):		return new CScriptVar((da < db) ? 1ll : 0ll);
				case LEX_LEQUAL:	return new CScriptVar((da <= db) ? 1ll : 0ll);
				case _T('>'):		return new CScriptVar((da > db) ? 1ll : 0ll);
				case LEX_GEQUAL:	return new CScriptVar((da >= db) ? 1ll : 0ll);
				default:			throw new CScriptException(_T("Operation ") + CScriptLex::getTokenStr(op) + _T(" not supported on the Int datatype"));
			}
		}
		else
		{
			// use doubles
			double da = a->getDouble();
			double db = b->getDouble();

			switch (op)
			{
				case _T('+'):		return new CScriptVar(da + db);
				case _T('-'):		return new CScriptVar(da - db);
				case _T('*'):		return new CScriptVar(da * db);
				case _T('/'):		return new CScriptVar(da / db);
				case LEX_EQUAL:		return new CScriptVar((da == db) ? 1ll : 0ll);
				case LEX_NEQUAL:	return new CScriptVar((da != db) ? 1ll : 0ll);
				case _T('<'):		return new CScriptVar((da < db) ? 1ll : 0ll);
				case LEX_LEQUAL:	return new CScriptVar((da <= db) ? 1ll : 0ll);
				case _T('>'):		return new CScriptVar((da > db) ? 1ll : 0ll);
				case LEX_GEQUAL:	return new CScriptVar((da >= db) ? 1ll : 0ll);
				default:			throw new CScriptException(_T("Operation ") + CScriptLex::getTokenStr(op) + _T(" not supported on the Double datatype"));
			}
		}
	}
	else if (a->isArray())
	{
		/* Just check pointers */
		switch (op)
		{
			case LEX_EQUAL:		return new CScriptVar((a == b) ? 1ll : 0ll);
			case LEX_NEQUAL:	return new CScriptVar((a != b) ? 1ll : 0ll);
			default:			throw new CScriptException(_T("Operation ") + CScriptLex::getTokenStr(op) + _T(" not supported on the Array datatype"));
		}
	}
	else if (a->isObject())
	{
		/* Just check pointers */
		switch (op)
		{
			case LEX_EQUAL:		return new CScriptVar((a == b) ? 1ll : 0ll);
			case LEX_NEQUAL:	return new CScriptVar((a != b) ? 1ll : 0ll);
			default:			throw new CScriptException(_T("Operation ") + CScriptLex::getTokenStr(op) + _T(" not supported on the Object datatype"));
		}
	}
	else
	{
		tstring da = a->getString();
		tstring db = b->getString();

		// use strings
		switch (op)
		{
			case _T('+'):		return new CScriptVar(da + db, SCRIPTVAR_STRING);
			case LEX_EQUAL:		return new CScriptVar((da==db) ? 1ll : 0ll);
			case LEX_NEQUAL:	return new CScriptVar((da!=db) ? 1ll : 0ll);
			case _T('<'):		return new CScriptVar((da<db) ? 1ll : 0ll);
			case LEX_LEQUAL:	return new CScriptVar((da<=db) ? 1ll : 0ll);
			case _T('>'):		return new CScriptVar((da>db) ? 1ll : 0ll);
			case LEX_GEQUAL:	return new CScriptVar((da>=db) ? 1ll : 0ll);
			default:			throw new CScriptException(_T("Operation ") + CScriptLex::getTokenStr(op) + _T(" not supported on the string datatype"));
		}
	}

	return 0;
}

void CScriptVar::copySimpleData(CScriptVar *val)
{
	data = val->data;
	intData = val->intData;
	doubleData = val->doubleData;
	flags = (flags & ~SCRIPTVAR_VARTYPEMASK) | (val->flags & SCRIPTVAR_VARTYPEMASK);
}

void CScriptVar::copyValue(CScriptVar *val)
{
	if (val)
	{
		copySimpleData(val);

		// remove all current children
		removeAllChildren();

		// copy children of 'val'
		CScriptVarLink *child = val->firstChild;
		while (child)
		{
			CScriptVar *copied;

			// don't copy the 'parent' object...
			if (child->name != TINYJS_PROTOTYPE_CLASS)
				copied = child->var->deepCopy();
			else
				copied = child->var;

			addChild(child->name, copied);

			child = child->nextSibling;
		}
	}
	else
	{
		setUndefined();
	}
}

CScriptVar *CScriptVar::deepCopy()
{
	CScriptVar *newVar = new CScriptVar();
	newVar->copySimpleData(this);

	// copy children
	CScriptVarLink *child = firstChild;
	while (child)
	{
		CScriptVar *copied;
		// don't copy the 'parent' object...
		if (child->name != TINYJS_PROTOTYPE_CLASS)
			copied = child->var->deepCopy();
		else
			copied = child->var;

		newVar->addChild(child->name, copied);
		child = child->nextSibling;
	}

	return newVar;
}

void CScriptVar::trace(tstring indentStr, const tstring &name)
{
	_tprintf_s(_T("%s'%s' = '%s' %s\n"), indentStr.c_str(), name.c_str(), getString().c_str(), getFlagsAsString().c_str());

	tstring indent = indentStr + _T(" ");
	CScriptVarLink *link = firstChild;

	while (link)
	{
		link->var->trace(indent, link->name);
		link = link->nextSibling;
	}
}

tstring CScriptVar::getFlagsAsString()
{
	tstring flagstr = _T("");

	if (flags & SCRIPTVAR_FUNCTION)
		flagstr = flagstr + _T("FUNCTION ");

	if (flags & SCRIPTVAR_OBJECT)
		flagstr = flagstr + _T("OBJECT ");

	if (flags & SCRIPTVAR_ARRAY)
		flagstr = flagstr + _T("ARRAY ");

	if (flags & SCRIPTVAR_NATIVE)
		flagstr = flagstr + _T("NATIVE ");

	if (flags & SCRIPTVAR_DOUBLE)
		flagstr = flagstr + _T("DOUBLE ");

	if (flags & SCRIPTVAR_INTEGER)
		flagstr = flagstr + _T("INTEGER ");

	if (flags & SCRIPTVAR_STRING)
		flagstr = flagstr + _T("STRING ");

	return flagstr;
}

tstring CScriptVar::getParsableString()
{
	// Numbers can just be put in directly
	if (isNumeric())
		return getString();

	if (isFunction())
	{
		tostringstream funcStr;
		funcStr << _T("function (");

		// get list of parameters
		CScriptVarLink *link = firstChild;
		while (link)
		{
			funcStr << link->name;

			if (link->nextSibling)
				funcStr << _T(",");

			link = link->nextSibling;
		}

		// add function body
		funcStr << _T(") ") << getString();

		return funcStr.str();
	}

	// if it is a string then we quote it
	if (isString())
		return getJSString(getString());

	if (isNull())
		return _T("null");

	return _T("undefined");
}

void CScriptVar::getJSON(tostringstream &destination, const tstring linePrefix)
{
	if (isObject())
	{
		tstring indentedLinePrefix = linePrefix + _T("  ");

		// children - handle with bracketed list
		destination << _T("{ \n");

		CScriptVarLink *link = firstChild;
		while (link)
		{
			destination << indentedLinePrefix;
			destination  << getJSString(link->name);
			destination  << " : ";

			link->var->getJSON(destination, indentedLinePrefix);
			link = link->nextSibling;

			if (link)
			{
				destination  << ",\n";
			}
		}

		destination << _T("\n") << linePrefix << _T("}");
	}
	else if (isArray())
	{
		tstring indentedLinePrefix = linePrefix + _T("  ");
		destination << _T("[\n");

		int64_t len = getArrayLength();
		if (len > 10000)
			len = 10000; // we don't want to get stuck here!

		for (int64_t i = 0; i < len; i++)
		{
			getArrayIndex(i)->getJSON(destination, indentedLinePrefix);

			if (i < (len - 1))
				destination  << _T(",\n");
		}

		destination << _T("\n") << linePrefix << _T("]");
	}
	else
	{
	  // no children or a function... just write value directly
	  destination << getParsableString();
	}
}


void CScriptVar::setCallback(JSCallback callback, void *userdata)
{
	jsCallback = callback;
	jsCallbackUserData = userdata;
}

CScriptVar *CScriptVar::ref()
{
	refs++;
	return this;
}

void CScriptVar::unref()
{
	if (refs <= 0)
		_tprintf(_T("We have unreffed too far!\n"));

	if ((--refs) == 0)
	{
		delete this;
	}
}

int64_t CScriptVar::getRefs()
{
	return refs;
}


// ----------------------------------------------------------------------------------- CSCRIPT

CTinyJS::CTinyJS()
{
	l = 0;
	root = (new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT))->ref();

	// Add built-in classes
	stringClass = (new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT))->ref();
	arrayClass = (new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT))->ref();
	objectClass = (new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT))->ref();

	root->addChild(_T("String"), stringClass);
	root->addChild(_T("Array"), arrayClass);
	root->addChild(_T("Object"), objectClass);
}

CTinyJS::~CTinyJS()
{
	scopes.clear();

	stringClass->unref();
	arrayClass->unref();
	objectClass->unref();
	root->unref();
	
#if DEBUG_MEMORY
	show_allocated();
#endif
}

void CTinyJS::trace()
{
	root->trace();
}

void CTinyJS::execute(const tstring &code)
{
	CScriptLex *oldLex = l;
	vector<CScriptVar *> oldScopes = scopes;
	l = new CScriptLex(code);

#ifdef TINYJS_CALL_STACK
	call_stack.clear();
#endif
	scopes.clear();
	scopes.push_back(root);

	try
	{
		bool execute = true;
		while (l->tk)
			statement(execute);
	}
	catch (CScriptException *e)
	{
		tstringstream msg;
		msg << _T("Error ") << e->text;

#ifdef TINYJS_CALL_STACK
		for (int64_t i = (int64_t)call_stack.size() - 1; i >= 0; i--)
			msg << _T("\n") << i << _T(": ") << call_stack.at(i);
#endif
		msg << _T(" at ") << l->getPosition();
		last_error = msg.str();

		delete l;
		l = oldLex;

		//throw new CScriptException(msg.str());
	}

	delete l;
	l = oldLex;
	scopes = oldScopes;
}

CScriptVarLink CTinyJS::evaluateComplex(const tstring &code)
{
	CScriptLex *oldLex = l;
	vector<CScriptVar*> oldScopes = scopes;
	
	l = new CScriptLex(code);
#ifdef TINYJS_CALL_STACK
	call_stack.clear();
#endif

	scopes.clear();
	scopes.push_back(root);

	CScriptVarLink *v = 0;

	try
	{
		bool execute = true;

		do
		{
			CLEAN(v);
			v = base(execute);

			if (l->tk != LEX_EOF)
				l->match(_T(';'));
		}
		while (l->tk != LEX_EOF);
	}
	catch (CScriptException *e)
	{
		tostringstream msg;
		msg << _T("Error ") << e->text;

#ifdef TINYJS_CALL_STACK
		for (int64_t i = (int64_t)call_stack.size() - 1; i >= 0; i--)
			msg << _T("\n") << i << _T(": ") << call_stack.at(i);
#endif
		msg << _T(" at ") << l->getPosition();

		delete l;
		l = oldLex;

		throw new CScriptException(msg.str());
	}

	delete l;
	l = oldLex;
	scopes = oldScopes;

	if (v)
	{
		CScriptVarLink r = *v;
		CLEAN(v);

		return r;
	}

	// return undefined...
	return CScriptVarLink(new CScriptVar());
}

tstring CTinyJS::evaluate(const tstring &code)
{
	return evaluateComplex(code).var->getString();
}

void CTinyJS::parseFunctionArguments(CScriptVar *funcVar)
{
	l->match(_T('('));

	while (l->tk != _T(')'))
	{
		funcVar->addChildNoDup(l->tkStr);
		l->match(LEX_ID);

		if (l->tk != _T(')'))
			l->match(_T(','));
	}

	l->match(_T(')'));
}

void CTinyJS::addNative(const tstring &funcDesc, JSCallback ptr, void *userdata)
{
	CScriptLex *oldLex = l;
	l = new CScriptLex(funcDesc);

	CScriptVar *base = root;

	l->match(LEX_R_FUNCTION);
	tstring funcName = l->tkStr;
	l->match(LEX_ID);

	// Check for dots, we might want to do something like function String.substring ...
	while (l->tk == _T('.'))
	{
		l->match(_T('.'));

		CScriptVarLink *link = base->findChild(funcName);

		// if it doesn't exist, make an object class
		if (!link)
			link = base->addChild(funcName, new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT));

		base = link->var;
		funcName = l->tkStr;
		l->match(LEX_ID);
	}

	CScriptVar *funcVar = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE);
	funcVar->setCallback(ptr, userdata);
	parseFunctionArguments(funcVar);

	delete l;
	l = oldLex;

	base->addChild(funcName, funcVar);
}

CScriptVarLink *CTinyJS::parseFunctionDefinition()
{
	// actually parse a function...
	l->match(LEX_R_FUNCTION);
	tstring funcName = TINYJS_TEMP_NAME;

	// we can have functions without names
	if (l->tk==LEX_ID)
	{
		funcName = l->tkStr;
		l->match(LEX_ID);
	}

	CScriptVarLink *funcVar = new CScriptVarLink(new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION), funcName);

	parseFunctionArguments(funcVar->var);
	int64_t funcBegin = l->tokenStart;
	bool noexecute = false;
	block(noexecute);
	funcVar->var->data = l->getSubString(funcBegin);

	return funcVar;
}

/** Handle a function call (assumes we've parsed the function name and we're
 * on the start bracket). 'parent' is the object that contains this method,
 * if there was one (otherwise it's just a normnal function).
 */
CScriptVarLink *CTinyJS::functionCall(bool &execute, CScriptVarLink *function, CScriptVar *parent)
{
	if (execute)
	{
		if (!function->var->isFunction())
		{
			tstring errorMsg = _T("Expecting '");
			errorMsg = errorMsg + function->name + _T("' to be a function");
			throw new CScriptException(errorMsg.c_str());
		}

		l->match(_T('('));

		// create a new symbol table entry for execution of this function
		CScriptVar *functionRoot = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION);
		if (parent)
			functionRoot->addChildNoDup(_T("this"), parent);

		// grab in all parameters
		CScriptVarLink *v = function->var->firstChild;
		while (v)
		{
			CScriptVarLink *value = base(execute);
			if (value)
			{
				if (execute)
				{
					if (value->var->isBasic())
					{
						// pass by value
						functionRoot->addChild(v->name, value->var->deepCopy());
					}
					else
					{
						// pass by reference
						functionRoot->addChild(v->name, value->var);
					}
				}

				CLEAN(value);
			}

			if (l->tk != _T(')'))
				l->match(_T(','));

			v = v->nextSibling;
		}

		l->match(_T(')'));

		// setup a return variable
		CScriptVarLink *returnVar = NULL;

		// execute function!
		// add the function's execute space to the symbol table so we can recurse
		CScriptVarLink *returnVarLink = functionRoot->addChild(TINYJS_RETURN_VAR);
		scopes.push_back(functionRoot);

		#ifdef TINYJS_CALL_STACK
		call_stack.push_back(function->name + _T(" from ") + l->getPosition());
		#endif

		if (function->var->isNative())
		{
			function->var->jsCallback(functionRoot, function->var->jsCallbackUserData);
		}
		else
		{
			/* we just want to execute the block, but something could
			 * have messed up and left us with the wrong ScriptLex, so
			 * we want to be careful here... */
			CScriptException *exception = 0;
			CScriptLex *oldLex = l;
			CScriptLex *newLex = new CScriptLex(function->var->getString());

			l = newLex;
			try
			{
				block(execute);
				// because return will probably have called this, and set execute to false

				execute = true;
			}
			catch (CScriptException *e)
			{
				exception = e;
			}

			delete newLex;
			l = oldLex;

			if (exception)
				throw exception;
		}

#ifdef TINYJS_CALL_STACK
		if (!call_stack.empty()) call_stack.pop_back();
#endif
		{
			scopes.pop_back();
		}

		/* get the real return var before we remove it from our function */
		returnVar = new CScriptVarLink(returnVarLink->var);
		functionRoot->removeLink(returnVarLink);
		delete functionRoot;

		if (returnVar)
			return returnVar;
		else
			return new CScriptVarLink(new CScriptVar());
	}
	else 
	{
		// function, but not executing - just parse args and be done
		l->match(_T('('));
		while (l->tk != _T(')'))
		{
			CScriptVarLink *value = base(execute);
			CLEAN(value);
			if (l->tk != _T(')'))
			l->match(_T(','));
		}

		l->match(_T(')'));

		if (l->tk == _T('{')) // TODO: why is this here?
		{
			block(execute);
		}

		/* function will be a blank scriptvarlink if we're not executing,
		 * so just return it rather than an alloc/free */
		return function;
	}

	return nullptr;
}

CScriptVarLink *CTinyJS::factor(bool &execute)
{
	if (l->tk == _T('('))
	{
		l->match(_T('('));

		CScriptVarLink *a = base(execute);

		l->match(_T(')'));

		return a;
	}

	if (l->tk == LEX_R_TRUE)
	{
		l->match(LEX_R_TRUE);
		return new CScriptVarLink(new CScriptVar(1ll));
	}

	if (l->tk == LEX_R_FALSE)
	{
		l->match(LEX_R_FALSE);
		return new CScriptVarLink(new CScriptVar(0ll));
	}

	if (l->tk == LEX_R_NULL)
	{
		l->match(LEX_R_NULL);
		return new CScriptVarLink(new CScriptVar(TINYJS_BLANK_DATA,SCRIPTVAR_NULL));
	}

	if (l->tk == LEX_R_UNDEFINED)
	{
		l->match(LEX_R_UNDEFINED);
		return new CScriptVarLink(new CScriptVar(TINYJS_BLANK_DATA,SCRIPTVAR_UNDEFINED));
	}

	if (l->tk == LEX_ID)
	{
		CScriptVarLink *a = execute ? findInScopes(l->tkStr) : new CScriptVarLink(new CScriptVar());

		// The parent if we're executing a method call
		CScriptVar *parent = 0;

		if (execute && !a)
		{
		  // Variable doesn't exist! JavaScript says we should create it
		  // (we won't add it here. This is done in the assignment operator)
		  a = new CScriptVarLink(new CScriptVar(), l->tkStr);
		}

		l->match(LEX_ID);

		while ((l->tk == _T('(')) || (l->tk == _T('.')) || (l->tk == _T('[')))
		{
			if (l->tk == _T('(')) // ------------------------------------- Function Call
			{
				a = functionCall(execute, a, parent);
			}
			else if (l->tk == _T('.')) // ------------------------------------- Record Access
			{
				l->match(_T('.'));
				if (execute)
				{
					const tstring &name = l->tkStr;

					CScriptVarLink *child = a->var->findChild(name);

					if (!child)
						child = findInParentClasses(a->var, name);

					if (!child)
					{
						// if we haven't found this defined yet, use the built-in 'length' properly
						if (a->var->isArray() && (name == _T("length")))
						{
							int64_t l = a->var->getArrayLength();
							child = new CScriptVarLink(new CScriptVar(l));
						}
						else if (a->var->isString() && (name == _T("length")))
						{
							int64_t l = a->var->getString().size();
							child = new CScriptVarLink(new CScriptVar(l));
						}
						else
						{
							child = a->var->addChild(name);
						}
					}

					parent = a->var;
					a = child;
				}

				l->match(LEX_ID);
			}
			else if (l->tk == _T('[')) // ------------------------------------- Array Access
			{
				l->match(_T('['));

				CScriptVarLink *index = base(execute);

				l->match(_T(']'));

				if (execute)
				{
					CScriptVarLink *child = a->var->findChildOrCreate(index->var->getString());
					parent = a->var;
					a = child;
				}

				CLEAN(index);
			}
		}

		return a;
	}

	if ((l->tk == LEX_INT) || (l->tk == LEX_FLOAT))
	{
		CScriptVar *a = new CScriptVar(l->tkStr, ((l->tk == LEX_INT) ? SCRIPTVAR_INTEGER : SCRIPTVAR_DOUBLE));
		l->match(l->tk);
	
		return new CScriptVarLink(a);
	}

	if (l->tk == LEX_STR)
	{
		CScriptVar *a = new CScriptVar(l->tkStr, SCRIPTVAR_STRING);
		l->match(LEX_STR);
	
		return new CScriptVarLink(a);
	}

	if (l->tk == _T('{'))
	{
		CScriptVar *contents = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);

		// JSON-style object definition
		l->match(_T('{'));

		while (l->tk != _T('}'))
		{
			tstring id = l->tkStr;

			// we only allow strings or IDs on the left hand side of an initialisation
			if (l->tk == LEX_STR)
				l->match(LEX_STR);
			else
				l->match(LEX_ID);

			l->match(_T(':'));

			if (execute)
			{
				CScriptVarLink *a = base(execute);
				contents->addChild(id, a->var);
				CLEAN(a);
			}

			// no need to clean here, as it will definitely be used
				if (l->tk != _T('}'))
					l->match(_T(','));
		}

		l->match(_T('}'));
		return new CScriptVarLink(contents);
	}

	if (l->tk == _T('['))
	{
		CScriptVar *contents = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_ARRAY);

		// JSON-style array
		l->match(_T('['));

		int64_t idx = 0;

		while (l->tk != _T(']'))
		{
			if (execute)
			{
				TCHAR idx_str[16]; // big enough for 2^32
				_stprintf_s(idx_str, sizeof(idx_str), _T("%" PRId64), idx);

				CScriptVarLink *a = base(execute);
				contents->addChild(idx_str, a->var);

				CLEAN(a);
			}

			// no need to clean here, as it will definitely be used
			if (l->tk != _T(']'))
				l->match(_T(','));

			idx++;
		}

		l->match(_T(']'));
		return new CScriptVarLink(contents);
	}

	if (l->tk == LEX_R_FUNCTION)
	{
		CScriptVarLink *funcVar = parseFunctionDefinition();

		if (funcVar->name != TINYJS_TEMP_NAME)
			_tprintf_s(_T("Functions not defined at statement-level are not meant to have a name"));

		return funcVar;
	}

	if (l->tk == LEX_R_NEW)
	{
		// new -> create a new object
		l->match(LEX_R_NEW);

		const tstring &className = l->tkStr;

		if (execute)
		{
			CScriptVarLink *objClassOrFunc = findInScopes(className);
			if (!objClassOrFunc)
			{
				_tprintf_s(_T("%s is not a valid class name"), className.c_str());
				return new CScriptVarLink(new CScriptVar());
			}

			l->match(LEX_ID);

			CScriptVar *obj = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
			CScriptVarLink *objLink = new CScriptVarLink(obj);

			if (objClassOrFunc->var->isFunction())
			{
				CLEAN(functionCall(execute, objClassOrFunc, obj));
			}
			else
			{
				obj->addChild(TINYJS_PROTOTYPE_CLASS, objClassOrFunc->var);

				if (l->tk == _T('('))
				{
					l->match(_T('('));
					l->match(_T(')'));
				}
			}

			return objLink;
		}
		else
		{
			l->match(LEX_ID);

			if (l->tk == _T('('))
			{
				l->match(_T('('));
				l->match(_T(')'));
			}
		}
	}

	// Nothing we can do here... just hope it's the end...
	l->match(LEX_EOF);

	return 0;
}

CScriptVarLink *CTinyJS::unary(bool &execute)
{
	CScriptVarLink *a = nullptr;

	if (l->tk == _T('!'))
	{
		l->match(_T('!')); // binary not
		a = factor(execute);

		if (execute)
		{
			CScriptVar zero(0ll);
			CScriptVar *res = a->var->mathsOp(&zero, LEX_EQUAL);

			CREATE_LINK(a, res);
		}
	}
	else
	{
		a = factor(execute);
	}

	return a;
}

CScriptVarLink *CTinyJS::term(bool &execute)
{
	CScriptVarLink *a = unary(execute);
	
	while ((l->tk == _T('*')) || (l->tk == _T('/')) || (l->tk == _T('%')))
	{
		int64_t op = l->tk;
		
		l->match(l->tk);
		
		CScriptVarLink *b = unary(execute);
		if (execute)
		{
			CScriptVar *res = a->var->mathsOp(b->var, op);
			CREATE_LINK(a, res);
		}
		
		CLEAN(b);
	}
	return a;
}

CScriptVarLink *CTinyJS::expression(bool &execute)
{
	bool negate = false;
	
	if (l->tk == _T('-'))
	{
		l->match(_T('-'));
		negate = true;
	}
	
	CScriptVarLink *a = term(execute);
	if (negate)
	{
		CScriptVar zero(0ll);
		CScriptVar *res = zero.mathsOp(a->var, _T('-'));
		CREATE_LINK(a, res);
	}
	
	while ((l->tk == _T('+')) || (l->tk == _T('-')) || (l->tk == LEX_PLUSPLUS) || (l->tk == LEX_MINUSMINUS))
	{
		int64_t op = l->tk;
		l->match(l->tk);
		
		if ((op == LEX_PLUSPLUS) || (op == LEX_MINUSMINUS))
		{
			if (execute)
			{
				CScriptVar one((int64_t)1);
				CScriptVar *res = a->var->mathsOp(&one, (op == LEX_PLUSPLUS) ? _T('+') : _T('-'));
				
				CScriptVarLink *oldValue = new CScriptVarLink(a->var);
				
				// in-place add/subtract
				a->replaceWith(res);
				
				CLEAN(a);
				
				a = oldValue;
			}
		}
		else
		{
			CScriptVarLink *b = term(execute);
			if (execute)
			{
				// not in-place, so just replace
				CScriptVar *res = a->var->mathsOp(b->var, op);
				CREATE_LINK(a, res);
			}

			CLEAN(b);
		}
	}
	
	return a;
}

CScriptVarLink *CTinyJS::shift(bool &execute)
{
	CScriptVarLink *a = expression(execute);
	if ((l->tk == LEX_LSHIFT) || (l->tk == LEX_RSHIFT) || (l->tk == LEX_RSHIFTUNSIGNED))
	{
		int64_t op = l->tk;
		l->match(op);

		CScriptVarLink *b = base(execute);
		int64_t shift = execute ? b->var->getInt() : 0;

		CLEAN(b);

		if (execute)
		{
			if (op == LEX_LSHIFT)
				a->var->setInt(a->var->getInt() << shift);

			if (op == LEX_RSHIFT)
				a->var->setInt(a->var->getInt() >> shift);

			if (op == LEX_RSHIFTUNSIGNED)
				a->var->setInt(((uint64_t)a->var->getInt()) >> shift);
		}
	}

	return a;
}

CScriptVarLink *CTinyJS::condition(bool &execute)
{
	CScriptVarLink *a = shift(execute);
	CScriptVarLink *b;

	while ((l->tk == LEX_EQUAL) || (l->tk == LEX_NEQUAL) ||
		(l->tk == LEX_TYPEEQUAL) || (l->tk == LEX_NTYPEEQUAL) ||
		(l->tk == LEX_LEQUAL) || (l->tk == LEX_GEQUAL) ||
		(l->tk == _T('<')) || (l->tk == _T('>')))
	{
		int64_t op = l->tk;

		l->match(l->tk);

		b = shift(execute);

		if (execute)
		{
			CScriptVar *res = a->var->mathsOp(b->var, op);
			CREATE_LINK(a,res);
		}

		CLEAN(b);
	}

	return a;
}

CScriptVarLink *CTinyJS::logic(bool &execute)
{
	CScriptVarLink *a = condition(execute);
	CScriptVarLink *b;

	while ((l->tk == _T('&')) || (l->tk == _T('|')) || (l->tk == _T('^')) || (l->tk == LEX_ANDAND) || (l->tk == LEX_OROR))
	{
		bool noexecute = false;
		int64_t op = l->tk;

		l->match(l->tk);

		bool shortCircuit = false;
		bool boolean = false;

		// if we have short-circuit ops, then if we know the outcome
		// we don't bother to execute the other op. Even if not
		// we need to tell mathsOp it's an & or |
		if (op == LEX_ANDAND)
		{
			op = _T('&');
			shortCircuit = !a->var->getBool();
			boolean = true;
		}
		else if (op == LEX_OROR)
		{
			op = _T('|');
			shortCircuit = a->var->getBool();
			boolean = true;
		}

		b = condition(shortCircuit ? noexecute : execute);
		if (execute && !shortCircuit)
		{
			if (boolean)
			{
				CScriptVar *newa = new CScriptVar(a->var->getBool() ? 1ll : 0ll);
				CScriptVar *newb = new CScriptVar(b->var->getBool() ? 1ll : 0ll);

				CREATE_LINK(a, newa);
				CREATE_LINK(b, newb);
			}

			CScriptVar *res = a->var->mathsOp(b->var, op);
			CREATE_LINK(a, res);
		}

		CLEAN(b);
	}

	return a;
}

CScriptVarLink *CTinyJS::ternary(bool &execute)
{
	CScriptVarLink *lhs = logic(execute);
	bool noexec = false;
	if (l->tk == _T('?'))
	{
		l->match(_T('?'));

		if (!execute)
		{
			CLEAN(lhs);
			CLEAN(base(noexec));
			l->match(_T(':'));
			CLEAN(base(noexec));
		}
		else
		{
			bool first = lhs->var->getBool();
			CLEAN(lhs);

			if (first)
			{
				lhs = base(execute);
				l->match(_T(':'));
				CLEAN(base(noexec));
			}
			else
			{
				CLEAN(base(noexec));
				l->match(_T(':'));
				lhs = base(execute);
			}
		}
	}

	return lhs;
}

CScriptVarLink *CTinyJS::base(bool &execute)
{
	CScriptVarLink *lhs = ternary(execute);
	
	if ((l->tk == _T('=')) || (l->tk == LEX_PLUSEQUAL) || (l->tk == LEX_MINUSEQUAL))
	{
		// If we're assigning to this and we don't have a parent,
		// add it to the symbol table root as per JavaScript
		if (execute && !lhs->owned)
		{
			if (lhs->name.length() > 0)
			{
				CScriptVarLink *realLhs = root->addChildNoDup(lhs->name, lhs->var);
				CLEAN(lhs);
				lhs = realLhs;
			}
			else
			{
				_tprintf_s(_T("Trying to assign to an un-named type\n"));
			}
		}
		
		int64_t op = l->tk;
		l->match(l->tk);
		
		CScriptVarLink *rhs = base(execute);
		
		if (execute)
		{
			if (op == _T('='))
			{
				lhs->replaceWith(rhs);
			}
			else if (op == LEX_PLUSEQUAL)
			{
				CScriptVar *res = lhs->var->mathsOp(rhs->var, _T('+'));
				lhs->replaceWith(res);
			}
			else if (op == LEX_MINUSEQUAL)
			{
				CScriptVar *res = lhs->var->mathsOp(rhs->var, _T('-'));
				lhs->replaceWith(res);
			}
		}
		
		CLEAN(rhs);
	}
	return lhs;
}

void CTinyJS::block(bool &execute)
{
	l->match(_T('{'));

	if (execute)
	{
		while (l->tk && (l->tk != _T('}')))
			statement(execute);

		l->match(_T('}'));
	}
	else
	{
		// fast skip of blocks
		int64_t brackets = 1;

		while (l->tk && brackets)
		{
			if (l->tk == _T('{'))
				brackets++;

			if (l->tk == _T('}'))
				brackets--;

			l->match(l->tk);
		}
	}
}

void CTinyJS::statement(bool &execute)
{
	if (l->tk == LEX_ID ||
		l->tk == LEX_INT ||
		l->tk == LEX_FLOAT ||
		l->tk == LEX_STR ||
		l->tk == _T('-'))
	{
		// Execute a simple statement that only contains basic arithmetic...
		CLEAN(base(execute));
		l->match(';');
	}
	else if (l->tk == _T('{'))
	{
		/* A block of code */
		block(execute);
	}
	else if (l->tk == _T(';'))
	{
		// Empty statement - to allow things like ;;;
		l->match(_T(';'));
	}
	else if (l->tk == LEX_R_VAR)
	{
		/* variable creation. TODO - we need a better way of parsing the left
		 * hand side. Maybe just have a flag called can_create_var that we
		 * set and then we parse as if we're doing a normal equals.*/
		l->match(LEX_R_VAR);

		while (l->tk != _T(';'))
		{
			CScriptVarLink *a = 0;
			if(execute)
				a = scopes.back()->findChildOrCreate(l->tkStr);

			l->match(LEX_ID);

			// now do stuff defined with dots
			while (l->tk == _T('.'))
			{
				l->match(_T('.'));

				if(execute)
				{
					CScriptVarLink *lastA = a;
					a = lastA->var->findChildOrCreate(l->tkStr);
				}

				l->match(LEX_ID);
			}

			// sort out initialiser
			if (l->tk == _T('='))
			{
				l->match(_T('='));

				CScriptVarLink *var = base(execute);
				if(execute)
					a->replaceWith(var);

				CLEAN(var);
			}

			if (l->tk != _T(';'))
				l->match(_T(','));
		}

		l->match(';');
	}
	else if (l->tk == LEX_R_IF)
	{
		l->match(LEX_R_IF);

		l->match(_T('('));
		CScriptVarLink *var = base(execute);
		l->match(_T(')'));

		bool cond = execute && var->var->getBool();
		CLEAN(var);

		bool noexecute = false; // because we need to be abl;e to write to it

		statement(cond ? execute : noexecute);

		if (l->tk == LEX_R_ELSE)
		{
			l->match(LEX_R_ELSE);
			statement(cond ? noexecute : execute);
		}
	} else if (l->tk == LEX_R_WHILE)
	{
		// We do repetition by pulling out the string representing our statement
		// there's definitely some opportunity for optimisation here
		l->match(LEX_R_WHILE);
		l->match(_T('('));

		int64_t whileCondStart = l->tokenStart;
		bool noexecute = false;
		CScriptVarLink *cond = base(execute);
		bool loopCond = execute && cond->var->getBool();
		CLEAN(cond);
		CScriptLex *whileCond = l->getSubLex(whileCondStart);

		l->match(_T(')'));

		int64_t whileBodyStart = l->tokenStart;
		statement(loopCond ? execute : noexecute);

		CScriptLex *whileBody = l->getSubLex(whileBodyStart);
		CScriptLex *oldLex = l;

		int64_t loopCount = TINYJS_LOOP_MAX_ITERATIONS;
		while (loopCond && (loopCount-- > 0))
		{
			whileCond->reset();
			l = whileCond;

			cond = base(execute);
			loopCond = execute && cond->var->getBool();

			CLEAN(cond);

			if (loopCond)
			{
				whileBody->reset();
				l = whileBody;
				statement(execute);
			}
		}

		l = oldLex;
		delete whileCond;
		delete whileBody;

		if (loopCount <= 0)
		{
			root->trace();
			_tprintf_s(_T("WHILE Loop exceeded %lld iterations at %s\n"), TINYJS_LOOP_MAX_ITERATIONS, l->getPosition().c_str());
			throw new CScriptException(_T("LOOP_ERROR"));
		}
	}
	else if (l->tk == LEX_R_FOR)
	{
		l->match(LEX_R_FOR);

		l->match(_T('('));

		statement(execute); // initialisation

		int64_t forCondStart = l->tokenStart;
		bool noexecute = false;

		CScriptVarLink *cond = base(execute); // condition
		bool loopCond = execute && cond->var->getBool();

		CLEAN(cond);

		CScriptLex *forCond = l->getSubLex(forCondStart);

		l->match(_T(';'));

		int64_t forIterStart = l->tokenStart;

		CLEAN(base(noexecute)); // iterator

		CScriptLex *forIter = l->getSubLex(forIterStart);

		l->match(_T(')'));

		int64_t forBodyStart = l->tokenStart;
		statement(loopCond ? execute : noexecute);
		CScriptLex *forBody = l->getSubLex(forBodyStart);
		CScriptLex *oldLex = l;

		if (loopCond)
		{
			forIter->reset();
			l = forIter;
			CLEAN(base(execute));
		}

		int64_t loopCount = TINYJS_LOOP_MAX_ITERATIONS;

		while (execute && loopCond && (loopCount-- > 0))
		{
			forCond->reset();
			l = forCond;

			cond = base(execute);
			loopCond = cond->var->getBool();

			CLEAN(cond);

			if (execute && loopCond)
			{
				forBody->reset();
				l = forBody;
				statement(execute);
			}

			if (execute && loopCond)
			{
				forIter->reset();
				l = forIter;
				CLEAN(base(execute));
			}
		}

		l = oldLex;

		delete forCond;
		delete forIter;
		delete forBody;

		if (loopCount <= 0)
		{
			root->trace();
			_tprintf_s(_T("FOR Loop exceeded %lld iterations at %s\n"),TINYJS_LOOP_MAX_ITERATIONS,l->getPosition().c_str());

			throw new CScriptException(_T("LOOP_ERROR"));
		}
	}
	else if (l->tk == LEX_R_RETURN)
	{
		l->match(LEX_R_RETURN);
		CScriptVarLink *result = 0;

		if (l->tk != _T(';'))
			result = base(execute);

		if (execute)
		{
			CScriptVarLink *resultVar = scopes.back()->findChild(TINYJS_RETURN_VAR);

			if (resultVar)
				resultVar->replaceWith(result);
			else
				_tprintf_s(_T("RETURN statement, but not in a function.\n"));

			execute = false;
		}

		CLEAN(result);
		l->match(';');
	}
	else if (l->tk==LEX_R_FUNCTION)
	{
		CScriptVarLink *funcVar = parseFunctionDefinition();
		if (execute)
		{
			if (funcVar->name == TINYJS_TEMP_NAME)
				_tprintf_s(_T("Functions defined at statement-level are meant to have a name\n"));
			else
				scopes.back()->addChildNoDup(funcVar->name,funcVar->var);
		}

		CLEAN(funcVar);
	}
	else
	{
		l->match(LEX_EOF);
	}
}

/// Get the given variable specified by a path (var1.var2.etc), or return 0
CScriptVar *CTinyJS::getScriptVariable(const tstring &path)
{
	// traverse path
	size_t prevIdx = 0;
	size_t thisIdx = path.find(_T('.'));

	if (thisIdx == string::npos)
		thisIdx = path.length();

	CScriptVar *var = root;
	while (var && prevIdx<path.length())
	{
		tstring el = path.substr(prevIdx, thisIdx-prevIdx);
		CScriptVarLink *varl = var->findChild(el);

		var = varl ? varl->var : 0;

		prevIdx = thisIdx + 1;

		thisIdx = path.find(_T('.'), prevIdx);

		if (thisIdx == string::npos)
			thisIdx = path.length();
	}

	return var;
}

/// Get the value of the given variable, or return 0
const tstring *CTinyJS::getVariable(const tstring &path)
{
	CScriptVar *var = getScriptVariable(path);

	// return result
	if (var)
		return &var->getString();
	else
		return 0;
}

/// set the value of the given variable, return trur if it exists and gets set
bool CTinyJS::setVariable(const tstring &path, const tstring &varData)
{
	CScriptVar *var = getScriptVariable(path);

	// return result
	if (var)
	{
		if (var->isInt())
		{
			var->setInt((int64_t)_tcstol(varData.c_str(), 0, 0));
		}
		else if (var->isDouble())
		{
			var->setDouble(_tcstod(varData.c_str(), 0));
		}
		else
		{
			var->setString(varData.c_str());
		}

		return true;
	}    

	return false;
}

/// Finds a child, looking recursively up the scopes
CScriptVarLink *CTinyJS::findInScopes(const tstring &childName)
{
	for (int64_t s = scopes.size() - 1; s >= 0; s--)
	{
		CScriptVarLink *v = scopes[s]->findChild(childName);

		if (v)
			return v;
	}

	return NULL;
}

/// Look up in any parent classes of the given object
CScriptVarLink *CTinyJS::findInParentClasses(CScriptVar *object, const tstring &name)
{
	// Look for links to actual parent classes
	CScriptVarLink *parentClass = object->findChild(TINYJS_PROTOTYPE_CLASS);

	while (parentClass)
	{
		CScriptVarLink *implementation = parentClass->var->findChild(name);
		if (implementation)
			return implementation;

		parentClass = parentClass->var->findChild(TINYJS_PROTOTYPE_CLASS);
	}

	// else fake it for strings and finally objects
	if (object->isString())
	{
		CScriptVarLink *implementation = stringClass->findChild(name);
		if (implementation)
			return implementation;
	}

	if (object->isArray())
	{
		CScriptVarLink *implementation = arrayClass->findChild(name);
		if (implementation)
			return implementation;
	}

	CScriptVarLink *implementation = objectClass->findChild(name);
	if (implementation)
		return implementation;

	return 0;
}
