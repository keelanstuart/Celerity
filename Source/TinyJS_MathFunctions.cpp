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

#define k_E                 expf(1.0f)
#define k_PI                3.14159f

#define F_ABS(a)                ((a) >= 0 ? (a) : (-(a)))
#define F_MIN(a,b)              ((a) > (b) ? (b) : (a))
#define F_MAX(a,b)              ((a) > (b) ? (a) : (b))
#define F_SGN(a)                ((a) > 0 ? 1 : ((a) < 0 ? -1 : 0 ))
#define F_RNG(a,minval,maxval)  ((a) < (minval) ? minval : ((a) > (maxval) ? maxval : a ))
#define F_ROUND(a)              ((a) > 0 ? (int64_t) ((a) + 0.5f) : (int64_t) ((a) - 0.5f) )

//CScriptVar shortcut macro
#define scIsInt(a)              ( c->GetParameter(a)->IsInt() )
#define scIsFloat(a)            ( c->GetParameter(a)->IsFloat() )  
#define scGetInt(a)             ( c->GetParameter(a)->GetInt() )
#define scGetFloat(a)           ( c->GetParameter(a)->GetFloat() )  
#define scReturnInt(a)          ( c->GetReturnVar()->SetInt(a) )
#define scReturnFloat(a)        ( c->GetReturnVar()->SetFloat(a) )  

#ifdef _MSC_VER
namespace
{
	float asinhf(const float value)
	{
		float returned;

		if (value > 0)
			returned = logf(value + sqrtf(value * value + 1));
		else
			returned = -logf(-value + sqrtf(value * value + 1));

		return(returned);
	}

	float acoshf(const float value)
	{
		float returned;

		if (value > 0)
			returned = logf(value + sqrtf(value * value - 1));
		else
			returned = -logf(-value + sqrtf(value * value - 1));

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
	else if (scIsFloat(_T("a")))
	{
		scReturnFloat(F_ABS(scGetFloat(_T("a"))));
	}
}

//Math.round(a) - returns nearest round of given value
void scMathRound(CScriptVar *c, void *userdata)
{
	if (scIsInt(_T("a")))
	{
		scReturnInt(F_ROUND(scGetInt(_T("a"))));
	}
	else if (scIsFloat(_T("a")))
	{
		scReturnFloat(round(scGetFloat(_T("a"))));
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
		scReturnFloat(F_MIN(scGetFloat(_T("a")), scGetFloat(_T("b"))));
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
		scReturnFloat(F_MAX(scGetFloat(_T("a")), scGetFloat(_T("b"))));
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
		scReturnFloat(F_RNG(scGetFloat(_T("x")), scGetFloat(_T("a")), scGetFloat(_T("b"))));
	}
}

//Math.sign(a) - returns sign of given value (-1==negative,0=zero,1=positive)
void scMathSign(CScriptVar *c, void *userdata)
{
	if (scIsInt(_T("a")))
	{
		scReturnInt(F_SGN(scGetInt(_T("a"))));
	}
	else if (scIsFloat(_T("a")))
	{
		scReturnFloat((float)F_SGN(scGetFloat(_T("a"))));
	}
}

//Math.even(a) - returns if the value is even (1==even,0==odd)
void scMathEven(CScriptVar *c, void *userdata)
{
	if (scIsInt(_T("a")))
	{
		scReturnInt((scGetInt(_T("a"))) & 1);
	}
	else if (scIsFloat(_T("a")))
	{
		scReturnFloat((float)((int)(scGetFloat(_T("a"))) & 1));
	}
}

//Math.PI() - returns PI value
void scMathPI(CScriptVar *c, void *userdata)
{
	scReturnFloat(k_PI);
}

//Math.toDegrees(a) - returns degree value of a given angle in radians
void scMathToDegrees(CScriptVar *c, void *userdata)
{
	scReturnFloat(glm::degrees(scGetFloat(_T("a"))));
}

//Math.toRadians(a) - returns radians value of a given angle in degrees
void scMathToRadians(CScriptVar *c, void *userdata)
{
	scReturnFloat(glm::radians(scGetFloat(_T("a"))));
}

//Math.sin(a) - returns trig. sine of given angle in radians
void scMathSin(CScriptVar *c, void *userdata)
{
	scReturnFloat(sinf(scGetFloat(_T("a"))));
}

//Math.asin(a) - returns trig. arcsine of given angle in radians
void scMathASin(CScriptVar *c, void *userdata)
{
	scReturnFloat(asinf(scGetFloat(_T("a"))));
}

//Math.cos(a) - returns trig. cosine of given angle in radians
void scMathCos(CScriptVar *c, void *userdata)
{
	scReturnFloat(cosf(scGetFloat(_T("a"))));
}

//Math.acos(a) - returns trig. arccosine of given angle in radians
void scMathACos(CScriptVar *c, void *userdata)
{
	scReturnFloat(acosf(scGetFloat(_T("a"))));
}

//Math.tan(a) - returns trig. tangent of given angle in radians
void scMathTan(CScriptVar *c, void *userdata)
{
	scReturnFloat(tanf(scGetFloat(_T("a"))));
}

//Math.atan(a) - returns trig. arctangent of given angle in radians
void scMathATan(CScriptVar *c, void *userdata)
{
	scReturnFloat(atanf(scGetFloat(_T("a"))));
}

//Math.sinh(a) - returns trig. hyperbolic sine of given angle in radians
void scMathSinh(CScriptVar *c, void *userdata)
{
	scReturnFloat(sinhf(scGetFloat(_T("a"))));
}

//Math.asinh(a) - returns trig. hyperbolic arcsine of given angle in radians
void scMathASinh(CScriptVar *c, void *userdata)
{
	scReturnFloat(asinh(scGetFloat(_T("a"))));
}

//Math.cosh(a) - returns trig. hyperbolic cosine of given angle in radians
void scMathCosh(CScriptVar *c, void *userdata)
{
	scReturnFloat(coshf(scGetFloat(_T("a"))));
}

//Math.acosh(a) - returns trig. hyperbolic arccosine of given angle in radians
void scMathACosh(CScriptVar *c, void *userdata)
{
	scReturnFloat(acosh(scGetFloat(_T("a"))));
}

//Math.tanh(a) - returns trig. hyperbolic tangent of given angle in radians
void scMathTanh(CScriptVar *c, void *userdata)
{
	scReturnFloat(tanhf(scGetFloat(_T("a"))));
}

//Math.atan(a) - returns trig. hyperbolic arctangent of given angle in radians
void scMathATanh(CScriptVar *c, void *userdata)
{
	scReturnFloat(atanf(scGetFloat(_T("a"))));
}

//Math.E() - returns E Neplero value
void scMathE(CScriptVar *c, void *userdata)
{
	scReturnFloat(k_E);
}

//Math.log(a) - returns natural logaritm (base E) of given value
void scMathLog(CScriptVar *c, void *userdata)
{
	scReturnFloat(logf(scGetFloat(_T("a"))));
}

//Math.log10(a) - returns logaritm(base 10) of given value
void scMathLog10(CScriptVar *c, void *userdata)
{
	scReturnFloat(log10f(scGetFloat(_T("a"))));
}

//Math.exp(a) - returns e raised to the power of a given number
void scMathExp(CScriptVar *c, void *userdata)
{
	scReturnFloat(expf(scGetFloat(_T("a"))));
}

//Math.pow(a,b) - returns the result of a number raised to a power (a)^(b)
void scMathPow(CScriptVar *c, void *userdata)
{
	scReturnFloat(powf(scGetFloat(_T("a")), scGetFloat(_T("b"))));
}

//Math.sqr(a) - returns square of given value
void scMathSqr(CScriptVar *c, void *userdata)
{
	scReturnFloat((scGetFloat(_T("a")) * scGetFloat(_T("a"))));
}

//Math.sqrt(a) - returns square root of given value
void scMathSqrt(CScriptVar *c, void *userdata)
{
	scReturnFloat(sqrtf(scGetFloat(_T("a"))));
}

//Math.random(lo, hi) - returns a random value between lo and hi
void scRandom(CScriptVar *c, void *userdata)
{
	CScriptVar *plo = c->GetParameter(_T("lo"));
	CScriptVar *phi = c->GetParameter(_T("hi"));

	uint16_t r = rand();
	if (plo->IsFloat() || phi->IsFloat())
	{
		float pct = (float)r / (float)RAND_MAX;

		float l = plo->GetFloat();
		float h = phi->GetFloat();
		float t = (pct * (h - l)) + l;
		scReturnFloat(t);
	}
	else
	{
		int64_t l = plo->GetInt();
		int64_t h = phi->GetInt();
		int64_t t = (r % (h - l)) + l;
		scReturnInt(t);
	}
}

static const TCHAR *elnames[4] ={ _T("x"), _T("y"), _T("z"), _T("w") };

//Math.lerp(a, b, t) - linearly interpolates between two values. result is (b - a) * t + a
void scMathLerp(CScriptVar *c, void *userdata)
{
	CScriptVar *pa = c->GetParameter(_T("a"));
	CScriptVar *pb = c->GetParameter(_T("b"));
	CScriptVar *pr = c->GetReturnVar();
	int64_t elct = pa->GetNumChildren();

	if (elct != pb->GetNumChildren() || !pr || (elct > 4))
		return;

	CScriptVar *pt = c->GetParameter(_T("t"));
	float t = glm::fclamp(pt->GetFloat(), 0.0f, 1.0f);

	if (pa->IsFloat())
	{
		pr->SetFloat((pb->GetFloat() - pa->GetFloat()) * t + pa->GetFloat());
	}
	else
	{
		for (int64_t i = 0; i < elct; i++)
		{
			CScriptVarLink *pacomp = pa->GetChild(i);
			CScriptVarLink *pbcomp = pb->GetChild(i);
			CScriptVarLink *prcomp = pr->FindChildOrCreate(pacomp->m_Name.c_str());

			float va = pacomp->m_Var->GetFloat();
			float vb = pbcomp->m_Var->GetFloat();

			prcomp->m_Var->SetFloat((vb - va) * t + va);
		}
	}
}


//Math.slerp(a, b, t) - spherical linear interpolation for quaternions
void scMathSlerp(CScriptVar *c, void *userdata)
{
	CScriptVar *pa = c->GetParameter(_T("a"));
	CScriptVar *pb = c->GetParameter(_T("b"));
	CScriptVar *pr = c->GetReturnVar();
	int64_t elct = pa->GetNumChildren();

	if (elct != pb->GetNumChildren() || !pr || (elct != 4))
		return;

	CScriptVar *pt = c->GetParameter(_T("t"));
	float t = pt->GetFloat();

	CScriptVarLink *prcomp[4];

	glm::fquat qa, qb;

	for (int64_t i = 0; i < elct; i++)
	{
		CScriptVarLink *pacomp = pa->GetChild(i);
		qa[(glm::fquat::length_type)i] = pacomp->m_Var->GetFloat();

		CScriptVarLink *pbcomp = pb->GetChild(i);
		qb[(glm::fquat::length_type)i] = pbcomp->m_Var->GetFloat();

		prcomp[(glm::fquat::length_type)i] = pr->FindChildOrCreate(pacomp->m_Name.c_str());
	}

	qa = glm::normalize(qa);
	qb = glm::normalize(qb);

	float d = glm::dot(qa, qb);
	if (d < 0.0f)
		qb = -qb; // flip it

	glm::fquat qr = glm::normalize(glm::slerp(qa, qb, t));

	for (int64_t i = 0; i < elct; i++)
	{
		prcomp[i]->m_Var->SetFloat(qr[(glm::fquat::length_type)i]);
	}
}


void scVec3Dot(CScriptVar *c, void *userdata)
{
	CScriptVar *pa = c->GetParameter(_T("a"));
	CScriptVar *pb = c->GetParameter(_T("b"));
	CScriptVar *pr = c->GetReturnVar();
	int64_t elct = pa->GetNumChildren();

	if (elct != pb->GetNumChildren() || !pr || (elct != 3))
		return;

	glm::fvec3 va, vb;

	for (int64_t i = 0; i < elct; i++)
	{
		CScriptVarLink *pacomp = pa->GetChild(i);
		va[(glm::fvec3::length_type)i] = pacomp->m_Var->GetFloat();

		CScriptVarLink *pbcomp = pb->GetChild(i);
		vb[(glm::fvec3::length_type)i] = pbcomp->m_Var->GetFloat();
	}

	float r = glm::dot(va, vb);

	pr->SetFloat(r);
}


void scVec3Cross(CScriptVar *c, void *userdata)
{
	CScriptVar *pa = c->GetParameter(_T("a"));
	CScriptVar *pb = c->GetParameter(_T("b"));
	CScriptVar *pr = c->GetReturnVar();
	int64_t elct = pa->GetNumChildren();

	if (elct != pb->GetNumChildren() || !pr || (elct != 3))
		return;

	CScriptVarLink *prcomp[4];

	glm::fvec3 va, vb, vr;

	for (int64_t i = 0; i < elct; i++)
	{
		CScriptVarLink *pacomp = pa->GetChild(i);
		va[(glm::fvec3::length_type)i] = pacomp->m_Var->GetFloat();

		CScriptVarLink *pbcomp = pb->GetChild(i);
		vb[(glm::fvec3::length_type)i] = pbcomp->m_Var->GetFloat();

		prcomp[(glm::fvec3::length_type)i] = pr->FindChildOrCreate(pacomp->m_Name.c_str());
	}

	vr = glm::cross(va, vb);

	for (int64_t i = 0; i < elct; i++)
	{
		prcomp[i]->m_Var->SetFloat(vr[(glm::fvec3::length_type)i]);
	}
}


void scVec3Length(CScriptVar *c, void *userdata)
{
	CScriptVar *pa = c->GetParameter(_T("a"));
	CScriptVar *pr = c->GetReturnVar();
	int64_t elct = pa->GetNumChildren();

	if (!pr || (elct != 3))
		return;

	glm::fvec3 va;

	for (int64_t i = 0; i < elct; i++)
	{
		CScriptVarLink *pacomp = pa->GetChild(i);
		va[(glm::fvec3::length_type)i] = pacomp->m_Var->GetFloat();
	}

	float r = glm::fastLength(va);

	pr->SetFloat(r);
}


void scVec3Normalize(CScriptVar *c, void *userdata)
{
	CScriptVar *pa = c->GetParameter(_T("a"));
	CScriptVar *pr = c->GetReturnVar();
	int64_t elct = pa->GetNumChildren();

	if (!pr || (elct != 3))
		return;

	CScriptVarLink *prcomp[4];

	glm::fvec3 va, vr;

	for (int64_t i = 0; i < elct; i++)
	{
		CScriptVarLink *pacomp = pa->GetChild(i);
		va[(glm::fvec3::length_type)i] = pacomp->m_Var->GetFloat();

		prcomp[(glm::fvec3::length_type)i] = pr->FindChildOrCreate(pacomp->m_Name.c_str());
	}

	vr = glm::normalize(va);

	for (int64_t i = 0; i < elct; i++)
	{
		prcomp[i]->m_Var->SetFloat(vr[(glm::fvec3::length_type)i]);
	}
}


void scVec3Add(CScriptVar *c, void *userdata)
{
	CScriptVar *pa = c->GetParameter(_T("a"));
	CScriptVar *pb = c->GetParameter(_T("b"));
	CScriptVar *pr = c->GetReturnVar();
	int64_t elct = pa->GetNumChildren();

	if (!pr || (elct != 3))
		return;

	CScriptVarLink *prcomp[4];

	glm::fvec3 va, vb, vr;

	for (int64_t i = 0; i < elct; i++)
	{
		CScriptVarLink *pacomp = pa->GetChild(i);
		va[(glm::fvec3::length_type)i] = pacomp->m_Var->GetFloat();

		CScriptVarLink *pbcomp = pb->GetChild(i);
		vb[(glm::fvec3::length_type)i] = pbcomp ? pbcomp->m_Var->GetFloat() : pb->GetFloat();

		prcomp[(glm::fvec3::length_type)i] = pr->FindChildOrCreate(pacomp->m_Name.c_str());
	}

	vr = va + vb;

	for (int64_t i = 0; i < elct; i++)
	{
		prcomp[i]->m_Var->SetFloat(vr[(glm::fvec3::length_type)i]);
	}
}


void scVec3Sub(CScriptVar *c, void *userdata)
{
	CScriptVar *pa = c->GetParameter(_T("a"));
	CScriptVar *pb = c->GetParameter(_T("b"));
	CScriptVar *pr = c->GetReturnVar();
	int64_t elct = pa->GetNumChildren();

	if (!pr || (elct != 3))
		return;

	CScriptVarLink *prcomp[4];

	glm::fvec3 va, vb, vr;

	for (int64_t i = 0; i < elct; i++)
	{
		CScriptVarLink *pacomp = pa->GetChild(i);
		va[(glm::fvec3::length_type)i] = pacomp->m_Var->GetFloat();

		CScriptVarLink *pbcomp = pb->GetChild(i);
		vb[(glm::fvec3::length_type)i] = pbcomp ? pbcomp->m_Var->GetFloat() : pb->GetFloat();

		prcomp[(glm::fvec3::length_type)i] = pr->FindChildOrCreate(pacomp->m_Name.c_str());
	}

	vr = va - vb;

	for (int64_t i = 0; i < elct; i++)
	{
		prcomp[i]->m_Var->SetFloat(vr[(glm::fvec3::length_type)i]);
	}
}


void scVec3Mul(CScriptVar *c, void *userdata)
{
	CScriptVar *pa = c->GetParameter(_T("a"));
	CScriptVar *pb = c->GetParameter(_T("b"));
	CScriptVar *pr = c->GetReturnVar();
	int64_t elct = pa->GetNumChildren();

	if (!pr || (elct != 3))
		return;

	CScriptVarLink *prcomp[4];

	glm::fvec3 va, vb, vr;

	for (int64_t i = 0; i < elct; i++)
	{
		CScriptVarLink *pacomp = pa->GetChild(i);
		va[(glm::fvec3::length_type)i] = pacomp->m_Var->GetFloat();

		CScriptVarLink *pbcomp = pb->GetChild(i);
		vb[(glm::fvec3::length_type)i] = pbcomp ? pbcomp->m_Var->GetFloat() : pb->GetFloat();

		prcomp[(glm::fvec3::length_type)i] = pr->FindChildOrCreate(pacomp->m_Name.c_str());
	}

	vr = va * vb;

	for (int64_t i = 0; i < elct; i++)
	{
		prcomp[i]->m_Var->SetFloat(vr[(glm::fvec3::length_type)i]);
	}
}


void scVec3Div(CScriptVar *c, void *userdata)
{
	CScriptVar *pa = c->GetParameter(_T("a"));
	CScriptVar *pb = c->GetParameter(_T("b"));
	CScriptVar *pr = c->GetReturnVar();
	int64_t elct = pa->GetNumChildren();

	if (!pr || (elct != 3))
		return;

	CScriptVarLink *prcomp[4];

	glm::fvec3 va, vb, vr;

	for (int64_t i = 0; i < elct; i++)
	{
		CScriptVarLink *pacomp = pa->GetChild(i);
		va[(glm::fvec3::length_type)i] = pacomp->m_Var->GetFloat();

		CScriptVarLink *pbcomp = pb->GetChild(i);
		vb[(glm::fvec3::length_type)i] = pbcomp ? pbcomp->m_Var->GetFloat() : pb->GetFloat();

		prcomp[(glm::fvec3::length_type)i] = pr->FindChildOrCreate(pacomp->m_Name.c_str());
	}

	vr = va / vb;

	for (int64_t i = 0; i < elct; i++)
	{
		prcomp[i]->m_Var->SetFloat(vr[(glm::fvec3::length_type)i]);
	}
}


// multiply-add
void scVec3Madd(CScriptVar *c, void *userdata)
{
	CScriptVar *pa = c->GetParameter(_T("a"));
	CScriptVar *pb = c->GetParameter(_T("b"));
	CScriptVar *pc = c->GetParameter(_T("c"));
	CScriptVar *pr = c->GetReturnVar();
	int64_t elct = pa->GetNumChildren();

	if (!pr || (elct != 3))
		return;

	CScriptVarLink *prcomp[4];

	glm::fvec3 va, vb, vc, vr;

	for (int64_t i = 0; i < elct; i++)
	{
		CScriptVarLink *pacomp = pa->GetChild(i);
		va[(glm::fvec3::length_type)i] = pacomp->m_Var->GetFloat();

		CScriptVarLink *pbcomp = pb->GetChild(i);
		vb[(glm::fvec3::length_type)i] = pbcomp ? pbcomp->m_Var->GetFloat() : pb->GetFloat();

		CScriptVarLink *pccomp = pc->GetChild(i);
		vc[(glm::fvec3::length_type)i] = pccomp ? pccomp->m_Var->GetFloat() : pc->GetFloat();

		prcomp[(glm::fvec3::length_type)i] = pr->FindChildOrCreate(pacomp->m_Name.c_str());
	}

	vr = (va * vb) + vc;

	for (int64_t i = 0; i < elct; i++)
	{
		prcomp[i]->m_Var->SetFloat(vr[(glm::fvec3::length_type)i]);
	}
}

// project v onto n (n is normalized inside; zero-safe)
// return: vector with same child layout as 'v' (x, y, z)
void scVec3Project(CScriptVar *c, void *userdata)
{
	CScriptVar *pv = c->GetParameter(_T("v"));
	CScriptVar *pn = c->GetParameter(_T("n"));
	CScriptVar *pr = c->GetReturnVar();
	if (!pv || !pn || !pr)
		return;

	int64_t elct = pv->GetNumChildren();
	if (elct != 3)
		return;

	glm::fvec3 v, n;

	// read v
	for (int64_t i = 0; i < elct; ++i)
	{
		CScriptVarLink *pvcomp = pv->GetChild(i);
		v[(glm::fvec3::length_type)i] = pvcomp->m_Var->GetFloat();
	}

	// read n (allow scalar-or-vector, like madd does)
	if (pn->GetNumChildren() == 3)
	{
		for (int64_t i = 0; i < 3; ++i)
		{
			CScriptVarLink *pncomp = pn->GetChild(i);
			n[(glm::fvec3::length_type)i] = pncomp->m_Var->GetFloat();
		}
	}
	else
	{
		// if scalar, treat it as a scale on v's axis? Not meaningful for projection.
		// Fallback: use (pn,0,0) so callers don't crash; better to always pass a vector.
		n = glm::fvec3(pn->GetFloat(), 0.0f, 0.0f);
	}

	glm::fvec3 r(0);
	float nlen2 = glm::dot(n, n);
	if (nlen2 > 0.0f)
	{
		glm::fvec3 nhat = n * (1.0f / std::sqrt(nlen2));
		r = glm::dot(v, nhat) * nhat;
	} // else stays zero

	// write result children using names from 'v'
	for (int64_t i = 0; i < elct; ++i)
	{
		CScriptVarLink *pvcomp = pv->GetChild(i);
		CScriptVarLink *prcomp = pr->FindChildOrCreate(pvcomp->m_Name.c_str());
		prcomp->m_Var->SetFloat(r[(glm::fvec3::length_type)i]);
	}
}


// reject v from n = v - project(v, n)  (n normalized inside; zero-safe)
void scVec3Reject(CScriptVar *c, void *userdata)
{
	CScriptVar *pv = c->GetParameter(_T("v"));
	CScriptVar *pn = c->GetParameter(_T("n"));
	CScriptVar *pr = c->GetReturnVar();
	if (!pv || !pn || !pr)
		return;

	int64_t elct = pv->GetNumChildren();
	if (elct != 3)
		return;

	glm::fvec3 v, n;

	for (int64_t i = 0; i < elct; ++i)
	{
		CScriptVarLink *pvcomp = pv->GetChild(i);
		v[(glm::fvec3::length_type)i] = pvcomp->m_Var->GetFloat();
	}

	if (pn->GetNumChildren() == 3)
	{
		for (int64_t i = 0; i < 3; ++i)
		{
			CScriptVarLink *pncomp = pn->GetChild(i);
			n[(glm::fvec3::length_type)i] = pncomp->m_Var->GetFloat();
		}
	}
	else
	{
		n = glm::fvec3(pn->GetFloat(), 0.0f, 0.0f);
	}

	glm::fvec3 r = v;
	float nlen2 = glm::dot(n, n);
	if (nlen2 > 0.0f)
	{
		glm::fvec3 nhat = n * (1.0f / std::sqrt(nlen2));
		r -= glm::dot(v, nhat) * nhat;
	}

	for (int64_t i = 0; i < elct; ++i)
	{
		CScriptVarLink *pvcomp = pv->GetChild(i);
		CScriptVarLink *prcomp = pr->FindChildOrCreate(pvcomp->m_Name.c_str());
		prcomp->m_Var->SetFloat(r[(glm::fvec3::length_type)i]);
	}
}


// scalar projection length: (v, n); returns float in the return var
void scVec3ProjLen(CScriptVar *c, void *userdata)
{
	CScriptVar *pv = c->GetParameter(_T("v"));
	CScriptVar *pn = c->GetParameter(_T("n"));
	CScriptVar *pr = c->GetReturnVar();
	if (!pv || !pn || !pr)
		return;

	size_t elct = pv->GetNumChildren();
	if (elct != 3)
		return;

	glm::fvec3 v, n;
	for (size_t i = 0; i < elct; ++i)
		v[(uint32_t)i] = pv->GetChild(i)->m_Var->GetFloat();

	if (pn->GetNumChildren() == 3)
	{
		for (size_t i = 0; i < 3; ++i)
			n[(uint32_t)i] = pn->GetChild(i)->m_Var->GetFloat();
	}
	else
	{
		n = glm::fvec3(pn->GetFloat(), 0.0f, 0.0f);
	}

	float d = 0.f;
	float nlen2 = glm::dot(n, n);
	if (nlen2 > 0.0f)
	{
		glm::fvec3 nhat = n * (1.0f / std::sqrt(nlen2));
		d = glm::dot(v, nhat);
	}

	pr->SetFloat(d);
}



// ----------------------------------------------- Register Functions
void registerMathFunctions(CTinyJS *tinyJS)
{

// --- Math and Trigonometry functions ---
	tinyJS->AddNative(_T("function Math.abs(a)"), scMathAbs, 0);
	tinyJS->AddNative(_T("function Math.round(a)"), scMathRound, 0);
	tinyJS->AddNative(_T("function Math.min(a,b)"), scMathMin, 0);
	tinyJS->AddNative(_T("function Math.max(a,b)"), scMathMax, 0);
	tinyJS->AddNative(_T("function Math.range(x,a,b)"), scMathRange, 0);
	tinyJS->AddNative(_T("function Math.sign(a)"), scMathSign, 0);
	tinyJS->AddNative(_T("function Math.even(a)"), scMathEven, 0);
	tinyJS->AddNative(_T("function Math.random(lo, hi)"), scRandom, 0);


	tinyJS->AddNative(_T("function Math.PI()"), scMathPI, 0);
	tinyJS->AddNative(_T("function Math.toDegrees(a)"), scMathToDegrees, 0);
	tinyJS->AddNative(_T("function Math.toRadians(a)"), scMathToRadians, 0);
	tinyJS->AddNative(_T("function Math.sin(a)"), scMathSin, 0);
	tinyJS->AddNative(_T("function Math.asin(a)"), scMathASin, 0);
	tinyJS->AddNative(_T("function Math.cos(a)"), scMathCos, 0);
	tinyJS->AddNative(_T("function Math.acos(a)"), scMathACos, 0);
	tinyJS->AddNative(_T("function Math.tan(a)"), scMathTan, 0);
	tinyJS->AddNative(_T("function Math.atan(a)"), scMathATan, 0);
	tinyJS->AddNative(_T("function Math.sinh(a)"), scMathSinh, 0);
	tinyJS->AddNative(_T("function Math.asinh(a)"), scMathASinh, 0);
	tinyJS->AddNative(_T("function Math.cosh(a)"), scMathCosh, 0);
	tinyJS->AddNative(_T("function Math.acosh(a)"), scMathACosh, 0);
	tinyJS->AddNative(_T("function Math.tanh(a)"), scMathTanh, 0);
	tinyJS->AddNative(_T("function Math.atanh(a)"), scMathATanh, 0);

	tinyJS->AddNative(_T("function Math.E()"), scMathE, 0);
	tinyJS->AddNative(_T("function Math.log(a)"), scMathLog, 0);
	tinyJS->AddNative(_T("function Math.log10(a)"), scMathLog10, 0);
	tinyJS->AddNative(_T("function Math.exp(a)"), scMathExp, 0);
	tinyJS->AddNative(_T("function Math.pow(a,b)"), scMathPow, 0);

	tinyJS->AddNative(_T("function Math.sqr(a)"), scMathSqr, 0);
	tinyJS->AddNative(_T("function Math.sqrt(a)"), scMathSqrt, 0);

	tinyJS->AddNative(_T("function Math.lerp(a, b, t)"), scMathLerp, 0);
	tinyJS->AddNative(_T("function Math.slerp(a, b, t)"), scMathSlerp, 0);

	tinyJS->AddNative(_T("function Vec3.dot(a, b)"), scVec3Dot, 0);
	tinyJS->AddNative(_T("function Vec3.cross(a, b)"), scVec3Cross, 0);
	tinyJS->AddNative(_T("function Vec3.length(a)"), scVec3Length, 0);
	tinyJS->AddNative(_T("function Vec3.normalize(a)"), scVec3Normalize, 0);
	tinyJS->AddNative(_T("function Vec3.add(a, b)"), scVec3Add, 0);
	tinyJS->AddNative(_T("function Vec3.sub(a, b)"), scVec3Sub, 0);
	tinyJS->AddNative(_T("function Vec3.mul(a, b)"), scVec3Mul, 0);
	tinyJS->AddNative(_T("function Vec3.div(a, b)"), scVec3Div, 0);
	tinyJS->AddNative(_T("function Vec3.madd(a, b, c)"), scVec3Madd, 0);
	tinyJS->AddNative(_T("function Vec3.project(v, n)"), scVec3Project, 0);
	tinyJS->AddNative(_T("function Vec3.reject(v, n)"), scVec3Reject, 0);
	tinyJS->AddNative(_T("function Vec3.projlen(v, n)"), scVec3ProjLen, 0);
}
