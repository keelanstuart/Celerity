// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Renderer.h>
#include <functional>

namespace c3
{

	class RenderMethod
	{

	public:

		using OrderedDrawFunction = std::function<void(int)>;
		static void C3_API ForEachOrderedDrawDo(OrderedDrawFunction func);

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

			// Called when starting the technique. Tells you how many passes there are.
			virtual bool Begin(size_t &passes) = NULL;

			// Applies the render settings for the given pass, typically in a loop run after Begin
			virtual Renderer::RenderStateOverrideFlags ApplyPass(size_t idx) = NULL;

			// Called at the end of the technique
			virtual void End() = NULL;

			// Sets the draw order for things rendered with this technique
			virtual void SetDrawOrder(int order) = NULL;

			// Gets the draw order for this technique. The default is 0. Higher numbers draw later, after other stuff.
			// You need to handle this in your Component's PreRender function.
			// TODO: improve this maybe?
			virtual int GetDrawOrder() const = NULL;

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

		// Gets the index of the active technique (the technique used when this method is active and things are rendered)
		virtual bool GetActiveTechniqueIndex(size_t &idx) const = NULL;

		// Gets the active technique (the technique used when this method is active and things are rendered)
		virtual Technique *GetActiveTechnique() const = NULL;

		// Sets the active technique by index (the technique used when this method is active and things are rendered)
		virtual bool SetActiveTechnique(size_t idx) = NULL;

		// Finds a Technique by name
		virtual bool FindTechnique(const TCHAR *name, size_t &idx) const = NULL;

		// Gets the shader mode (the "options" value passed when loading shaders for this render method)
		virtual const TCHAR *GetShaderMode() = NULL;

		// Sets the shader mode (the "options" value passed when loading shaders for this render method)
		virtual void SetShaderMode(const TCHAR *mode) = NULL;

	};

};
