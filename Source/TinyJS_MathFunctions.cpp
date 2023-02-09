/*
 * TinyJS
 *
 * A single-file Javascript-alike engine
 *
 * -  Math and Trigonometry functions
 *
 * Authored By O.Z.L.B. <ozlbinfo@gmail.com>
 *
 * Copyright (C) 2011 O.Z.L.B.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS"), WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "pch.h"

#include <math.h>
#include <cstdlib>
#include <sstream>
#include "TinyJS_MathFunctions.h"

using namespace std;

#define k_E                 exp(1.0)
#define k_PI                3.1415926535897932384626433832795

#define F_ABS(a)            ((a)>=0 ? (a) : (-(a)))
#define F_MIN(a,b)          ((a)>(b) ? (b) : (a))
#define F_MAX(a,b)          ((a)>(b) ? (a) : (b))
#define F_SGN(a)            ((a)>0 ? 1 : ((a)<0 ? -1 : 0 ))
#define F_RNG(a,min,max)    ((a)<(min) ? min : ((a)>(max) ? max : a ))
#define F_ROUND(a)          ((a)>0 ? (int64_t) ((a)+0.5) : (int64_t) ((a)-0.5) )

//CScriptVar shortcut macro
#define scIsInt(a)          ( c->getParameter(a)->isInt() )
#define scIsDouble(a)       ( c->getParameter(a)->isDouble() )  
#define scGetInt(a)         ( c->getParameter(a)->getInt() )
#define scGetDouble(a)      ( c->getParameter(a)->getDouble() )  
#define scReturnInt(a)      ( c->getReturnVar()->setInt(a) )
#define scReturnDouble(a)   ( c->getReturnVar()->setDouble(a) )  

#ifdef _MSC_VER
namespace
{
	double asinh(const double &value)
	{
		double returned;

		if (value > 0)
			returned = log(value + sqrt(value * value + 1));
		else
			returned = -log(-value + sqrt(value * value + 1));

		return(returned);
	}

	double acosh(const double &value)
	{
		double returned;

		if (value > 0)
			returned = log(value + sqrt(value * value - 1));
		else
			returned = -log(-value + sqrt(value * value - 1));

		return(returned);
	}
}
#endif

//Math.abs(x) - returns absolute of given value
void scMathAbs(CScriptVar *c, void *userdata)
{
	if (scIsInt(_T("a")))
	{
		scReturnInt(F_ABS(scGetInt(_T("a"))));
	}
	else if (scIsDouble(_T("a")))
	{
		scReturnDouble(F_ABS(scGetDouble(_T("a"))));
	}
}

//Math.round(a) - returns nearest round of given value
void scMathRound(CScriptVar *c, void *userdata)
{
	if (scIsInt(_T("a")))
	{
		scReturnInt(F_ROUND(scGetInt(_T("a"))));
	}
	else if (scIsDouble(_T("a")))
	{
		scReturnDouble(round(scGetDouble(_T("a"))));
	}
}

//Math.min(a,b) - returns minimum of two given values 
void scMathMin(CScriptVar *c, void *userdata)
{
	if ((scIsInt(_T("a"))) && (scIsInt(_T("b"))))
	{
		scReturnInt(F_MIN(scGetInt(_T("a")), scGetInt(_T("b"))));
	}
	else
	{
		scReturnDouble(F_MIN(scGetDouble(_T("a")), scGetDouble(_T("b"))));
	}
}

//Math.max(a,b) - returns maximum of two given values  
void scMathMax(CScriptVar *c, void *userdata)
{
	if ((scIsInt(_T("a"))) && (scIsInt(_T("b"))))
	{
		scReturnInt(F_MAX(scGetInt(_T("a")), scGetInt(_T("b"))));
	}
	else
	{
		scReturnDouble(F_MAX(scGetDouble(_T("a")), scGetDouble(_T("b"))));
	}
}

//Math.range(x,a,b) - returns value limited between two given values  
void scMathRange(CScriptVar *c, void *userdata)
{
	if ((scIsInt(_T("x"))))
	{
		scReturnInt(F_RNG(scGetInt(_T("x")), scGetInt(_T("a")), scGetInt(_T("b"))));
	}
	else
	{
		scReturnDouble(F_RNG(scGetDouble(_T("x")), scGetDouble(_T("a")), scGetDouble(_T("b"))));
	}
}

//Math.sign(a) - returns sign of given value (-1==negative,0=zero,1=positive)
void scMathSign(CScriptVar *c, void *userdata)
{
	if (scIsInt(_T("a")))
	{
		scReturnInt(F_SGN(scGetInt(_T("a"))));
	}
	else if (scIsDouble(_T("a")))
	{
		scReturnDouble(F_SGN(scGetDouble(_T("a"))));
	}
}

//Math.PI() - returns PI value
void scMathPI(CScriptVar *c, void *userdata)
{
	scReturnDouble(k_PI);
}

//Math.toDegrees(a) - returns degree value of a given angle in radians
void scMathToDegrees(CScriptVar *c, void *userdata)
{
	scReturnDouble((180.0 / k_PI) * (scGetDouble(_T("a"))));
}

//Math.toRadians(a) - returns radians value of a given angle in degrees
void scMathToRadians(CScriptVar *c, void *userdata)
{
	scReturnDouble((k_PI / 180.0) * (scGetDouble(_T("a"))));
}

//Math.sin(a) - returns trig. sine of given angle in radians
void scMathSin(CScriptVar *c, void *userdata)
{
	scReturnDouble(sin(scGetDouble(_T("a"))));
}

//Math.asin(a) - returns trig. arcsine of given angle in radians
void scMathASin(CScriptVar *c, void *userdata)
{
	scReturnDouble(asin(scGetDouble(_T("a"))));
}

//Math.cos(a) - returns trig. cosine of given angle in radians
void scMathCos(CScriptVar *c, void *userdata)
{
	scReturnDouble(cos(scGetDouble(_T("a"))));
}

//Math.acos(a) - returns trig. arccosine of given angle in radians
void scMathACos(CScriptVar *c, void *userdata)
{
	scReturnDouble(acos(scGetDouble(_T("a"))));
}

//Math.tan(a) - returns trig. tangent of given angle in radians
void scMathTan(CScriptVar *c, void *userdata)
{
	scReturnDouble(tan(scGetDouble(_T("a"))));
}

//Math.atan(a) - returns trig. arctangent of given angle in radians
void scMathATan(CScriptVar *c, void *userdata)
{
	scReturnDouble(atan(scGetDouble(_T("a"))));
}

//Math.sinh(a) - returns trig. hyperbolic sine of given angle in radians
void scMathSinh(CScriptVar *c, void *userdata)
{
	scReturnDouble(sinh(scGetDouble(_T("a"))));
}

//Math.asinh(a) - returns trig. hyperbolic arcsine of given angle in radians
void scMathASinh(CScriptVar *c, void *userdata)
{
	scReturnDouble(asinh((long double)scGetDouble(_T("a"))));
}

//Math.cosh(a) - returns trig. hyperbolic cosine of given angle in radians
void scMathCosh(CScriptVar *c, void *userdata)
{
	scReturnDouble(cosh(scGetDouble(_T("a"))));
}

//Math.acosh(a) - returns trig. hyperbolic arccosine of given angle in radians
void scMathACosh(CScriptVar *c, void *userdata)
{
	scReturnDouble(acosh((long double)scGetDouble(_T("a"))));
}

//Math.tanh(a) - returns trig. hyperbolic tangent of given angle in radians
void scMathTanh(CScriptVar *c, void *userdata)
{
	scReturnDouble(tanh(scGetDouble(_T("a"))));
}

//Math.atan(a) - returns trig. hyperbolic arctangent of given angle in radians
void scMathATanh(CScriptVar *c, void *userdata)
{
	scReturnDouble(atan(scGetDouble(_T("a"))));
}

//Math.E() - returns E Neplero value
void scMathE(CScriptVar *c, void *userdata)
{
	scReturnDouble(k_E);
}

//Math.log(a) - returns natural logaritm (base E) of given value
void scMathLog(CScriptVar *c, void *userdata)
{
	scReturnDouble(log(scGetDouble(_T("a"))));
}

//Math.log10(a) - returns logaritm(base 10) of given value
void scMathLog10(CScriptVar *c, void *userdata)
{
	scReturnDouble(log10(scGetDouble(_T("a"))));
}

//Math.exp(a) - returns e raised to the power of a given number
void scMathExp(CScriptVar *c, void *userdata)
{
	scReturnDouble(exp(scGetDouble(_T("a"))));
}

//Math.pow(a,b) - returns the result of a number raised to a power (a)^(b)
void scMathPow(CScriptVar *c, void *userdata)
{
	scReturnDouble(pow(scGetDouble(_T("a")), scGetDouble(_T("b"))));
}

//Math.sqr(a) - returns square of given value
void scMathSqr(CScriptVar *c, void *userdata)
{
	scReturnDouble((scGetDouble(_T("a")) * scGetDouble(_T("a"))));
}

//Math.sqrt(a) - returns square root of given value
void scMathSqrt(CScriptVar *c, void *userdata)
{
	scReturnDouble(sqrt(scGetDouble(_T("a"))));
}

//Math.random(lo, hi) - returns a random value between lo and hi
void scRandom(CScriptVar *c, void *userdata)
{
	CScriptVar *plo = c->getParameter(_T("lo"));
	CScriptVar *phi = c->getParameter(_T("hi"));

	uint16_t r = rand();
	if (plo->isDouble() || phi->isDouble())
	{
		float pct = (float)r / (float)USHRT_MAX;
		scReturnDouble((pct * (phi->getDouble() - plo->getDouble())) + plo->getDouble());
	}
	else
	{
		scReturnInt((r % (phi->getInt() - plo->getInt())) + plo->getInt());
	}
}

static const TCHAR *elnames[4] = {_T("x"), _T("y"), _T("z"), _T("w")};

//Math.lerp(a, b, t) - linearly interpolates between two values. result is (b - a) * t + a
void scMathLerp(CScriptVar *c, void *userdata)
{
	CScriptVar *pa = c->getParameter(_T("a"));
	CScriptVar *pb = c->getParameter(_T("b"));
	CScriptVar *pr = c->getReturnVar();
	int64_t elct = pa->getChildren();

	if (elct != pb->getChildren() || !pr || (elct > 4))
		return;

	CScriptVar *pt = c->getParameter(_T("t"));

	int64_t i;
	while ((i = pr->getChildren()) < elct)
	{
		pr->addChild(elnames[i]);
	}

	CScriptVarLink *pac = pa->firstChild;
	CScriptVarLink *pbc = pb->firstChild;
	CScriptVarLink *pcc = pr->firstChild;
	for (i = 0; i < elct; i++)
	{
		double va = pac->var->getDouble();
		pcc->var->setDouble((pbc->var->getDouble() - va) * pt->getDouble() + va);

		pac = pac->nextSibling;
		pbc = pbc->nextSibling;
		pcc = pcc->nextSibling;
	}
}


//Math.slerp(a, b, t) - spherical linear interpolation for quaternions
void scMathSlerp(CScriptVar *c, void *userdata)
{
	CScriptVar *pa = c->getParameter(_T("a"));
	CScriptVar *pb = c->getParameter(_T("b"));
	CScriptVar *pr = c->getReturnVar();
	int64_t elct = pa->getChildren();

	if ((elct != 4) || elct != pb->getChildren() || !pr)
		return;

	CScriptVar *pt = c->getParameter(_T("t"));

	glm::fquat qa, qb;
	CScriptVarLink *pac = pa->firstChild;
	CScriptVarLink *pbc = pb->firstChild;
	glm::fquat::length_type i;
	for (i = 0; i < 4; i++)
	{
		qa[i] = (float)pac->var->getDouble();
		qb[i] = (float)pbc->var->getDouble();

		pac = pac->nextSibling;
		pbc = pbc->nextSibling;
	}

	glm::fquat qr = glm::slerp(qa, qb, (float)pt->getDouble());

	i = (glm::fquat::length_type)pr->getChildren();
	for (; i < 4; i++)
	{
		pr->addChild(elnames[i]);
	}

	CScriptVarLink *prc = pr->firstChild;
	for (i = 0; i < 4; i++)
	{
		prc->var->setDouble(qr[i]);

		prc = prc->nextSibling;
	}
}


// ----------------------------------------------- Register Functions
void registerMathFunctions(CTinyJS *tinyJS)
{

// --- Math and Trigonometry functions ---
	tinyJS->addNative(_T("function Math.abs(a)"), scMathAbs, 0);
	tinyJS->addNative(_T("function Math.round(a)"), scMathRound, 0);
	tinyJS->addNative(_T("function Math.min(a,b)"), scMathMin, 0);
	tinyJS->addNative(_T("function Math.max(a,b)"), scMathMax, 0);
	tinyJS->addNative(_T("function Math.range(x,a,b)"), scMathRange, 0);
	tinyJS->addNative(_T("function Math.sign(a)"), scMathSign, 0);
	tinyJS->addNative(_T("function Math.random(lo, hi)"), scRandom, 0);

	tinyJS->addNative(_T("function Math.PI()"), scMathPI, 0);
	tinyJS->addNative(_T("function Math.toDegrees(a)"), scMathToDegrees, 0);
	tinyJS->addNative(_T("function Math.toRadians(a)"), scMathToRadians, 0);
	tinyJS->addNative(_T("function Math.sin(a)"), scMathSin, 0);
	tinyJS->addNative(_T("function Math.asin(a)"), scMathASin, 0);
	tinyJS->addNative(_T("function Math.cos(a)"), scMathCos, 0);
	tinyJS->addNative(_T("function Math.acos(a)"), scMathACos, 0);
	tinyJS->addNative(_T("function Math.tan(a)"), scMathTan, 0);
	tinyJS->addNative(_T("function Math.atan(a)"), scMathATan, 0);
	tinyJS->addNative(_T("function Math.sinh(a)"), scMathSinh, 0);
	tinyJS->addNative(_T("function Math.asinh(a)"), scMathASinh, 0);
	tinyJS->addNative(_T("function Math.cosh(a)"), scMathCosh, 0);
	tinyJS->addNative(_T("function Math.acosh(a)"), scMathACosh, 0);
	tinyJS->addNative(_T("function Math.tanh(a)"), scMathTanh, 0);
	tinyJS->addNative(_T("function Math.atanh(a)"), scMathATanh, 0);

	tinyJS->addNative(_T("function Math.E()"), scMathE, 0);
	tinyJS->addNative(_T("function Math.log(a)"), scMathLog, 0);
	tinyJS->addNative(_T("function Math.log10(a)"), scMathLog10, 0);
	tinyJS->addNative(_T("function Math.exp(a)"), scMathExp, 0);
	tinyJS->addNative(_T("function Math.pow(a,b)"), scMathPow, 0);

	tinyJS->addNative(_T("function Math.sqr(a)"), scMathSqr, 0);
	tinyJS->addNative(_T("function Math.sqrt(a)"), scMathSqrt, 0);

	tinyJS->addNative(_T("function Math.lerp(a, b, t)"), scMathLerp, 0);
	tinyJS->addNative(_T("function Math.slerp(a, b, t)"), scMathSlerp, 0);
}
