#include <C3.h>
#include <C3ResourceManager.h>

#include <stb_image.h>
#include <iostream>


#pragma once

#pragma pack(push, 1)

struct STGAHeader
{
	uint8_t IDLength;			// 0x00		Size of the Image ID field
	uint8_t ColorMapType;		// 0x01		Color map type
	uint8_t ImageType;			// 0x02		Image type code
	uint16_t CMapStart;			// 0x03		Color map origin
	uint16_t CMapLength;		// 0x05		Color map length
	uint8_t CMapDepth;			// 0x07		Depth of color map
	uint16_t XOffset;			// 0x08		X origin of the image
	uint16_t YOffset;			// 0x0A		Y origin of the image
	uint16_t Width;				// 0x0C		Width of the image
	uint16_t Height;			// 0x0E		Height of image
	uint8_t PixelDepth;			// 0x10		Bits per pixel
	uint8_t ImageDescriptor;	// 0x11		Image descriptor byte
};

#pragma pack(pop)

#define TGA_NONE				0
#define TGA_PALETTED			1
#define TGA_TRUECOLOR			2
#define TGA_MONO				3
#define TGA_PALETTED_COMP		9
#define TGA_TRUECOLOR_COMP		10
#define TGA_MONO_COMP			11


inline float EdgeFunction(const glm::fvec2 &v0, const glm::fvec2 &v1, const glm::fvec2 &v2)
{
	return (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
}

template<typename T>
class TRaster
{

public:
	TRaster() : m_Dim(0, 0)
	{
	}

	using FillFunc = std::function<T(size_t, size_t)>;
	TRaster(size_t xdim, size_t ydim, FillFunc ff) : m_Dim(0, 0)
	{
		Resize(xdim, ydim);

		size_t i = 0;
		for (size_t y = 0; y < m_Dim.y; y++)
		{
			for (size_t x = 0; x < m_Dim.x; x++, i++)
			{
				m_Image[i] = ff(x, y);
			}
		}
	}

	TRaster(size_t xdim, size_t ydim, std::optional<T> fillval = std::nullopt) : m_Dim(xdim, ydim)
	{
		Resize(xdim, ydim, fillval);
	}

	TRaster(size_t xdim, size_t ydim, const T *source, bool flip) : m_Dim(xdim, ydim)
	{
		m_Image.resize(xdim * ydim);

		if (flip)
		{
			for (size_t y = 1; y <= ydim; y++)
			{
				memcpy(GetRow(ydim - y), source, sizeof(T) * xdim);
				source += xdim;
			}
		}
		else
		{
			memcpy(m_Image.data(), source, xdim * ydim * sizeof(T));
		}
	}

	size_t GetWidth() const
	{
		return m_Dim.x;
	}

	size_t GetHeight() const
	{
		return m_Dim.y;
	}

	void SetName(const TCHAR *name)
	{
		m_Name = name;
	}

	const TCHAR *GetName()
	{
		return m_Name.c_str();
	}

	void Fill(FillFunc ff)
	{
		size_t i = 0;
		for (size_t y = 0; y < m_Dim.y; y++)
		{
			for (size_t x = 0; x < m_Dim.x; x++, i++)
			{
				m_Image[i] = ff(x, y);
			}
		}
	}

	void Flip()
	{
		T *rtop = GetRow(0);
		T *rbot = GetRow(m_Dim.y - 1);
		std::vector<T> tmp(m_Dim.x);

		while (rtop < rbot)
		{
			memcpy(tmp.data(), rtop, sizeof(T) * m_Dim.x);
			memcpy(rtop, rbot, sizeof(T) * m_Dim.x);
			memcpy(rbot, tmp.data(), sizeof(T) * m_Dim.x);

			rtop += m_Dim.x;
			rbot -= m_Dim.x;
		}
	}

	enum class VAlignMode
	{
		TOP,
		CENTER,
		BOTTOM
	};

	enum class HAlignMode
	{
		LEFT,
		CENTER,
		RIGHT
	};

	// Clips or expands the raster given the align modes and, if expanding, fills the new area
	void Resize(size_t xdim, size_t ydim,
		std::optional<T> fillval = std::nullopt,
		VAlignMode valign = VAlignMode::CENTER, HAlignMode halign = HAlignMode::CENTER)
	{
		// TODO: temp buffer allocation, alignment, etc
		m_Dim.x = (uint32_t)xdim;
		m_Dim.y = (uint32_t)ydim;

		size_t sz = m_Dim.x * m_Dim.y;

		if (fillval.has_value())
			m_Image.resize(sz, *fillval);
		else
			m_Image.resize(sz);
	}

	void SetPixel(size_t x, size_t y, T val)
	{
		if ((x >= m_Dim.x) || (y >= m_Dim.y))
			return;

		m_Image[y * m_Dim.x + x] = val;
	}

	std::optional<T> GetPixel(size_t x, size_t y) const
	{
		if ((x >= m_Dim.x) || (y >= m_Dim.y))
			return std::nullopt;

		return std::make_optional<T>(m_Image[y * m_Dim.x + x]);
	}

	// Use PixelOp to receive an x/y coordinate into the raster and return a value to be placed at that location
	using PixelOp = std::function<std::optional<T>(size_t, size_t, T)>;
	using BlendOp = std::function<T(T, T)>;

	// Iterates over pixels in a rectangle calling your function (op) and optionally a function to blend values that you optionally provide
	void ProcessRect(int64_t x1, int64_t y1, int64_t x2, int64_t y2, PixelOp op, std::optional<BlendOp> blend = std::nullopt)
	{
		if (x1 > x2)
			std::swap(x1, x2);

		if (y1 > y2)
			std::swap(y1, y2);

		int64_t dx = x2 - x1;
		int64_t dy = y2 - y1;

		if (!dx || !dy)
			return;

		// if no BlendOp was supplied, we force a replacement
		BlendOp b = blend.has_value() ? *blend : [](T s, T d) -> T { return s; };

		for (int64_t y = y1; y < y2; y++)
		{
			int64_t wrapped_y = y % m_Dim.y;

			T *row = m_Image.data() + (wrapped_y * m_Dim.x);

			for (int64_t x = x1; x < x2; x++)
			{
				int64_t wrapped_x = x % m_Dim.x;

				std::optional<T> v = op(x, y, row[wrapped_x % m_Dim.x]);
				if (v.has_value())
				{
					row[wrapped_x] = b(*v, row[wrapped_x]);
				}
			}
		}
	}

	// Gets the start of the row
	T *GetRow(size_t y)
	{
		return m_Image.data() + (y * m_Dim.x);
	}

	// ShaderOp takes 3 interpolants
	using ShaderOp = std::function<std::optional<T>(float, float, float)>;

	// Rasterizes a triangle with points in uv space
	void RasterizeTriangle(glm::fvec2 v0, glm::fvec2 v1, glm::fvec2 v2, ShaderOp shader, std::optional<BlendOp> blend = std::nullopt)
	{
		// Compute bounding box
		glm::fvec2 uv1 = glm::fvec2(std::min({v0.x, v1.x, v2.x}), std::min({v0.y, v1.y, v2.y}));
		glm::fvec2 uv2 = glm::fvec2(std::max({v0.x, v1.x, v2.x}), std::max({v0.y, v1.y, v2.y}));

		size_t x1 = (size_t)(uv1.x * (float)m_Dim.x);
		size_t x2 = (size_t)(uv2.x * (float)m_Dim.x);
		size_t y1 = (size_t)(uv1.y * (float)m_Dim.y);
		size_t y2 = (size_t)(uv2.y * (float)m_Dim.y);

		// Calculate edge functions
		float area = EdgeFunction(v0, v1, v2);

		const float epsilon = 1e-6f;

		// skip degenerate triangles
		if (std::abs(area) < epsilon)
			return;

		float dxf = (float)(x2 - x1);
		float dyf = (float)(y2 - y1);

		ProcessRect(x1, y1, x2, y2, [&](int64_t x, int64_t y, T cur) -> std::optional<T>
		{
			glm::fvec2 uv(((float)x + 0.5f) / (float)m_Dim.x, ((float)y + 0.5f) / m_Dim.y);

			// Barycentric coordinates
			float w0 = EdgeFunction(v1, v2, uv) / area;
			float w1 = EdgeFunction(v2, v0, uv) / area;
			float w2 = EdgeFunction(v0, v1, uv) / area;

			// Inside triangle check
			if ((std::abs(w0) >= epsilon) && (std::abs(w1) >= epsilon) && (std::abs(w2) >= epsilon))
				return shader(w0, w1, w2);

			return std::nullopt;
		}, blend);
	}


	void SaveToTGA(const TCHAR *filename)
	{
		FILE *f;
		if ((_tfopen_s(&f, filename, _T("wb")) == 0) && f)
		{
			STGAHeader tgaheader = {0};
			tgaheader.Width = (uint16_t)m_Dim.x;
			tgaheader.Height = (uint16_t)m_Dim.y;
			tgaheader.PixelDepth = sizeof(T) << 3;
			tgaheader.ImageType = (sizeof(T) == 1) ? TGA_MONO : TGA_TRUECOLOR;
			tgaheader.ImageDescriptor = (sizeof(T) == 4) ? 8 : 0;

			fwrite(&tgaheader, sizeof(STGAHeader), 1, f);

			const size_t pixelCount = m_Dim.x * m_Dim.y;

			if constexpr (is_u8vec3<T>::value)
			{
				// Swap R and B channels
				std::vector<T> converted(pixelCount);
				for (size_t i = 0; i < pixelCount; ++i)
				{
					converted[i].x = m_Image[i].z;
					converted[i].y = m_Image[i].y;
					converted[i].z = m_Image[i].x;
				}
				fwrite(converted.data(), sizeof(T), pixelCount, f);
			}
			else if constexpr (is_u8vec4<T>::value)
			{
				// Swap channels
				std::vector<T> converted(pixelCount);
				for (size_t i = 0; i < pixelCount; ++i)
				{
					converted[i].x = m_Image[i].w;
					converted[i].y = m_Image[i].z;
					converted[i].z = m_Image[i].y;
					converted[i].w = m_Image[i].x;
				}
				fwrite(converted.data(), sizeof(T), pixelCount, f);
			}
			else
			{
				// Write directly if no swizzling needed
				fwrite(m_Image.data(), sizeof(T), pixelCount, f);
			}

			fclose(f);
		}
	}

	// Helper trait for uint8_t
	template<typename T>
	struct is_u8 : std::false_type {};

	template<>
	struct is_u8<uint8_t> : std::true_type {};

	// Helper trait for uint16_t
	template<typename T>
	struct is_u16 : std::false_type {};

	template<>
	struct is_u16<uint16_t> : std::true_type {};

	// Helper trait for u8vec3
	template<typename T>
	struct is_u8vec3 : std::false_type {};

	template<>
	struct is_u8vec3<glm::u8vec3> : std::true_type {};

	// Helper trait for u8vec4
	template<typename T>
	struct is_u8vec4 : std::false_type {};

	template<>
	struct is_u8vec4<glm::u8vec4> : std::true_type {};


public:

	tstring m_Name;
	std::vector<T> m_Image;
	glm::uvec2 m_Dim;

};

using U8Raster = TRaster<uint8_t>;
using RGBRaster = TRaster<glm::u8vec3>;
using RGBARaster = TRaster<glm::u8vec4>;

namespace c3
{
	namespace util
	{
		void LoadU8Image(ResourceManager *rm, const TCHAR *filename, U8Raster &img);
		void LoadRGBImage(ResourceManager *rm, const TCHAR *filename, RGBRaster &img);
		void LoadRGBAImage(ResourceManager *rm, const TCHAR *filename, RGBARaster &img);
	};
};