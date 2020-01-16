// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Object.h>

namespace c3
{

	/// A Comportment is a behavior controller that attaches to an Object and is, given it's flags,
	/// Updated or Rendered, thus giving plain "vanilla" Objects more interesting characteristics.
	/// Comportments exist because Objects are abstract and, therefore, do not directly expose
	/// their members to Object implementations in plug-in modules. 

	/// See the macros at the bottom to see how to get custom Comportments integrated

	class Comportment
	{

	public:

		virtual void Release() = NULL;

		/// Returns the ComportmentType that built this Comportment
		/// IMPORTANT: YOU DO NOT NEED TO IMPLEMENT THIS METHOD YOURSELF IF YOU USE REGISTER_COMPORTMENTTYPE
		virtual ComportmentType *GetType() = NULL;

		/// Returns the flags that reveal what this Comportment should be used for, such as rendering or character-like behavior
		virtual props::TFlags64 Flags() = NULL;

		/// Called to initialize the Comportment on a given Object, allowing it to allocate any resources
		virtual bool Initialize(Object *pobject) = NULL;

		/// Called by the Object that owns the Comportment during it's own Update
		virtual void Update(Object *pobject, float elapsed_time = 0.0f) = NULL;

		/// Called by the Object that owns the Comportment during it's own Prerender
		/// If any Comportment Prerender succeeds, Object::Render is called
		virtual bool Prerender(Object *pobject, props::TFlags64 rendflags) = NULL;

		/// Called by the Object that owns the Comportment during it's own Render
		virtual void Render(Object *pobject, props::TFlags64 rendflags) = NULL;

	};


	/// Implement a companion ComportmentType for each Comportment and register it with the Factory in order to
	/// have Prototypes that build complete Objects. Why? To maintain heap coherency; plug-ins are responsible
	/// for creation and destruction of their own Comportments.

	class ComportmentType
	{

	public:

		/// Constructs a new instance of it's associated Comportment and assigns it to the given Object.
		/// Optionally copies the properties of a given Comportment instance
		virtual Comportment *Build() const = NULL;

		/// Destoys an instance of it's associated Comportment
		virtual void Destroy(Comportment *pcomportment) const = NULL;

		/// Returns the user-defined GUID that identifies the ComportmentType type
		virtual GUID GetGUID() const = NULL;

		/// Returns the name of the ComportmentType
		virtual const TCHAR *GetName() const = NULL;

		/// Returns a string that describes the associated Comportment class
		virtual const TCHAR *GetDescription() const = NULL;

	};


	/// This is the standard way of registering your comportments
	/// Do your own thing but beware!

	#define COMPORTMENTTYPE(comportment_class) comportment_class##Type

	/// THIS GOES IN YOUR HEADER
	#define DEFINE_COMPORTMENTTYPE(comportment_class, comportmentimpl_class, guid, name, description)					\
		class COMPORTMENTTYPE(comportment_class) : public c3::ComportmentType											\
		{																												\
			friend class comportment_class;																				\
			public:																										\
			static COMPORTMENTTYPE(comportment_class) self;																\
			static void Register(c3::Factory *factory) {																\
				if (factory) { factory->RegisterComportmentType(&self); } }												\
			static void Unregister(c3::Factory *factory) {																\
				if (factory) { factory->UnregisterComportmentType(&self); } }											\
			COMPORTMENTTYPE(comportment_class)() { }																	\
			virtual ~COMPORTMENTTYPE(comportment_class)() { }															\
			virtual c3::Comportment *Build() const { return new comportmentimpl_class(); }								\
			virtual void Destroy(c3::Comportment *pc) const { pc->Release(); }											\
			virtual GUID GetGUID() const { return GUID(guid); }															\
			virtual const TCHAR *GetName() const { return name; }														\
			virtual const TCHAR *GetDescription() const { return description; }											\
		}

	/// THIS GOES IN YOUR SOURCE
	#define DECLARE_COMPORTMENTTYPE(comportment_class, comportmentimpl_class)											\
		COMPORTMENTTYPE(comportment_class) COMPORTMENTTYPE(comportment_class)::self;									\
		c3::ComportmentType *comportmentimpl_class::GetType() { return &COMPORTMENTTYPE(comportment_class)::self; }		\
		const c3::ComportmentType *comportment_class::Type() { return (const c3::ComportmentType *)&COMPORTMENTTYPE(comportment_class)::self; }

	/// DO THIS AFTER YOU CALL c3::System::Create OR WHEN YOUR PLUG-IN IS INITIALIZED
	#define REGISTER_COMPORTMENTTYPE(comportment_class, factory)	COMPORTMENTTYPE(comportment_class)::Register(factory)

	/// DO THIS WHEN YOU UNLOAD YOUR PLUGIN OR BEFORE YOU CALL c3::System::Release
	#define UNREGISTER_COMPORTMENTTYPE(comportment_class, factory)	COMPORTMENTTYPE(comportment_class)::Unregister(factory)

};
