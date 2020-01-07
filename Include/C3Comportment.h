// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Object.h>

namespace c3
{

	class ComportmentType;


	/// A Comportment is a behavior controller that attaches to an Object and is, given it's flags,
	/// Updated or Rendered, thus giving plain "vanilla" Objects more interesting characteristics.
	/// Comportments exist because Objects are abstract and, therefore, do not directly expose
	/// their members to Object implementations in plug-in modules. 

	class Comportment : public props::IPropertyChangeListener
	{

	public:

		/// Returns the ComportmentType that built this Comportment
		virtual ComportmentType *GetType() = NULL;

		/// Returns the Object that owns this Comportment
		virtual Object *GetOwner() = NULL;

		/// Returns the flags that reveal what this Comportment should be used for, such as rendering or character-like behavior
		virtual const props::TFlags64 &Flags() const = NULL;

		/// Returns the properties held by this Comportment instance
		virtual props::IPropertySet *GetProperties() = NULL;

		/// Called by the Object that owns the Comportment during it's own Update
		virtual void Update(float elapsed_time = 0.0f) = NULL;

		/// Called by the Object that owns the Comportment during it's own Render
		virtual bool Render(props::TFlags64 rendflags) = NULL;

	};


	/// Implement a companion ComportmentType for each Comportment and register it with the Factory in order to
	/// have Prototypes that build complete Objects. Why? To maintain heap coherency; plug-ins are responsible
	/// for creation and destruction of their own Comportments.

	class ComportmentType
	{

	public:

		/// Constructs a new instance of it's associated Comportment and assigns it to the given Object.
		/// Optionally copies the properties of a given Comportment instance
		virtual Comportment *Build(Object *owner, Comportment *pcomportment = nullptr) = NULL;

		/// Destoys an instance of it's associated Comportment
		virtual void Destroy(Comportment *pcomportment) = NULL;

		/// Returns the user-defined GUID that identifies the ComportmentType type
		virtual GUID GetGUID() = NULL;

		/// Returns the name of the ComportmentType
		virtual const TCHAR *GetName() = NULL;

		/// Returns a string that describes the associated Comportment class
		virtual const TCHAR *GetDescription() = NULL;

	};

};