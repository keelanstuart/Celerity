// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#include "pch.h"
#include <timeapi.h>
#include "resource.h"

#include <stb_image.h>

#define NOMINMAX
#include <algorithm>
namespace Gdiplus
{
	using std::min;
	using std::max;
};

#include <gdiplus.h>
#include <gdiplusimaging.h>
#include <gdiplusgraphics.h>


HMODULE g_C3Mod = NULL;

Gdiplus::Bitmap *g_SplashImage = nullptr;
int g_SplashImageW = 0, g_SplashImageH = 0;
HWND g_hPostSplashWnd = NULL;


INT_PTR CALLBACK SplashDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;

	if (uMsg == WM_PAINT)
	{
		Gdiplus::Graphics gfx(GetDC(hwnd));
		gfx.DrawImage(g_SplashImage, Gdiplus::Point(0, 0));
		ret = TRUE;
	}
	else if (uMsg != WM_ERASEBKGND)
	{
		ret = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return ret;
}

typedef bool (WINAPI *lpfnSLWA) (HWND hWnd, COLORREF cr, 
	BYTE bAlpha, DWORD dwFlags);

#define SPLASH_TIME		4000

DWORD WINAPI SplashThreadProc(LPVOID lpParameter)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	// Initialize GDI+.
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	// Checking return status from GdiplusStartup 
	Gdiplus::Status status = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	if (status != Gdiplus::Ok)
		return 0;

	HMODULE huser32 = GetModuleHandle(_T("USER32.DLL"));
	lpfnSLWA gSetLayeredWindowAttributes = (lpfnSLWA)GetProcAddress(huser32, "SetLayeredWindowAttributes");

	HRSRC hres = ::FindResource(g_C3Mod, MAKEINTRESOURCE(IDB_PNG_SPLASH), L"PNG");
	if (!hres)
		return 0;

	HGLOBAL hglob = LoadResource(g_C3Mod, hres);
	if (!hglob)
		return 0;

	LPVOID buf = ::LockResource(hglob);
	if (!buf)
	{
		FreeResource(hglob);
		return 0;
	}

	int numchans = 0;
	stbi_uc *pimg = stbi_load_from_memory((const stbi_uc *)buf, ::SizeofResource(g_C3Mod, hres), &g_SplashImageW, &g_SplashImageH, &numchans, 0);

	// evidently the BITMAP is in BGRA format...? Swap r and b
	struct SPixelRGBA
	{
		uint8_t r, g, b, a;
	} *pix = (SPixelRGBA *)pimg;

	g_SplashImage = new Gdiplus::Bitmap(g_SplashImageW, g_SplashImageH, PixelFormat32bppARGB);

	Gdiplus::BitmapData dst;
	g_SplashImage->LockBits(nullptr, Gdiplus::ImageLockMode::ImageLockModeWrite, PixelFormat32bppARGB, &dst);
	for (size_t y = 0; y < g_SplashImageH; y++)
	{
		struct SPixelABGR
		{
			uint8_t b, g, r, a;
		} *_pix = (SPixelABGR *)((char *)dst.Scan0 + (dst.Stride * y));

		for (size_t x = 0; x < g_SplashImageW; x++)
		{
			_pix[x].r = pix->r;
			_pix[x].g = pix->g;
			_pix[x].b = pix->b;
			_pix[x].a = pix->a;//(pix->a != 255) ? pix->a : (((pix->r == 255) && (pix->g == 0) && (pix->b == 255)) ? 0 : 255);
			pix++;
		}
	}
	g_SplashImage->UnlockBits(&dst);

	UnlockResource(hglob);
	FreeResource(hglob);

	HWND splashwnd = CreateDialog(g_C3Mod, MAKEINTRESOURCE(IDD_DIALOG_SPLASH), NULL, SplashDlgProc);

	if (splashwnd)
	{
		::SetWindowLong(splashwnd, GWL_EXSTYLE, ::GetWindowLong(splashwnd, GWL_EXSTYLE) | WS_EX_LAYERED);

		if (gSetLayeredWindowAttributes)
		{
			SetLayeredWindowAttributes(splashwnd, RGB(255, 0, 255), 0, LWA_COLORKEY);
			//gSetLayeredWindowAttributes(splashwnd, 0, 0, LWA_ALPHA);
		}

		//::SetWindowLong(splashwnd, GWL_STYLE, ::GetWindowLong(splashwnd, GWL_STYLE) | WS_VISIBLE);
		ShowWindow(splashwnd, SW_SHOWNOACTIVATE);

		MSG Message;
		int32_t time_start = timeGetTime();
		int32_t time_current = time_start;
		int32_t time_elapsed = 0;
		do
		{
			while (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
			{
				// Pass the message along...
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}

			if (gSetLayeredWindowAttributes)
			{
				float pct = (float)time_elapsed / (float)SPLASH_TIME;
				float sine = std::min<float>(std::max<float>(0.0f, (sin(pct * 3.14159f) * 2.5f)), 1.0f);
				uint32_t opacity = (uint32_t)(sine * 255.0f);
				//gSetLayeredWindowAttributes(splashwnd, 0, opacity, LWA_ALPHA);
				RedrawWindow(splashwnd, 0, 0, RDW_UPDATENOW);
			}

			Sleep(1);

			time_current = timeGetTime();

			time_elapsed = time_current - time_start;

		} while (time_elapsed < SPLASH_TIME);

		if (g_hPostSplashWnd)
			BringWindowToTop(g_hPostSplashWnd);

		DestroyWindow(splashwnd);
	}

	delete g_SplashImage;
	g_SplashImage = nullptr;

	Gdiplus::GdiplusShutdown(gdiplusToken);

	CoUninitialize();

	return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	g_C3Mod = hModule;

	switch (ul_reason_for_call)
    {
		case DLL_PROCESS_ATTACH:
#if 1 //!defined(_DEBUG)
			CreateThread(NULL, USHRT_MAX, SplashThreadProc, NULL, 0, NULL);
#endif
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			break;
    }

	return TRUE;
}

