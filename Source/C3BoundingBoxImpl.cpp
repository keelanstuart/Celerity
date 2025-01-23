// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3BoundingBoxImpl.h>

using namespace c3;

uint16_t BoundingBoxImpl::s_Edge[EDGE::NUMEDGES][2] = 
{
	// x to X edges
	/* BBEDGE_xyz_Xyz */
	{CORNER::xyz, CORNER::Xyz},

	/* BBEDGE_xYz_XYz */
	{CORNER::XYz, CORNER::xYz},

	/* BBEDGE_xyZ_XyZ */
	{CORNER::xyZ, CORNER::XyZ},

	/* BBEDGE_xYZ_XYZ */
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


void PlaneFromPoints(const glm::fvec3 &p1, const glm::fvec3 &p2, const glm::fvec3 &p3, BoundingBoxImpl::Plane &out_plane)
{
	out_plane.point = (p1 + p2 + p3) / 3.0f;

	glm::fvec3 a = glm::normalize(p2 - p1);
	glm::fvec3 b = glm::normalize(p3 - p1);
	out_plane.normal = glm::cross(a, b);
}


float PlaneDotCoord(const BoundingBoxImpl::Plane &plane, const glm::fvec3 &point)
{
	glm::fvec3 v = point - plane.point;
	glm::fvec3 n = glm::normalize(v);
	float d = glm::dot(n, plane.normal);
	return d;
}


const uint16_t *BoundingBoxImpl::GetIndices() const
{
	return (const uint16_t *)s_Edge;
}


const glm::fvec3 *BoundingBoxImpl::GetCorners() const
{
	return m_Corner_Unaligned;
}


const glm::fvec3 *BoundingBoxImpl::GetAlignedCorners() const
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

	memset(m_Corner_Unaligned, 0, sizeof(glm::fvec3) * CORNER::NUMCORNERS);
	memset(m_Corner, 0, sizeof(glm::fvec3) * CORNER::NUMCORNERS);

	memset(m_EdgeLen, 0, sizeof(float) * EDGE::NUMEDGES);
	memset(m_EdgeDir, 0, sizeof(glm::fvec3) * EDGE::NUMEDGES);

	m_bNeedsAlignment = true;
}


BoundingBoxImpl::BoundingBoxImpl(const BoundingBox *other)
{
	assert(other);

	memcpy(&m_Corner_Unaligned, &((const BoundingBoxImpl *)other)->m_Corner_Unaligned, sizeof(glm::fvec3) * CORNER::NUMCORNERS);
	memcpy(&m_Corner, ((const BoundingBoxImpl *)other)->m_Corner, sizeof(glm::fvec3) * CORNER::NUMCORNERS);

	m_bNeedsAlignment = true;
}

BoundingBoxImpl & BoundingBoxImpl::operator =(const BoundingBoxImpl &other)
{
	memcpy(&m_Corner_Unaligned, other.m_Corner_Unaligned, sizeof(glm::fvec3) * CORNER::NUMCORNERS);
	memcpy(&m_Corner, other.m_Corner, sizeof(glm::fvec3) * CORNER::NUMCORNERS);

	m_bNeedsAlignment = true;
	return *this;
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
	if (!box)
		return;

	for (size_t i = 0; i < CORNER::NUMCORNERS; i++)
		m_Corner_Unaligned[i] = ((BoundingBoxImpl *)box)->m_Corner_Unaligned[i];

	m_bNeedsAlignment = true;
}


void BoundingBoxImpl::SetExtents(const glm::fvec3 *min_bounds, const glm::fvec3 *max_bounds)
{
	m_Corner_Unaligned[CORNER::xyz] = *min_bounds;
	m_Corner_Unaligned[CORNER::Xyz] = glm::fvec3(max_bounds->x, min_bounds->y, min_bounds->z);
	m_Corner_Unaligned[CORNER::xYz] = glm::fvec3(min_bounds->x, max_bounds->y, min_bounds->z);
	m_Corner_Unaligned[CORNER::XYz] = glm::fvec3(max_bounds->x, max_bounds->y, min_bounds->z);
	m_Corner_Unaligned[CORNER::xyZ] = glm::fvec3(min_bounds->x, min_bounds->y, max_bounds->z);
	m_Corner_Unaligned[CORNER::XyZ] = glm::fvec3(max_bounds->x, min_bounds->y, max_bounds->z);
	m_Corner_Unaligned[CORNER::xYZ] = glm::fvec3(min_bounds->x, max_bounds->y, max_bounds->z);
	m_Corner_Unaligned[CORNER::XYZ] = *max_bounds;

	m_bNeedsAlignment = true;
}


void BoundingBoxImpl::SetAsFrustum(const glm::fmat4x4 *viewmat, const glm::fmat4x4 *projmat, const RECT *viewport)
{
	float x1 = (float)viewport->left;
	float y1 = (float)viewport->top;
	float x2 = (float)viewport->right;
	float y2 = (float)viewport->bottom;
	float mx = (x2 - x1) / 2.0f;
	float my = (y2 - y1) / 2.0f;

	glm::uvec4 v(viewport->left, viewport->top, viewport->right, viewport->bottom);

	glm::fvec3 cn = glm::unProject(glm::fvec3(mx, my, 0), *viewmat, *projmat, v);
	glm::fvec3 cf = glm::unProject(glm::fvec3(mx, my, 1), *viewmat, *projmat, v);
	glm::fvec3 ln = glm::unProject(glm::fvec3(x1, my, 0), *viewmat, *projmat, v);
	glm::fvec3 lf = glm::unProject(glm::fvec3(x1, my, 1), *viewmat, *projmat, v);
	glm::fvec3 rn = glm::unProject(glm::fvec3(x2, my, 0), *viewmat, *projmat, v);
	glm::fvec3 rf = glm::unProject(glm::fvec3(x2, my, 1), *viewmat, *projmat, v);
	glm::fvec3 tn = glm::unProject(glm::fvec3(mx, y2, 0), *viewmat, *projmat, v);
	glm::fvec3 tf = glm::unProject(glm::fvec3(mx, y2, 1), *viewmat, *projmat, v);
	glm::fvec3 bn = glm::unProject(glm::fvec3(mx, y1, 0), *viewmat, *projmat, v);
	glm::fvec3 bf = glm::unProject(glm::fvec3(mx, y1, 1), *viewmat, *projmat, v);

	glm::fvec3 lno = cn - ln; // left near offset
	glm::fvec3 lfo = cf - lf; // left far offset

	glm::fvec3 rno = cn - rn; // right near offset
	glm::fvec3 rfo = cf - rf; // right far offset

	glm::fvec3 tno = tn - cn; // top near offset
	glm::fvec3 tfo = tf - cf; // top far offset

	glm::fvec3 bno = bn - cn; // bottom near offset
	glm::fvec3 bfo = bf - cf; // bottom far offset

	m_Corner[CORNER::xyz] = cn + lno + bno; 	// xyz
	m_Corner[CORNER::Xyz] = cn + rno + bno;		// Xyz
	m_Corner[CORNER::xYz] = cn + lno + tno;		// xYz
	m_Corner[CORNER::XYz] = cn + rno + tno;		// XYz
	m_Corner[CORNER::xyZ] = cf + lfo + bfo;		// xyZ
	m_Corner[CORNER::XyZ] = cf + rfo + bfo;		// XyZ
	m_Corner[CORNER::xYZ] = cf + lfo + tfo;		// xYZ
	m_Corner[CORNER::XYZ] = cf + rfo + tfo;		// XYZ

	PlaneFromPoints(m_Corner[CORNER::xyz], m_Corner[CORNER::xYz], m_Corner[CORNER::xyZ], m_Face[FACE::MINX]);		// Left
	PlaneFromPoints(m_Corner[CORNER::Xyz], m_Corner[CORNER::XyZ], m_Corner[CORNER::XYz], m_Face[FACE::MAXX]);		// Right
	PlaneFromPoints(m_Corner[CORNER::xyz], m_Corner[CORNER::xyZ], m_Corner[CORNER::Xyz], m_Face[FACE::MINY]);		// Bottom
	PlaneFromPoints(m_Corner[CORNER::xYz], m_Corner[CORNER::XYz], m_Corner[CORNER::xYZ], m_Face[FACE::MAXY]);		// Top
	PlaneFromPoints(m_Corner[CORNER::xyz], m_Corner[CORNER::Xyz], m_Corner[CORNER::xYz], m_Face[FACE::MINZ]);		// Near
	PlaneFromPoints(m_Corner[CORNER::xyZ], m_Corner[CORNER::xYZ], m_Corner[CORNER::XyZ], m_Face[FACE::MAXZ]);		// Far

	for (size_t i = 0; i < EDGE::NUMEDGES; i++)
	{
		m_EdgeDir[i] = m_Corner[s_Edge[i][1]] - m_Corner[s_Edge[i][0]];
		m_EdgeLen[i] = glm::length(m_EdgeDir[i]);
		m_EdgeDir[i] = glm::normalize(m_EdgeDir[i]);
	}

	m_bNeedsAlignment = false;
}


void BoundingBoxImpl::Align(const glm::fmat4x4 *matrix)
{
	if (matrix)
	{
		for (size_t i = 0; i < CORNER::NUMCORNERS; i++)
			m_Corner[i] = *matrix * glm::fvec4(m_Corner_Unaligned[i], 1.0f);
	}
	else
	{
		memcpy(m_Corner, m_Corner_Unaligned, sizeof(glm::fvec3) * CORNER::NUMCORNERS);
	}

	PlaneFromPoints(m_Corner[CORNER::xyz], m_Corner[CORNER::xyZ], m_Corner[CORNER::xYz], m_Face[FACE::MINX]);		// Left
	PlaneFromPoints(m_Corner[CORNER::Xyz], m_Corner[CORNER::XYz], m_Corner[CORNER::XyZ], m_Face[FACE::MAXX]);		// Right
	PlaneFromPoints(m_Corner[CORNER::xyz], m_Corner[CORNER::Xyz], m_Corner[CORNER::xyZ], m_Face[FACE::MINY]);		// Bottom
	PlaneFromPoints(m_Corner[CORNER::xYz], m_Corner[CORNER::xYZ], m_Corner[CORNER::XYz], m_Face[FACE::MAXY]);		// Top
	PlaneFromPoints(m_Corner[CORNER::xyz], m_Corner[CORNER::xYz], m_Corner[CORNER::Xyz], m_Face[FACE::MINZ]);		// Near
	PlaneFromPoints(m_Corner[CORNER::xyZ], m_Corner[CORNER::XyZ], m_Corner[CORNER::xYZ], m_Face[FACE::MAXZ]);		// Far

	for (size_t i = 0; i < EDGE::NUMEDGES; i++)
	{
		m_EdgeDir[i] = m_Corner[s_Edge[i][1]] - m_Corner[s_Edge[i][0]];
		m_EdgeLen[i] = glm::length(m_EdgeDir[i]);
		m_EdgeDir[i] = glm::normalize(m_EdgeDir[i]);
	}

	m_bNeedsAlignment = false;
}


bool BoundingBoxImpl::IsPointInside(const glm::fvec3 *ppt) const
{
	assert(ppt);
	assert(!m_bNeedsAlignment);

	// Go through each plane and see if the point is inside it
	for (size_t i = 0; i < FACE::NUMFACES; i++)
	{
		if (PlaneDotCoord(m_Face[i], *ppt) > 0)
			return false;
	}

	// If any point is inside all 6 frustum planes, it is inside the box
	return true;
}


bool BoundingBoxImpl::IsBoxInside(const BoundingBox *pbox) const
{
	if (!pbox)
		return false;

	BoundingBoxImpl *pboxi = (BoundingBoxImpl *)pbox;

	assert(!m_bNeedsAlignment);
	assert(!pboxi->m_bNeedsAlignment);

	uint32_t outside[CORNER::NUMCORNERS] = {0};

	for (uint32_t i = 0; i < CORNER::NUMCORNERS; i++)
	{
		// Go through each plane and see if the point is inside it
		for (size_t j = 0; j < FACE::NUMFACES; j++)
		{
			if (PlaneDotCoord(m_Face[j], pboxi->m_Corner[i]) > 0)
				outside[i] |= 1 << j;
		}
	}

	uint32_t outside_all = -1;
	for (uint32_t i = 0; i < CORNER::NUMCORNERS; i++)
		outside_all &= outside[i];
	if (outside_all != 0)
		return false;

	for (uint32_t i = 0; i < CORNER::NUMCORNERS; i++)
	{
		if (pboxi->IsPointInside(&m_Corner[i]))
			return true;
	}

	// Do any of the other edges penetrate the faces of the our box
	for (uint32_t j = 0; j < EDGE::NUMEDGES; j++)
	{
		for (uint32_t i = 0; i < FACE::NUMFACES; i++ )
		{
			float dist;
			if (glm::intersectRayPlane(pboxi->m_Corner[s_Edge[j][0]], pboxi->m_EdgeDir[j], m_Face[i].point, m_Face[i].normal, dist))
			{
				if (dist < pboxi->m_EdgeLen[j])
					return true;
			}
		}
	}

	// Do any of our edges penetrate the faces of the other box?
	for (uint32_t j = 0; j < EDGE::NUMEDGES; j++)
	{
		for (uint32_t i = 0; i < FACE::NUMFACES; i++)
		{
			float dist;
			if (glm::intersectRayPlane(m_Corner[s_Edge[j][0]], m_EdgeDir[j], pboxi->m_Face[i].point, pboxi->m_Face[i].normal, dist))
			{
				if (dist < m_EdgeLen[j])
					return true;
			}
		}
	}

	return false;
}


bool BoundingBoxImpl::IsSphereInside(const glm::fvec3 *centroid, float radius) const
{
	if (IsPointInside(centroid))
		return true;

#if 0
	for (uint32_t i = 0; i < FACE::NUMFACES; i++)
	{
		float d = glm::abs(glm::dot(glm::fvec3(m_Face[i].point - *centroid) + m_Face[i].w);

		// Check if the distance is less than or equal to the sphere's radius
		if (d <= radius)
			return true;
	}
#endif

	return false;
}


bool BoundingBoxImpl::CheckCollision(const glm::fvec3 *raypos, const glm::fvec3 *rayvec, float *dist) const
{
	const float EPSILON = 1e-6f;

	bool inside = true;
	float tMin = -std::numeric_limits<float>::infinity();
	float tMax = std::numeric_limits<float>::infinity();

	glm::vec3 dir = glm::normalize(*rayvec);

	for (size_t plidx = 0; plidx < FACE::NUMFACES; plidx++)
	{
		float denom = glm::dot(m_Face[plidx].normal, dir);
		float numer = glm::dot(m_Face[plidx].normal, m_Face[plidx].point - *raypos);

		if (std::fabs(denom) < EPSILON)
		{
			// Ray is parallel to the plane
			if (numer < 0)
				return false;				// Ray is outside the plane
			else
				continue;					// Ray is inside or on the plane; no need to update tMin or tMax
		}

		float t = numer / denom;

		if (denom < 0)
		{
			tMin = std::max(tMin, t);		// Ray is entering through this plane
			inside = false;
		}
		else
		{
			tMax = std::min(tMax, t);		// Ray is exiting through this plane
			if (t < 0)
				inside = false;
		}

		// Early exit if no intersection is possible
		if (tMin > tMax)
			return false;
	}

	// If tMax < 0, the intersection is behind the ray origin
	if (tMax < 0)
		return false;

	// The ray intersects the polyhedron
	if (dist)
		*dist = inside ? 0 : ((tMin >= 0) ? tMin : tMax);

	return true;
}


void BoundingBoxImpl::IncludeBounds(const BoundingBox *pbox)
{
	BoundingBoxImpl *pb = (BoundingBoxImpl *)pbox;

	m_Corner_Unaligned[CORNER::xyz].x = std::min(m_Corner_Unaligned[CORNER::xyz].x, pb->m_Corner_Unaligned[CORNER::xyz].x);
	m_Corner_Unaligned[CORNER::xyz].y = std::min(m_Corner_Unaligned[CORNER::xyz].y, pb->m_Corner_Unaligned[CORNER::xyz].y);
	m_Corner_Unaligned[CORNER::xyz].z = std::min(m_Corner_Unaligned[CORNER::xyz].z, pb->m_Corner_Unaligned[CORNER::xyz].z);

	m_Corner_Unaligned[CORNER::Xyz].x = std::max(m_Corner_Unaligned[CORNER::Xyz].x, pb->m_Corner_Unaligned[CORNER::Xyz].x);
	m_Corner_Unaligned[CORNER::Xyz].y = std::min(m_Corner_Unaligned[CORNER::Xyz].y, pb->m_Corner_Unaligned[CORNER::Xyz].y);
	m_Corner_Unaligned[CORNER::Xyz].z = std::min(m_Corner_Unaligned[CORNER::Xyz].z, pb->m_Corner_Unaligned[CORNER::Xyz].z);

	m_Corner_Unaligned[CORNER::xYz].x = std::min(m_Corner_Unaligned[CORNER::xYz].x, pb->m_Corner_Unaligned[CORNER::xYz].x);
	m_Corner_Unaligned[CORNER::xYz].y = std::max(m_Corner_Unaligned[CORNER::xYz].y, pb->m_Corner_Unaligned[CORNER::xYz].y);
	m_Corner_Unaligned[CORNER::xYz].z = std::min(m_Corner_Unaligned[CORNER::xYz].z, pb->m_Corner_Unaligned[CORNER::xYz].z);

	m_Corner_Unaligned[CORNER::XYz].x = std::max(m_Corner_Unaligned[CORNER::XYz].x, pb->m_Corner_Unaligned[CORNER::XYz].x);
	m_Corner_Unaligned[CORNER::XYz].y = std::max(m_Corner_Unaligned[CORNER::XYz].y, pb->m_Corner_Unaligned[CORNER::XYz].y);
	m_Corner_Unaligned[CORNER::XYz].z = std::min(m_Corner_Unaligned[CORNER::XYz].z, pb->m_Corner_Unaligned[CORNER::XYz].z);

	m_Corner_Unaligned[CORNER::xyZ].x = std::min(m_Corner_Unaligned[CORNER::xyZ].x, pb->m_Corner_Unaligned[CORNER::xyZ].x);
	m_Corner_Unaligned[CORNER::xyZ].y = std::min(m_Corner_Unaligned[CORNER::xyZ].y, pb->m_Corner_Unaligned[CORNER::xyZ].y);
	m_Corner_Unaligned[CORNER::xyZ].z = std::max(m_Corner_Unaligned[CORNER::xyZ].z, pb->m_Corner_Unaligned[CORNER::xyZ].z);

	m_Corner_Unaligned[CORNER::XyZ].x = std::max(m_Corner_Unaligned[CORNER::XyZ].x, pb->m_Corner_Unaligned[CORNER::XyZ].x);
	m_Corner_Unaligned[CORNER::XyZ].y = std::min(m_Corner_Unaligned[CORNER::XyZ].y, pb->m_Corner_Unaligned[CORNER::XyZ].y);
	m_Corner_Unaligned[CORNER::XyZ].z = std::max(m_Corner_Unaligned[CORNER::XyZ].z, pb->m_Corner_Unaligned[CORNER::XyZ].z);

	m_Corner_Unaligned[CORNER::xYZ].x = std::min(m_Corner_Unaligned[CORNER::xYZ].x, pb->m_Corner_Unaligned[CORNER::xYZ].x);
	m_Corner_Unaligned[CORNER::xYZ].y = std::max(m_Corner_Unaligned[CORNER::xYZ].y, pb->m_Corner_Unaligned[CORNER::xYZ].y);
	m_Corner_Unaligned[CORNER::xYZ].z = std::max(m_Corner_Unaligned[CORNER::xYZ].z, pb->m_Corner_Unaligned[CORNER::xYZ].z);

	m_Corner_Unaligned[CORNER::XYZ].x = std::max(m_Corner_Unaligned[CORNER::XYZ].x, pb->m_Corner_Unaligned[CORNER::XYZ].x);
	m_Corner_Unaligned[CORNER::XYZ].y = std::max(m_Corner_Unaligned[CORNER::XYZ].y, pb->m_Corner_Unaligned[CORNER::XYZ].y);
	m_Corner_Unaligned[CORNER::XYZ].z = std::max(m_Corner_Unaligned[CORNER::XYZ].z, pb->m_Corner_Unaligned[CORNER::XYZ].z);

	m_bNeedsAlignment = true;
}
