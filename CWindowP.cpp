#include "CWindowP.h"

/*
*/
bool CWindowP::HandleMessage(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
	{
		POINT p = {};
		RECT r = {};

		GetWindowRect(hWnd, &r);
		GetCursorPos(&p);

		if (p.x >= r.left)
		{
			if (p.x <= r.right)
			{
				if (p.y >= r.top)
				{
					if (p.y <= r.bottom)
					{
						ShowCursor(TRUE);

						return true;
					}
				}
			}
		}

		return true;

		break;
	}
	}

	return false;
}
