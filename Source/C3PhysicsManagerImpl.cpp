// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3PhysicsManagerImpl.h>
#include <C3PhysicalImpl.h>
#include <C3MatrixStackImpl.h>

using namespace c3;


PhysicsManagerImpl::PhysicsManagerImpl(System *psys) : m_pSys(psys)
{
	m_World = nullptr;
	m_Space = nullptr;
	m_Joints = nullptr;

	//DefaultThreadingHolder::initializeDefaultThreading();
}


PhysicsManagerImpl::~PhysicsManagerImpl()
{
	Shutdown();

	//DefaultThreadingHolder::finalizeDefaultThreading();
}


bool PhysicsManagerImpl::Initialize()
{
	bool ret = true;

#if defined(ODE_SUPPORT) && ODE_SUPPORT
	dInitODE2(0);

	m_World = dWorldCreate();
	m_Space = dHashSpaceCreate(0);
	m_Joints = dJointGroupCreate(4096);

	dWorldSetStepThreadingImplementation(m_World, nullptr, nullptr);

	dWorldSetERP(m_World, 0.1f);

	dWorldSetCFM(m_World, 1e-5f);

	dWorldSetContactMaxCorrectingVel(m_World, 0.7f);

	dWorldSetContactSurfaceLayer(m_World, 0.001f);

	dWorldSetAutoDisableFlag(m_World, 1);
#endif

	return ret;
}


void PhysicsManagerImpl::Shutdown()
{
#if defined(ODE_SUPPORT) && ODE_SUPPORT
	if (m_Joints)
	{
		dJointGroupDestroy(m_Joints);
		m_Joints = nullptr;
	}

	if (m_Space)
	{
		dSpaceDestroy(m_Space);
		m_Space = nullptr;
	}

	if (m_World)
	{
		dWorldDestroy(m_World);
		m_World = nullptr;
	}

	dCloseODE();
#endif
}


void PhysicsManagerImpl::Update(float time)
{
#if defined(ODE_SUPPORT) && ODE_SUPPORT
	// if there's no world, time is paused, or there are no dynamic objects, don't bother with this...
	if ((time == 0.0f) || !m_World || m_Dynamics.empty())
		return;

	Environment *penv = m_pSys->GetEnvironment();
	glm::fvec3 grav;
	penv->GetGravity(&grav);
	dWorldSetGravity(m_World, grav.x, grav.y, grav.z);

	dSpaceCollide(m_Space, this,
		[](void *data, dGeomID o1, dGeomID o2)
		{
			PhysicsManagerImpl *_this = (PhysicsManagerImpl *)data;

			// Create a contact joint if a collision is detected
			dContact contact;
			contact.surface.mode = dContactBounce;
			contact.surface.bounce = 0.2f;    // Example bounce parameter
			contact.surface.bounce_vel = 0.1f;
			contact.surface.mu = dInfinity; // Friction coefficient

			if (dCollide(o1, o2, 1, &contact.geom, sizeof(dContact)))
			{
				dJointID contactJoint = dJointCreateContact(_this->m_World, _this->m_Joints, &contact);
				dJointAttach(contactJoint, dGeomGetBody(o1), dGeomGetBody(o2));
			}
		}
	);

	dWorldQuickStep(m_World, time);

	dJointGroupEmpty(m_Joints);

	for (auto o : m_Dynamics)
	{
		Physical *pphys = (Physical *)(o.first->FindComponent(Physical::Type()));
		if (pphys && (pphys->GetCollisionMode() == Physical::CollisionMode::DYNAMIC))
		{
			Positionable *ppos = (Positionable *)(o.first->FindComponent(Positionable::Type()));

			const dReal *_pf = dBodyGetPosition(o.second);
			ppos->SetPosVec((glm::fvec3 *)_pf);

			const dReal *_of = dBodyGetQuaternion(o.second);
			ppos->SetOriQuat((glm::fquat *)_of);
		}
	}
#endif
}


bool PhysicsManagerImpl::CreateCollisionMesh(const Model *pmod, dTriMeshDataID &ret, const glm::fmat4x4 *sclmat)
{
	if (!pmod)
		return false;

#if defined(ODE_SUPPORT) && ODE_SUPPORT
	// permanently allocate some space for vertices
	static std::vector<float> verts;
	// permanently allocate some space for indices
	static std::vector<uint32_t> indices;

	PhysMeshMap::iterator it = m_Meshes.find(pmod);
	if (it == m_Meshes.end())
	{
		if (verts.size() == 0)
			verts.resize(1 << 20);

		if (indices.size() == 0)
			indices.resize(1 << 20);

		verts.clear();
		indices.clear();

		size_t idxofs = 0;

		IndexBuffer *ib = nullptr;
		VertexBuffer *vb = nullptr;

		for (Model::NodeIndex i = 0, maxi = pmod->GetNodeCount(); i < maxi; i++)
		{
			if (!((Model *)pmod)->NodeCollidability(i))
				continue;

			MatrixStackImpl ms;
			std::function<void(Model::SubMeshIndex)> CalcNodeMatrix = [&](Model::SubMeshIndex midx)
			{
				Model::SubMeshIndex pidx = pmod->GetParentNode(midx);
				if (pidx != Model::NO_PARENT)
					CalcNodeMatrix(pidx);
				ms.Push(pmod->GetTransform(midx));
			};
			CalcNodeMatrix(i);
			if (sclmat)
				ms.Push(sclmat);

			size_t mc = pmod->GetMeshCountOnNode(i);
			for (Model::SubMeshIndex j = 0; j < mc; j++)
			{
				const Mesh *pm = pmod->GetMesh(pmod->GetMeshFromNode(i, j));

				ib = pm->GetIndexBuffer();
				vb = pm->GetVertexBuffer();

				void *vbdata = nullptr;
				vb->Lock(&vbdata, 0, nullptr, VBLOCKFLAG_READ);
				size_t vbstride = vb->VertexSize();

				void *ibdata = nullptr;
				ib->Lock(&ibdata, 0, IndexBuffer::IndexSize::IS_NONE, IBLOCKFLAG_READ);
				size_t ibstride = ib->GetIndexSize();

				float *v = verts.data();

				// Append vertex positions (world-space)
				for (size_t vi = 0, maxvi = vb->Count(); vi < maxvi; vi++)
				{
					glm::fvec3 tv = glm::fvec4(*((glm::fvec3 *)vbdata), 0) * *(ms.Top());
					memcpy(v, &tv, sizeof(float) * 3);

					vbdata = (uint8_t *)vbdata + vbstride;
					v += 3;
				}

				uint32_t *i = indices.data();

				// Append indices (offset by current vertex count)
				switch (ib->GetIndexSize())
				{
					case IndexBuffer::IS_8BIT:
					{
						uint8_t *pi8 = (uint8_t *)ibdata;
						for (size_t ii = 0, maxii = ib->Count(); ii < maxii; ii++, pi8++, i++)
						{
							*i = (uint32_t)*pi8;
						}
						break;
					}

					case IndexBuffer::IS_16BIT:
					{
						uint16_t *pi16 = (uint16_t *)ibdata;
						for (size_t ii = 0, maxii = ib->Count(); ii < maxii; ii++, pi16++, i++)
						{
							*i = (uint32_t)*pi16;
						}
						break;
					}

					case IndexBuffer::IS_32BIT:
					{
						uint32_t *pi32 = (uint32_t *)ibdata;
						for (size_t ii = 0, maxii = ib->Count(); ii < maxii; ii++, pi32++, i++)
						{
							*i = (uint32_t)*pi32;
						}
						break;
					}
				}

				idxofs += vb->Count();

				vb->Unlock();
				ib->Unlock();
			}
		}

		assert(vb && ib);

		ret = dGeomTriMeshDataCreate();
		dGeomTriMeshDataBuildSimple(ret, verts.data(), (int)vb->Count(), indices.data(), (int)ib->Count());

		m_Meshes.insert(PhysMeshMap::value_type(pmod, ret));
		return true;
	}

	ret = it->second;
#endif
	return true;
}


bool PhysicsManagerImpl::AddObject(const Object *pobj)
{
#if defined(ODE_SUPPORT) && ODE_SUPPORT
	DynamicObjectMap::iterator it = m_Dynamics.find(pobj);
	if (it == m_Dynamics.end())
	{
		PhysicalImpl *pphys = (PhysicalImpl *)(pobj->FindComponent(Physical::Type()));

		dBodyID b = (pphys->GetCollisionMode() == Physical::CollisionMode::DYNAMIC) ? dBodyCreate(m_World) : nullptr;

		if (b)
		{
			pphys->u_ODEBody = b;

			dBodySetData(b, (void *)pobj);
		}

		m_Dynamics.insert(DynamicObjectMap::value_type(pobj, b));


		dGeomID gid = nullptr;

		glm::fvec3 p(0, 0, 0);
		glm::fquat o(0, 0, 1, 0);
		glm::fvec3 s(1, 1, 1);
		Positionable *ppos = dynamic_cast<Positionable *>(pobj->FindComponent(Positionable::Type()));
		if (ppos)
		{
			ppos->GetPosVec(&p);
			ppos->GetOriQuat(&o);
			ppos->GetScl(&s);
		}

		MatrixStackImpl ms;
		std::function<void(const Object *)> CalcObjMatrix = [&](const Object *pobj)
		{
			const Object *ppar = pobj->GetParent();
			if (ppar)
				CalcObjMatrix(ppar);
			Positionable *plpos = (Positionable *)pobj->FindComponent(Positionable::Type());
			if (plpos)
			{
				glm::fmat4x4 m = glm::scale(*ppos->GetScl());
				ms.Push(&m);
			}
		};
		CalcObjMatrix(pobj);

		if (pphys)
		{

			switch (pphys->GetColliderShape())
			{
				case Physical::ColliderShape::NONE:
					break;

				case Physical::ColliderShape::MODELBOUNDS:
				{
#if 0
					ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(pobj->FindComponent(ModelRenderer::Type()));
					if (pmr)
					{
						dTriMeshDataID tmd;
						if (CreateCollisionMesh(pmr->GetModel(), tmd, ms.Top()))
						{
							gid = dCreateTriMesh(m_Space, tmd, nullptr, nullptr, nullptr);
							dMassSetTrimesh(&pphys->m_ODEMass, 1.0f, gid);
						}
					}
#endif
					break;
				}

				case Physical::ColliderShape::MODEL:
				{
					ModelRenderer *pmr = dynamic_cast<ModelRenderer *>(pobj->FindComponent(ModelRenderer::Type()));
					if (pmr)
					{
						dTriMeshDataID tmd;
						if (CreateCollisionMesh(pmr->GetModel(), tmd, ms.Top()))
						{
							gid = dCreateTriMesh(m_Space, tmd, nullptr, nullptr, nullptr);
							dMassSetTrimesh(&pphys->m_ODEMass, 1.0f, gid);
						}
					}
					break;
				}

				case Physical::ColliderShape::SPHERE:
					if (ppos)
					{
						float r = (float)(s.length());
						gid = dCreateSphere(m_Space, r);
						dMassSetSphere(&pphys->m_ODEMass, 1.0f, r);
					}
					break;

				case Physical::ColliderShape::CYLINDER:
					if (ppos)
					{
						float r = (float)(((glm::fvec2)s).length());
						gid = dCreateCylinder(m_Space, r, s.z);
						dMassSetCylinder(&pphys->m_ODEMass, 1.0f, 3 /* z, according to the manual */, r, s.z);
					}
					break;

				case Physical::ColliderShape::CAPSULE:
					if (ppos)
					{
						float r = (float)(((glm::fvec2)s).length());
						gid = dCreateCapsule(m_Space, r, s.z);
						dMassSetCapsule(&pphys->m_ODEMass, 1.0f, 3 /* z, according to the manual */, r, s.z);
					}
					break;
			}
		}

		if (!gid)
			gid = dCreateBox(m_Space, 1.0f, 1.0f, 1.0f);

		if (b)
		{
			// dynamic or kinematic

			if (pphys->GetCollisionMode() == Physical::CollisionMode::DYNAMIC)
				dBodySetDynamic(b);
			else
				dBodySetKinematic(b);

			dGeomSetBody(gid, b);

			dBodySetPosition(b, p.x, p.y, p.z);
			dBodySetQuaternion(b, (const dReal *)&o);
		}
		else
		{
			// static

			dGeomSetPosition(gid, p.x, p.y, p.z);
			dGeomSetQuaternion(gid, *((dQuaternion *)&o));
		}

		return true;
	}
#endif

	return false;
}


bool PhysicsManagerImpl::RemoveObject(const Object *pobj)
{
#if defined(ODE_SUPPORT) && ODE_SUPPORT
	DynamicObjectMap::iterator it = m_Dynamics.find(pobj);
	if (it != m_Dynamics.end())
	{
		// don't try to destroy static objects
		if (it->second)
			dBodyDestroy(it->second);

		m_Dynamics.erase(it);

		return true;
	}
#endif

	return false;
}


bool PhysicsManagerImpl::MakeObjectKinetic(const Object *pobj, bool kinematic)
{
#if defined(ODE_SUPPORT) && ODE_SUPPORT
	DynamicObjectMap::iterator it = m_Dynamics.find(pobj);
	if (it != m_Dynamics.end())
	{
		if (kinematic && !dBodyIsKinematic(it->second))
		{
			dBodySetKinematic(it->second);
			return true;
		}
		else if (!kinematic && dBodyIsKinematic(it->second))
		{
			dBodySetDynamic(it->second);
			return true;
		}
	}
#endif

	return false;
}


void PhysicsManagerImpl::UpdateObject(const Object *pobj)
{
#if defined(ODE_SUPPORT) && ODE_SUPPORT
	PhysicalImpl *pphys = (PhysicalImpl *)pobj->FindComponent(Physical::Type());
	if (!pphys)
		return;

	Positionable *ppos = dynamic_cast<Positionable *>(pobj->FindComponent(Positionable::Type()));
	if (!ppos)
		return;

	glm::fvec3 p(0, 0, 0);
	ppos->GetPosVec(&p);

	glm::fquat o(0, 0, 1, 0);
	ppos->GetOriQuat(&o);

	glm::fvec3 s(1, 1, 1);
	ppos->GetScl(&s);

	switch (pphys->GetCollisionMode())
	{
		case Physical::CollisionMode::KINEMATIC:
		case Physical::CollisionMode::DYNAMIC:
#if defined(USE_PHYSICS_MANAGER) && USE_PHYSICS_MANAGER
			dBodySetPosition(pphys->u_ODEBody, p.x, p.y, p.z);
			dBodySetQuaternion(pphys->u_ODEBody, (const dReal *)&o);
#endif
			break;
		case Physical::CollisionMode::STATIC:
#if defined(USE_PHYSICS_MANAGER) && USE_PHYSICS_MANAGER
			dGeomSetPosition(pphys->u_ODEGeom, p.x, p.y, p.z);
			dGeomSetQuaternion(pphys->u_ODEGeom, (const dReal *)&o);
#endif
			break;
	}
#endif
}
