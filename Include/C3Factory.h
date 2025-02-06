// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Object.h>
#include <C3Component.h>
#include <C3Prototype.h>
#include <C3FlowNode.h>

#include <tinyxml2.h>

namespace c3
{

	/// The Celerity Factory is the hub for creating all Object instances and for obtaining registered ComponentTypes.
	/// As in previous versions of Celerity, Prototypes are used to easily create objects with specific initial
	/// properties, and the Factory is also where those are maintained.

	class Factory
	{

	public:

		/// Builds a new Object, optionally applying the attributes of a Prototype
		virtual Object *Build(const Prototype *pproto = nullptr, GUID *override_guid = nullptr, Object *pparent = nullptr) = NULL;

		/// Builds a new Object, optionally applying the attributes of an existing Object
		virtual Object *Build(const Object *pobject, GUID *override_guid = nullptr, Object *pparent = nullptr, bool build_children = false) = NULL;

		/// Creates a new Prototype, optionally copying the attributes from an existing Prototype
		virtual Prototype *CreatePrototype(const Prototype *pproto = nullptr) = NULL;

		/// Creates a new Prototype, optionally copying the attributes of an existing Object
		virtual Prototype *MakePrototype(const Object *pobject = nullptr) = NULL;

		/// Removes a Prototype from the Factory
		virtual void RemovePrototype(Prototype *pproto) = NULL;

		/// Returns the number of Prototypes that are currently registered
		virtual size_t GetNumPrototypes() const = NULL;

		/// Returns a Prototype by index (use GetNumPrototypes to determine the highest available)
		virtual Prototype *GetPrototype(size_t index) const = NULL;

		/// Finds a Prototype by name
		virtual Prototype *FindPrototype(const TCHAR *name, bool case_sensitive = true) const = NULL;

		/// Finds a Prototype by GUID
		virtual Prototype *FindPrototype(GUID guid) const = NULL;

		/// Loads one or more Prototypes from a stream
		virtual bool LoadPrototypes(genio::IInputStream *is, const TCHAR *source = nullptr) = NULL;

		/// Loads one or more Prototypes from a tinyxml2 root node
		virtual bool LoadPrototypes(const tinyxml2::XMLNode *proot, const TCHAR *source = nullptr) = NULL;

		/// You can determine which Prototypes you want to save by implementing a PROTO_SAVE_HUERISTIC_FUNCTION
		/// and passing it to SavePrototypes. Simply return true if the Prototype should be stored, false otherwise.
		using PROTO_SAVE_HUERISTIC_FUNCTION = std::function<bool(Prototype *)>;

		/// Saves [maybe, given a hueristic - nullptr saves all] all Prototypes to a stream
		virtual bool SavePrototypes(genio::IOutputStream *os, PROTO_SAVE_HUERISTIC_FUNCTION pshfunc = nullptr) = NULL;

		/// Saves [maybe, given a hueristic - nullptr saves all] all Prototypes to a tinyxml2 root node
		virtual bool SavePrototypes(tinyxml2::XMLNode *proot, PROTO_SAVE_HUERISTIC_FUNCTION pshfunc = nullptr) = NULL;

		/// Makes a ComponentType discoverable by all clients of the Celerity System that created this Factory
		virtual bool RegisterComponentType(ComponentType *pctype) = NULL;

		/// Removes the given ComponentType from the Factory
		virtual bool UnregisterComponentType(ComponentType *pctype) = NULL;

		/// Returns the number of ComponentTypes currently registered
		virtual size_t GetNumComponentTypes() const = NULL;

		/// Returns a ComponentType given an index
		virtual const ComponentType *GetComponentType(size_t index) const = NULL;

		/// Returns a ComponentType with the given name, or nullptr if one is not found
		virtual const ComponentType *FindComponentType(const TCHAR *name, bool case_sensitive = true) const = NULL;

		/// Returns a ComponentType with the given GUID, or nullptr if one is not found
		virtual const ComponentType *FindComponentType(GUID guid) const = NULL;

		/// Makes a FlowNodeType discoverable by all clients of the Celerity System that created this Factory
		virtual bool RegisterFlowNodeType(FlowNodeType *pfntype) = NULL;

		/// Removes the given FlowNodeType from the Factory
		virtual bool UnregisterFlowNodeType(FlowNodeType *pfntype) = NULL;

		/// Returns the number of FlowNodeTypes currently registered
		virtual size_t GetNumFlowNodeTypes() const = NULL;

		/// Returns a FlowNodeType given an index
		virtual const FlowNodeType *GetFlowNodeType(size_t index) const = NULL;

		/// Returns a FlowNodeType with the given name, or nullptr if one is not found
		virtual const FlowNodeType *FindFlowNodeType(const TCHAR *name, bool case_sensitive = true) const = NULL;

		/// Returns a FlowNodeType with the given GUID, or nullptr if one is not found
		virtual const FlowNodeType *FindFlowNodeType(GUID guid) const = NULL;

	};

};