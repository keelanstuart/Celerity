// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class SystemImpl;

	class FactoryImpl : public Factory
	{

	protected:
		SystemImpl *m_pSys;

		typedef std::deque<ComponentType *> TComponentTypeArray;
		static TComponentTypeArray s_ComponentTypes;

		typedef std::deque<FlowNodeType *> TFlowNodeTypeArray;
		static TFlowNodeTypeArray s_FlowNodeTypes;

		typedef std::deque<Prototype *> TPrototypeArray;
		TPrototypeArray m_Prototypes;

	public:

		FactoryImpl(SystemImpl *psys);

		virtual ~FactoryImpl();

		virtual Object *Build(const Prototype *pproto, GUID *override_guid = nullptr, Object *pparent = nullptr);

		virtual Object *Build(const Object *pobject, GUID *override_guid = nullptr, Object *pparent = nullptr, bool build_children = false);

		virtual Prototype *CreatePrototype(const Prototype *pproto = nullptr);

		virtual Prototype *MakePrototype(const Object *pobject);

		virtual void RemovePrototype(Prototype *pproto);

		virtual size_t GetNumPrototypes() const;

		virtual Prototype *GetPrototype(size_t index) const;

		virtual Prototype *FindPrototype(const TCHAR *name, bool case_sensitive) const;

		virtual Prototype *FindPrototype(GUID guid) const;

		virtual bool LoadPrototypes(genio::IInputStream *is, const TCHAR *source = nullptr);

		virtual bool LoadPrototypes(const tinyxml2::XMLNode *proot, const TCHAR *source = nullptr);

		virtual bool SavePrototypes(genio::IOutputStream *os, PROTO_SAVE_HUERISTIC_FUNCTION pshfunc = nullptr);

		virtual bool SavePrototypes(tinyxml2::XMLNode *proot, PROTO_SAVE_HUERISTIC_FUNCTION pshfunc = nullptr);

		virtual bool RegisterComponentType(ComponentType *pctype);

		virtual bool UnregisterComponentType(ComponentType *pctype);
		
		virtual size_t GetNumComponentTypes() const;

		virtual const ComponentType *GetComponentType(size_t index) const;

		virtual const ComponentType *FindComponentType(const TCHAR *name, bool case_sensitive) const;

		virtual const ComponentType *FindComponentType(GUID guid) const;

		virtual bool RegisterFlowNodeType(FlowNodeType *pfntype);

		virtual bool UnregisterFlowNodeType(FlowNodeType *pfntype);

		virtual size_t GetNumFlowNodeTypes() const;

		virtual const FlowNodeType *GetFlowNodeType(size_t index) const;

		virtual const FlowNodeType *FindFlowNodeType(const TCHAR *name, bool case_sensitive = true) const;

		virtual const FlowNodeType *FindFlowNodeType(GUID guid) const;

	};

};