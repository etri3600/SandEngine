#include "SWindows.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
 
LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool SWindows::Init()
{
	mhInstance = GetModuleHandle(0);

	//Register Framework
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = mhInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = L"Menu";
	wcex.lpszClassName = L"Class";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (RegisterClassEx(&wcex) == 0)
	{
		OutputErrorMessage(S_FALSE);
	}
	//End Register Framework

	//Init Window Handler
	mhWnd = CreateWindow(L"Class", L"Windows", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, mhInstance, NULL);

	if (!mhWnd)
	{
		OutputErrorMessage(S_FALSE);
		return false;
	}
	//End Init Window Handler

	ShowWindow(mhWnd, SW_SHOWDEFAULT);
	UpdateWindow(mhWnd);

	return true;
}

bool SWindows::Tick()
{
	MSG msg;

	// Main message loop:
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT)
		return false;
	
	return true;
}

void SWindows::OutputErrorMessage(HRESULT hr)
{
	if (FAILED(hr))
	{
		DWORD error = GetLastError();
		LPVOID lpMsgBuf;
		WCHAR ErrorString[256];

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			(LPTSTR)&lpMsgBuf,
			0, NULL);


		wsprintfW(ErrorString, TEXT("%s"), lpMsgBuf);
		OutputDebugString(ErrorString);

#if defined(_DEBUG)
		DebugBreak();
#endif
	}
}

LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		break;
	case WM_KEYDOWN:
		break;
	case WM_KEYUP:
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}