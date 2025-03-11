// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Utility.h>

namespace c3
{

	class ObjectImpl : public Object
	{

	protected:
		SystemImpl *m_pSys;
		tstring m_Name;
		GUID m_GUID;
		props::TFlags64 m_Flags;

		props::IPropertySet *m_Props;

		typedef std::deque<Component *> TComponentArray;
		TComponentArray m_Components;

		Object *m_pParent;

		TObjectArray m_Children;

	public:

		ObjectImpl(SystemImpl *psys, GUID guid);

		virtual ~ObjectImpl();

		virtual System *GetSystem() const final;

		virtual void Release() final;

		virtual const TCHAR *GetName() const final;

		virtual void SetName(const TCHAR *name) final;

		virtual GUID GetGuid() const final;

		virtual Object *GetParent() const final;

		virtual void SetParent(Object *pparent) final;

		virtual size_t GetNumChildren() const final;

		virtual Object *GetChild(size_t index) const final;

		virtual void AddChild(Object *pchild) final;

		virtual void RemoveChild(Object *pchild, bool release = false) final;

		virtual props::TFlags64 &Flags() final;

		virtual props::IPropertySet *GetProperties() final;

		virtual size_t GetNumComponents() const final;

		virtual Component *GetComponent(size_t index) const final;

		virtual Component *FindComponent(const ComponentType *pctype) const final;

		virtual Component *AddComponent(const ComponentType *pctype, bool init = true) final;

		virtual void RemoveComponent(Component *pcomponent) final;

		virtual bool HasComponent(const ComponentType *pcomptype) const final;

		virtual void Update(float elapsed_time = 0.0f) final;

		virtual bool Render(Object::RenderFlags flags, int draworder, const glm::fmat4x4 *pmat = nullptr) final;

		virtual bool Load(genio::IInputStream *is, Object *parent, MetadataLoadFunc loadmd, CameraLoadFunc loadcam, EnvironmentLoadFunc loadenv, CustomLoadFunc loadcust) final;

		virtual bool Save(genio::IOutputStream *os, SaveFlags saveflags, MetadataSaveFunc savemd, CameraSaveFunc savecam, EnvironmentSaveFunc saveenv, CustomSaveFunc savecust) const final;

		virtual void PostLoad() final;

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance = nullptr, Object **ppHitObj = nullptr, props::TFlags64 flagmask = OF_DRAW, size_t child_depth = 0, bool force = false) const final;

		virtual void PropertyChanged(const props::IProperty *pprop) final;

	};

};
