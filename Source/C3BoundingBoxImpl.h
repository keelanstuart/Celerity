// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


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

		struct Plane
		{
			glm::fvec3 point;
			glm::fvec3 normal;
		};

		BoundingBoxImpl();
		BoundingBoxImpl(const BoundingBox *other);

		BoundingBoxImpl & operator =(const BoundingBoxImpl &other);

		virtual ~BoundingBoxImpl();

		virtual void Release();

		virtual void SetFromBounds(const BoundingBox *box);

		virtual void SetExtents(const glm::fvec3 *min_bounds, const glm::fvec3 *max_bounds);

		virtual void SetAsFrustum(const glm::fmat4x4 *viewmat, const glm::fmat4x4 *projmat, const RECT *viewport);

		virtual void Align(const glm::fmat4x4 *matrix);

		virtual bool IsPointInside(const glm::fvec3 *ppt) const;
		virtual bool IsBoxInside(const BoundingBox *pbox) const;
		virtual bool IsSphereInside(const glm::fvec3 *centroid, float radius) const;

		virtual bool CheckCollision(const glm::fvec3 *raypos, const glm::fvec3 *rayvec, float *dist = nullptr, float *norm = nullptr) const;

		// this will adjust the current bounding box by pbox's aligned bounds
		virtual void IncludeBounds(const BoundingBox *pbox);

		virtual const uint16_t *GetIndices() const;
		virtual const glm::fvec3 *GetCorners() const;
		virtual const glm::fvec3 *GetAlignedCorners() const;

	protected:

		Plane m_Face[FACE::NUMFACES];								// planes representing each of the six faces

		glm::fvec3 m_Corner[CORNER::NUMCORNERS];					// points in space representing each of the eight aligned corners
		glm::fvec3 m_Corner_Unaligned[CORNER::NUMCORNERS];			// points in space representing each of the eight unaligned corners

		// this is packed as bytes, with the type being 16-bit
		// integers so that this can be passed directly to the
		// rendering API as indices for drawing the box as lines.
	#pragma pack(push, 1)
		static uint16_t s_Edge[EDGE::NUMEDGES][2];		// a line segment representing each of the twelve edges
	#pragma pack(pop)
		float m_EdgeLen[EDGE::NUMEDGES];
		glm::fvec3 m_EdgeDir[EDGE::NUMEDGES];

		bool m_bNeedsAlignment;
	};

};