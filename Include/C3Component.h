// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Object.h>

namespace c3
{

	/// A Component is a behavior controller that attaches to an Object and is, given it's flags,
	/// Updated or Rendered, thus giving plain "vanilla" Objects more interesting characteristics.
	/// Components exist because Objects are abstract and, therefore, do not directly expose
	/// their members to Object implementations in plug-in modules. 

	/// See the macros at the bottom to see how to get custom Components integrated

	class C3_API Component
	{

	public:

		typedef enum
		{
			ALLOW_MULTIPLE = 0,		// Indicates that multiple Component instances of this type can be added to a single Object instance

		} EComponentFlag;

		static constexpr uint64_t COMPFLAG(EComponentFlag f) { return (1LL << (f)); }

		virtual void Release() = NULL;

		/// Returns the ComponentType that built this Component
		/// IMPORTANT: YOU DO NOT NEED TO IMPLEMENT THIS METHOD YOURSELF IF YOU USE REGISTER_COMPONENTTYPE
		virtual const ComponentType *GetType() const = NULL;

		/// Returns the flags that reveal what this Component should be used for, such as rendering or character-like behavior
		virtual props::TFlags64 Flags() const = NULL;

		/// Called to initialize the Component on a given Object, allowing it to allocate any resources
		virtual bool Initialize(Object *pobject) = NULL;

		/// Called by the Object that owns the Component during it's own Update
		virtual void Update(Object *pobject, float elapsed_time = 0.0f) = NULL;

		/// Called by the Object that owns the Component during it's own Prerender
		/// If any Component Prerender succeeds, Object::Render is called
		virtual bool Prerender(Object *pobject, Object::RenderFlags flags) = NULL;

		/// Called by the Object that owns the Component during it's own Render
		virtual void Render(Object *pobject, Object::RenderFlags flags) = NULL;

		/// Called when a property on the owning Object has changed
		virtual void PropertyChanged(const props::IProperty *pprop) = NULL;

		/// Casts a ray against the component
		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance) const = NULL;

	};


	/// Implement a companion ComponentType for each Component and register it with the Factory in order to
	/// have Prototypes that build complete Objects. Why? To maintain heap coherency; plug-ins are responsible
	/// for creation and destruction of their own Components.

	class ComponentType
	{

	public:

		/// Constructs a new instance of it's associated Component and assigns it to the given Object.
		/// Optionally copies the properties of a given Component instance
		virtual Component *Build() const = NULL;

		/// Destoys an instance of it's associated Component
		virtual void Destroy(Component *pcomponent) const = NULL;

		/// Returns the user-defined GUID that identifies the ComponentType type
		virtual GUID GetGUID() const = NULL;

		/// Returns the name of the ComponentType
		virtual const TCHAR *GetName() const = NULL;

		/// Returns a string that describes the associated Component class
		virtual const TCHAR *GetDescription() const = NULL;

	};


	/// This is the standard way of registering your Component
	/// Do your own thing, but beware!

#define COMPONENTTYPE(component_class) component_class##Type

/// THIS GOES IN YOUR HEADER
#define DEFINE_COMPONENTTYPE(component_class, componentimpl_class, guid, name, description)						\
		class COMPONENTTYPE(component_class) : public c3::ComponentType											\
		{																										\
			friend class component_class;																		\
			public:																								\
			static COMPONENTTYPE(component_class) self;															\
			static void Register(c3::Factory *factory) {														\
				if (factory) { factory->RegisterComponentType(&self); } }										\
			static void Unregister(c3::Factory *factory) {														\
				if (factory) { factory->UnregisterComponentType(&self); } }										\
			COMPONENTTYPE(component_class)() { }																\
			virtual ~COMPONENTTYPE(component_class)() { }														\
			virtual c3::Component *Build() const { return new componentimpl_class(); }							\
			virtual void Destroy(c3::Component *pc) const { pc->Release(); }									\
			virtual GUID GetGUID() const { return GUID(guid); }													\
			virtual const TCHAR *GetName() const { return _T(name); }											\
			virtual const TCHAR *GetDescription() const { return _T(description); }								\
		}

/// THIS GOES IN YOUR SOURCE
#define DECLARE_COMPONENTTYPE(component_class, componentimpl_class)															\
		COMPONENTTYPE(component_class) COMPONENTTYPE(component_class)::self;												\
		const c3::ComponentType *componentimpl_class::GetType() const { return &COMPONENTTYPE(component_class)::self; }		\
		const c3::ComponentType *component_class::Type() { return (const c3::ComponentType *)&COMPONENTTYPE(component_class)::self; }

	/// DO THIS AFTER YOU CALL c3::System::Create OR WHEN YOUR PLUG-IN IS INITIALIZED
#define REGISTER_COMPONENTTYPE(component_class, factory)	COMPONENTTYPE(component_class)::Register(factory)

/// DO THIS WHEN YOU UNLOAD YOUR PLUGIN OR BEFORE YOU CALL c3::System::Release
#define UNREGISTER_COMPONENTTYPE(component_class, factory)	COMPONENTTYPE(component_class)::Unregister(factory)

};
