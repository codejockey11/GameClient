#pragma once

#include "framework.h"

#include "CWindow.h"

class CWindowP : public CWindow
{
public:

    bool HandleMessage(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);
};