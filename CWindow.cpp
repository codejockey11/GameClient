#include "CWindow.h"

/*
*/
CWindow::CWindow()
{
	memset(this, 0x00, sizeof(CWindow));
}

/*
*/
CWindow::~CWindow()
{
	SAFE_DELETE(m_title);
	SAFE_DELETE(m_class);

	DestroyWindow(m_hWnd);
}

/*
*/
bool CWindow::HandleMessage(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
{
	return false;
}

/*
*/
bool CWindow::InitInstance(const char* title, int32_t width, int32_t height, int32_t posx, int32_t posy)
{
	m_title = new CString(title);

	m_width = width;
	m_height = height;

	int32_t x = (GetSystemMetrics(SM_CXSCREEN) - m_width) / 2;
	int32_t y = (GetSystemMetrics(SM_CYSCREEN) - m_height) / 2;

	if (posx > 0)
	{
		x = posx;
	}

	if (posy > 0)
	{
		y = posy;
	}

	m_hWnd = CreateWindow(m_class->GetWide(), m_title->GetWide(), WS_TILEDWINDOW,
		x, y,
		m_width, m_height,
		nullptr, nullptr, m_hInst, nullptr);

	if (!m_hWnd)
	{
		return false;
	}

	SetWindowText(m_hWnd, m_title->GetWide());

	ShowWindow(m_hWnd, SW_SHOWDEFAULT);
	UpdateWindow(m_hWnd);

	return true;
}

/*
*/
void CWindow::Register(HINSTANCE hInstance, WNDPROC wndProc, const char* className, int32_t menu)
{
	m_hInst = hInstance;

	m_class = new CString(className);

	m_wcex = {};

	m_wcex.cbSize = sizeof(WNDCLASSEX);

	m_wcex.style = CS_HREDRAW | CS_VREDRAW;
	m_wcex.lpfnWndProc = wndProc;
	m_wcex.cbClsExtra = 0;
	m_wcex.cbWndExtra = 0;
	m_wcex.hInstance = m_hInst;
	m_wcex.hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_ICON1));
	m_wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	m_wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	if (menu)
	{
		m_wcex.lpszMenuName = MAKEINTRESOURCEW(menu);
	}

	m_wcex.lpszClassName = m_class->GetWide();
	m_wcex.hIconSm = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_ICON2));

	RegisterClassExW(&m_wcex);
}