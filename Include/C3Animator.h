// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Component.h>


namespace c3
{

	// Animator is a Component that maintains a state containing one, or more, animations and manages transitioning from one state to the other.
	// The interpolated animation state is then applied to a ModelRenderer's InstanceData, transforming nodes

	class C3_API Animator : public Component
	{

	public:

		static const ComponentType *Type();

		// Returns true if the given state exists
		virtual bool HasState(const TCHAR *name) const = NULL;

		// Gets the name of the currently active state
		virtual const TCHAR *GetCurrentState() const = NULL;

		// Sets the current state
		virtual void SetCurrentState(const TCHAR *name) = NULL;

		// Forces a transition to the next state, whatever that is
		virtual void ForceStateChange() = NULL;

	};

};
