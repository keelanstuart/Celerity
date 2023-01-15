// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Object.h>
#include <C3Component.h>

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

		/// Returns the name of the group to which this prototype belongs
		/// It is used as a way to organize prototypes in the editor...
		/// '\\' or '/' delimits the hierarchy.
		virtual const TCHAR *GetGroup() = NULL;

		/// Sets the group to which this Prototype belongs
		/// It is used as a way to organize prototypes in the editor...
		/// '\\' or '/' delimits the hierarchy.
		virtual void SetGroup(const TCHAR *group) = NULL;

		/// Allows the flags conferred to an Object instance to be manipulated
		virtual props::TFlags64 &Flags() = NULL;

		/// Returns the properties conferred to an Object instance to be manipulated
		virtual props::IPropertySet *GetProperties() = NULL;

		/// Adds a Component to the Prototype
		virtual bool AddComponent(const ComponentType *pcomp) = NULL;

		/// Removes the given Component from the Prototype
		virtual bool RemoveComponent(const ComponentType *pcomp) = NULL;

		// Returns true if the Prototype has the given Component
		virtual bool HasComponent(const ComponentType *pcomp) = NULL;

		/// Returns the number of Components that are part of the Prototype
		virtual size_t GetNumComponents() = NULL;

		/// Returns the Component at the given index
		virtual const ComponentType *GetComponent(size_t index) = NULL;

	};

};