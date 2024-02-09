// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Renderer.h>

namespace c3
{

	class RenderMethod
	{

	public:

		class Pass
		{

		public:

			virtual void SetFrameBufferName(const TCHAR *name) = NULL;
			virtual bool GetFrameBufferName(tstring &name) const = NULL;

			virtual void SetShaderComponentFilename(Renderer::ShaderComponentType type, const TCHAR *filename) = NULL;
			virtual bool GetShaderComponentFilename(Renderer::ShaderComponentType type, tstring &filename) const = NULL;
			virtual ShaderProgram *GetShader() const = NULL;

			virtual void SetBlendMode(Renderer::BlendMode blendmode = Renderer::BlendMode::BM_REPLACE) = NULL;
			virtual bool GetBlendMode(Renderer::BlendMode &blendmode) const = NULL;

			virtual void SetEquation(Renderer::BlendEquation blendeq = Renderer::BlendEquation::BE_ADD) = NULL;
			virtual bool GetBlendEquation(Renderer::BlendEquation &blendeq) const = NULL;

			virtual void SetCullMode(Renderer::CullMode cullmode = Renderer::CullMode::CM_BACK) = NULL;
			virtual bool GetCullMode(Renderer::CullMode &cullmode) const = NULL;

			virtual void SetWindingOrder(Renderer::WindingOrder order = Renderer::WindingOrder::WO_CW) = NULL;
			virtual bool GetWindingOrder(Renderer::WindingOrder &order) const = NULL;

			virtual void SetDepthMode(Renderer::DepthMode depthmode = Renderer::DepthMode::DM_READWRITE) = NULL;
			virtual bool GetDepthMode(Renderer::DepthMode &depthmode) const = NULL;

			virtual void SetFillMode(Renderer::FillMode fillmode = Renderer::FillMode::FM_FILL) = NULL;
			virtual bool GetFillMode(Renderer::FillMode &fillmode) const = NULL;

		};

		class Technique
		{

		public:

			using ETechMode = enum
			{
				TECHMODE_NORMAL = 0,
				TECHMODE_SKIN,

				TECHMODE_NUMTYPES
			};

			// Sets the name of the Technique
			virtual void SetName(const TCHAR *name) = NULL;

			// Returns the name of the Technique
			virtual const TCHAR *GetName() const = NULL;

			// Returns the number of Passes that this Technique has
			virtual size_t GetNumPasses() const = NULL;

			// Returns the Pass at the given index
			virtual Pass *GetPass(size_t idx) const = NULL;

			// Adds a Pass to this Technique
			virtual Pass *AddPass() = NULL;

			virtual void SetMode(ETechMode mode = TECHMODE_NORMAL) = NULL;

			virtual bool Begin(size_t &passes) const = NULL;

			virtual Renderer::RenderStateOverrideFlags ApplyPass(size_t idx) const = NULL;

			virtual void End() const = NULL;
		};

		virtual void Release() = NULL;

		// Sets the name of the RenderMethod
		virtual void SetName(const TCHAR *name) = NULL;

		// Returns the name of the RenderMethod
		virtual const TCHAR *GetName() const = NULL;

		// Returns the number of Techniques that this RenderMethod has
		virtual size_t GetNumTechniques() const = NULL;

		// Returns the Technique at the given index
		virtual Technique *GetTechnique(size_t idx) const = NULL;

		// Adds a Technique to this RenderMethod
		virtual Technique *AddTechnique() = NULL;

		virtual bool GetActiveTechniqueIndex(size_t &idx) const = NULL;
		virtual Technique *GetActiveTechnique() const = NULL;

		virtual bool SetActiveTechnique(size_t idx) = NULL;

		virtual bool FindTechnique(const TCHAR *name, size_t &idx) const = NULL;

	};

};
