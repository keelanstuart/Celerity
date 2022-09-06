// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once


#include <glm/glm.hpp>

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

#pragma pack(push, 1)
		struct SRGBColor
		{
			uint8_t r, g, b;
		};

		struct SRGBAColor
		{
			uint8_t r, g, b, a;
		};
#pragma pack(pop)

		constexpr SRGBAColor iWhite = {255, 255, 255, 255};
		constexpr SRGBAColor iWhiteTT = {255, 255, 255, 192};
		constexpr SRGBAColor iWhiteHT = {255, 255, 255, 128};
		constexpr SRGBAColor iWhiteQT = {255, 255, 255, 64};
		constexpr SRGBAColor iWhiteFT = {255, 255, 255, 0};
		constexpr SRGBAColor iBlack = {0, 0, 0, 255};
		constexpr SRGBAColor iBlackTT = {0, 0, 0, 192};
		constexpr SRGBAColor iBlackHT = {0, 0, 0, 128};
		constexpr SRGBAColor iBlackQT = {0, 0, 0, 64};
		constexpr SRGBAColor iBlackFT = {0, 0, 0, 0};
		constexpr SRGBAColor iGrey = {128, 128, 128, 255};
		constexpr SRGBAColor iLightGrey = {192, 192, 192, 255};
		constexpr SRGBAColor iDarkGrey = {64, 64, 64, 255};
		constexpr SRGBAColor iVeryDarkGrey = {16, 16, 16, 255};
		constexpr SRGBAColor iRed = {255, 0, 0, 255};
		constexpr SRGBAColor iDarkRed = {128, 0, 0, 255};
		constexpr SRGBAColor iVeryDarkRed = {64, 0, 0, 255};
		constexpr SRGBAColor iGreen = {0, 255, 0, 255};
		constexpr SRGBAColor iDarkGreen = {0, 128, 0, 255};
		constexpr SRGBAColor iVeryDarkGreen = {0, 64, 0, 255};
		constexpr SRGBAColor iBlue = {0, 0, 255, 255};
		constexpr SRGBAColor iDarkBlue = {0, 0, 128, 255};
		constexpr SRGBAColor iVeryDarkBlue = {0, 0, 64, 255};
		constexpr SRGBAColor iMagenta = {255, 0, 255, 255};
		constexpr SRGBAColor iDarkMagenta = {128, 0, 128, 255};
		constexpr SRGBAColor iYellow = {255, 255, 0, 255};
		constexpr SRGBAColor iDarkYellow = {128, 128, 0, 255};
		constexpr SRGBAColor iCyan = {0, 255, 255, 255};
		constexpr SRGBAColor iDarkCyan = {0, 128, 128, 255};
	}
};