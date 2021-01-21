// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3FrustumImpl.h>

using namespace c3;


FrustumImpl::FrustumImpl()
{
	edge[FRUSTEDGE_xyz_Xyz][0] = FRUSTCORN_xyz;		// x to X edges
	edge[FRUSTEDGE_xyz_Xyz][1] = FRUSTCORN_Xyz;
	edge[FRUSTEDGE_xYz_XYz][0] = FRUSTCORN_xYz;
	edge[FRUSTEDGE_xYz_XYz][1] = FRUSTCORN_XYz;
	edge[FRUSTEDGE_xyZ_XyZ][0] = FRUSTCORN_xyZ;
	edge[FRUSTEDGE_xyZ_XyZ][1] = FRUSTCORN_XyZ;
	edge[FRUSTEDGE_xYZ_XYZ][0] = FRUSTCORN_xYZ;
	edge[FRUSTEDGE_xYZ_XYZ][1] = FRUSTCORN_XYZ;

	edge[FRUSTEDGE_xyz_xYz][0] = FRUSTCORN_xyz;		// y to Y edges
	edge[FRUSTEDGE_xyz_xYz][1] = FRUSTCORN_xYz;
	edge[FRUSTEDGE_Xyz_XYz][0] = FRUSTCORN_Xyz;
	edge[FRUSTEDGE_Xyz_XYz][1] = FRUSTCORN_XYz;
	edge[FRUSTEDGE_xyZ_xYZ][0] = FRUSTCORN_xyZ;
	edge[FRUSTEDGE_xyZ_xYZ][1] = FRUSTCORN_xYZ;
	edge[FRUSTEDGE_XyZ_XYZ][0] = FRUSTCORN_XyZ;
	edge[FRUSTEDGE_XyZ_XYZ][1] = FRUSTCORN_XYZ;

	edge[FRUSTEDGE_xyz_xyZ][0] = FRUSTCORN_xyz;		// z to Z edges
	edge[FRUSTEDGE_xyz_xyZ][1] = FRUSTCORN_xyZ;
	edge[FRUSTEDGE_Xyz_XyZ][0] = FRUSTCORN_Xyz;
	edge[FRUSTEDGE_Xyz_XyZ][1] = FRUSTCORN_XyZ;
	edge[FRUSTEDGE_xYz_xYZ][0] = FRUSTCORN_xYz;
	edge[FRUSTEDGE_xYz_xYZ][1] = FRUSTCORN_xYz;
	edge[FRUSTEDGE_XYz_XYZ][0] = FRUSTCORN_XYz;
	edge[FRUSTEDGE_XYz_XYZ][1] = FRUSTCORN_XYZ;
}


FrustumImpl::~FrustumImpl()
{

}


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
	normal = glm::normalize(glm::cross(edge1, edge2));

	PlaneFromPointNormal(p1, normal, out_plane);
}


bool PlaneIntersectLine(const glm::fvec4 &plane, const glm::fvec3 v1, const glm::fvec3 &v2, glm::fvec3 *out_point)
{
	glm::fvec3 norm;
	norm.x = plane.x;
	norm.y = plane.y;
	norm.z = plane.z;

	glm::fvec3 dir;
	dir.x = v2.x - v1.x;
	dir.y = v2.y - v1.y;
	dir.z = v2.z - v1.z;

	float dot = glm::dot(norm, dir);
	if (!dot)
		return false;

	if (out_point)
	{
		float temp = (plane.w + glm::dot(norm, v1)) / dot;
		out_point->x = v1.x - temp * dir.x;
		out_point->y = v1.y - temp * dir.y;
		out_point->z = v1.z - temp * dir.z;
	}

	return true;
}


void FrustumImpl::CalculateForView(const glm::fmat4x4 *viewmat, const glm::fmat4x4 *projmat)
{
	corner[FRUSTCORN_xyz].x = -1.0f;
	corner[FRUSTCORN_xyz].y = -1.0f;
	corner[FRUSTCORN_xyz].z = 0.0f;
	
	corner[FRUSTCORN_Xyz].x = 1.0f;
	corner[FRUSTCORN_Xyz].y = -1.0f;
	corner[FRUSTCORN_Xyz].z = 0.0f;

	corner[FRUSTCORN_xYz].x = -1.0f;
	corner[FRUSTCORN_xYz].y = 1.0f;
	corner[FRUSTCORN_xYz].z = 0.0f;

	corner[FRUSTCORN_XYz].x = 1.0f;
	corner[FRUSTCORN_XYz].y = 1.0f;
	corner[FRUSTCORN_XYz].z = 0.0f;

	corner[FRUSTCORN_xyZ].x = -1.0f;
	corner[FRUSTCORN_xyZ].y = -1.0f;
	corner[FRUSTCORN_xyZ].z = 1.0f;

	corner[FRUSTCORN_XyZ].x = 1.0f;
	corner[FRUSTCORN_XyZ].y = -1.0f;
	corner[FRUSTCORN_XyZ].z = 1.0f;

	corner[FRUSTCORN_xYZ].x = -1.0f;
	corner[FRUSTCORN_xYZ].y = 1.0f;
	corner[FRUSTCORN_xYZ].z = 1.0f;

	corner[FRUSTCORN_XYZ].x = 1.0f;
	corner[FRUSTCORN_XYZ].y = 1.0f;
	corner[FRUSTCORN_XYZ].z = 1.0f;

	glm::fmat4x4 m;
    m = glm::inverse(*viewmat * *projmat);

	for (uint32_t i = 0; i < FRUSTCORN_NUMCORNERS; i++)
		corner[i] = glm::fvec4(corner[i].x, corner[i].y, corner[i].z, 1.0f) * m;

	PlaneFromPoints(corner[FRUSTCORN_xyz], corner[FRUSTCORN_Xyz], corner[FRUSTCORN_xYz], face[FRUSTFACE_NEAR]);		// Near
	PlaneFromPoints(corner[FRUSTCORN_xYZ], corner[FRUSTCORN_XYZ], corner[FRUSTCORN_XyZ], face[FRUSTFACE_FAR]);		// Far

	PlaneFromPoints(corner[FRUSTCORN_xYz], corner[FRUSTCORN_xYZ], corner[FRUSTCORN_xyZ], face[FRUSTFACE_LEFT]);		// Left
	PlaneFromPoints(corner[FRUSTCORN_XYZ], corner[FRUSTCORN_XYz], corner[FRUSTCORN_XyZ], face[FRUSTFACE_RIGHT]);	// Right

	PlaneFromPoints(corner[FRUSTCORN_xYz], corner[FRUSTCORN_XYz], corner[FRUSTCORN_xYZ], face[FRUSTFACE_TOP]);		// Top
	PlaneFromPoints(corner[FRUSTCORN_Xyz], corner[FRUSTCORN_xyz], corner[FRUSTCORN_xyZ], face[FRUSTFACE_BOTTOM]);	// Bottom
}


void FrustumImpl::CalculateForBounds(const glm::fvec3 *boundsmin, const glm::fvec3 *boundsmax, const glm::fmat4x4 *matrix)
{
	corner[FRUSTCORN_xyz].x = boundsmin->x;
	corner[FRUSTCORN_xyz].y = boundsmin->y;
	corner[FRUSTCORN_xyz].z = boundsmin->z;
	
	corner[FRUSTCORN_xyZ].x = boundsmin->x;
	corner[FRUSTCORN_xyZ].y = boundsmin->y;
	corner[FRUSTCORN_xyZ].z = boundsmax->z;

	corner[FRUSTCORN_xYz].x = boundsmin->x;
	corner[FRUSTCORN_xYz].y = boundsmax->y;
	corner[FRUSTCORN_xYz].z = boundsmin->z;

	corner[FRUSTCORN_xYZ].x = boundsmin->x;
	corner[FRUSTCORN_xYZ].y = boundsmax->y;
	corner[FRUSTCORN_xYZ].z = boundsmax->z;

	corner[FRUSTCORN_Xyz].x = boundsmax->x;
	corner[FRUSTCORN_Xyz].y = boundsmin->y;
	corner[FRUSTCORN_Xyz].z = boundsmin->z;

	corner[FRUSTCORN_XyZ].x = boundsmax->x;
	corner[FRUSTCORN_XyZ].y = boundsmin->y;
	corner[FRUSTCORN_XyZ].z = boundsmax->z;

	corner[FRUSTCORN_XYz].x = boundsmax->x;
	corner[FRUSTCORN_XYz].y = boundsmax->y;
	corner[FRUSTCORN_XYz].z = boundsmin->z;

	corner[FRUSTCORN_XYZ].x = boundsmax->x;
	corner[FRUSTCORN_XYZ].y = boundsmax->y;
	corner[FRUSTCORN_XYZ].z = boundsmax->z;

	for (size_t i = 0; i < FRUSTCORN_NUMCORNERS; i++)
		corner[i] = glm::fvec4(corner[i].x, corner[i].y, corner[i].z, 1.0f) * *matrix;

	PlaneFromPoints(corner[FRUSTCORN_xyz], corner[FRUSTCORN_xYz], corner[FRUSTCORN_Xyz], face[FRUSTFACE_NEAR]);		// Near
	PlaneFromPoints(corner[FRUSTCORN_xyZ], corner[FRUSTCORN_xYZ], corner[FRUSTCORN_XyZ], face[FRUSTFACE_FAR]);		// Far

	PlaneFromPoints(corner[FRUSTCORN_xyz], corner[FRUSTCORN_xyZ], corner[FRUSTCORN_xYz], face[FRUSTFACE_LEFT]);		// Left
	PlaneFromPoints(corner[FRUSTCORN_Xyz], corner[FRUSTCORN_XyZ], corner[FRUSTCORN_XYz], face[FRUSTFACE_RIGHT]);	// Right

	PlaneFromPoints(corner[FRUSTCORN_xYz], corner[FRUSTCORN_xYZ], corner[FRUSTCORN_XYz], face[FRUSTFACE_TOP]);		// Top
	PlaneFromPoints(corner[FRUSTCORN_xyz], corner[FRUSTCORN_xyZ], corner[FRUSTCORN_xYZ], face[FRUSTFACE_BOTTOM]);	// Bottom
}


bool FrustumImpl::IsPointInside(float x, float y, float z)
{
	uint32_t bOutside = 0;

	// Go through each plane and see if the point is inside it
	for (uint32_t i = 0; i < FRUSTFACE_NUMFACES; i++)
	{
		if (face[i].x * x +
			face[i].y * y +
			face[i].z * z +
			face[i].w < 0)
			bOutside |= (1 << i);
	}

	// If any point is inside all 6 frustum planes, it is inside the frustum
	if (!bOutside)
		return true;

	return false;
}


bool FrustumImpl::IsPointInside(const glm::fvec3 *point)
{
	if (!point)
		return false;

	return IsPointInside(point->x, point->y, point->z);
}


bool FrustumImpl::IsFrustumInside(const Frustum *frust)
{
	FrustumImpl *f = (FrustumImpl *)frust;

	uint32_t bOutside[FRUSTCORN_NUMCORNERS] = {0, 0, 0, 0, 0, 0, 0, 0};

	// Check boundary vertices against all 6 frustum planes, 
	// and store result (1 if outside) in a bitfield
	for (size_t j = 0; j < FRUSTCORN_NUMCORNERS; j++)
	{
		for (size_t i = 0; i < FRUSTFACE_NUMFACES; i++)
		{
			if (face[i].x * f->corner[j].x +
				face[i].y * f->corner[j].y +
				face[i].z * f->corner[j].z +
				face[i].w < 0)
			{
				bOutside[j] |= (1 << i);
			}
		}

		// If any point is inside all 6 frustum planes, it is inside
		// the frustum, so the object must be rendered.
		if (!bOutside[j])
			return true;
	}

	// If all points are outside any single frustum plane, the object is
	// outside the frustum
	if ((bOutside[0] & bOutside[1] & bOutside[2] & bOutside[3] & 
		bOutside[4] & bOutside[5] & bOutside[6] & bOutside[7]) != 0)
	{
		return false;
	}

	glm::fvec3 dummy;

	// Do any of the frustum edges penetrate the faces of the bounding box
	for (size_t j = 0; j < FRUSTEDGE_NUMEDGES; j++)
	{
		for (size_t i = 0; i < FRUSTFACE_NUMFACES; i++ )
		{
			if (PlaneIntersectLine(face[i], f->corner[f->edge[j][0]], f->corner[f->edge[j][1]], nullptr))
				return true;
		}
	}

	// Do any of the bounding box edges penetrate the faces of the frustum?
	for (size_t j = 0; j < FRUSTEDGE_NUMEDGES; j++)
	{
		for (size_t i = 0; i < FRUSTFACE_NUMFACES; i++)
		{
			if (PlaneIntersectLine(f->face[i], corner[edge[j][0]], corner[edge[j][1]], nullptr))
				return true;
		}
	}

    // Now see if frustum is contained in bounding box
    // If any frustum corner point is outside any plane of the bounding box,
    // the frustum is not contained in the bounding box, so the object
    // is outside the frustum
	for (size_t j = 0; j < FRUSTCORN_NUMCORNERS; j++)
	{
		for (size_t i = 0; i < FRUSTFACE_NUMFACES; i++)
		{
			if (f->face[i].x * corner[j].x +
				f->face[i].y * corner[j].y +
				f->face[i].z * corner[j].z +
				f->face[i].w < 0)
			{
				return false;
			}
		}
	}

	return true;
}
