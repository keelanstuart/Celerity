// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Object.h>
#include <C3Feature.h>
#include <C3Prototype.h>

#include <tinyxml2.h>

namespace c3
{

	/// The Celerity Factory is the hub for creating all Object instances and for obtaining registered FeatureTypes.
	/// As in previous versions of Celerity, Prototypes are used to easily create objects with specific initial
	/// properties, and the Factory is also where those are maintained.

	class Factory
	{

	public:

		/// Builds a new Object, optionally applying the attributes of a Prototype
		virtual Object *Build(Prototype *pproto = nullptr, GUID *override_guid = nullptr) = NULL;

		/// Builds a new Object, optionally applying the attributes of an existing Object
		virtual Object *Build(Object *pobject, GUID *override_guid = nullptr) = NULL;

		/// Creates a new Prototype, optionally copying the attributes from an existing Prototype
		virtual Prototype *CreatePrototype(Prototype *pproto = nullptr) = NULL;

		/// Creates a new Prototype, optionally copying the attributes of an existing Object
		virtual Prototype *MakePrototype(Object *pobject = nullptr) = NULL;

		/// Removes a Prototype from the Factory
		virtual void RemovePrototype(Prototype *pproto) = NULL;

		/// Returns the number of Prototypes that are currently registered
		virtual size_t GetNumPrototypes() = NULL;

		/// Returns a Prototype by index (use GetNumPrototypes to determine the highest available)
		virtual Prototype *GetPrototype(size_t index) = NULL;

		/// Finds a Prototype by name
		virtual Prototype *FindPrototype(const TCHAR *name, bool case_sensitive = true) = NULL;

		/// Finds a Prototype by GUID
		virtual Prototype *FindPrototype(GUID guid) = NULL;

		/// Loads one or more Prototypes from a stream
		virtual bool LoadPrototypes(genio::IInputStream *is) = NULL;

		/// Loads one or more Prototypes from a tinyxml2 root node
		virtual bool LoadPrototypes(tinyxml2::XMLNode *proot) = NULL;

		/// You can determine which Prototypes you want to save by implementing a PROTO_SAVE_HUERISTIC_FUNCTION
		/// and passing it to SavePrototypes. Simply return true if the Prototype should be stored, false otherwise.
		typedef bool (__cdecl *PROTO_SAVE_HUERISTIC_FUNCTION)(Prototype *pproto);

		/// Saves [maybe, given a hueristic - nullptr saves all] all Prototypes to a stream
		virtual bool SavePrototypes(genio::IOutputStream *os, PROTO_SAVE_HUERISTIC_FUNCTION pshfunc = nullptr) = NULL;

		/// Saves [maybe, given a hueristic - nullptr saves all] all Prototypes to a tinyxml2 root node
		virtual bool SavePrototypes(tinyxml2::XMLNode *proot, PROTO_SAVE_HUERISTIC_FUNCTION pshfunc = nullptr) = NULL;

		/// Makes a FeatureType discoverable by all clients of the Celerity System that created this Factory
		virtual bool RegisterFeatureType(FeatureType *pctype) = NULL;

		/// Removes the given FeatureType from the Factory
		virtual bool UnregisterFeatureType(FeatureType *pctype) = NULL;

		/// Returns the number of FeatureTypes currently registered
		virtual size_t GetNumFeatureTypes() = NULL;

		/// Returns a FeatureType given an index
		virtual const FeatureType *GetFeatureType(size_t index) = NULL;

		/// Returns a FeatureType with the given name, or nullptr if one is not found
		virtual const FeatureType *FindFeatureType(const TCHAR *name, bool case_sensitive = true) = NULL;

		/// Returns a FeatureType with the given GUID, or nullptr if one is not found
		virtual const FeatureType *FindFeatureType(GUID guid) = NULL;

	};

};