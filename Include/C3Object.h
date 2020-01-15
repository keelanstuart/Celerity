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

		enum
		{
			SHIFT_OBJFLAG_UPDATE = 0,
			SHIFT_OBJFLAG_DRAW,
			SHIFT_OBJFLAG_DRAWINEDITOR,
			SHIFT_OBJFLAG_POSCHANGED,
			SHIFT_OBJFLAG_ORICHANGED,
			SHIFT_OBJFLAG_SCLCHANGED,
			SHIFT_OBJFLAG_KILL,
			SHIFT_OBJFLAG_TEMPORARY,
			SHIFT_OBJFLAG_CHECKCOLLISIONS,
			SHIFT_OBJFLAG_TRACKCAMX,
			SHIFT_OBJFLAG_TRACKCAMY,
			SHIFT_OBJFLAG_TRACKCAMZ,
			SHIFT_OBJFLAG_TRACKCAMLITERAL,
			SHIFT_OBJFLAG_BILLBOARD,
			SHIFT_OBJFLAG_CHILDRENDIRTY,
			SHIFT_OBJFLAG_PARENTDIRTY,
		};

		#define OBJFLAG_UPDATE				(1 << SHIFT_OBJFLAG_UPDATE)				// Clearing this ensures the object won't update
		#define OBJFLAG_DRAW				(1 << SHIFT_OBJFLAG_DRAW)				// Clearing this ensures the object won't draw
		#define OBJFLAG_DRAWINEDITOR		(1 << SHIFT_OBJFLAG_DRAWINEDITOR)		// Setting this will make the object draw in the editor... note: it is up to a tool to handle this
		#define OBJFLAG_POSCHANGED			(1 << SHIFT_OBJFLAG_POSCHANGED)			// WARNING: not recommended that you change this manually
		#define OBJFLAG_ORICHANGED			(1 << SHIFT_OBJFLAG_ORICHANGED)			// WARNING: not recommended that you change this manually
		#define OBJFLAG_SCLCHANGED			(1 << SHIFT_OBJFLAG_SCLCHANGED)			// WARNING: not recommended that you change this manually
		#define OBJFLAG_KILL				(1 << SHIFT_OBJFLAG_KILL)				// The object is marked for death
		#define OBJFLAG_TEMPORARY			(1 << SHIFT_OBJFLAG_TEMPORARY)			// Temporary objects will not persist when parent objects are saved
		#define OBJFLAG_CHECKCOLLISIONS		(1 << SHIFT_OBJFLAG_CHECKCOLLISIONS		// Indicates that the Object should respond to collisions
		#define OBJFLAG_TRACKCAMX			(1 << SHIFT_OBJFLAG_TRACKCAMX)			// Move with the active camera X
		#define OBJFLAG_TRACKCAMY			(1 << SHIFT_OBJFLAG_TRACKCAMY)			// " Y
		#define OBJFLAG_TRACKCAMZ			(1 << SHIFT_OBJFLAG_TRACKCAMZ)			// " Z
		#define OBJFLAG_TRACKCAMLITERAL		(1 << SHIFT_OBJFLAG_TRACKCAMLITERAL		// Distinguishes between eye or focus position when following the camera
		#define OBJFLAG_BILLBOARD			(1 << SHIFT_OBJFLAG_BILLBOARD)			// Aligns the object to the view matrix of the renderer when drawn
		#define OBJFLAG_CHILDRENDIRTY		(1 << SHIFT_OBJFLAG_CHILDRENDIRTY)		// Indicates that the children have changed since the last update
		#define OBJFLAG_PARENTDIRTY			(1 << SHIFT_OBJFLAG_PARENTDIRTY)		// Indicates that the children have changed since the last update
		
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

		/// Returns the number of direct decendents from the Object
		virtual size_t GetNumChildren() = NULL;

		/// Returns the child Object at the given index, or nullptr if there is not one
		virtual Object *GetChild(size_t index) = NULL;

		/// Adds a child Object
		virtual void AddChild(Object *pchild) = NULL;

		/// Allows access to the Object's flags
		virtual props::TFlags64 &Flags() = NULL;

		/// Returns the IPropertySet owned by the Object
		virtual props::IPropertySet *GetProperties() = NULL;

		/// Returns the number of Comportments currently attached to this Object
		virtual size_t GetNumComportments() = NULL;

		/// Returns the attached Comportment at the given index
		virtual Comportment *GetComportment(size_t index) = NULL;

		/// Returns an attached Comportment with the given ComportmentType
		virtual Comportment *FindComportment(const ComportmentType *pctype) = NULL;

		/// Creates, attaches, and returns a Comportment of the given ComportmentType
		virtual Comportment *AddComportment(const ComportmentType *pctype) = NULL;

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
