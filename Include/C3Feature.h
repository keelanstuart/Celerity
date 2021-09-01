// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Object.h>

namespace c3
{

	/// A Feature is a behavior controller that attaches to an Object and is, given it's flags,
	/// Updated or Rendered, thus giving plain "vanilla" Objects more interesting characteristics.
	/// Features exist because Objects are abstract and, therefore, do not directly expose
	/// their members to Object implementations in plug-in modules. 

	/// See the macros at the bottom to see how to get custom Features integrated

	class Feature
	{

	public:

		virtual void Release() = NULL;

		/// Returns the FeatureType that built this Feature
		/// IMPORTANT: YOU DO NOT NEED TO IMPLEMENT THIS METHOD YOURSELF IF YOU USE REGISTER_FEATURETYPE
		virtual FeatureType *GetType() = NULL;

		/// Returns the flags that reveal what this Feature should be used for, such as rendering or character-like behavior
		virtual props::TFlags64 Flags() = NULL;

		/// Called to initialize the Feature on a given Object, allowing it to allocate any resources
		virtual bool Initialize(Object *pobject) = NULL;

		/// Called by the Object that owns the Feature during it's own Update
		virtual void Update(Object *pobject, float elapsed_time = 0.0f) = NULL;

		/// Called by the Object that owns the Feature during it's own Prerender
		/// If any Feature Prerender succeeds, Object::Render is called
		virtual bool Prerender(Object *pobject, props::TFlags64 rendflags) = NULL;

		/// Called by the Object that owns the Feature during it's own Render
		virtual void Render(Object *pobject, props::TFlags64 rendflags) = NULL;

		/// Called when a property on the owning Object has changed
		virtual void PropertyChanged(const props::IProperty *pprop) = NULL;

	};


	/// Implement a companion FeatureType for each Feature and register it with the Factory in order to
	/// have Prototypes that build complete Objects. Why? To maintain heap coherency; plug-ins are responsible
	/// for creation and destruction of their own Features.

	class FeatureType
	{

	public:

		/// Constructs a new instance of it's associated Feature and assigns it to the given Object.
		/// Optionally copies the properties of a given Feature instance
		virtual Feature *Build() const = NULL;

		/// Destoys an instance of it's associated Feature
		virtual void Destroy(Feature *pfeature) const = NULL;

		/// Returns the user-defined GUID that identifies the FeatureType type
		virtual GUID GetGUID() const = NULL;

		/// Returns the name of the FeatureType
		virtual const TCHAR *GetName() const = NULL;

		/// Returns a string that describes the associated Feature class
		virtual const TCHAR *GetDescription() const = NULL;

	};


	/// This is the standard way of registering your Feature
	/// Do your own thing, but beware!

#define FEATURETYPE(feature_class) feature_class##Type

/// THIS GOES IN YOUR HEADER
#define DEFINE_FEATURETYPE(feature_class, featureimpl_class, guid, name, description)					\
		class FEATURETYPE(feature_class) : public c3::FeatureType											\
		{																									\
			friend class feature_class;																		\
			public:																							\
			static FEATURETYPE(feature_class) self;															\
			static void Register(c3::Factory *factory) {													\
				if (factory) { factory->RegisterFeatureType(&self); } }										\
			static void Unregister(c3::Factory *factory) {													\
				if (factory) { factory->UnregisterFeatureType(&self); } }									\
			FEATURETYPE(feature_class)() { }																\
			virtual ~FEATURETYPE(feature_class)() { }														\
			virtual c3::Feature *Build() const { return new featureimpl_class(); }							\
			virtual void Destroy(c3::Feature *pc) const { pc->Release(); }									\
			virtual GUID GetGUID() const { return GUID(guid); }												\
			virtual const TCHAR *GetName() const { return _T(name); }										\
			virtual const TCHAR *GetDescription() const { return _T(description); }							\
		}

	/// THIS GOES IN YOUR SOURCE
#define DECLARE_FEATURETYPE(feature_class, featureimpl_class)											\
		FEATURETYPE(feature_class) FEATURETYPE(feature_class)::self;										\
		c3::FeatureType *featureimpl_class::GetType() { return &FEATURETYPE(feature_class)::self; }			\
		const c3::FeatureType *feature_class::Type() { return (const c3::FeatureType *)&FEATURETYPE(feature_class)::self; }

	/// DO THIS AFTER YOU CALL c3::System::Create OR WHEN YOUR PLUG-IN IS INITIALIZED
#define REGISTER_FEATURETYPE(feature_class, factory)	FEATURETYPE(feature_class)::Register(factory)

/// DO THIS WHEN YOU UNLOAD YOUR PLUGIN OR BEFORE YOU CALL c3::System::Release
#define UNREGISTER_FEATURETYPE(feature_class, factory)	FEATURETYPE(feature_class)::Unregister(factory)

};
