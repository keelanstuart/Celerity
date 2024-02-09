// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Environment.h>
#include <functional>

namespace c3
{

	class EnvironmentImpl : public Environment
	{

	protected:

		template <class T> class AnimatableStackValue
		{
		public:
			// returns the interpolated value between a and b and time t
			using INTERPOLATOR = std::function<T(T &a, T &b, float t)>;

			AnimatableStackValue(T initval, INTERPOLATOR t_interpfunc)
			{
				val = ret = initval;
				stack.push_back(initval);
				transtime = 0.0f;
				timeleft = 0.0f;
				interp = t_interpfunc;
			}

			void Push(T newval, float transition_time)
			{
				stack.push_back(newval);
				transtime = timeleft = transition_time;
			}

			void Set(T newval, float transition_time)
			{
				stack.back() = newval;
				transtime = timeleft = transition_time;
			}

			void Pop(float transition_time)
			{
				if (stack.size() > 1)
				{
					val = stack.back();
					stack.pop_back();
					transtime = timeleft = transition_time;
				}
			}

			void Update(float elapsed_seconds)
			{
				if (transtime > 0)
				{
					timeleft = std::max<float>(timeleft - elapsed_seconds, 0);
					if (timeleft == 0)
					{
						transtime = 0;
						val = stack.back();
					}

					ret = interp(val, stack.back(), timeleft / transtime);
				}
				else
					ret = stack.back();
			}

			T val, ret;
			std::deque<T> stack;
			float transtime;
			float timeleft;
			INTERPOLATOR interp;
		};

		AnimatableStackValue<glm::fvec3> m_Gravity;
		AnimatableStackValue<glm::fvec3> m_BackgroundColor;
		AnimatableStackValue<glm::fvec3> m_AmbientColor;
		AnimatableStackValue<glm::fvec3> m_SunColor;
		AnimatableStackValue<glm::fvec3> m_SunDirection;
		AnimatableStackValue<glm::fvec4> m_FogColor;
		AnimatableStackValue<float> m_FogDensity;
		AnimatableStackValue<float> m_FogStart;
		AnimatableStackValue<float> m_FogEnd;

	public:

		EnvironmentImpl();

		void Update(float elapsed_seconds);

		virtual const glm::fvec3 *GetGravity(glm::fvec3 *gravityptr = nullptr) const;
		virtual const glm::fvec3 *GetBackgroundColor(glm::fvec3 *bgcolorptr = nullptr) const;
		virtual const glm::fvec3 *GetAmbientColor(glm::fvec3 *ambcolorptr = nullptr) const;
		virtual const glm::fvec3 *GetSunDirection(glm::fvec3 *sundirptr = nullptr) const;
		virtual const glm::fvec3 *GetSunColor(glm::fvec3 *suncolorptr = nullptr) const;
		virtual const glm::fvec4 *GetFogColor(glm::fvec4 *fogcolorptr = nullptr) const;
		virtual float GetFogDensity() const;
		virtual float GetFogStart() const;
		virtual float GetFogEnd() const;

		virtual void PushGravity(glm::fvec3 gravity, float transition_time = 0.0f);
		virtual void PushBackgroundColor(glm::fvec3 bgcolor, float transition_time = 0.0f);
		virtual void PushAmbientColor(glm::fvec3 ambcolor, float transition_time = 0.0f);
		virtual void PushSunDirection(glm::fvec3 sundir, float transition_time = 0.0f);
		virtual void PushSunColor(glm::fvec3 suncolor, float transition_time = 0.0f);
		virtual void PushFogColor(glm::fvec4 fogcolor, float transition_time = 0.0f);
		virtual void PushFogDensity(float fogdensity, float transition_time = 0.0f);
		virtual void PushFogStart(float fogstart, float transition_time = 0.0f);
		virtual void PushFogEnd(float fogend, float transition_time = 0.0f);

		virtual void SetGravity(glm::fvec3 gravity, float transition_time = 0.0f);
		virtual void SetBackgroundColor(glm::fvec3 bgcolor, float transition_time = 0.0f);
		virtual void SetAmbientColor(glm::fvec3 ambcolor, float transition_time = 0.0f);
		virtual void SetSunDirection(glm::fvec3 sundir, float transition_time = 0.0f);
		virtual void SetSunColor(glm::fvec3 suncolor, float transition_time = 0.0f);
		virtual void SetFogColor(glm::fvec4 fogcolor, float transition_time = 0.0f);
		virtual void SetFogDensity(float fogdensity, float transition_time = 0.0f);
		virtual void SetFogStart(float fogstart, float transition_time = 0.0f);
		virtual void SetFogEnd(float fogend, float transition_time = 0.0f);

		virtual void PopGravity(float transition_time = 0.0f);
		virtual void PopBackgroundColor(float transition_time = 0.0f);
		virtual void PopAmbientColor(float transition_time = 0.0f);
		virtual void PopSunDirection(float transition_time = 0.0f);
		virtual void PopSunColor(float transition_time = 0.0f);
		virtual void PopFogColor(float transition_time = 0.0f);
		virtual void PopFogDensity(float transition_time = 0.0f);
		virtual void PopFogStart(float transition_time = 0.0f);
		virtual void PopFogEnd(float transition_time = 0.0f);

	};

};