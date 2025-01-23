// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <PowerProps.h>
#include <C3FlowGraph.h>

namespace c3
{

	class FlowNodeCore
	{

	public:

		template <typename T> class InputPin
		{
			
		};

		virtual size_t GetNumInputPins() = NULL;

		/// Allows access to the FlowNode's flags
		virtual props::TFlags64 &Flags() = NULL;

		/// Returns the IPropertySet owned by the FlowNode
		virtual props::IPropertySet *GetProperties() = NULL;

		/// Loads the FlowNode from a stream
		virtual bool Load(genio::IInputStream *is) = NULL;

		/// Saves an FlowNode to a stream
		virtual bool Save(genio::IOutputStream *os, props::TFlags64 saveflags) = NULL;

	};

	class FlowNode : public props::IPropertyChangeListener
	{

	public:



		/// Frees any resources that the node may have allocated
		virtual void Release() = NULL;

		/// Returns the FlowNode's GUID
		virtual GUID GetGuid() = NULL;

		/// Returns the owner of this FlowNode
		virtual FlowGraph *GetOwner() = NULL;

		/// Sets the FlowNode's owner
		virtual void SetOwner(FlowGraph *powner) = NULL;

		virtual void Trigger() = NULL;


		/// Called once Load has finished
		virtual void PostLoad() = NULL;

	};

	/// Implement a companion FlowNodeType for each FlowNode and register it with the Factory

	class FlowNodeType
	{

	public:

		/// Constructs a new instance of it's associated FlowNode and assigns it to the given Object.
		/// Optionally copies the properties of a given Component instance
		virtual FlowNode *Build() const = NULL;

		/// Destoys an instance of it's associated FlowNode
		virtual void Destroy(FlowNode *pcomponent) const = NULL;

		/// Returns the user-defined GUID that identifies the FlowNode type
		virtual GUID GetGUID() const = NULL;

		/// Returns the name of the FlowNodeType
		virtual const TCHAR *GetName() const = NULL;

		/// Returns a string that describes the associated FlowNode class
		virtual const TCHAR *GetDescription() const = NULL;

	};


	/// This is the standard way of registering your FlowNode
	/// Do your own thing, but beware!

#define FLOWNODETYPE(flownode_class) flownode_class##Type

	/// THIS GOES IN YOUR HEADER
#define DEFINE_FLOWNODETYPE(flownode_class, flownodeimpl_class, guid, name, description)						\
		class FLOWNODETYPE(flownode_class) : public c3::FlowNodeType											\
		{																										\
			friend class flownode_class;																		\
			public:																								\
			static FLOWNODETYPE(flownode_class) self;															\
			static void Register(c3::Factory *factory) {														\
				if (factory) { factory->RegisterFlowNodeType(&self); } }										\
			static void Unregister(c3::Factory *factory) {														\
				if (factory) { factory->UnregisterFlowNodeType(&self); } }										\
			FLOWNODETYPE(flownode_class)() { }																	\
			virtual ~FLOWNODETYPE(flownode_class)() { }															\
			virtual c3::FlowNode *Build() const { return new flownodeimpl_class(); }							\
			virtual void Destroy(c3::FlowNode *pc) const { pc->Release(); }										\
			virtual GUID GetGUID() const { return GUID(guid); }													\
			virtual const TCHAR *GetName() const { return _T(name); }											\
			virtual const TCHAR *GetDescription() const { return _T(description); }								\
		}

	/// THIS GOES IN YOUR SOURCE
#define DECLARE_FLOWNODETYPE(flownode_class, flownodeimpl_class)												\
		FLOWNODETYPE(flownode_class) FLOWNODETYPE(flownode_class)::self;										\
		c3::FlowNodeType *flownodeimpl_class::GetType() { return &FLOWNODETYPE(flownode_class)::self; }			\
		const c3::FlowNodeType *flownode_class::Type() { return (const c3::FlowNodeType *)&FLOWNODETYPE(flownode_class)::self; }

	/// DO THIS AFTER YOU CALL c3::System::Create OR WHEN YOUR PLUG-IN IS INITIALIZED
#define REGISTER_FLOWNODETYPE(flownode_class, factory)	FLOWNODETYPE(flownode_class)::Register(factory)

	/// DO THIS WHEN YOU UNLOAD YOUR PLUGIN OR BEFORE YOU CALL c3::System::Release
#define UNREGISTER_FLOWNODETYPE(flownode_class, factory)	FLOWNODETYPE(flownode_class)::Unregister(factory)

};
