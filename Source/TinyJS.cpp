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
   Version 0.11 :  Making sure the 'm_Root' variable never changes
				   'symbol_base' added for the current base of the sybmbol table
   Version 0.12 :  Added FindChildOrCreate, changed string passing to use references
				   Fixed broken string encoding in getJSString()
				   Removed getInitCode and added getJSON instead
				   Added nil
				   Added rough JSON parsing
				   Improved example app
   Version 0.13 :  Added tokenEnd/tokenLastEnd to lexer to avoid parsing whitespace
				   Ability to define functions without names
				   Can now do "var mine = function(a,b) { ... };"
				   Slightly better 'Trace' function
				   Added FindChildOrCreateByPath function
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
   Version 0.16 :  Removal of un-needed FindRecursive calls
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
				   Ints + floats are now stored in binary form (faster + more precise)
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
				   Fixed getString() for float on some platforms
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
		  Move 'shift' implementation into MathsOp

 */

#include "pch.h"

#include "TinyJS.h"
#include <assert.h>
#include <inttypes.h>

 /* Frees the given link IF it isn't m_Owned by anything else */
#define CLEAN(x) { CScriptVarLink *__v = x; if (__v && !__v->m_Owned) { delete __v; } }

 /* Create a LINK to point to VAR and free the old link.
 * BUT this is more clever - it tries to keep the old link if it's not m_Owned to save allocations */
#define CREATE_LINK(LINK, VAR) { if (!LINK || LINK->m_Owned) LINK = new CScriptVarLink(VAR); else LINK->ReplaceWith(VAR); }

#include <string>
#include <string.h>
#include <sstream>
#include <cstdlib>
#include <stdio.h>

using namespace std;


bool IsNumber(const TCHAR *str)
{
	if (!str)
		return false;

	while (*str++)
	{
		if (!_istdigit(*str))
			return false;
	}

	return true;
}


bool IsIDChar(TCHAR ch)
{
	return (_istalpha(ch) || (ch == _T('_')));
}


bool IsIDString(const TCHAR *s)
{
	if (!IsIDChar(*s))
		return false;

	while (*s++)
	{
		if (!(IsIDChar(*s) || _istdigit(*s)))
			return false;
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


// ----------------------------------------------------------------------------------- CSCRIPTEXCEPTION

CScriptException::CScriptException(const tstring &exceptionText)
{
	text = exceptionText;
}

// ----------------------------------------------------------------------------------- CSCRIPTLEX

CScriptLex::TScriptLexStore CScriptLex::s_Store;
CScriptLex *CScriptLex::GetNew(const TCHAR *text, bool copy_text, size_t start_idx, size_t end_idx)
{
	CScriptLex *ret = nullptr;

	for (auto &i : s_Store)
	{
		if (!i.m_bInUse)
		{
			ret = &i;
			break;
		}
	}

	if (!ret)
	{
		size_t r = s_Store.size();
		s_Store.resize(r + 8);
		ret = &s_Store[r];
	}

	ret->dataOwned = copy_text;
	ret->data = (text && copy_text) ? _tcsdup(text) : (TCHAR *)text;
	ret->dataStart = start_idx;
	size_t textlen = text ? _tcslen(text) : 0;
	ret->dataEnd = min(end_idx, textlen);
	ret->m_bInUse = true;

	ret->reset();

	return ret;
}

void CScriptLex::Release()
{
	m_bInUse = false;
}

CScriptLex::CScriptLex()
{
	data = nullptr;
	dataOwned = false;
	dataStart = 0;
	dataEnd = 0;
	m_bInUse = false;

	reset();
}

CScriptLex::~CScriptLex(void)
{
	if (dataOwned)
		free((void *)data);
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
	switch (token)
	{
		case _T('!'): return _T("\'!\'");
		case _T('\"'): return _T("\'\"\'");
		case _T('#'): return _T("\'#\'");
		case _T('$'): return _T("\'$\'");
		case _T('%'): return _T("\'%\'");
		case _T('&'): return _T("\'&\'");
		case _T('\''): return _T("\'\'\'");
		case _T('('): return _T("\'(\'");
		case _T(')'): return _T("\')\'");
		case _T('*'): return _T("\'*\'");
		case _T('+'): return _T("\'+\'");
		case _T(','): return _T("\',\'");
		case _T('-'): return _T("\'-\'");
		case _T('.'): return _T("\'.\'");
		case _T('/'): return _T("\'/\'");
		case _T('0'): return _T("\'0\'");
		case _T('1'): return _T("\'1\'");
		case _T('2'): return _T("\'2\'");
		case _T('3'): return _T("\'3\'");
		case _T('4'): return _T("\'4\'");
		case _T('5'): return _T("\'5\'");
		case _T('6'): return _T("\'6\'");
		case _T('7'): return _T("\'7\'");
		case _T('8'): return _T("\'8\'");
		case _T('9'): return _T("\'9\'");
		case _T(':'): return _T("\':\'");
		case _T(';'): return _T("\';\'");
		case _T('<'): return _T("\'<\'");
		case _T('='): return _T("\'=\'");
		case _T('>'): return _T("\'>\'");
		case _T('?'): return _T("\'?\'");
		case _T('@'): return _T("\'@\'");
		case _T('A'): return _T("\'A\'");
		case _T('B'): return _T("\'B\'");
		case _T('C'): return _T("\'C\'");
		case _T('D'): return _T("\'D\'");
		case _T('E'): return _T("\'E\'");
		case _T('F'): return _T("\'F\'");
		case _T('G'): return _T("\'G\'");
		case _T('H'): return _T("\'H\'");
		case _T('I'): return _T("\'I\'");
		case _T('J'): return _T("\'J\'");
		case _T('K'): return _T("\'K\'");
		case _T('L'): return _T("\'L\'");
		case _T('M'): return _T("\'M\'");
		case _T('N'): return _T("\'N\'");
		case _T('O'): return _T("\'O\'");
		case _T('P'): return _T("\'P\'");
		case _T('Q'): return _T("\'Q\'");
		case _T('R'): return _T("\'R\'");
		case _T('S'): return _T("\'S\'");
		case _T('T'): return _T("\'T\'");
		case _T('U'): return _T("\'U\'");
		case _T('V'): return _T("\'V\'");
		case _T('W'): return _T("\'W\'");
		case _T('X'): return _T("\'X\'");
		case _T('Y'): return _T("\'Y\'");
		case _T('Z'): return _T("\'Z\'");
		case _T('['): return _T("\'[\'");
		case _T('\\'): return _T("\'\\\'");
		case _T(']'): return _T("\']\'");
		case _T('^'): return _T("\'^\'");
		case _T('_'): return _T("\'_\'");
		case _T('`'): return _T("\'`\'");
		case _T('a'): return _T("\'a\'");
		case _T('b'): return _T("\'b\'");
		case _T('c'): return _T("\'c\'");
		case _T('d'): return _T("\'d\'");
		case _T('e'): return _T("\'e\'");
		case _T('f'): return _T("\'f\'");
		case _T('g'): return _T("\'g\'");
		case _T('h'): return _T("\'h\'");
		case _T('i'): return _T("\'i\'");
		case _T('j'): return _T("\'j\'");
		case _T('k'): return _T("\'k\'");
		case _T('l'): return _T("\'l\'");
		case _T('m'): return _T("\'m\'");
		case _T('n'): return _T("\'n\'");
		case _T('o'): return _T("\'o\'");
		case _T('p'): return _T("\'p\'");
		case _T('q'): return _T("\'q\'");
		case _T('r'): return _T("\'r\'");
		case _T('s'): return _T("\'s\'");
		case _T('t'): return _T("\'t\'");
		case _T('u'): return _T("\'u\'");
		case _T('v'): return _T("\'v\'");
		case _T('w'): return _T("\'w\'");
		case _T('x'): return _T("\'x\'");
		case _T('y'): return _T("\'y\'");
		case _T('z'): return _T("\'z\'");
		case _T('{'): return _T("\'{\'");
		case _T('|'): return _T("\'|\'");
		case _T('}'): return _T("\'}\'");
		case _T('~'): return _T("'~'");

		case LEX_EOF: return _T("EOF");
		case LEX_ID: return _T("ID");
		case LEX_INT: return _T("INT");
		case LEX_FLOAT: return _T("FLOAT");
		case LEX_STR: return _T("STRING");
		case LEX_EQUAL: return _T("==");
		case LEX_TYPEEQUAL: return _T("===");
		case LEX_NEQUAL: return _T("!=");
		case LEX_NTYPEEQUAL: return _T("!==");
		case LEX_LEQUAL: return _T("<=");
		case LEX_LSHIFT: return _T("<<");
		case LEX_LSHIFTEQUAL: return _T("<<=");
		case LEX_GEQUAL: return _T(">=");
		case LEX_RSHIFT: return _T(">>");
		case LEX_RSHIFTUNSIGNED: return _T(">>");
		case LEX_RSHIFTEQUAL: return _T(">>=");
		case LEX_PLUSEQUAL: return _T("+=");
		case LEX_MINUSEQUAL: return _T("-=");
		case LEX_MULEQUAL: return _T("*=");
		case LEX_DIVEQUAL: return _T("/=");
		case LEX_PLUSPLUS: return _T("++");
		case LEX_MINUSMINUS: return _T("--");
		case LEX_ANDEQUAL: return _T("&=");
		case LEX_ANDAND: return _T("&&");
		case LEX_OREQUAL: return _T("|=");
		case LEX_OROR: return _T("||");
		case LEX_XOREQUAL: return _T("^=");
				// reserved words
		case LEX_R_IF: return _T("if");
		case LEX_R_ELSE: return _T("else");
		case LEX_R_DO: return _T("do");
		case LEX_R_WHILE: return _T("while");
		case LEX_R_FOR: return _T("for");
		case LEX_R_BREAK: return _T("break");
		case LEX_R_CONTINUE: return _T("continue");
		case LEX_R_FUNCTION: return _T("function");
		case LEX_R_RETURN: return _T("return");
		case LEX_R_VAR: return _T("var");
		case LEX_R_TRUE: return _T("true");
		case LEX_R_FALSE: return _T("false");
		case LEX_R_NULL: return _T("null");
		case LEX_R_UNDEFINED: return _T("undefined");
		case LEX_R_NEW: return _T("new");
	}

	return _T("unknown");
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

	while (currCh && _istspace(currCh))
		getNextCh();

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
		while (currCh && ((currCh != _T('*')) || (nextCh != _T('/'))))
			getNextCh();

		getNextCh();
		getNextCh();
		getNextToken();

		return;
	}

	// record beginning of this token
	tokenStart = dataPos - 2;

	// tokens
	if (IsIDChar(currCh))
	{ //  IDs
		while (IsIDChar(currCh) || _istdigit(currCh))
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
	else if (_istdigit(currCh))
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
		while (_istdigit(currCh) || (isHex && _istxdigit(currCh)))
		{
			tkStr += currCh;
			getNextCh();
		}

		if (!isHex && (currCh == _T('.')))
		{
			tk = LEX_FLOAT;
			tkStr += _T('.');
			getNextCh();

			while (_istdigit(currCh))
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

			while (_istdigit(currCh))
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

						tkStr += (TCHAR)_tcstoll(buf, 0, 16);
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

							tkStr += (TCHAR)_tcstoll(buf, 0, 8);
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
		else if ((tk == _T('*')) && (currCh == _T('=')))
		{
			tk = LEX_MULEQUAL;
			getNextCh();
		}
		else if ((tk == _T('/')) && (currCh == _T('=')))
		{
			tk = LEX_DIVEQUAL;
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
		return CScriptLex::GetNew(data, false, lastPosition, lastCharIdx);
	else
		return CScriptLex::GetNew(data, false, lastPosition, dataEnd);
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

CScriptVarLink::CScriptVarLink(CScriptVar *var, const TCHAR *name)
{
#if DEBUG_MEMORY
	mark_allocated(this);
#endif

	m_Name = name;
	m_Sibling.next = nullptr;
	m_Sibling.prev = nullptr;
	m_Var = var->ref();
	m_Owned = false;
}

CScriptVarLink::CScriptVarLink(const CScriptVarLink &link)
{
	// Copy constructor
#if DEBUG_MEMORY
	mark_allocated(this);
#endif

	m_Name = link.m_Name;
	m_Sibling.next = nullptr;
	m_Sibling.prev = nullptr;
	m_Var = link.m_Var->ref();
	m_Owned = false;
}

CScriptVarLink::~CScriptVarLink()
{
#if DEBUG_MEMORY
	mark_deallocated(this);
#endif

	m_Var->unref();
}

void CScriptVarLink::ReplaceWith(CScriptVar *newVar)
{
	CScriptVar *oldVar = m_Var;
	m_Var = newVar->ref();
	oldVar->unref();
}

void CScriptVarLink::ReplaceWith(CScriptVarLink *newVar)
{
	if (newVar)
		ReplaceWith(newVar->m_Var);
	else
		ReplaceWith(new CScriptVar());
}

int64_t CScriptVarLink::GetIntName()
{
	return _ttoi(m_Name.c_str());
}

void CScriptVarLink::SetIntName(int64_t n)
{
	TCHAR sIdx[64];
	_stprintf_s(sIdx, sizeof(sIdx), _T("%" PRId64), n);
	m_Name = sIdx;
}

// ----------------------------------------------------------------------------------- CSCRIPTVAR


CScriptVar::CScriptVar()
{
	m_RefCt = 0;
#if DEBUG_MEMORY
	mark_allocated(this);
#endif
	init();
	flags = 0;
}

CScriptVar::CScriptVar(const TCHAR *str)
{
	m_RefCt = 0;

#if DEBUG_MEMORY
	mark_allocated(this);
#endif

	init();
	flags = SVF_STRING;
	strData = str;
}


CScriptVar::CScriptVar(const TCHAR *varData, JSFlagsType varFlags)
{
	m_RefCt = 0;

#if DEBUG_MEMORY
	mark_allocated(this);
#endif

	init();
	flags = varFlags;

	if (varFlags & SVF_INTEGER)
	{
		intData = _tcstoll(varData, 0, 0);
	}
	else if (varFlags & SVF_FLOAT)
	{
		floatData = _tcstof(varData, 0);
	}
	else
		strData = varData;
}

CScriptVar::CScriptVar(float val)
{
	m_RefCt = 0;

#if DEBUG_MEMORY
	mark_allocated(this);
#endif

	init();
	SetFloat(val);
}

CScriptVar::CScriptVar(int64_t val)
{
	m_RefCt = 0;

#if DEBUG_MEMORY
	mark_allocated(this);
#endif

	init();
	SetInt(val);
}

CScriptVar::~CScriptVar(void)
{
#if DEBUG_MEMORY
	mark_deallocated(this);
#endif

	RemoveAllChildren();
}

void CScriptVar::init()
{
	m_Child.first = nullptr;
	m_Child.last = nullptr;
	flags = 0;
	jsCallback = nullptr;
	jsCallbackUserData = 0;
	strData = TINYJS_BLANK_DATA;
	intData = 0;
	floatData = 0;
}

CScriptVar *CScriptVar::GetReturnVar()
{
	return GetParameter(TINYJS_RETURN_VAR);
}

void CScriptVar::SetReturnVar(CScriptVar *var)
{
	FindChildOrCreate(TINYJS_RETURN_VAR)->ReplaceWith(var);
}


CScriptVar *CScriptVar::GetParameter(const TCHAR *name)
{
	return FindChildOrCreate(name)->m_Var;
}

CScriptVarLink *CScriptVar::FindChild(const TCHAR *child_name)
{
	CScriptVarLink *v = m_Child.first;
	while (v)
	{
		if (v->m_Name.compare(child_name) == 0)
			return v;

		v = v->m_Sibling.next;
	}
	return 0;
}

CScriptVarLink *CScriptVar::FindChildOrCreate(const TCHAR *childName, JSFlagsType varFlags)
{
	CScriptVarLink *l = FindChild(childName);
	if (l)
		return l;

	return AddChild(childName, new CScriptVar(TINYJS_BLANK_DATA, varFlags));
}

CScriptVarLink *CScriptVar::FindChildOrCreateByPath(const TCHAR *path)
{
	const TCHAR *p = _tcschr(path, _T('.'));
	if (!p)
		return FindChildOrCreate(path);

	*((TCHAR *)p) = _T('\0');
	CScriptVarLink *ret = FindChildOrCreate(path, SVF_OBJECT)->m_Var->FindChildOrCreateByPath(p);
	*((TCHAR *)p) = _T('.');

	return ret;
}

CScriptVarLink *CScriptVar::AddChild(const TCHAR *child_name, CScriptVar *child, bool overwrite)
{
	if (IsUndefined())
	{
		flags = SVF_OBJECT;
	}

	// if no child supplied, create one
	if (!child)
		child = new CScriptVar();

	CScriptVarLink *link = new CScriptVarLink(child, child_name);
	link->m_Owned = true;

	if (m_Child.last)
	{
		CScriptVarLink *v = nullptr;

		if (overwrite)
			v = FindChild(child_name);

		if (overwrite && v)
		{
			v->ReplaceWith(child);
		}
		else
		{
			m_Child.last->m_Sibling.next = link;
			link->m_Sibling.prev = m_Child.last;
			m_Child.last = link;
		}
	}
	else
	{
		m_Child.first = link;
		m_Child.last = link;
	}

	return link;
}

void CScriptVar::RemoveChild(CScriptVar *child)
{
	CScriptVarLink *link = m_Child.first;

	while (link)
	{
		if (link->m_Var == child)
			break;

		link = link->m_Sibling.next;
	}

	RemoveLink(link);
}

void CScriptVar::RemoveLink(CScriptVarLink *link)
{
	if (!link)
		return;

	if (link->m_Sibling.next)
		link->m_Sibling.next->m_Sibling.prev = link->m_Sibling.prev;

	if (link->m_Sibling.prev)
		link->m_Sibling.prev->m_Sibling.next = link->m_Sibling.next;

	if (m_Child.last == link)
		m_Child.last = link->m_Sibling.prev;

	if (m_Child.first == link)
		m_Child.first = link->m_Sibling.next;

	delete link;
}

void CScriptVar::RemoveAllChildren()
{
	CScriptVarLink *c = m_Child.first;
	while (c)
	{
		CScriptVarLink *t = c->m_Sibling.next;
		delete c;
		c = t;
	}

	m_Child.first = 0;
	m_Child.last = 0;
}

CScriptVar *CScriptVar::GetArrayIndex(int64_t idx)
{
	TCHAR sIdx[64];
	_stprintf_s(sIdx, sizeof(sIdx), _T("%" PRId64), idx);

	CScriptVarLink *link = FindChild(sIdx);
	if (link)
		return link->m_Var;
	else
		return new CScriptVar(TINYJS_BLANK_DATA, SVF_NULL); // undefined
}

void CScriptVar::SetArrayIndex(int64_t idx, CScriptVar *value)
{
	TCHAR sIdx[64];
	_stprintf_s(sIdx, sizeof(sIdx), _T("%" PRId64), idx);
	CScriptVarLink *link = FindChild(sIdx);

	if (link)
	{
		if (value->IsUndefined())
			RemoveLink(link);
		else
			link->ReplaceWith(value);
	}
	else
	{
		if (!value->IsUndefined())
			AddChild(sIdx, value);
	}
}

int64_t CScriptVar::GetArrayLength()
{
	int64_t highest = -1;
	if (!IsArray())
		return 0;

	CScriptVarLink *link = m_Child.first;
	while (link)
	{
		if (IsNumber(link->m_Name.c_str()))
		{
			int64_t val = _ttoi(link->m_Name.c_str());
			if (val > highest)
				highest = val;
		}

		link = link->m_Sibling.next;
	}

	return highest + 1;
}

int64_t CScriptVar::GetNumChildren()
{
	int64_t n = 0;

	CScriptVarLink *link = m_Child.first;
	while (link)
	{
		n++;
		link = link->m_Sibling.next;
	}

	return n;
}

int64_t CScriptVar::GetInt()
{
	/* strtol understands about hex and octal */
	if (IsInt())
		return intData;

	if (IsNull())
		return 0;

	if (IsUndefined())
		return 0;

	if (IsFloat())
		return (int64_t)floatData;

	return 0;
}

float CScriptVar::GetFloat()
{
	if (IsFloat())
		return floatData;

	if (IsInt())
		return (float)intData;

	if (IsNull())
		return 0;

	if (IsUndefined())
		return 0;

	return 0; /* or NaN? */
}

const TCHAR *CScriptVar::GetString()
{
	/* Because we can't return a string that is generated on demand.
	 * I should really just use char* :) */
	static const TCHAR *s_null = _T("null");
	static const TCHAR *s_undefined = _T("undefined");

	tstringstream ss;

	if (IsInt())
	{
		ss << intData;
		strData = ss.str();
		return strData.c_str();
	}

	if (IsFloat())
	{
		ss << floatData;
		strData = ss.str();
		return strData.c_str();
	}

	if (IsNull())
		return s_null;

	if (IsUndefined())
		return s_undefined;

	// are we just a string here?
	return strData.c_str();
}

void CScriptVar::SetInt(int64_t val)
{
	flags = (flags & ~SVF_VARTYPE_MASK) | SVF_INTEGER;
	intData = val;
	floatData = 0;
	strData = TINYJS_BLANK_DATA;
}

void CScriptVar::SetFloat(float val)
{
	flags = (flags & ~SVF_VARTYPE_MASK) | SVF_FLOAT;
	floatData = val;
	intData = 0;
	strData = TINYJS_BLANK_DATA;
}

void CScriptVar::SetString(const TCHAR *str)
{
	// name sure it's not still a number or integer
	flags = (flags & ~SVF_VARTYPE_MASK) | SVF_STRING;
	strData = str;
	intData = 0;
	floatData = 0;
}

void CScriptVar::SetUndefined()
{
	// name sure it's not still a number or integer
	flags &= ~SVF_VARTYPE_MASK;
	strData = TINYJS_BLANK_DATA;
	intData = 0;
	floatData = 0;
	RemoveAllChildren();
}

void CScriptVar::SetArray()
{
	// name sure it's not still a number or integer
	flags = (flags & ~SVF_VARTYPE_MASK) | SVF_ARRAY;
	strData = TINYJS_BLANK_DATA;
	intData = 0;
	floatData = 0;
	RemoveAllChildren();
}

bool CScriptVar::Equals(CScriptVar *v)
{
	CScriptVar *resV = MathsOp(v, LEX_EQUAL);
	bool res = resV->GetBool();
	delete resV;
	return res;
}

CScriptVar *CScriptVar::MathsOp(CScriptVar *b, int64_t op)
{
	CScriptVar *a = this;

	// Type equality check
	if ((op == LEX_TYPEEQUAL) || (op == LEX_NTYPEEQUAL))
	{
		// check type first, then call again to check strData
		bool eql = ((a->flags & SVF_VARTYPE_MASK) == (b->flags & SVF_VARTYPE_MASK));

		if (eql)
		{
			CScriptVar *contents = a->MathsOp(b, LEX_EQUAL);

			if (!contents->GetBool())
				eql = false;

			if (!contents->m_RefCt)
				delete contents;
		}

		if (op == LEX_TYPEEQUAL)
			return new CScriptVar(eql ? 1ll : 0ll);
		else
			return new CScriptVar(!eql ? 0ll : 1ll);
	}

	// do maths...
	if (a->IsUndefined() && b->IsUndefined())
	{
		if (op == LEX_EQUAL)
			return new CScriptVar(1ll);
		else if (op == LEX_NEQUAL)
			return new CScriptVar(0ll);
		else
			return new CScriptVar(); // undefined
	}
	else if ((a->IsNumeric() || a->IsUndefined()) && (b->IsNumeric() || b->IsUndefined()))
	{
		if (!a->IsFloat() && !b->IsFloat())
		{
			// use ints
			int64_t da = a->GetInt();
			int64_t db = b->GetInt();

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
			// use floats
			float da = a->GetFloat();
			float db = b->GetFloat();

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
				default:			throw new CScriptException(_T("Operation ") + CScriptLex::getTokenStr(op) + _T(" not supported on the Float datatype"));
			}
		}
	}
	else if (a->IsArray())
	{
		/* Just check pointers */
		switch (op)
		{
			case LEX_EQUAL:		return new CScriptVar((a == b) ? 1ll : 0ll);
			case LEX_NEQUAL:	return new CScriptVar((a != b) ? 1ll : 0ll);
			default:			throw new CScriptException(_T("Operation ") + CScriptLex::getTokenStr(op) + _T(" not supported on the Array datatype"));
		}
	}
	else if (a->IsObject())
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
		tstring da = a->GetString();
		tstring db = b->GetString();

		// use strings
		switch (op)
		{
			case _T('+'):		return new CScriptVar(tstring(da + db).c_str(), SVF_STRING);
			case LEX_EQUAL:		return new CScriptVar((da == db) ? 1ll : 0ll);
			case LEX_NEQUAL:	return new CScriptVar((da != db) ? 1ll : 0ll);
			case _T('<'):		return new CScriptVar((da < db) ? 1ll : 0ll);
			case LEX_LEQUAL:	return new CScriptVar((da <= db) ? 1ll : 0ll);
			case _T('>'):		return new CScriptVar((da > db) ? 1ll : 0ll);
			case LEX_GEQUAL:	return new CScriptVar((da >= db) ? 1ll : 0ll);
			default:			throw new CScriptException(_T("Operation ") + CScriptLex::getTokenStr(op) + _T(" not supported on the string datatype"));
		}
	}

	return 0;
}

void CScriptVar::CopySimpleData(CScriptVar *val)
{
	strData = val->strData;
	intData = val->intData;
	floatData = val->floatData;
	flags = (flags & ~SVF_VARTYPE_MASK) | (val->flags & SVF_VARTYPE_MASK);
}

void CScriptVar::CopyValue(CScriptVar *val)
{
	if (val)
	{
		CopySimpleData(val);

		// remove all current children
		RemoveAllChildren();

		// copy children of 'val'
		CScriptVarLink *child = val->m_Child.first;
		while (child)
		{
			CScriptVar *copied;

			// don't copy the 'parent' object...
			if (child->m_Name != TINYJS_PROTOTYPE_CLASS)
				copied = child->m_Var->DeepCopy();
			else
				copied = child->m_Var;

			AddChild(child->m_Name.c_str(), copied);

			child = child->m_Sibling.next;
		}
	}
	else
	{
		SetUndefined();
	}
}

CScriptVar *CScriptVar::DeepCopy()
{
	CScriptVar *newVar = new CScriptVar();
	newVar->CopySimpleData(this);

	// copy children
	CScriptVarLink *child = m_Child.first;
	while (child)
	{
		CScriptVar *copied;
		// don't copy the 'parent' object...
		if (child->m_Name != TINYJS_PROTOTYPE_CLASS)
			copied = child->m_Var->DeepCopy();
		else
			copied = child->m_Var;

		newVar->AddChild(child->m_Name.c_str(), copied);
		child = child->m_Sibling.next;
	}

	return newVar;
}

void CScriptVar::Trace(tstring indentStr, const tstring &name)
{
	_tprintf_s(_T("%s'%s' = '%s' %llx\n"), indentStr.c_str(), name.c_str(), GetString(), flags);

	tstring indent = indentStr + _T(" ");
	CScriptVarLink *link = m_Child.first;

	while (link)
	{
		link->m_Var->Trace(indent, link->m_Name);
		link = link->m_Sibling.next;
	}
}



void CScriptVar::SetCallback(JSCallback callback, void *userdata)
{
	jsCallback = callback;
	jsCallbackUserData = userdata;
}

CScriptVar *CScriptVar::ref()
{
	m_RefCt++;
	return this;
}

void CScriptVar::unref()
{
	if (m_RefCt <= 0)
		_tprintf(_T("We have unreffed too far!\n"));

	if ((--m_RefCt) == 0)
	{
		delete this;
	}
}

int64_t CScriptVar::getRefs()
{
	return m_RefCt;
}


// ----------------------------------------------------------------------------------- CSCRIPT

CTinyJS::CTinyJS()
{
	l = 0;
	m_Root = (new CScriptVar(TINYJS_BLANK_DATA, SVF_OBJECT))->ref();

	// Add built-in classes
	stringClass = (new CScriptVar(TINYJS_BLANK_DATA, SVF_OBJECT))->ref();
	arrayClass = (new CScriptVar(TINYJS_BLANK_DATA, SVF_OBJECT))->ref();
	objectClass = (new CScriptVar(TINYJS_BLANK_DATA, SVF_OBJECT))->ref();

	m_Root->AddChild(_T("String"), stringClass);
	m_Root->AddChild(_T("Array"), arrayClass);
	m_Root->AddChild(_T("Object"), objectClass);
}

CTinyJS::~CTinyJS()
{
	scopes.clear();

	stringClass->unref();
	arrayClass->unref();
	objectClass->unref();
	m_Root->unref();

#if DEBUG_MEMORY
	show_allocated();
#endif
}

void CTinyJS::Trace()
{
	m_Root->Trace();
}

bool CTinyJS::Execute(const TCHAR *code)
{
	bool ret = true;

	CScriptLex *oldLex = l;
	vector<CScriptVar *> oldScopes = scopes;
	l = CScriptLex::GetNew(code);

#ifdef TINYJS_CALL_STACK
	call_stack.clear();
#endif
	scopes.clear();
	scopes.push_back(m_Root);

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

		m_pSys->GetLog()->Print(last_error.c_str());
		m_pSys->GetLog()->Print(_T("\n"));

		ret = false;
	}

	l->Release();
	l = oldLex;
	scopes = oldScopes;

	return ret;
}

void CTinyJS::parseFunctionArguments(CScriptVar *funcVar)
{
	l->match(_T('('));

	while (l->tk != _T(')'))
	{
		funcVar->AddChild(l->tkStr.c_str(), nullptr, true);
		l->match(LEX_ID);

		if (l->tk != _T(')'))
			l->match(_T(','));
	}

	l->match(_T(')'));
}

void CTinyJS::AddNative(const TCHAR *funcDesc, JSCallback ptr, void *userdata)
{
	CScriptLex *oldLex = l;
	l = CScriptLex::GetNew(funcDesc, false);

	CScriptVar *base = m_Root;

	l->match(LEX_R_FUNCTION);
	tstring funcName = l->tkStr;
	l->match(LEX_ID);

	// Check for dots, we might want to do something like function String.substring ...
	while (l->tk == _T('.'))
	{
		l->match(_T('.'));

		CScriptVarLink *link = base->FindChild(funcName.c_str());

		// if it doesn't exist, make an object class
		if (!link)
			link = base->AddChild(funcName.c_str(), new CScriptVar(TINYJS_BLANK_DATA, SVF_OBJECT));

		base = link->m_Var;
		funcName = l->tkStr;
		l->match(LEX_ID);
	}

	CScriptVar *funcVar = new CScriptVar(TINYJS_BLANK_DATA, SVF_FUNCTION | SVF_NATIVE);
	funcVar->SetCallback(ptr, userdata);
	parseFunctionArguments(funcVar);

	l->Release();
	l = oldLex;

	base->AddChild(funcName.c_str(), funcVar);
}

CScriptVarLink *CTinyJS::parseFunctionDefinition()
{
	// actually parse a function...
	l->match(LEX_R_FUNCTION);
	tstring funcName = TINYJS_TEMP_NAME;

	// we can have functions without names
	if (l->tk == LEX_ID)
	{
		funcName = l->tkStr;
		l->match(LEX_ID);
	}

	CScriptVarLink *funcVar = new CScriptVarLink(new CScriptVar(TINYJS_BLANK_DATA, SVF_FUNCTION), funcName.c_str());

	parseFunctionArguments(funcVar->m_Var);
	int64_t funcBegin = l->tokenStart;
	bool noexecute = false;
	block(noexecute);
	funcVar->m_Var->strData = l->getSubString(funcBegin);

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
		if (!function->m_Var->IsFunction())
		{
			tstring errorMsg = _T("Expecting '");
			errorMsg = errorMsg + function->m_Name + _T("' to be a function");
			throw new CScriptException(errorMsg.c_str());
		}

		l->match(_T('('));

		// create a new symbol table entry for execution of this function
		CScriptVar *functionRoot = new CScriptVar(TINYJS_BLANK_DATA, SVF_FUNCTION);
		if (parent)
			functionRoot->AddChild(_T("this"), parent, true);

		// grab in all parameters
		CScriptVarLink *v = function->m_Var->m_Child.first;
		while (v)
		{
			CScriptVarLink *value = base(execute);
			if (value)
			{
				if (execute)
				{
					if (value->m_Var->IsBasic())
					{
						// pass by value
						functionRoot->AddChild(v->m_Name.c_str(), value->m_Var->DeepCopy());
					}
					else
					{
						// pass by reference
						functionRoot->AddChild(v->m_Name.c_str(), value->m_Var);
					}
				}

				CLEAN(value);
			}

			if (l->tk != _T(')'))
				l->match(_T(','));

			v = v->m_Sibling.next;
		}

		l->match(_T(')'));

		// setup a return variable
		CScriptVarLink *returnVar = NULL;

		// execute function!
		// add the function's execute space to the symbol table so we can recurse
		CScriptVarLink *returnVarLink = functionRoot->AddChild(TINYJS_RETURN_VAR);
		scopes.push_back(functionRoot);

#ifdef TINYJS_CALL_STACK
		call_stack.push_back(function->m_Name + _T(" from ") + l->getPosition());
#endif

		if (function->m_Var->IsNative())
		{
			function->m_Var->jsCallback(functionRoot, function->m_Var->jsCallbackUserData);
		}
		else
		{
			/* we just want to execute the block, but something could
			 * have messed up and left us with the wrong ScriptLex, so
			 * we want to be careful here... */
			CScriptException *exception = 0;
			CScriptLex *oldLex = l;
			CScriptLex *newLex = CScriptLex::GetNew(function->m_Var->GetString(), false);

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

			newLex->Release();
			newLex = nullptr;

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
		returnVar = new CScriptVarLink(returnVarLink->m_Var);
		functionRoot->RemoveLink(returnVarLink);
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
		return new CScriptVarLink(new CScriptVar(TINYJS_BLANK_DATA, SVF_NULL));
	}

	if (l->tk == LEX_R_UNDEFINED)
	{
		l->match(LEX_R_UNDEFINED);
		return new CScriptVarLink(new CScriptVar(TINYJS_BLANK_DATA, 0));
	}

	if (l->tk == LEX_ID)
	{
		CScriptVarLink *a = execute ? FindInScopes(l->tkStr.c_str()) : new CScriptVarLink(new CScriptVar());

		// The parent if we're executing a method call
		CScriptVar *parent = 0;

		if (execute && !a)
		{
		  // Variable doesn't exist! JavaScript says we should create it
		  // (we won't add it here. This is done in the assignment operator)
			a = new CScriptVarLink(new CScriptVar(), l->tkStr.c_str());
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
					const TCHAR *name = l->tkStr.c_str();

					CScriptVarLink *child = a->m_Var->FindChild(name);

					if (!child)
						child = FindInParentClasses(a->m_Var, name);

					if (!child)
					{
						// if we haven't found this defined yet, use the built-in 'length' properly
						if (a->m_Var->IsArray() && !_tcscmp(name, _T("length")))
						{
							int64_t len = a->m_Var->GetArrayLength();
							child = new CScriptVarLink(new CScriptVar(len));
						}
						else if (a->m_Var->IsString() && !_tcscmp(name, _T("length")))
						{
							const TCHAR *s = a->m_Var->GetString();
							int64_t len = s ? _tcslen(s) : 0;
							child = new CScriptVarLink(new CScriptVar(len));
						}
						else
						{
							child = a->m_Var->AddChild(name);
						}
					}

					parent = a->m_Var;
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
					CScriptVarLink *child = a->m_Var->FindChildOrCreate(index->m_Var->GetString());
					parent = a->m_Var;
					a = child;
				}

				CLEAN(index);
			}
		}

		return a;
	}

	if ((l->tk == LEX_INT) || (l->tk == LEX_FLOAT))
	{
		CScriptVar *a = new CScriptVar(l->tkStr.c_str(), ((l->tk == LEX_INT) ? SVF_INTEGER : SVF_FLOAT));
		l->match(l->tk);

		return new CScriptVarLink(a);
	}

	if (l->tk == LEX_STR)
	{
		CScriptVar *a = new CScriptVar(l->tkStr.c_str(), SVF_STRING);
		l->match(LEX_STR);

		return new CScriptVarLink(a);
	}

	if (l->tk == _T('{'))
	{
		CScriptVar *contents = new CScriptVar(TINYJS_BLANK_DATA, SVF_OBJECT);

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
				contents->AddChild(id.c_str(), a->m_Var);
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
		CScriptVar *contents = new CScriptVar(TINYJS_BLANK_DATA, SVF_ARRAY);

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
				contents->AddChild(idx_str, a->m_Var);

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

		if (funcVar->m_Name != TINYJS_TEMP_NAME)
			m_pSys->GetLog()->Print(_T("Functions not defined at statement-level are not meant to have a name"));

		return funcVar;
	}

	if (l->tk == LEX_R_NEW)
	{
		// new -> create a new object
		l->match(LEX_R_NEW);

		const tstring &className = l->tkStr;

		if (execute)
		{
			CScriptVarLink *objClassOrFunc = FindInScopes(className.c_str());
			if (!objClassOrFunc)
			{
				m_pSys->GetLog()->Print(_T("\"%s\" is not a valid class name"), className.c_str());
				return new CScriptVarLink(new CScriptVar());
			}

			l->match(LEX_ID);

			CScriptVar *obj = new CScriptVar(TINYJS_BLANK_DATA, SVF_OBJECT);
			CScriptVarLink *objLink = new CScriptVarLink(obj);

			if (objClassOrFunc->m_Var->IsFunction())
			{
				CLEAN(functionCall(execute, objClassOrFunc, obj));
			}
			else
			{
				obj->AddChild(TINYJS_PROTOTYPE_CLASS, objClassOrFunc->m_Var);

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
			CScriptVar *res = a->m_Var->MathsOp(&zero, LEX_EQUAL);

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
			CScriptVar *res = a->m_Var->MathsOp(b->m_Var, op);
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
		CScriptVar *res = zero.MathsOp(a->m_Var, _T('-'));
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
				CScriptVar *res = a->m_Var->MathsOp(&one, (op == LEX_PLUSPLUS) ? _T('+') : _T('-'));

				CScriptVarLink *oldValue = new CScriptVarLink(a->m_Var);

				// in-place add/subtract
				a->ReplaceWith(res);

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
				CScriptVar *res = a->m_Var->MathsOp(b->m_Var, op);
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
		int64_t shift = execute ? b->m_Var->GetInt() : 0;

		CLEAN(b);

		if (execute)
		{
			if (op == LEX_LSHIFT)
				a->m_Var->SetInt(a->m_Var->GetInt() << shift);

			if (op == LEX_RSHIFT)
				a->m_Var->SetInt(a->m_Var->GetInt() >> shift);

			if (op == LEX_RSHIFTUNSIGNED)
				a->m_Var->SetInt(((uint64_t)a->m_Var->GetInt()) >> shift);
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
			CScriptVar *res = a->m_Var->MathsOp(b->m_Var, op);
			CREATE_LINK(a, res);
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
		// we need to tell MathsOp it's an & or |
		if (op == LEX_ANDAND)
		{
			op = _T('&');
			shortCircuit = !a->m_Var->GetBool();
			boolean = true;
		}
		else if (op == LEX_OROR)
		{
			op = _T('|');
			shortCircuit = a->m_Var->GetBool();
			boolean = true;
		}

		b = condition(shortCircuit ? noexecute : execute);
		if (execute && !shortCircuit)
		{
			if (boolean)
			{
				CScriptVar *newa = new CScriptVar(a->m_Var->GetBool() ? 1ll : 0ll);
				CScriptVar *newb = new CScriptVar(b->m_Var->GetBool() ? 1ll : 0ll);

				CREATE_LINK(a, newa);
				CREATE_LINK(b, newb);
			}

			CScriptVar *res = a->m_Var->MathsOp(b->m_Var, op);
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
			bool first = lhs->m_Var->GetBool();
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

	if ((l->tk == _T('=')) || (l->tk == LEX_PLUSEQUAL) || (l->tk == LEX_MINUSEQUAL) || (l->tk == LEX_MULEQUAL) || (l->tk == LEX_DIVEQUAL))
	{
		// If we're assigning to this and we don't have a parent,
		// add it to the symbol table m_Root as per JavaScript
		if (execute && !lhs->m_Owned)
		{
			if (lhs->m_Name.length() > 0)
			{
				CScriptVarLink *realLhs = m_Root->AddChild(lhs->m_Name.c_str(), lhs->m_Var, true);
				CLEAN(lhs);
				lhs = realLhs;
			}
			else
			{
				m_pSys->GetLog()->Print(_T("Trying to assign to an un-named type\n"));
			}
		}

		int64_t op = l->tk;
		l->match(l->tk);

		CScriptVarLink *rhs = base(execute);

		if (execute)
		{
			if (op == _T('='))
			{
				lhs->ReplaceWith(rhs);
			}
			else if (op == LEX_PLUSEQUAL)
			{
				CScriptVar *res = lhs->m_Var->MathsOp(rhs->m_Var, _T('+'));
				lhs->ReplaceWith(res);
			}
			else if (op == LEX_MINUSEQUAL)
			{
				CScriptVar *res = lhs->m_Var->MathsOp(rhs->m_Var, _T('-'));
				lhs->ReplaceWith(res);
			}
			else if (op == LEX_MULEQUAL)
			{
				CScriptVar *res = lhs->m_Var->MathsOp(rhs->m_Var, _T('*'));
				lhs->ReplaceWith(res);
			}
			else if (op == LEX_DIVEQUAL)
			{
				CScriptVar *res = lhs->m_Var->MathsOp(rhs->m_Var, _T('/'));
				lhs->ReplaceWith(res);
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
			if (execute)
				a = scopes.back()->FindChildOrCreate(l->tkStr.c_str());

			l->match(LEX_ID);

			// now do stuff defined with dots
			while (l->tk == _T('.'))
			{
				l->match(_T('.'));

				if (execute)
				{
					CScriptVarLink *lastA = a;
					a = lastA->m_Var->FindChildOrCreate(l->tkStr.c_str());
				}

				l->match(LEX_ID);
			}

			// sort out initialiser
			if (l->tk == _T('='))
			{
				l->match(_T('='));

				CScriptVarLink *var = base(execute);
				if (execute)
					a->ReplaceWith(var);

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

		bool cond = execute && var->m_Var->GetBool();
		CLEAN(var);

		bool noexecute = false; // because we need to be abl;e to write to it

		statement(cond ? execute : noexecute);

		if (l->tk == LEX_R_ELSE)
		{
			l->match(LEX_R_ELSE);
			statement(cond ? noexecute : execute);
		}
	}
	else if (l->tk == LEX_R_WHILE)
	{
		// We do repetition by pulling out the string representing our statement
		// there's definitely some opportunity for optimisation here
		l->match(LEX_R_WHILE);
		l->match(_T('('));

		int64_t whileCondStart = l->tokenStart;
		bool noexecute = false;
		CScriptVarLink *cond = base(execute);
		bool loopCond = execute && cond->m_Var->GetBool();
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
			loopCond = execute && cond->m_Var->GetBool();

			CLEAN(cond);

			if (loopCond)
			{
				whileBody->reset();
				l = whileBody;
				statement(execute);
			}
		}

		l = oldLex;
		whileCond->Release();
		whileBody->Release();

		if (loopCount <= 0)
		{
			m_Root->Trace();
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
		bool loopCond = execute && cond->m_Var->GetBool();

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
			loopCond = cond->m_Var->GetBool();

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

		forCond->Release();
		forIter->Release();
		forBody->Release();

		if (loopCount <= 0)
		{
			m_Root->Trace();
			m_pSys->GetLog()->Print(_T("FOR Loop exceeded %lld iterations at %s\n"), TINYJS_LOOP_MAX_ITERATIONS, l->getPosition().c_str());

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
			CScriptVarLink *resultVar = scopes.back()->FindChild(TINYJS_RETURN_VAR);

			if (resultVar)
				resultVar->ReplaceWith(result);
			else
				m_pSys->GetLog()->Print(_T("RETURN statement, but not in a function.\n"));

			execute = false;
		}

		CLEAN(result);
		l->match(';');
	}
	else if (l->tk == LEX_R_FUNCTION)
	{
		CScriptVarLink *funcVar = parseFunctionDefinition();
		if (execute)
		{
			if (funcVar->m_Name == TINYJS_TEMP_NAME)
				m_pSys->GetLog()->Print(_T("Functions defined at statement-level are meant to have a name\n"));
			else
				scopes.back()->AddChild(funcVar->m_Name.c_str(), funcVar->m_Var, true);
		}

		CLEAN(funcVar);
	}
	else
	{
		l->match(LEX_EOF);
	}
}

/// Get the given variable specified by a path (var1.var2.etc), or return 0
CScriptVar *CTinyJS::GetScriptVariable(const TCHAR *path)
{
	if (!path)
		return nullptr;

	CScriptVar *var = m_Root;
	while (var && *path)
	{
		const TCHAR *p = _tcschr(path, _T('.'));

		if (p)
			*((TCHAR *)p) = _T('\0');

		CScriptVarLink *varl = var->FindChild(path);

		if (p)
			*((TCHAR *)p) = _T('.');

		var = varl ? varl->m_Var : nullptr;
		if (!p)
			break;

		path = p + 1;
	}

	return var;
}

/// Get the value of the given variable, or return 0
const TCHAR *CTinyJS::GetVariable(const TCHAR *path)
{
	CScriptVar *var = GetScriptVariable(path);

	// return result
	if (var)
		return var->GetString();

	return nullptr;
}

/// set the value of the given variable, return trur if it exists and gets set
bool CTinyJS::SetVariable(const TCHAR *path, const TCHAR *vardata)
{
	if (!vardata)
		vardata = TINYJS_BLANK_DATA;

	CScriptVar *var = GetScriptVariable(path);

	// return result
	if (var)
	{
		if (var->IsInt())
		{
			var->SetInt((int64_t)_tcstoll(vardata, 0, 0));
		}
		else if (var->IsFloat())
		{
			var->SetFloat(_tcstof(vardata, 0));
		}
		else
		{
			var->SetString(vardata);
		}

		return true;
	}

	return false;
}

/// Finds a child, looking recursively up the scopes
CScriptVarLink *CTinyJS::FindInScopes(const TCHAR *child_name)
{
	for (int64_t s = scopes.size() - 1; s >= 0; s--)
	{
		CScriptVarLink *v = scopes[s]->FindChild(child_name);

		if (v)
			return v;
	}

	return NULL;
}

/// Look up in any parent classes of the given object
CScriptVarLink *CTinyJS::FindInParentClasses(CScriptVar *object, const TCHAR *name)
{
	// Look for links to actual parent classes
	CScriptVarLink *parentClass = object->FindChild(TINYJS_PROTOTYPE_CLASS);

	while (parentClass)
	{
		CScriptVarLink *implementation = parentClass->m_Var->FindChild(name);
		if (implementation)
			return implementation;

		parentClass = parentClass->m_Var->FindChild(TINYJS_PROTOTYPE_CLASS);
	}

	// else fake it for strings and finally objects
	if (object->IsString())
	{
		CScriptVarLink *implementation = stringClass->FindChild(name);
		if (implementation)
			return implementation;
	}

	if (object->IsArray())
	{
		CScriptVarLink *implementation = arrayClass->FindChild(name);
		if (implementation)
			return implementation;
	}

	CScriptVarLink *implementation = objectClass->FindChild(name);
	if (implementation)
		return implementation;

	return 0;
}
