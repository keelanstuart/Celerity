// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


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

		virtual Object *Build(Prototype *pproto, GUID *override_guid);

		virtual Object *Build(Object *pobject, GUID *override_guid);

		virtual Prototype *CreatePrototype(Prototype *pproto);

		virtual Prototype *MakePrototype(Object *pobject);

		virtual void RemovePrototype(Prototype *pproto);

		virtual size_t GetNumPrototypes();

		virtual Prototype *GetPrototype(size_t index);

		virtual Prototype *FindPrototype(const TCHAR *name, bool case_sensitive);

		virtual Prototype *FindPrototype(GUID guid);

		virtual bool LoadPrototypes(genio::IInputStream *is);

		virtual bool LoadPrototypes(tinyxml2::XMLNode *proot);

		virtual bool SavePrototypes(genio::IOutputStream *os, PROTO_SAVE_HUERISTIC_FUNCTION pshfunc);

		virtual bool SavePrototypes(tinyxml2::XMLNode *proot, PROTO_SAVE_HUERISTIC_FUNCTION pshfunc);

		virtual bool RegisterComponentType(ComponentType *pctype);

		virtual bool UnregisterComponentType(ComponentType *pctype);
		
		virtual size_t GetNumComponentTypes();

		virtual const ComponentType *GetComponentType(size_t index);

		virtual const ComponentType *FindComponentType(const TCHAR *name, bool case_sensitive);

		virtual const ComponentType *FindComponentType(GUID guid);

		virtual bool RegisterFlowNodeType(FlowNodeType *pfntype);

		virtual bool UnregisterFlowNodeType(FlowNodeType *pfntype);

		virtual size_t GetNumFlowNodeTypes();

		virtual const FlowNodeType *GetFlowNodeType(size_t index);

		virtual const FlowNodeType *FindFlowNodeType(const TCHAR *name, bool case_sensitive = true);

		virtual const FlowNodeType *FindFlowNodeType(GUID guid);

	};

};