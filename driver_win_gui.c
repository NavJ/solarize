#include "resource.h"

#include <windows.h>

const char g_szClassName[] = "solarizeWindowClass";

// step 4: the window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_COMMAND:
    // handle menu presses
    switch(LOWORD(wParam)) {
    case ID_FILE_OPEN:
      break;
    case ID_FILE_EXIT:
      PostMessage(hwnd, WM_CLOSE, 0, 0);
      break;
    }
    break;
  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  case WM_LBUTTONDOWN: {
    char szFileName[MAX_PATH];
    HINSTANCE hInstance = GetModuleHandle(NULL);
    GetModuleFileName(hInstance, szFileName, MAX_PATH);
    MessageBox(hwnd, szFileName, "This Program is:", MB_OK | MB_ICONINFORMATION);
  }
  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   LPSTR lpCmdLine, int nCmdShow) {
  WNDCLASSEX wc;
  HWND hwnd;
  MSG Msg;

  // step 1: register window class
  wc.cbSize        = sizeof(WNDCLASSEX);
  wc.style         = 0;
  wc.lpfnWndProc   = WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = g_szClassName;
  wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
  // add menu
  wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
  
  if (!RegisterClassEx(&wc)) {
    MessageBox(NULL, "Window Reg failed!", "ERROR!", MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }

  // step 2: make the window
  hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
			g_szClassName,
			"Window Title",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
			NULL, NULL, hInstance, NULL);
  if (hwnd == NULL) {
    MessageBox(NULL, "Window creation failed!", "ERROR!", MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  // step 3: message loop
  while (GetMessage(&Msg, NULL, 0, 0) > 0) {
    TranslateMessage(&Msg);
    DispatchMessage(&Msg);
  }
  
  return Msg.wParam;
}
