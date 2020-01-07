// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3Comportment.h>
#include <PowerProps.h>
#include <GenIO.h>

namespace c3
{

	class Comportment;
	class ComportmentType;

	/// All world objects in Celerity are Objects. Objects have their own properties and maintain
	/// a list of Comportments -- the dictionary definition of "Comportment" is: behavior, bearing -- and
	/// can receive time-based updates and process requests to draw themselves.

	/// Previous versions of Celerity followed a paradigm of sub-classes derived from the base object class to be
	/// written by plug-in creators -- with the API caveat that all member data was exposed. I'm not saying
	/// the abstract interfaces in Celerity v3 are the only viable or reasonable choice for modern C++ libraries, but
	/// the advantage is easy to see: "clean", "simple", and "portable" code is easier to deal with

	class Object : public props::IPropertyChangeListener
	{

	public:

		/// Returns the Celerity System in which the Object exists
		virtual System *GetSystem() = NULL;

		/// Frees any resources that the Object may have allocated
		virtual void Release() = NULL;

		/// Returns the name of the Object
		virtual const TCHAR *GetName() = NULL;

		/// Sets the name of the Object
		virtual void SetName(const TCHAR *name) = NULL;

		/// Returns the Object's GUID
		virtual GUID GetGuid() = NULL;

		/// Returns the owner of this Object
		virtual Object *GetOwner() = NULL;

		/// Sets the Object's owner
		virtual void SetOwner(Object *powner) = NULL;

		/// Allows access to the Object's flags
		virtual props::TFlags64 &Flags() = NULL;

		/// Returns the IPropertySet owned by the Object
		virtual props::IPropertySet *GetProperties() = NULL;

		/// Returns the number of Comportments currently attached to this Object
		virtual size_t GetNumComportments() = NULL;

		/// Returns the attached Comportment at the given index
		virtual Comportment *GetComportment(size_t index) = NULL;

		/// Creates, attaches, and returns a Comportment of the given ComportmentType
		virtual Comportment *AddComportment(ComportmentType *pctype) = NULL;

		/// Removes the given Comportment
		virtual void RemoveComportment(Comportment *pcomportmemt) = NULL;

		/// Updates the object by the given amount of elapsed time (in seconds)
		virtual void Update(float elapsed_time = 0.0f) = NULL;

		/// Called prior to actually rendering the object; returns true if Render should be called, false if not
		virtual bool Prerender(props::TFlags64 rendflags) = NULL;

		/// Called to render the object; returns true if Postrender should be called, false if not
		virtual bool Render(props::TFlags64 rendflags) = NULL;

		/// Called after the object is rendered
		virtual void Postrender(props::TFlags64 rendflags) = NULL;

		/// Loads the Object from a stream
		virtual bool Load(genio::IInputStream *is) = NULL;

		/// Saves an Object to a stream
		virtual bool Save(genio::IOutputStream *os, props::TFlags64 saveflags) = NULL;

		/// Called once Load has finished
		virtual void PostLoad() = NULL;

	};

};
