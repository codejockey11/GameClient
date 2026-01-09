#pragma once

#include "framework.h"

#include "../GameCommon/CString.h"

class CWindow
{
public:

	HINSTANCE m_hInst;

	HWND m_hWnd;

	int32_t m_width;
	int32_t m_height;

	CString* m_class;
	CString* m_title;

	WNDCLASSEXW m_wcex;

	CWindow();
	~CWindow();

	bool HandleMessage(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);
	bool InitInstance(const char* title, int32_t width, int32_t height, int32_t posx, int32_t posy);
	void Register(HINSTANCE hInstance, WNDPROC wndProc, const char* className, int32_t menu);
};

/*

Override HandleMessage in a polymorphed object to handle messages in a CWindow container
and call that funtion from the exe's WndProc function defined for the object.
Now one can define as many windows as needed without cluttering the main exe.

.h file

CWindowPoly : public CWindow
{
public:

	bool HandleMessage(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);
};

.cpp file

bool CWindowPoly::HandleMessage(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_CREATE:
	{
		return true;
	}
	}

	return false;
}

main exe

CWindowPoly* m_window = new CWindowPoly();

hInstance is from mWinMain

m_window->Register(hInstance, PolyProc, "GameClass", NULL);
m_window->InitInstance("Game Window", 1440, 900, 0, 0);

LRESULT CALLBACK PolyProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
{
	if (m_window->HandleMessage(hWnd, message, wParam, lParam))
	{
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
*/