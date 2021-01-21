// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class PrototypeImpl : public Prototype
	{

	protected:
		System *m_pSys;
		tstring m_Name;
		GUID m_GUID;

		props::TFlags64 m_Flags;

		props::IPropertySet *m_Props;

		typedef std::deque<const FeatureType *> TFeatureArray;
		TFeatureArray m_Features;

	public:

		PrototypeImpl(System *psys, GUID guid);

		virtual ~PrototypeImpl();

		virtual const TCHAR *GetName();

		virtual void SetName(const TCHAR *name);

		virtual GUID GetGUID();

		virtual props::TFlags64 &Flags();

		virtual props::IPropertySet *GetProperties();

		virtual bool AddFeature(const FeatureType *pcomp);

		virtual bool RemoveFeature(const FeatureType *pcomp);

		virtual size_t GetNumFeatures();

		virtual const FeatureType *GetFeature(size_t index);

	};

};