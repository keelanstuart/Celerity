// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


#pragma once

#include <C3RenderMethod.h>
#include <C3RendererImpl.h>
#include <C3Resource.h>
#include <C3BoundingBox.h>
#include <C3FrameBuffer.h>
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
			friend class RenderMethodImpl;

		protected:

			ShaderProgram *m_ShaderProg;
			FrameBuffer *m_FrameBuffer;
			Subscription m_FBSub;
			std::optional<tstring> m_ShaderCompFilename[Renderer::ShaderComponentType::ST_NUMTYPES];
			std::optional<tstring> m_FrameBufferName;
			props::TFlags64 m_FrameBufferFlags;
			Renderer::RenderStateOverrideFlags m_StateRestorationMask;
			std::optional<Renderer::BlendMode> m_BlendMode;
			std::optional<Renderer::BlendEquation> m_BlendEq;
			std::optional<Renderer::BlendMode> m_AlphaBlendMode;
			std::optional<Renderer::BlendEquation> m_AlphaBlendEq;
			std::optional<Renderer::ChannelMask> m_ChannelWriteMask;
			std::optional<Renderer::BlendMode> m_BlendModeCh[FrameBuffer::MAX_COLORTARGETS];
			std::optional<Renderer::BlendEquation> m_BlendEqCh[FrameBuffer::MAX_COLORTARGETS];
			std::optional<Renderer::BlendMode> m_AlphaBlendModeCh[FrameBuffer::MAX_COLORTARGETS];
			std::optional<Renderer::BlendEquation> m_AlphaBlendEqCh[FrameBuffer::MAX_COLORTARGETS];
			std::optional<Renderer::ChannelMask> m_ChannelWriteMaskCh[FrameBuffer::MAX_COLORTARGETS];
			std::optional<Renderer::CullMode> m_CullMode;
			std::optional<Renderer::WindingOrder> m_WindingOrder;
			std::optional<Renderer::DepthMode> m_DepthMode;
			std::optional<Renderer::Test> m_DepthTest;
			std::optional<Renderer::FillMode> m_FillMode;
			RenderMethod *m_pOwner;

		public:
			PassImpl();

			virtual ~PassImpl();

			Renderer::RenderStateOverrideFlags Apply(Renderer *prend);
			bool LoadSetting(const tinyxml2::XMLElement *proot);

			virtual void SetFrameBufferName(const TCHAR *name);
			virtual bool GetFrameBufferName(tstring &name) const;
			virtual void InvalidateFrameBuffer();

			virtual void SetFrameBufferFlags(props::TFlags64 flags);
			virtual props::TFlags64 GetFrameBufferFlags() const;

			virtual void SetShaderComponentFilename(Renderer::ShaderComponentType type, const TCHAR *filename);
			virtual bool GetShaderComponentFilename(Renderer::ShaderComponentType type, tstring &filename) const;
			virtual ShaderProgram *GetShader() const;

			virtual void SetBlendMode(Renderer::BlendMode blendmode = Renderer::BlendMode::BM_REPLACE);
			virtual bool GetBlendMode(Renderer::BlendMode &blendmode) const;

			virtual void SetBlendEquation(Renderer::BlendEquation blendeq = Renderer::BlendEquation::BE_ADD);
			virtual bool GetBlendEquation(Renderer::BlendEquation &blendeq) const;

			virtual void SetAlphaBlendMode(Renderer::BlendMode blendmode = Renderer::BlendMode::BM_REPLACE);
			virtual bool GetAlphaBlendMode(Renderer::BlendMode &blendmode) const;

			virtual void SetAlphaBlendEquation(Renderer::BlendEquation blendeq = Renderer::BlendEquation::BE_ADD);
			virtual bool GetAlphaBlendEquation(Renderer::BlendEquation &blendeq) const;

			virtual void SetCullMode(Renderer::CullMode cullmode = Renderer::CullMode::CM_BACK);
			virtual bool GetCullMode(Renderer::CullMode &cullmode) const;

			virtual void SetWindingOrder(Renderer::WindingOrder order = Renderer::WindingOrder::WO_CW);
			virtual bool GetWindingOrder(Renderer::WindingOrder &order) const;

			virtual void SetDepthMode(Renderer::DepthMode depthmode = Renderer::DepthMode::DM_READWRITE);
			virtual bool GetDepthMode(Renderer::DepthMode &depthmode) const;

			virtual void SetDepthTest(Renderer::Test depthtest = Renderer::Test::DT_LESSER);
			virtual bool GetDepthTest(Renderer::Test &depthtest) const;

			virtual void SetFillMode(Renderer::FillMode fillmode = Renderer::FillMode::FM_FILL);
			virtual bool GetFillMode(Renderer::FillMode &fillmode) const;

		};

		typedef std::vector<PassImpl> TPassVector;

		class TechniqueImpl : public RenderMethod::Technique
		{
			friend class RenderMethodImpl;

		protected:
			RenderMethodImpl *m_pOwner;
			tstring m_Name;
			
			TPassVector m_Passes;
			size_t m_Mode;
			std::optional<int> m_DrawOrder;

			bool m_RestoreOld;
			Renderer::RenderStateOverrideFlags m_OldState;
			Renderer::BlendMode m_OldBlendMode;
			Renderer::BlendEquation m_OldBlendEq;
			Renderer::BlendMode m_OldAlphaBlendMode;
			Renderer::BlendEquation m_OldAlphaBlendEq;
			Renderer::CullMode m_OldCullMode;
			Renderer::WindingOrder m_OldWindingOrder;
			Renderer::DepthMode m_OldDepthMode;
			Renderer::Test m_OldDepthTest;
			Renderer::FillMode m_OldFillMode;

		public:
			TechniqueImpl(RenderMethodImpl *powner);

			virtual void SetName(const TCHAR *name);
			virtual const TCHAR *GetName() const;
			virtual size_t GetNumPasses() const;
			virtual Pass *GetPass(size_t idx) const;
			virtual Pass *AddPass();
			virtual bool Begin(size_t &passes, bool restore_old = false);
			virtual Renderer::RenderStateOverrideFlags ApplyPass(size_t idx);
			virtual void End();
			virtual void SetDrawOrder(int order);
			virtual int GetDrawOrder() const;

		};

		friend class TechniqueImpl;

		typedef std::vector<TechniqueImpl> TTechniqueVector;

	protected:
		TTechniqueVector m_Techniques;
		tstring m_Name;
		size_t m_ActiveTech;
		tstring m_ShaderMode;

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
		virtual bool SetActiveTechniqueByName(const TCHAR *name);

		virtual bool FindTechnique(const TCHAR *name, size_t &idx) const;

		bool Load(const tinyxml2::XMLElement *proot, const TCHAR *options);

		// Gets the shader mode (the "options" value passed when loading shaders for this render method)
		virtual const TCHAR *GetShaderMode();

		// Sets the shader mode (the "options" value passed when loading shaders for this render method)
		virtual void SetShaderMode(const TCHAR *mode);

		void RestoreStates(Renderer *prend);

	private:
		void LoadTechnique(const tinyxml2::XMLElement *proot, const TCHAR *options);

		void LoadPass(TechniqueImpl *ptech, const tinyxml2::XMLElement *proot, const TCHAR *options);

	};

	DEFINE_RESOURCETYPE(RenderMethod, 0, GUID({0x2e5ad016, 0x955c, 0x4dba, { 0x8f, 0x15, 0xf1, 0x39, 0x4e, 0x20, 0x82, 0xea }}), "RenderMethod", "Render Methods", "c3rm", "c3rm");

};
