// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	class BoundingBoxImpl : public BoundingBox
	{

	public:
		struct FACE
		{
			enum eFACE
			{
				MINX = 0,
				MAXX,
				MINY,
				MAXY,
				MINZ,
				MAXZ,

				NUMFACES
			};
		};

		struct CORNER
		{
			enum eCORNER
			{
				xyz = 0,
				Xyz,
				xYz,
				XYz,
				xyZ,
				XyZ,
				xYZ,
				XYZ,

				NUMCORNERS
			};
		};

		struct EDGE
		{
			enum eEDGE
			{
				xyz_Xyz = 0,		// x to X edge
				xYz_XYz,			// x to X edge
				xyZ_XyZ,			// x to X edge
				xYZ_XYZ,			// x to X edge

				xyz_xYz,			// y to Y edge
				Xyz_XYz,			// y to Y edge
				xyZ_xYZ,			// y to Y edge
				XyZ_XYZ,			// y to Y edge

				xyz_xyZ,			// z to Z edge
				Xyz_XyZ,			// z to Z edge
				xYz_xYZ,			// z to Z edge
				XYz_XYZ,			// z to Z edge

				NUMEDGES
			};
		};

		struct AXES
		{
			enum eAXES
			{
				X = 0,
				Y,
				Z,

				NUMAXES
			};
		};

		BoundingBoxImpl();
		virtual ~BoundingBoxImpl();

		virtual void Release();

		virtual void SetFromBounds(const BoundingBox *box);

		virtual void SetOrigin(float x, float y, float z);
		virtual void SetOrigin(const glm::fvec3 *org);
		virtual const glm::fvec3 *GetOrigin(glm::fvec3 *org = nullptr) const;

		virtual void SetExtents(float x, float y, float z);
		virtual void SetExtents(const glm::fvec3 *ext);
		virtual const glm::fvec3 *GetExtents(glm::fvec3 *ext = nullptr) const;

		virtual void SetAsFrustum(const glm::fmat4x4 *viewmat, const glm::fmat4x4 *projmat, const RECT *viewport);

		virtual void Align(const glm::fmat4x4 *matrix);

		virtual bool IsPointInside(const glm::fvec3 *ppt);
		virtual bool IsBoxInside(const BoundingBox *pbox);
		virtual bool IsSphereInside(const glm::fvec3 *centroid, float radius);

		virtual bool CheckCollision(const glm::fvec3 *raypos, const glm::fvec3 *rayvec, float *dist = nullptr);

		// this will adjust the current bounding box by pbox's aligned bounds
		virtual void IncludeBounds(const BoundingBox *pbox);

		virtual const uint16_t *GetIndices() const;
		virtual const glm::fvec3 *GetCorners() const;

		const glm::fvec4 *GetFace(FACE::eFACE f) const;

	protected:

		glm::fvec4 m_Face[FACE::NUMFACES];				// a plane representing each of the six faces
		glm::fvec3 m_Corner[CORNER::NUMCORNERS];				// a point in space representing each of the eight corners

		glm::fvec3 m_vRange_Unaligned[3];
		glm::fvec3 m_vOrg_Unaligned;

		glm::fvec3 m_vRange_Aligned[3];
		glm::fvec3 m_vOrg_Aligned;

		// this is packed as bytes, with the type being 16-bit
		// integers so that this can be passed directly to the
		// rendering API as indices for drawing the box as lines.
	#pragma pack(push, 1)
		static uint16_t m_Edge[EDGE::NUMEDGES][2];		// a line segment representing each of the twelve edges
	#pragma pack(pop)

		bool m_bNeedsAlignment;
	};

};