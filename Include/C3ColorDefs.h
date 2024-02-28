// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once


namespace c3
{
	namespace Color
	{
		constexpr glm::fvec4 fWhite(1.0f, 1.0f, 1.0f, 1.0f);
		constexpr glm::fvec4 fWhiteTT(1.0f, 1.0f, 1.0f, 0.75f);
		constexpr glm::fvec4 fWhiteHT(1.0f, 1.0f, 1.0f, 0.5f);
		constexpr glm::fvec4 fWhiteQT(1.0f, 1.0f, 1.0f, 0.25f);
		constexpr glm::fvec4 fWhiteFT(1.0f, 1.0f, 1.0f, 0.0f);
		constexpr glm::fvec4 fBlack(0.0f, 0.0f, 0.0f, 1.0f);
		constexpr glm::fvec4 fBlackTT(0.0f, 0.0f, 0.0f, 0.75f);
		constexpr glm::fvec4 fBlackHT(0.0f, 0.0f, 0.0f, 0.5f);
		constexpr glm::fvec4 fBlackQT(0.0f, 0.0f, 0.0f, 0.25f);
		constexpr glm::fvec4 fBlackFT(0.0f, 0.0f, 0.0f, 0.0f);
		constexpr glm::fvec4 fGrey(0.5f, 0.5f, 0.5f, 1.0f);
		constexpr glm::fvec4 fLightGrey(0.75f, 0.75f, 0.75f, 1.0f);
		constexpr glm::fvec4 fDarkGrey(0.25f, 0.25f, 0.25f, 1.0f);
		constexpr glm::fvec4 fVeryDarkGrey(0.1f, 0.1f, 0.1f, 1.0f);
		constexpr glm::fvec4 fRed(1.0f, 0.0f, 0.0f, 1.0f);
		constexpr glm::fvec4 fDarkRed(0.5f, 0.0f, 0.0f, 1.0f);
		constexpr glm::fvec4 fVeryDarkRed(0.25f, 0.0f, 0.0f, 1.0f);
		constexpr glm::fvec4 fGreen(0.0f, 1.0f, 0.0f, 1.0f);
		constexpr glm::fvec4 fDarkGreen(0.0f, 0.5f, 0.0f, 1.0f);
		constexpr glm::fvec4 fVeryDarkGreen(0.0f, 0.25f, 0.0f, 1.0f);
		constexpr glm::fvec4 fBlue(0.0f, 0.0f, 1.0f, 1.0f);
		constexpr glm::fvec4 fDarkBlue(0.0f, 0.0f, 0.5f, 1.0f);
		constexpr glm::fvec4 fVeryDarkBlue(0.0f, 0.0f, 0.25f, 1.0f);
		constexpr glm::fvec4 fMagenta(1.0f, 0.0f, 1.0f, 1.0f);
		constexpr glm::fvec4 fDarkMagenta(0.5f, 0.0f, 0.5f, 1.0f);
		constexpr glm::fvec4 fYellow(1.0f, 1.0f, 0.0f, 1.0f);
		constexpr glm::fvec4 fDarkYellow(0.5f, 0.5f, 0.0f, 1.0f);
		constexpr glm::fvec4 fCyan(0.0f, 1.0f, 1.0f, 1.0f);
		constexpr glm::fvec4 fDarkCyan(0.0f, 0.5f, 0.5f, 1.0f);
		constexpr glm::fvec4 fNaturalSunlight(1.0f, 0.95f, 0.85f, 1.0f);
		constexpr glm::fvec4 fEveningSunlight(0.6f, 0.4f, 0.3f, 1.0f);

#pragma pack(push, 1)
		struct SRGBColor
		{
			uint8_t r, g, b;
		};

		union SRGBAColor
		{
			struct { uint8_t r, g, b, a; };
			uint32_t i;
		};
#pragma pack(pop)

#ifndef RGBA
#define RGBA(r, g, b, a)		((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)|(((DWORD)(BYTE)(a))<<24)))
#endif

#define RED(c)					(c & 0xff)
#define GREEN(c)				((c >> 8) & 0xff)
#define BLUE(c)					((c >> 16) & 0xff)
#define ALPHA(c)				((c >> 24) & 0xff)

		inline void ConvertIntToVec(SRGBAColor ic, props::TVec4F &vc)
		{
			vc.r = (float)(ic.r) / 255.0f;
			vc.g = (float)(ic.g) / 255.0f;
			vc.b = (float)(ic.b) / 255.0f;
			vc.a = (float)(ic.a) / 255.0f;
		}

		inline void ConvertIntToVec(SRGBColor ic, props::TVec3F &vc)
		{
			vc.r = (float)(ic.r) / 255.0f;
			vc.g = (float)(ic.g) / 255.0f;
			vc.b = (float)(ic.b) / 255.0f;
		}

		inline SRGBAColor ConvertVecToInt(props::TVec4F &vc)
		{
			SRGBAColor ret;

			ret.r = (uint8_t)(std::min(std::max(0.0f, vc.r), 1.0f) * 255.0f);
			ret.g = (uint8_t)(std::min(std::max(0.0f, vc.g), 1.0f) * 255.0f);
			ret.b = (uint8_t)(std::min(std::max(0.0f, vc.b), 1.0f) * 255.0f);
			ret.a = (uint8_t)(std::min(std::max(0.0f, vc.a), 1.0f) * 255.0f);

			return ret;
		}

		inline SRGBColor ConvertVecToInt(props::TVec3F &vc)
		{
			SRGBColor ret;

			ret.r = (uint8_t)(std::min(std::max(0.0f, vc.r), 1.0f) * 255.0f);
			ret.g = (uint8_t)(std::min(std::max(0.0f, vc.g), 1.0f) * 255.0f);
			ret.b = (uint8_t)(std::min(std::max(0.0f, vc.b), 1.0f) * 255.0f);

			return ret;
		}


		constexpr SRGBAColor iWhite = {255, 255, 255, 255};
		constexpr SRGBAColor iWhiteTT = {255, 255, 255, 192};		// white, three-quarter transparency
		constexpr SRGBAColor iWhiteHT = {255, 255, 255, 128};		// white, half transparency
		constexpr SRGBAColor iWhiteQT = {255, 255, 255, 64};		// white, quarter transparency
		constexpr SRGBAColor iWhiteFT = {255, 255, 255, 0};			// white, full transparency

		constexpr SRGBAColor iBlack = {0, 0, 0, 255};
		constexpr SRGBAColor iBlackTT = {0, 0, 0, 192};				// black, three-quarter transparency
		constexpr SRGBAColor iBlackHT = {0, 0, 0, 128};				// black, half transparency
		constexpr SRGBAColor iBlackQT = {0, 0, 0, 64};				// black, quarter transparency
		constexpr SRGBAColor iBlackFT = {0, 0, 0, 0};				// black, full transparency

		constexpr SRGBAColor iGrey = {128, 128, 128, 255};
		constexpr SRGBAColor iGreyTT = {128, 128, 128, 192};		// grey, three-quarter transparency
		constexpr SRGBAColor iGreyHT = {128, 128, 128, 128};		// grey, half transparency
		constexpr SRGBAColor iGreyQT = {128, 128, 128, 64};			// grey, quarter transparency
		constexpr SRGBAColor iGreyFT = {128, 128, 128, 0};			// grey, full transparency

		constexpr SRGBAColor iLightGrey = {192, 192, 192, 255};
		constexpr SRGBAColor iLightGreyTT = {192, 192, 192, 192};
		constexpr SRGBAColor iLightGreyHT = {192, 192, 192, 128};
		constexpr SRGBAColor iLightGreyQT = {192, 192, 192, 64};
		constexpr SRGBAColor iLightGreyFT = {192, 192, 192, 0};

		constexpr SRGBAColor iDarkGrey = {64, 64, 64, 255};
		constexpr SRGBAColor iDarkGreyTT = {64, 64, 64, 192};
		constexpr SRGBAColor iDarkGreyHT = {64, 64, 64, 128};
		constexpr SRGBAColor iDarkGreyQT = {64, 64, 64, 64};
		constexpr SRGBAColor iDarkGreyFT = {64, 64, 64, 0};

		constexpr SRGBAColor iVeryDarkGrey = {16, 16, 16, 255};
		constexpr SRGBAColor iVeryDarkGreyTT = {16, 16, 16, 192};
		constexpr SRGBAColor iVeryDarkGreyHT = {16, 16, 16, 128};
		constexpr SRGBAColor iVeryDarkGreyQT = {16, 16, 16, 64};
		constexpr SRGBAColor iVeryDarkGreyFT = {16, 16, 16, 0};

		constexpr SRGBAColor iRed = {255, 0, 0, 255};
		constexpr SRGBAColor iRedTT = {255, 0, 0, 192};
		constexpr SRGBAColor iRedHT = {255, 0, 0, 128};
		constexpr SRGBAColor iRedQT = {255, 0, 0, 64};
		constexpr SRGBAColor iRedFT = {255, 0, 0, 0};

		constexpr SRGBAColor iDarkRed = {128, 0, 0, 255};
		constexpr SRGBAColor iDarkRedTT = {128, 0, 0, 192};
		constexpr SRGBAColor iDarkRedHT = {128, 0, 0, 128};
		constexpr SRGBAColor iDarkRedQT = {128, 0, 0, 64};
		constexpr SRGBAColor iDarkRedFT = {128, 0, 0, 0};

		constexpr SRGBAColor iVeryDarkRed = {64, 0, 0, 255};
		constexpr SRGBAColor iVeryDarkRedTT = {64, 0, 0, 192};
		constexpr SRGBAColor iVeryDarkRedHT = {64, 0, 0, 128};
		constexpr SRGBAColor iVeryDarkRedQT = {64, 0, 0, 64};
		constexpr SRGBAColor iVeryDarkRedFT = {64, 0, 0, 0};

		constexpr SRGBAColor iGreen = {0, 255, 0, 255};
		constexpr SRGBAColor iGreenTT = {0, 255, 0, 192};
		constexpr SRGBAColor iGreenHT = {0, 255, 0, 128};
		constexpr SRGBAColor iGreenQT = {0, 255, 0, 64};
		constexpr SRGBAColor iGreenFT = {0, 255, 0, 0};

		constexpr SRGBAColor iDarkGreen = {0, 128, 0, 255};
		constexpr SRGBAColor iDarkGreenTT = {0, 128, 0, 192};
		constexpr SRGBAColor iDarkGreenHT = {0, 128, 0, 128};
		constexpr SRGBAColor iDarkGreenQT = {0, 128, 0, 64};
		constexpr SRGBAColor iDarkGreenFT = {0, 128, 0, 0};

		constexpr SRGBAColor iVeryDarkGreen = {0, 64, 0, 255};
		constexpr SRGBAColor iVeryDarkGreenTT = {0, 64, 0, 192};
		constexpr SRGBAColor iVeryDarkGreenHT = {0, 64, 0, 128};
		constexpr SRGBAColor iVeryDarkGreenQT = {0, 64, 0, 64};
		constexpr SRGBAColor iVeryDarkGreenFT = {0, 64, 0, 0};

		constexpr SRGBAColor iBlue = {0, 0, 255, 255};
		constexpr SRGBAColor iBlueTT = {0, 0, 255, 192};
		constexpr SRGBAColor iBlueHT = {0, 0, 255, 128};
		constexpr SRGBAColor iBlueQT = {0, 0, 255, 64};
		constexpr SRGBAColor iBlueFT = {0, 0, 255, 0};

		constexpr SRGBAColor iDarkBlue = {0, 0, 128, 255};
		constexpr SRGBAColor iDarkBlueTT = {0, 0, 128, 192};
		constexpr SRGBAColor iDarkBlueHT = {0, 0, 128, 128};
		constexpr SRGBAColor iDarkBlueQT = {0, 0, 128, 64};
		constexpr SRGBAColor iDarkBlueFT = {0, 0, 128, 0};

		constexpr SRGBAColor iVeryDarkBlue = {0, 0, 64, 255};
		constexpr SRGBAColor iVeryDarkBlueTT = {0, 0, 64, 192};
		constexpr SRGBAColor iVeryDarkBlueHT = {0, 0, 64, 128};
		constexpr SRGBAColor iVeryDarkBlueQT = {0, 0, 64, 64};
		constexpr SRGBAColor iVeryDarkBlueFT = {0, 0, 64, 0};

		constexpr SRGBAColor iMagenta = {255, 0, 255, 255};
		constexpr SRGBAColor iMagentaTT = {255, 0, 255, 192};
		constexpr SRGBAColor iMagentaHT = {255, 0, 255, 128};
		constexpr SRGBAColor iMagentaQT = {255, 0, 255, 64};
		constexpr SRGBAColor iMagentaFT = {255, 0, 255, 0};

		constexpr SRGBAColor iDarkMagenta = {128, 0, 128, 255};
		constexpr SRGBAColor iDarkMagentaTT = {128, 0, 128, 192};
		constexpr SRGBAColor iDarkMagentaHT = {128, 0, 128, 128};
		constexpr SRGBAColor iDarkMagentaQT = {128, 0, 128, 64};
		constexpr SRGBAColor iDarkMagentaFT = {128, 0, 128, 0};

		constexpr SRGBAColor iVeryDarkMagenta = {64, 0, 64, 255};
		constexpr SRGBAColor iVeryDarkMagentaTT = {64, 0, 64, 192};
		constexpr SRGBAColor iVeryDarkMagentaHT = {64, 0, 64, 128};
		constexpr SRGBAColor iVeryDarkMagentaQT = {64, 0, 64, 64};
		constexpr SRGBAColor iVeryDarkMagentaFT = {64, 0, 64, 0};

		constexpr SRGBAColor iYellow = {255, 255, 0, 255};
		constexpr SRGBAColor iYellowTT = {255, 255, 0, 192};
		constexpr SRGBAColor iYellowHT = {255, 255, 0, 128};
		constexpr SRGBAColor iYellowQT = {255, 255, 0, 64};
		constexpr SRGBAColor iYellowFT = {255, 255, 0, 0};

		constexpr SRGBAColor iDarkYellow = {128, 128, 0, 255};
		constexpr SRGBAColor iDarkYellowTT = {128, 128, 0, 192};
		constexpr SRGBAColor iDarkYellowHT = {128, 128, 0, 128};
		constexpr SRGBAColor iDarkYellowQT = {128, 128, 0, 64};
		constexpr SRGBAColor iDarkYellowFT = {128, 128, 0, 0};

		constexpr SRGBAColor iVeryDarkYellow = {128, 128, 0, 255};
		constexpr SRGBAColor iVeryDarkYellowTT = {128, 128, 0, 192};
		constexpr SRGBAColor iVeryDarkYellowHT = {128, 128, 0, 128};
		constexpr SRGBAColor iVeryDarkYellowQT = {128, 128, 0, 64};
		constexpr SRGBAColor iVeryDarkYellowFT = {128, 128, 0, 0};

		constexpr SRGBAColor iCyan = {0, 255, 255, 255};
		constexpr SRGBAColor iCyanTT = {0, 255, 255, 192};
		constexpr SRGBAColor iCyanHT = {0, 255, 255, 128};
		constexpr SRGBAColor iCyanQT = {0, 255, 255, 64};
		constexpr SRGBAColor iCyanFT = {0, 255, 255, 0};

		constexpr SRGBAColor iDarkCyan = {0, 128, 128, 255};
		constexpr SRGBAColor iDarkCyanTT = {0, 128, 128, 192};
		constexpr SRGBAColor iDarkCyanHT = {0, 128, 128, 128};
		constexpr SRGBAColor iDarkCyanQT = {0, 128, 128, 64};
		constexpr SRGBAColor iDarkCyanFT = {0, 128, 128, 0};

		constexpr SRGBAColor iVeryDarkCyan = {0, 64, 64, 255};
		constexpr SRGBAColor iVeryDarkCyanTT = {0, 64, 64, 192};
		constexpr SRGBAColor iVeryDarkCyanHT = {0, 64, 64, 128};
		constexpr SRGBAColor iVeryDarkCyanQT = {0, 64, 64, 64};
		constexpr SRGBAColor iVeryDarkCyanFT = {0, 64, 64, 0};

		constexpr SRGBAColor iOrange = {255, 128, 0, 255};
		constexpr SRGBAColor iOrangeTT = {255, 128, 0, 192};
		constexpr SRGBAColor iOrangeHT = {255, 128, 0, 128};
		constexpr SRGBAColor iOrangeQT = {255, 128, 0, 64};
		constexpr SRGBAColor iOrangeFT = {255, 128, 0, 0};

		constexpr SRGBAColor iDarkOrange = {192, 92, 0, 255};
		constexpr SRGBAColor iDarkOrangeTT = {192, 92, 0, 192};
		constexpr SRGBAColor iDarkOrangeHT = {192, 92, 0, 128};
		constexpr SRGBAColor iDarkOrangeQT = {192, 92, 0, 64};
		constexpr SRGBAColor iDarkOrangeFT = {192, 92, 0, 0};

		constexpr SRGBAColor iBrown = {128, 64, 0, 255};
		constexpr SRGBAColor iBrownTT = {128, 64, 0, 192};
		constexpr SRGBAColor iBrownHT = {128, 64, 0, 128};
		constexpr SRGBAColor iBrownQT = {128, 64, 0, 64};
		constexpr SRGBAColor iBrownFT = {128, 64, 0, 0};

		constexpr SRGBAColor iDarkBrown = {64, 32, 0, 255};
		constexpr SRGBAColor iDarkBrownTT = {64, 32, 0, 192};
		constexpr SRGBAColor iDarkBrownHT = {64, 32, 0, 128};
		constexpr SRGBAColor iDarkBrownQT = {64, 32, 0, 64};
		constexpr SRGBAColor iDarkBrownFT = {64, 32, 0, 0};
	}
};