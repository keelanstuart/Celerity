// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include <C3.h>
#include <C3PhysicsManager.h>

#if defined(ODE_SUPPORT) && ODE_SUPPORT
#include <ode\ode.h>
#include <ode\objects.h>
#include <ode\collision_space.h>
#else
using dTriMeshDataID = int *;
using dWorldID = int *;
using dSpaceID = int *;
using dJointGroupID = int *;
using dBodyID = int *;
#endif

#pragma once

namespace c3
{

	class PhysicsManagerImpl : public PhysicsManager
	{

	protected:
		using PhysMeshMap = std::unordered_map<const Model *, dTriMeshDataID>;
		PhysMeshMap m_Meshes;

		using DynamicObjectMap = std::unordered_map<const Object *, dBodyID>;
		DynamicObjectMap m_Dynamics;

		dWorldID m_World;
		dSpaceID m_Space;
		dJointGroupID m_Joints;

		System *m_pSys;


	public:
		PhysicsManagerImpl(System *psys);

		virtual ~PhysicsManagerImpl();

		virtual bool Initialize();

		virtual void Shutdown();

		virtual void Update(float time = 0.0f);

		virtual bool CreateCollisionMesh(const Model *pmod, dTriMeshDataID &ret, const glm::fmat4x4 *sclmat);

		virtual bool AddObject(const Object *pobj);

		virtual bool RemoveObject(const Object *pobj);

		virtual bool MakeObjectKinetic(const Object *pobj, bool kinematic);

		virtual void UpdateObject(const Object *pobj);

	};

};