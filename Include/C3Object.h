// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3Feature.h>
#include <PowerProps.h>
#include <GenIO.h>

namespace c3
{

	class Feature;
	class FeatureType;

	/// All world objects in Celerity are Objects. Objects have their own properties and maintain
	/// a list of Features -- the dictionary definition of "Feature" is: behavior, bearing -- and
	/// can receive time-based updates and process requests to draw themselves.

	/// Previous versions of Celerity followed a paradigm of sub-classes derived from the base object class to be
	/// written by plug-in creators -- with the API caveat that all member data was exposed. I'm not saying
	/// the abstract interfaces in Celerity v3 are the only viable or reasonable choice for modern C++ libraries, but
	/// the advantage is easy to see: "clean", "simple", and "portable" code is easier to deal with

	class Object : public props::IPropertyChangeListener
	{

	public:

		typedef enum
		{
			UPDATE = 0,					// Clearing this ensures the object won't update
			DRAW,						// Clearing this ensures the object won't draw
			DRAWINEDITOR,				// Setting this will make the object draw in the editor... note: it is up to a tool to handle this
			POSCHANGED,					// WARNING: not recommended that you change this manually
			ORICHANGED,					// WARNING: not recommended that you change this manually
			SCLCHANGED,					// WARNING: not recommended that you change this manually
			KILL,						// The object is marked for death
			TEMPORARY,					// Temporary objects will not persist when parent objects are saved
			CHECKCOLLISIONS,			// Indicates that the Object should respond to collisions
			TRACKCAMX,					// Move with the active camera X
			TRACKCAMY,					// " Y
			TRACKCAMZ,					// " Z
			TRACKCAMLITERAL,			// Distinguishes between eye or focus position when following the camera
			BILLBOARD,					// Aligns the object to the view matrix of the renderer when drawn
			CHILDRENDIRTY,				// Indicates that the children have changed since the last update
			PARENTDIRTY,				// Indicates that the parent has changed since the last update
		} EObjFlag;

		// compile time helper for making bitwise flags out of EObjFlag enum values
		// OBJFLAG(TRACKCAMX) | OBJFLAG(TRACKCAMY), for example, converts and conflates those flags at compile time
		static constexpr uint64_t OBJFLAG(EObjFlag f) { return (1LL << (f)); }
		

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

		/// Returns the number of Features currently attached to this Object
		virtual size_t GetNumFeatures() = NULL;

		/// Returns the attached Feature at the given index
		virtual Feature *GetFeature(size_t index) = NULL;

		/// Returns an attached Feature with the given FeatureType
		virtual Feature *FindFeature(const FeatureType *pctype) = NULL;

		/// Creates, attaches, and returns a Feature of the given FeatureType
		virtual Feature *AddFeature(const FeatureType *pctype, bool init = true) = NULL;

		/// Removes the given Feature
		virtual void RemoveFeature(Feature *pcomportmemt) = NULL;

		/// Updates the object by the given amount of elapsed time (in seconds)
		virtual void Update(float elapsed_time = 0.0f) = NULL;

		/// Called prior to actually rendering the object; returns true if Render should be called, false if not
		virtual bool Prerender(props::TFlags64 rendflags = OBJFLAG(DRAW)) = NULL;

		/// Called to render the object; returns true if Postrender should be called, false if not
		virtual bool Render(props::TFlags64 rendflags = OBJFLAG(DRAW)) = NULL;

		/// Called after the object is rendered
		virtual void Postrender(props::TFlags64 rendflags = OBJFLAG(DRAW)) = NULL;

		/// Loads the Object from a stream
		virtual bool Load(genio::IInputStream *is) = NULL;

		/// Saves an Object to a stream
		virtual bool Save(genio::IOutputStream *os, props::TFlags64 saveflags) = NULL;

		/// Called once Load has finished
		virtual void PostLoad() = NULL;

	};

};
