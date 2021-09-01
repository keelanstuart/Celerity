// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class SystemImpl;

	class FactoryImpl : public Factory
	{

	protected:
		SystemImpl *m_pSys;

		typedef std::deque<FeatureType *> TFeatureTypeArray;
		static TFeatureTypeArray s_FeatureTypes;

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

		virtual bool RegisterFeatureType(FeatureType *pctype);

		virtual bool UnregisterFeatureType(FeatureType *pctype);
		
		virtual size_t GetNumFeatureTypes();

		virtual const FeatureType *GetFeatureType(size_t index);

		virtual const FeatureType *FindFeatureType(const TCHAR *name, bool case_sensitive);

		virtual const FeatureType *FindFeatureType(GUID guid);

	};

};