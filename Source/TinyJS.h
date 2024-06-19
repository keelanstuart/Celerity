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

#ifndef TINYJS_H
#define TINYJS_H

// If defined, this keeps a note of all calls and where from in memory. This is slower, but good for debugging
#define TINYJS_CALL_STACK	FALSE

#ifdef _WIN32
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif
#include <string>
#include <vector>

#ifndef TRACE
#define TRACE printf
#endif // TRACE


const int64_t TINYJS_LOOP_MAX_ITERATIONS = 8192;

enum LEX_TYPES
{
	LEX_EOF = 0,
	LEX_ID = 256,
	LEX_INT,
	LEX_FLOAT,
	LEX_STR,

	LEX_EQUAL,
	LEX_TYPEEQUAL,
	LEX_NEQUAL,
	LEX_NTYPEEQUAL,
	LEX_LEQUAL,
	LEX_LSHIFT,
	LEX_LSHIFTEQUAL,
	LEX_GEQUAL,
	LEX_RSHIFT,
	LEX_RSHIFTUNSIGNED,
	LEX_RSHIFTEQUAL,
	LEX_PLUSEQUAL,
	LEX_MINUSEQUAL,
	LEX_MULEQUAL,
	LEX_DIVEQUAL,
	LEX_PLUSPLUS,
	LEX_MINUSMINUS,
	LEX_ANDEQUAL,
	LEX_ANDAND,
	LEX_OREQUAL,
	LEX_OROR,
	LEX_XOREQUAL,
	// reserved words
#define LEX_R_LIST_START LEX_R_IF
	LEX_R_IF,
	LEX_R_ELSE,
	LEX_R_DO,
	LEX_R_WHILE,
	LEX_R_FOR,
	LEX_R_BREAK,
	LEX_R_CONTINUE,
	LEX_R_FUNCTION,
	LEX_R_RETURN,
	LEX_R_VAR,
	LEX_R_TRUE,
	LEX_R_FALSE,
	LEX_R_NULL,
	LEX_R_UNDEFINED,
	LEX_R_NEW,

	LEX_R_LIST_END /* always the last entry */
};

#define SVF_FUNCTION					0x0001
#define SVF_OBJECT						0x0002
#define SVF_ARRAY						0x0004
#define SVF_FLOAT						0x0008      // floating point number
#define SVF_INTEGER						0x0010      // integer number
#define SVF_STRING						0x0020      // string
#define SVF_NULL						0x0040      // a legitimate, null value
#define SVF_NATIVE						0x0080      // to specify this is a native function
#define SVF_VALUE_STRING_CACHED			0x0100

#define SVF_NUMERIC_MASK				(SVF_NULL | SVF_FLOAT | SVF_INTEGER)
#define SVF_VARTYPE_MASK				(SVF_NULL | SVF_FLOAT | SVF_INTEGER | SVF_STRING | SVF_FUNCTION | SVF_OBJECT | SVF_ARRAY)

#define TINYJS_RETURN_VAR			_T("return")
#define TINYJS_PROTOTYPE_CLASS		_T("prototype")
#define TINYJS_TEMP_NAME			_T("")
#define TINYJS_BLANK_DATA			_T("")

#define TINYJS_RESERVED_STRING_SIZE		64

#define TINYJS_GETALL				-1

class CScriptException
{
public:
	tstring text;
	CScriptException(const tstring &exceptionText);
};

typedef int64_t		JSTokenType;
typedef uint32_t	JSFlagsType;

class CScriptLex
{
protected:
	typedef std::vector<CScriptLex> TScriptLexStore;
	static TScriptLexStore s_Store;

public:
	CScriptLex();
	~CScriptLex();
	static CScriptLex *GetNew(const TCHAR *text, bool copy_text = true, size_t start_idx = 0, size_t end_idx = TINYJS_GETALL);
	void Release();

	TCHAR currCh, nextCh;
	int64_t tk; ///< The type of the token that we have
	int64_t tokenStart; ///< Position in the data at the beginning of the token we have here
	int64_t tokenEnd; ///< Position in the data at the last character of the token we have here
	int64_t tokenLastEnd; ///< Position in the data at the last character of the last token
	tstring tkStr; ///< Data contained in the token we have here

	void match(int64_t expected_tk); ///< Lexical match wotsit
	static tstring getTokenStr(int64_t token); ///< Get the string representation of the given token
	void reset(); ///< Reset this lex so we can start again

	tstring getSubString(int64_t pos); ///< Return a sub-string from the given position up until right now
	CScriptLex *getSubLex(int64_t lastPosition); ///< Return a sub-lexer from the given position up until right now

	tstring getPosition(int64_t pos = TINYJS_GETALL); ///< Return a string representing the position in lines and columns of the character pos given

protected:
	/* When we go into a loop, we use getSubLex to get a lexer for just the sub-part of the
	   relevant string. This doesn't re-allocate and copy the string, but instead copies
	   the data pointer and sets dataOwned to false, and dataStart/dataEnd to the relevant things. */
	TCHAR *data; ///< Data string to get tokens from
	int64_t dataStart, dataEnd; ///< Start and end position in data string
	bool dataOwned; ///< Do we own this data string?

	int64_t dataPos; ///< Position in data (we CAN go past the end of the string here)
	bool m_bInUse;

	void getNextCh();
	void getNextToken(); ///< Get the text token from our text string
};

class CScriptVar;

typedef void (*JSCallback)(CScriptVar *var, void *userdata);

class CScriptVarLink
{
public:
	tstring m_Name;
	struct
	{
		CScriptVarLink *next;
		CScriptVarLink *prev;
	} m_Sibling;

	CScriptVar *m_Var;
	bool m_Owned;

	CScriptVarLink(CScriptVar *var, const TCHAR *name = TINYJS_TEMP_NAME);
	CScriptVarLink(const CScriptVarLink &link); ///< Copy constructor
	~CScriptVarLink();
	void ReplaceWith(CScriptVar *newVar); ///< Replace the Variable pointed to
	void ReplaceWith(CScriptVarLink *newVar); ///< Replace the Variable pointed to (just dereferences)
	int64_t GetIntName(); ///< Get the name as an integer (for arrays)
	void SetIntName(int64_t n); ///< Set the name as an integer (for arrays)
};

/// Variable class (containing a doubly-linked list of children)
class CScriptVar
{
public:
	CScriptVar(); ///< Create undefined
	CScriptVar(const TCHAR *val, JSFlagsType flags); ///< User defined
	CScriptVar(const TCHAR *val); ///< Create a string
	CScriptVar(float val);
	CScriptVar(int64_t val);
	~CScriptVar();

	CScriptVar *GetReturnVar(); ///< If this is a function, get the result value (for use by native functions)
	void SetReturnVar(CScriptVar *var); ///< Set the result value. Use this when setting complex return data as it avoids a deepCopy()
	CScriptVar *GetParameter(const TCHAR *name); ///< If this is a function, get the parameter with the given name (for use by native functions)

	CScriptVarLink *FindChild(const TCHAR *child_name); ///< Tries to Find a child with the given name, may return 0
	CScriptVarLink *FindChildOrCreate(const TCHAR *child_name, JSFlagsType varFlags = 0); ///< Tries to Find a child with the given name, or will create it with the given flags
	CScriptVarLink *FindChildOrCreateByPath(const TCHAR *path); ///< Tries to Find a child with the given path (separated by dots)
	CScriptVarLink *AddChild(const TCHAR *child_name, CScriptVar *child = nullptr, bool overwrite = false);
	void RemoveChild(CScriptVar *child);
	void RemoveLink(CScriptVarLink *link); ///< Remove a specific link (this is faster than Finding via a child)
	void RemoveAllChildren();
	CScriptVar *GetArrayIndex(int64_t idx); ///< The the value at an array index
	void SetArrayIndex(int64_t idx, CScriptVar *value); ///< Set the value at an array index
	int64_t GetArrayLength(); ///< If this is an array, return the number of items in it (else 0)
	int64_t GetNumChildren(); ///< Get the number of children

	int64_t GetInt();
	bool GetBool() { return GetInt() != 0; }
	float GetFloat();
	const TCHAR *GetString();
	void SetInt(int64_t val);
	void SetFloat(float val);
	void SetString(const TCHAR *val);
	void SetUndefined();
	void SetArray();
	bool Equals(CScriptVar *v);

	bool IsInt() { return (flags & SVF_INTEGER) != 0; }
	bool IsFloat() { return (flags & SVF_FLOAT) != 0; }
	bool IsString() { return (flags & SVF_STRING) != 0; }
	bool IsNumeric() { return (flags & SVF_NUMERIC_MASK) != 0; }
	bool IsFunction() { return (flags & SVF_FUNCTION) != 0; }
	bool IsObject() { return (flags & SVF_OBJECT) != 0; }
	bool IsArray() { return (flags & SVF_ARRAY) != 0; }
	bool IsNative() { return (flags & SVF_NATIVE) != 0; }
	bool IsUndefined() { return (flags & SVF_VARTYPE_MASK) == 0; }
	bool IsNull() { return (flags & SVF_NULL) != 0; }
	bool IsBasic() { return m_Child.first == 0; } ///< Is this *not* an array/object/etc

	CScriptVar *MathsOp(CScriptVar *b, int64_t op); ///< do a maths op with another script variable
	void CopyValue(CScriptVar *val); ///< copy the value from the value given
	CScriptVar *DeepCopy(); ///< deep copy this node and return the result

	void Trace(tstring indentStr = _T(""), const tstring &name = _T("")); ///< Dump out the contents of this using trace
	void SetCallback(JSCallback callback, void *userdata); ///< Set the callback for native functions

	struct
	{
		CScriptVarLink *first;
		CScriptVarLink *last;
	} m_Child;

	/// For memory management/garbage collection
	CScriptVar *ref(); ///< Add reference to this variable
	void unref(); ///< Remove a reference, and delete this variable if required
	int64_t getRefs(); ///< Get the number of references to this script variable
protected:
	int64_t m_RefCt; ///< The number of references held to this - used for garbage collection

	tstring strData; ///< The contents of this variable if it is a string
	int64_t intData; ///< The contents of this variable if it is an int64_t
	float floatData; ///< The contents of this variable if it is a float
	int64_t flags; ///< the flags determine the type of the variable - int64_t/float/string/etc
	JSCallback jsCallback; ///< Callback for native functions
	void *jsCallbackUserData; ///< user data passed as second argument to native functions

	void init(); ///< initialisation of data members

	/** Copy the basic data and flags from the variable given, with no
	  * children. Should be used internally only - by copyValue and deepCopy */
	void CopySimpleData(CScriptVar *val);

	friend class CTinyJS;
};

class CTinyJS
{
public:
	CTinyJS();
	~CTinyJS();

	bool Execute(const TCHAR *code);

	/// add a native function to be called from TinyJS
	/** example:
	   \code
		   void scRandInt(CScriptVar *c, void *userdata) { ... }
		   tinyJS->addNative("function randInt(min, max)", scRandInt, 0);
	   \endcode

	   or

	   \code
		   void scSubstring(CScriptVar *c, void *userdata) { ... }
		   tinyJS->addNative("function String.substring(lo, hi)", scSubstring, 0);
	   \endcode
	*/
	void AddNative(const TCHAR *funcDesc, JSCallback ptr, void *userdata);

	/// Get the given variable specified by a path (var1.var2.etc), or return 0
	CScriptVar *GetScriptVariable(const TCHAR *path);

	/// Get the value of the given variable, or return 0
	const TCHAR *GetVariable(const TCHAR *path);

	/// set the value of the given variable, return trur if it exists and gets set
	bool SetVariable(const TCHAR *path, const TCHAR *vardata);

	/// Send all variables to stdout
	void Trace();

	CScriptVar *m_Root;   /// m_Root of symbol table
	c3::System *m_pSys;
	c3::Object *m_pObj;
private:

	CScriptLex *l;             /// current lexer
	std::vector<CScriptVar *> scopes; /// stack of scopes when parsing
#ifdef TINYJS_CALL_STACK
	std::vector<tstring> call_stack; /// Names of places called so we can show when erroring
#endif
	tstring last_error;

	CScriptVar *stringClass; /// Built in string class
	CScriptVar *objectClass; /// Built in object class
	CScriptVar *arrayClass; /// Built in array class

	// parsing - in order of precedence
	CScriptVarLink *functionCall(bool &execute, CScriptVarLink *function, CScriptVar *parent);
	CScriptVarLink *factor(bool &execute);
	CScriptVarLink *unary(bool &execute);
	CScriptVarLink *term(bool &execute);
	CScriptVarLink *expression(bool &execute);
	CScriptVarLink *shift(bool &execute);
	CScriptVarLink *condition(bool &execute);
	CScriptVarLink *logic(bool &execute);
	CScriptVarLink *ternary(bool &execute);
	CScriptVarLink *base(bool &execute);
	void block(bool &execute);
	void statement(bool &execute);

	// parsing utility functions
	CScriptVarLink *parseFunctionDefinition();
	void parseFunctionArguments(CScriptVar *funcVar);

	CScriptVarLink *FindInScopes(const TCHAR *childName); ///< Finds a child, looking recursively up the scopes

	/// Look up in any parent classes of the given object
	CScriptVarLink *FindInParentClasses(CScriptVar *object, const TCHAR *name);
};

#endif
