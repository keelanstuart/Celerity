// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3RenderMethod.h>
#include <C3RendererImpl.h>
#include <C3Resource.h>
#include <C3BoundingBox.h>
#include <optional>


namespace c3
{

	class RenderMethodImpl : public RenderMethod
	{

		friend class RenderMethod;

	protected:
		Renderer *m_pRend;

		using DrawOrderCountMap = std::map<int, size_t>;
		static DrawOrderCountMap s_DrawOrders;

	public:

		class PassImpl : public RenderMethod::Pass
		{

		protected:

			ShaderProgram *m_ShaderProg;
			FrameBuffer *m_FrameBuffer;
			std::optional<tstring> m_ShaderCompFilename[Renderer::ShaderComponentType::ST_NUMTYPES];
			tstring m_ShaderMode;
			std::optional<tstring> m_FrameBufferName;
			props::TFlags64 m_FrameBufferFlags;
			std::optional<Renderer::BlendMode> m_BlendMode;
			std::optional<Renderer::BlendEquation> m_BlendEq;
			std::optional<Renderer::CullMode> m_CullMode;
			std::optional<Renderer::WindingOrder> m_WindingOrder;
			std::optional<Renderer::DepthMode> m_DepthMode;
			std::optional<Renderer::FillMode> m_FillMode;

		public:
			PassImpl();

			virtual ~PassImpl();

			Renderer::RenderStateOverrideFlags Apply(Renderer *prend);
			bool LoadSetting(const tinyxml2::XMLElement *proot);
			void SetShaderMode(const TCHAR *mode);

			virtual void SetFrameBufferName(const TCHAR *name);
			virtual bool GetFrameBufferName(tstring &name) const;

			virtual void SetFrameBufferFlags(props::TFlags64 flags);
			virtual props::TFlags64 GetFrameBufferFlags() const;

			virtual void SetShaderComponentFilename(Renderer::ShaderComponentType type, const TCHAR *filename);
			virtual bool GetShaderComponentFilename(Renderer::ShaderComponentType type, tstring &filename) const;
			virtual ShaderProgram *GetShader() const;

			virtual void SetBlendMode(Renderer::BlendMode blendmode = Renderer::BlendMode::BM_REPLACE);
			virtual bool GetBlendMode(Renderer::BlendMode &blendmode) const;

			virtual void SetEquation(Renderer::BlendEquation blendeq = Renderer::BlendEquation::BE_ADD);
			virtual bool GetBlendEquation(Renderer::BlendEquation &blendeq) const;

			virtual void SetCullMode(Renderer::CullMode cullmode = Renderer::CullMode::CM_BACK);
			virtual bool GetCullMode(Renderer::CullMode &cullmode) const;

			virtual void SetWindingOrder(Renderer::WindingOrder order = Renderer::WindingOrder::WO_CW);
			virtual bool GetWindingOrder(Renderer::WindingOrder &order) const;

			virtual void SetDepthMode(Renderer::DepthMode depthmode = Renderer::DepthMode::DM_READWRITE);
			virtual bool GetDepthMode(Renderer::DepthMode &depthmode) const;

			virtual void SetFillMode(Renderer::FillMode fillmode = Renderer::FillMode::FM_FILL);
			virtual bool GetFillMode(Renderer::FillMode &fillmode) const;

		};

		typedef std::vector<PassImpl> TPassVector;

		class TechniqueImpl : public RenderMethod::Technique
		{

		protected:
			Renderer *m_pRend;
			tstring m_Name;
			
			TPassVector m_Passes;
			size_t m_Mode;
			std::optional<int> m_DrawOrder;


		public:
			TechniqueImpl(Renderer *prend);

			virtual void SetName(const TCHAR *name);
			virtual const TCHAR *GetName() const;
			virtual size_t GetNumPasses() const;
			virtual Pass *GetPass(size_t idx) const;
			virtual Pass *AddPass();
			virtual bool Begin(size_t &passes) const;
			virtual Renderer::RenderStateOverrideFlags ApplyPass(size_t idx) const;
			virtual void End() const;
			virtual void SetDrawOrder(int order);
			virtual int GetDrawOrder() const;

		};

		typedef std::vector<TechniqueImpl> TTechniqueVector;

	protected:
		TTechniqueVector m_Techniques;
		tstring m_Name;
		size_t m_ActiveTech;

	public:
		RenderMethodImpl(Renderer *prend);

		virtual ~RenderMethodImpl();

		virtual void Release();

		// Sets the name of the RenderMethod
		virtual void SetName(const TCHAR *name);

		// Returns the name of the RenderMethod
		virtual const TCHAR *GetName() const;

		// Returns the number of Techniques that this RenderMethod has
		virtual size_t GetNumTechniques() const;

		// Returns the Technique at the given index
		virtual Technique *GetTechnique(size_t idx) const;

		// Adds a Technique to this RenderMethod
		virtual Technique *AddTechnique();

		virtual bool GetActiveTechniqueIndex(size_t &idx) const;
		virtual Technique *GetActiveTechnique() const;

		virtual bool SetActiveTechnique(size_t idx);

		virtual bool FindTechnique(const TCHAR *name, size_t &idx) const;

		bool Load(const tinyxml2::XMLElement *proot, const TCHAR *options);

	private:
		void LoadTechnique(const tinyxml2::XMLElement *proot, const TCHAR *options);

		void LoadPass(TechniqueImpl *ptech, const tinyxml2::XMLElement *proot, const TCHAR *options);

	};

	DEFINE_RESOURCETYPE(RenderMethod, 0, GUID({0x2e5ad016, 0x955c, 0x4dba, { 0x8f, 0x15, 0xf1, 0x39, 0x4e, 0x20, 0x82, 0xea }}), "RenderMethod", "Render Methods", "c3rm", "c3rm");

};
