// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3BoundingBoxImpl.h>

using namespace c3;

uint16_t BoundingBoxImpl::m_Edge[EDGE::NUMEDGES][2] = 
{
	// x to X edges
	/* [BBEDGE_xyz_Xyz][0] */
	{CORNER::xyz, CORNER::Xyz},

	/* [BBEDGE_xYz_XYz][0] */
	{CORNER::xYz, CORNER::XYz},

	/* [BBEDGE_xyZ_XyZ][0] */
	{CORNER::xyZ, CORNER::XyZ},

	/* [BBEDGE_xYZ_XYZ][0] */
	{CORNER::xYZ, CORNER::XYZ},

	// y to Y edges
	/* BBEDGE_xyz_xYz */
	{CORNER::xyz, CORNER::xYz},

	/* BBEDGE_Xyz_XYz */
	{CORNER::Xyz, CORNER::XYz},

	/* BBEDGE_xyZ_xYZ */
	{CORNER::xyZ, CORNER::xYZ},

	/* BBEDGE_XyZ_XYZ */
	{CORNER::XyZ, CORNER::XYZ},

	// z to Z edges

	/* BBEDGE_xyz_xyZ */
	{CORNER::xyz, CORNER::xyZ},

	/* BBEDGE_Xyz_XyZ */
	{CORNER::Xyz, CORNER::XyZ},

	/* BBEDGE_xYz_xYZ */
	{CORNER::xYz, CORNER::xYZ},

	/* BBEDGE_XYz_XYZ */
	{CORNER::XYz, CORNER::XYZ}
};


void PlaneFromPointNormal(const glm::fvec3 &point, const glm::fvec3 &normal, glm::fvec4 &out_plane)
{
	out_plane.x = normal.x;
	out_plane.y = normal.y;
	out_plane.z = normal.z;
	out_plane.w = -glm::dot(point, normal);
}


void PlaneFromPoints(const glm::fvec3 &p1, const glm::fvec3 &p2, const glm::fvec3 &p3, glm::fvec4 &out_plane)
{
	glm::fvec3 edge1, edge2, normal;

	edge1 = p2 - p1;
	edge2 = p3 - p1;
	normal = glm::cross(edge1, edge2);

	PlaneFromPointNormal(p1, glm::normalize(normal), out_plane);
}


float PlaneDotCoord(const glm::fvec4* pplane, const glm::fvec3* ppt)
{
	assert(pplane && ppt);

	return ((pplane->x * ppt->x) + (pplane->y * ppt->y) + (pplane->z * ppt->z) + pplane->w);
}


bool PlaneIntersectLine(const glm::fvec4 &plane, const glm::fvec3 &lp1, const glm::fvec3 &lp2, glm::fvec3 *collision_point)
{
	glm::fvec3 pn(plane.x, plane.y, plane.z);
	glm::fvec3 dir = lp2 - lp1;

	float d = glm::dot(pn, dir); // ??? seems like dir should be normalized

	if (d == 0.0f)
		return false;

	if (collision_point)
	{
		float temp = (plane.w + glm::dot(pn, lp1)) / d;
		*collision_point = lp1 - (temp * dir);
	}

	return true;
}


const uint16_t *BoundingBoxImpl::GetIndices() const
{
	return (const uint16_t *)m_Edge;
}

const glm::fvec3 *BoundingBoxImpl::GetCorners() const
{
	return m_Corner;
}


BoundingBox* BoundingBox::Create()
{
	return (BoundingBox *)(new BoundingBoxImpl());
}


BoundingBoxImpl::BoundingBoxImpl()
{
	memset(m_Face, 0, sizeof(glm::fvec4) * FACE::NUMFACES);
	memset(m_Corner, 0, sizeof(glm::fvec3) * CORNER::NUMCORNERS);

	memset(m_vRange_Unaligned, 0, sizeof(glm::fvec3) * AXES::NUMAXES);
	memset(m_vRange_Aligned, 0, sizeof(glm::fvec3) * AXES::NUMAXES);

	memset(&m_vOrg_Aligned, 0, sizeof(glm::fvec3));
	memset(&m_vOrg_Unaligned, 0, sizeof(glm::fvec3));

	m_bNeedsAlignment = true;
}


BoundingBoxImpl::~BoundingBoxImpl()
{
}


void BoundingBoxImpl::Release()
{
	delete this;
}


void BoundingBoxImpl::SetFromBounds(const BoundingBox *box)
{
	SetOrigin(box->GetOrigin());
	SetExtents(box->GetExtents());

	m_bNeedsAlignment = true;
}


void BoundingBoxImpl::SetOrigin(float x, float y, float z)
{
	m_vOrg_Unaligned.x = x;
	m_vOrg_Unaligned.y = y;
	m_vOrg_Unaligned.z = z;

	m_bNeedsAlignment = true;
}


void BoundingBoxImpl::SetOrigin(const glm::fvec3 *org)
{
	SetOrigin(org->x, org->y, org->z);
}


const glm::fvec3 *BoundingBoxImpl::GetOrigin(glm::fvec3 *org) const
{
	if (org)
	{
		*org = m_vOrg_Unaligned;
		return org;
	}

	return &m_vOrg_Unaligned;
}


void BoundingBoxImpl::SetExtents(float x, float y, float z)
{
	ZeroMemory(m_vRange_Unaligned, sizeof(glm::fvec3) * AXES::NUMAXES);

	m_vRange_Unaligned[AXES::X].x = x;
	m_vRange_Unaligned[AXES::Y].y = y;
	m_vRange_Unaligned[AXES::Z].z = z;

	m_bNeedsAlignment = true;
}


void BoundingBoxImpl::SetExtents(const glm::fvec3 *ext)
{
	SetExtents(ext->x, ext->y, ext->z);
}


const glm::fvec3 *BoundingBoxImpl::GetExtents(glm::fvec3 *ext) const
{
	if (ext)
	{
		ext->x = m_vRange_Unaligned[AXES::X].x;
		ext->y = m_vRange_Unaligned[AXES::Y].y;
		ext->z = m_vRange_Unaligned[AXES::Z].z;
	}

	return ext;
}


void BoundingBoxImpl::SetAsFrustum(const glm::fmat4x4 *viewmat, const glm::fmat4x4 *projmat, const RECT *viewport)
{
	m_Corner[CORNER::xyz] = glm::fvec3(0.0f, 0.0f, 0.0f);	// xyz
	m_Corner[CORNER::Xyz] = glm::fvec3(1.0f, 0.0f, 0.0f);	// Xyz
	m_Corner[CORNER::xYz] = glm::fvec3(0.0f, 1.0f, 0.0f);	// xYz
	m_Corner[CORNER::XYz] = glm::fvec3(1.0f, 1.0f, 0.0f);	// XYz
	m_Corner[CORNER::xyZ] = glm::fvec3(0.0f, 0.0f, 1.0f);	// xyZ
	m_Corner[CORNER::XyZ] = glm::fvec3(1.0f, 0.0f, 1.0f);	// XyZ
	m_Corner[CORNER::xYZ] = glm::fvec3(0.0f, 1.0f, 1.0f);	// xYZ
	m_Corner[CORNER::XYZ] = glm::fvec3(1.0f, 1.0f, 1.0f);	// XYZ

	glm::fmat4x4 identity;
	glm::uvec4 v(viewport->left, viewport->top, viewport->right - viewport->left, viewport->bottom);
	for (size_t i = 0; i < CORNER::NUMCORNERS; i++)
	{
		m_Corner[i] = glm::unProject(m_Corner[i], *viewmat, *projmat, v);
	}

	PlaneFromPoints(m_Corner[CORNER::xyz], m_Corner[CORNER::xYz], m_Corner[CORNER::Xyz], m_Face[FACE::MINZ]);		// Near
	PlaneFromPoints(m_Corner[CORNER::xyZ], m_Corner[CORNER::xYZ], m_Corner[CORNER::XyZ], m_Face[FACE::MAXZ]);		// Far
	PlaneFromPoints(m_Corner[CORNER::xyz], m_Corner[CORNER::xyZ], m_Corner[CORNER::xYz], m_Face[FACE::MINX]);		// Left
	PlaneFromPoints(m_Corner[CORNER::Xyz], m_Corner[CORNER::XyZ], m_Corner[CORNER::XYz], m_Face[FACE::MAXX]);		// Right
	PlaneFromPoints(m_Corner[CORNER::xYz], m_Corner[CORNER::xYZ], m_Corner[CORNER::XYz], m_Face[FACE::MAXY]);		// Top
	PlaneFromPoints(m_Corner[CORNER::xyz], m_Corner[CORNER::xyZ], m_Corner[CORNER::XyZ], m_Face[FACE::MINY]);		// Bottom

	m_bNeedsAlignment = false;
}


void BoundingBoxImpl::Align(const glm::fmat4x4 *matrix)
{
	glm::fmat4x4 dummymat;
	if (!matrix)
	{
		matrix = &dummymat;
	}

	m_vOrg_Aligned = *matrix * glm::fvec4(m_vOrg_Unaligned, 1.0f);
	for (size_t i = 0; i < AXES::NUMAXES; i++)
		m_vRange_Aligned[i] = *matrix * glm::fvec4(m_vRange_Unaligned[i], 0.0f);

	m_Corner[CORNER::xyz] = m_vOrg_Aligned;
	m_Corner[CORNER::xyZ] = m_vOrg_Aligned + m_vRange_Aligned[AXES::Z];
	m_Corner[CORNER::xYz] = m_vOrg_Aligned + m_vRange_Aligned[AXES::Y];
	m_Corner[CORNER::xYZ] = m_vOrg_Aligned + m_vRange_Aligned[AXES::Y] + m_vRange_Aligned[AXES::Z];
	m_Corner[CORNER::Xyz] = m_vOrg_Aligned + m_vRange_Aligned[AXES::X];
	m_Corner[CORNER::XyZ] = m_vOrg_Aligned + m_vRange_Aligned[AXES::X] + m_vRange_Aligned[AXES::Z];
	m_Corner[CORNER::XYz] = m_vOrg_Aligned + m_vRange_Aligned[AXES::X] + m_vRange_Aligned[AXES::Y];
	m_Corner[CORNER::XYZ] = m_vOrg_Aligned + m_vRange_Aligned[AXES::X] + m_vRange_Aligned[AXES::Y] + m_vRange_Aligned[AXES::Z];

	PlaneFromPoints(m_Corner[CORNER::xyz], m_Corner[CORNER::Xyz], m_Corner[CORNER::xYz],	m_Face[FACE::MINZ]);		// Near
	PlaneFromPoints(m_Corner[CORNER::xyZ], m_Corner[CORNER::xYZ], m_Corner[CORNER::XyZ],	m_Face[FACE::MAXZ]);		// Far

	PlaneFromPoints(m_Corner[CORNER::xyz], m_Corner[CORNER::xYz], m_Corner[CORNER::xyZ],	m_Face[FACE::MINX]);		// Left
	PlaneFromPoints(m_Corner[CORNER::Xyz], m_Corner[CORNER::XyZ], m_Corner[CORNER::XYz],	m_Face[FACE::MAXX]);		// Right

	PlaneFromPoints(m_Corner[CORNER::xYz], m_Corner[CORNER::XYz], m_Corner[CORNER::xYZ],	m_Face[FACE::MAXY]);		// Top
	PlaneFromPoints(m_Corner[CORNER::xyz], m_Corner[CORNER::xyZ], m_Corner[CORNER::Xyz],	m_Face[FACE::MINY]);		// Bottom

	m_bNeedsAlignment = false;
}


bool BoundingBoxImpl::IsPointInside(const glm::fvec3 *ppt)
{
	if (m_bNeedsAlignment)
		Align(nullptr);

	// Go through each plane and see if the point is inside it
	for (size_t i = 0; i < FACE::NUMFACES; i++)
	{
		if (PlaneDotCoord(&m_Face[i], ppt) < 0)
			return false;
	}

	// If any point is inside all 6 frustum planes, it is inside the box
	return true;
}


bool BoundingBoxImpl::IsBoxInside(const BoundingBox *pbox)
{
	BoundingBoxImpl *pboxi = (BoundingBoxImpl *)pbox;
	if (m_bNeedsAlignment)
		Align(nullptr);
	if (pboxi->m_bNeedsAlignment)
		Align(nullptr);

	// Have a counter for each corner of the box
	uint32_t bOutside[CORNER::NUMCORNERS] = {0, 0, 0, 0, 0, 0, 0, 0};

	uint32_t j;

	// Check boundary vertices against all 6 box planes, 
	// and store result (1 if outside) in a bitfield
	for (j = 0; j < CORNER::NUMCORNERS; j++)
	{
		for (uint32_t i = 0; i < FACE::NUMFACES; i++)
		{
			if (PlaneDotCoord(&m_Face[i], &pboxi->m_Corner[j]) < 0)
				bOutside[j] |= (1 << i);
		}

		// If any point is inside all 6 planes, it is inside the box
		if (!bOutside[j])
			return true;
	}

	// If all points are outside any single plane, the box isn't in us
	if ((bOutside[0] & bOutside[1] & bOutside[2] & bOutside[3] & 
		bOutside[4] & bOutside[5] & bOutside[6] & bOutside[7]) != 0)
	{
		return false;
	}

	glm::fvec3 dummy;

	// Do any of the other edges penetrate the faces of the our box
	for (j = 0; j < EDGE::NUMEDGES; j++)
	{
		for (uint32_t i = 0; i < FACE::NUMFACES; i++ )
		{
			if (PlaneIntersectLine(m_Face[i], pboxi->m_Corner[pboxi->m_Edge[j][0]], pboxi->m_Corner[pboxi->m_Edge[j][1]], &dummy))
				return true;
		}
	}

	// Do any of our edges penetrate the faces of the other box?
	for (j = 0; j < EDGE::NUMEDGES; j++)
	{
		for (uint32_t i = 0; i < FACE::NUMFACES; i++)
		{
			if (PlaneIntersectLine(pboxi->m_Face[i], m_Corner[m_Edge[j][0]], m_Corner[m_Edge[j][1]], &dummy))
				return true;
		}
	}

	// Now see if we are contained in the other box
	// If any m_Corner point is outside any plane of the other box,
	// we are not contained in the other box
	for (uint32_t j = 0; j < CORNER::NUMCORNERS; j++)
	{
		for (uint32_t i = 0; i < FACE::NUMFACES; i++)
		{
			if (PlaneDotCoord(&pboxi->m_Face[i], &m_Corner[j]) < 0)
				return false;
		}
	}

	return true;
}


bool BoundingBoxImpl::IsSphereInside(const glm::fvec3 *centroid, float radius)
{
	if (m_bNeedsAlignment)
		Align(nullptr);

	return false;
}

// the only way this should be able to fail is if the line is directly on one of the planes without
// having either the start or end point on that plane... I'm ok with that for right now
bool BoundingBoxImpl::CheckCollision(const glm::fvec3 *raypos, const glm::fvec3 *rayvec, float *dist)
{
	if (m_bNeedsAlignment)
	{
		Align(NULL);
	}

	glm::fvec3 tmp = *raypos;
	tmp += *rayvec;
	if (IsPointInside(raypos) || IsPointInside(&tmp))
		return true;

	glm::fvec3 pt;
	if (dist)
		*dist = FLT_MAX;
	bool ret = false;
	for (uint32_t i = 0; i < FACE::NUMFACES; i++)
	{
		if (PlaneIntersectLine(m_Face[i], *raypos, *rayvec, &pt))
		{
			if (dist)
			{
				pt -= *raypos;
				float tmpdist = glm::length(pt);
				if (tmpdist < *dist)
					*dist = tmpdist;
			}

			ret = true;
		}
	}

	return ret;
}

void BoundingBoxImpl::IncludeBounds(const BoundingBox *pbox)
{
	BoundingBoxImpl *pb = (BoundingBoxImpl *)pbox;

	glm::fvec3 low(FLT_MAX, FLT_MAX, FLT_MAX), high(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	if (m_vOrg_Unaligned.x < low.x) low.x = m_vOrg_Unaligned.x;
	if (m_vOrg_Unaligned.y < low.y) low.y = m_vOrg_Unaligned.y;
	if (m_vOrg_Unaligned.z < low.z) low.z = m_vOrg_Unaligned.z;

	if ((m_vRange_Unaligned[AXES::X].x + m_vOrg_Unaligned.x) > high.x) high.x = m_vRange_Unaligned[AXES::X].x + m_vOrg_Unaligned.x;
	if ((m_vRange_Unaligned[AXES::Y].y + m_vOrg_Unaligned.y) > high.y) high.y = m_vRange_Unaligned[AXES::Y].y + m_vOrg_Unaligned.y;
	if ((m_vRange_Unaligned[AXES::Z].z + m_vOrg_Unaligned.z) > high.z) high.z = m_vRange_Unaligned[AXES::Z].z + m_vOrg_Unaligned.z;

	for (uint32_t i = CORNER::xyz; i < CORNER::NUMCORNERS; i++)
	{
		if (pb->GetCorners()[i].x < low.x) low.x = pb->GetCorners()[i].x;
		if (pb->GetCorners()[i].y < low.y) low.y = pb->GetCorners()[i].y;
		if (pb->GetCorners()[i].z < low.z) low.z = pb->GetCorners()[i].z;

		if (pb->GetCorners()[i].x > high.x) high.x = pb->GetCorners()[i].x;
		if (pb->GetCorners()[i].y > high.y) high.y = pb->GetCorners()[i].y;
		if (pb->GetCorners()[i].z > high.z) high.z = pb->GetCorners()[i].z;
	}

	SetOrigin(&low);
	SetExtents(high.x - low.x, high.y - low.y, high.z - low.z);

	m_bNeedsAlignment = true;
}


const glm::fvec4 *BoundingBoxImpl::GetFace(FACE::eFACE f) const
{
	return &m_Face[f];
}
