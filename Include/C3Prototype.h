// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Object.h>
#include <C3Feature.h>

namespace c3
{

	/// Notionally, a Prototype is simply a set of attributes to assign to an Object.
	/// Prototypes can be created from existing Object instances, allowing that Object's
	/// particular configuration to be stored and re-created later.

	class Prototype
	{

	public:

		/// Returns the name of the Prototype
		virtual const TCHAR *GetName() = NULL;

		/// Sets the name of the Prototype
		virtual void SetName(const TCHAR *name) = NULL;

		/// Returns the system-generated GUID assigned to the Prototype
		virtual GUID GetGUID() = NULL;

		/// Allows the flags conferred to an Object instance to be manipulated
		virtual props::TFlags64 &Flags() = NULL;

		/// Returns the properties conferred to an Object instance to be manipulated
		virtual props::IPropertySet *GetProperties() = NULL;

		/// Adds a Feature to the Prototype
		virtual bool AddFeature(const FeatureType *pcomp) = NULL;

		/// Removes the given Feature from the Prototype
		virtual bool RemoveFeature(const FeatureType *pcomp) = NULL;

		/// Returns the number of Features that are part of the Prototype
		virtual size_t GetNumFeatures() = NULL;

		/// Returns the Feature at the given index
		virtual const FeatureType *GetFeature(size_t index) = NULL;

	};

};