// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3Component.h>
#include <PowerProps.h>
#include <GenIO.h>

namespace c3
{

	class Component;
	class ComponentType;

	/// All world objects in Celerity are Objects. Objects have their own properties and maintain
	/// a list of Components -- the dictionary definition of "Component" is: behavior, bearing -- and
	/// can receive time-based updates and process requests to draw themselves.

	/// Previous versions of Celerity followed a paradigm of sub-classes derived from the base object class to be
	/// written by plug-in creators -- with the API caveat that all member data was exposed. I'm not saying
	/// the abstract interfaces in Celerity v3 are the only viable or reasonable choice for modern C++ libraries, but
	/// the advantage is easy to see: "clean", "simple", and "portable" code is easier to deal with

	class Object : public props::IPropertyChangeListener
	{

	public:

		typedef props::TFlags64		ObjectFlags;

		// ObjectFlags
		#define OF_UPDATE			0x00000001					// Clearing this ensures the object won't update
		#define OF_DRAW				0x00000002					// Clearing this ensures the object won't draw
		#define OF_DRAWINEDITOR		0x00000004					// Setting this will make the object draw in the editor... note: it is up to a tool to handle this
		#define OF_POSCHANGED		0x00000008					// WARNING: not recommended that you change this manually
		#define OF_ORICHANGED		0x00000010					// WARNING: not recommended that you change this manually
		#define OF_SCLCHANGED		0x00000020					// WARNING: not recommended that you change this manually
		#define OF_KILL				0x00000040					// The object is marked for death
		#define OF_TEMPORARY		0x00000080					// Temporary objects will not persist when parent objects are saved
		#define OF_CHECKCOLLISIONS	0x00000100					// Indicates that the Object should respond to collisions
		#define OF_TRACKCAMX		0x00000200					// Move with the active camera X
		#define OF_TRACKCAMY		0x00000400					// " Y
		#define OF_TRACKCAMZ		0x00000800					// " Z
		#define OF_TRACKCAMLITERAL	0x00001000					// Distinguishes between eye or focus position when following the camera
		#define OF_BILLBOARD		0x00002000					// Aligns the object to the view matrix of the renderer when drawn
		#define OF_CHILDRENDIRTY	0x00004000					// Indicates that the children have changed since the last update
		#define OF_PARENTDIRTY		0x00008000					// Indicates that the parent has changed since the last update
		#define OF_LIGHT			0x00010000					// Indicates that the object emits light
		#define OF_CASTSHADOW		0x00020000					// Indicates that the object casts a shadow
		#define OF_NOMODELSCALE		0x00040000					// DEPRECATED; Affects only ModelRenderer; does not scale the model
		#define OF_LOCKED			0x00080000					// Don't allow changes in the editor

		#define OF_EXPANDED			0x10000000					// A flag for tools, indicates that the child objects should be displayed


		typedef props::TFlags64		RenderFlags;

		// RenderFlags
		#define RF_EDITORDRAW		0x00000001					// Indicates this should be drawn as it would be in an editor
		#define RF_LOCKSHADER		0x00000002					// Do not allow the shader to be changed
		#define RF_LOCKMATERIAL		0x00000004					// Do not allow the material to be changed
		#define RF_DRAWBOUNDS		0x00000008					// Draw the bounding box
		#define RF_SHADOW			0x00000010					// Draw the shadow
		#define RF_LIGHT			0x00000020					// Draw the light
		#define RF_FORCE			0x00000040					// Force drawing
		#define RF_AUXILIARY		0x00000080					// Drawing the auxiliary surface
		#define RF_GUI				0x00000100					// User Interface
		#define RF_SELECTED			0x00000200					// Drawing as a selection
		#define RF_EFFECT			0x00000400					// A special effect (drawn after everything else?!)


		typedef props::TFlags64		SaveFlags;

		// SaveFlags
		#define SF_REFERENCEFILE	0x00000001					// Save as a reference file


		/// Returns the Celerity System in which the Object exists
		virtual System *GetSystem() const = NULL;

		/// Frees any resources that the Object may have allocated
		virtual void Release() = NULL;

		/// Returns the name of the Object
		virtual const TCHAR *GetName() const = NULL;

		/// Sets the name of the Object
		virtual void SetName(const TCHAR *name) = NULL;

		/// Returns the Object's GUID
		virtual GUID GetGuid() const = NULL;

		/// Returns the parent of this Object
		virtual Object *GetParent() const = NULL;

		/// Sets the Object's owner
		virtual void SetParent(Object *pparent) = NULL;

		/// Returns the number of direct decendents from the Object
		virtual size_t GetNumChildren() const = NULL;

		/// Returns the child Object at the given index, or nullptr if there is not one
		virtual Object *GetChild(size_t index) const = NULL;

		/// Adds a child Object
		virtual void AddChild(Object *pchild) = NULL;

		/// Removes a child Object
		virtual void RemoveChild(Object *pchild, bool release = false) = NULL;

		/// Allows access to the Object's flags
		virtual ObjectFlags &Flags() = NULL;

		/// Returns the IPropertySet owned by the Object
		virtual props::IPropertySet *GetProperties() = NULL;

		/// Returns the number of Components currently attached to this Object
		virtual size_t GetNumComponents() const = NULL;

		/// Returns the attached Component at the given index
		virtual Component *GetComponent(size_t index) const = NULL;

		/// Returns an attached Component with the given ComponentType
		virtual Component *FindComponent(const ComponentType *pctype) const = NULL;

		/// Creates, attaches, and returns a Component of the given ComponentType
		virtual Component *AddComponent(const ComponentType *pctype, bool init = true) = NULL;

		/// Removes the given Component
		virtual void RemoveComponent(Component *pcomponent) = NULL;

		// Returns true if the Object has the given Component
		virtual bool HasComponent(const ComponentType *pcomptype) const = NULL;

		/// Updates the object by the given amount of elapsed time (in seconds)
		virtual void Update(float elapsed_time = 0.0f) = NULL;

		/// Called to render the object; returns true if Postrender should be called, false if not
		virtual bool Render(RenderFlags flags = 0, int draworder = 0, const glm::fmat4x4 *pmat = nullptr) = NULL;

		using MetadataLoadFunc = std::function<void(const tstring &name, const tstring &description, const tstring &author, const tstring &website, const tstring &copyright)>;
		using CameraLoadFunc = std::function<void(Object *camera, float yaw, float pitch)>;
		using EnvironmentLoadFunc = std::function<void(const glm::fvec4 &clearcolor, const glm::fvec4 &shadowcolor, const glm::fvec4 &fogcolor, const float &fogdensity)>;
		using CustomLoadFunc = std::function<void(genio::IInputStream *is)>; // BeginBlock has been called already - load things yourself

		/// Loads the Object from a stream
		virtual bool Load(genio::IInputStream *is, Object *parent, MetadataLoadFunc loadmd = nullptr, CameraLoadFunc loadcam = nullptr, EnvironmentLoadFunc loadenv = nullptr, CustomLoadFunc loadcust = nullptr) = NULL;

		using MetadataSaveFunc = std::function<void(tstring &name, tstring &description, tstring &author, tstring &website, tstring &copyright)>;
		using CameraSaveFunc = std::function<void(Object **camera, float &yaw, float &pitch)>;
		using EnvironmentSaveFunc = std::function<void(glm::fvec4 &clearcolor, glm::fvec4 &shadowcolor, glm::fvec4 &fogcolor, float &fogdensity)>;
		using CustomSaveFunc = std::function<void(genio::IOutputStream *os)>; // BeginBlock has been called already - load things yourself

		/// Saves an Object to a stream
		virtual bool Save(genio::IOutputStream *os, SaveFlags saveflags, MetadataSaveFunc savemd = nullptr, CameraSaveFunc savecam = nullptr, EnvironmentSaveFunc saveenv = nullptr, CustomSaveFunc savecust = nullptr) const = NULL;

		/// Called once Load has finished
		virtual void PostLoad() = NULL;

		/// Casts a ray against the object
		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat = nullptr, float *pDistance = nullptr, Object **ppHitObj = nullptr, props::TFlags64 flagmask = OF_DRAW, size_t child_depth = 0, bool force = false) const = NULL;

	};

};
