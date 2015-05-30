#include "solarize.h"
#include "resource.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <windows.h>

const char g_szClassName[] = "solarizeWindowClass";
struct img_t {
  unsigned char *pchData;
  int iWidth;
  int iHeight;
  int iChannels;
  char szName[MAX_PATH];
  size_t histogram[NCOLORS][MAX_CHANNELS];
} g_RawImage;

struct curr_img_t {
  unsigned char *pchData;
  int iChannels;
  int iSmoothWindow;
  int iLinThreshold;
  bool iInvert;
  size_t histogram[NCOLORS][MAX_CHANNELS];
} g_CurrentImage;

// step 4: the window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_COMMAND:
    // handle menu presses
    switch(LOWORD(wParam)) {
    case ID_FILE_OPEN: {
      // TODO: Refactor this open dialogue...
      char szOpenFile[MAX_PATH] = "";
      OPENFILENAME ofn;
      ZeroMemory(&ofn, sizeof(ofn));
      ofn.lStructSize = sizeof(ofn);
      ofn.hwndOwner = hwnd;
      ofn.lpstrFilter = "Image Files (*.jpg)\0*.jpg\0All Files (*.*)\0*.*\0";
      ofn.lpstrFile = szOpenFile;
      ofn.nMaxFile = MAX_PATH;
      ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
      ofn.lpstrDefExt = "jpg";

      if (GetOpenFileName(&ofn)) {
	int iWidth, iHeight, iChannels;
	unsigned char *pchLoaded = stbi_load(szOpenFile, &iWidth, &iHeight,
					     &iChannels, 0);
	if (pchLoaded == NULL) {
	  // failed to load the image
	  HINSTANCE hInstance = GetModuleHandle(NULL);
	  MessageBox(hwnd, szOpenFile,
		     "Invalid image:", MB_OK | MB_ICONEXCLAMATION);
	} else {
	  int i;
	  // dump image into the struct
	  if (g_RawImage.pchData != NULL) {
	    stbi_image_free(g_RawImage.pchData);
	  }

	  // compute histograms for each channel
	  for (i = 0; i < iChannels; i++) {
	    build_histogram(pchLoaded, iWidth * iHeight,
			    iChannels, i, g_RawImage.histogram[i]);
	  }

	  // set current image to be displayed
	  // free + malloc if we need to increase size, realloc if decrease
	  if (g_RawImage.iWidth * g_RawImage.iHeight * g_RawImage.iChannels <
	      iChannels * iWidth * iHeight) {
	    free(g_CurrentImage.pchData);
	    g_CurrentImage.pchData = malloc(iChannels * iWidth * iHeight);
	  } else {
	    realloc(g_CurrentImage.pchData, iChannels * iWidth * iHeight);
	  }
	  g_CurrentImage.iChannels = iChannels;
	  g_CurrentImage.iInvert = true;
	  g_CurrentImage.iSmoothWindow = DEFAULT_SMOOTH_WINDOW;
	  g_CurrentImage.iLinThreshold = DEFAULT_LIN_THRESHOLD;

	  // save original file data to speed up further changes
	  g_RawImage.pchData = pchLoaded;
	  g_RawImage.iWidth = iWidth;
	  g_RawImage.iHeight = iHeight;
	  g_RawImage.iChannels = iChannels;
	  strcpy(g_RawImage.szName, szOpenFile);

	  // TODO: Refactor this bit
	  for (i = 0; i < iChannels; i++) {
	    // copy & mod histogram
	    smooth_histogram(g_RawImage.histogram[i],
			     g_CurrentImage.iSmoothWindow,
			     g_CurrentImage.histogram[i]);
	  
	    // copy and solarize image data
	    // TODO: Remove this memcpy by refactoring solarize.c
	    memcpy(g_CurrentImage.pchData, g_RawImage.pchData,
		   iWidth * iHeight * iChannels);
	    solarize_channel(g_CurrentImage.histogram[i],
			     g_CurrentImage.pchData,
			     g_RawImage.iWidth * g_RawImage.iHeight,
			     g_RawImage.iChannels, i,
			     g_CurrentImage.iLinThreshold,
			     g_CurrentImage.iInvert);
	  }

	  // populate the bitmap
	  // TODO
	  
	  // display the solarized image
	  PostMessage(hwnd, WM_PAINT, 0, 0);
	}
      }
      break;
    }
    case ID_FILE_SAVE: {
      OPENFILENAME ofn;
      char szOpenFile[MAX_PATH] = "";
      ZeroMemory(&ofn, sizeof(ofn));
      ofn.lStructSize = sizeof(ofn);
      ofn.hwndOwner = hwnd;
      ofn.lpstrFilter = "Image Files (*.png)\0*.png\0All Files (*.*)\0*.*\0";
      ofn.lpstrFile = szOpenFile;
      ofn.nMaxFile = MAX_PATH;
      ofn.Flags = (OFN_EXPLORER | OFN_PATHMUSTEXIST |
		   OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT);
      ofn.lpstrDefExt = "png";

      if (GetSaveFileName(&ofn)) {
	int rc = stbi_write_png(szOpenFile,
				g_RawImage.iWidth, g_RawImage.iHeight,
				g_CurrentImage.iChannels,
				g_CurrentImage.pchData,
				g_RawImage.iWidth * g_CurrentImage.iChannels);
	// returns 0 on failure wtf?
	if (rc == 0) {
	  HINSTANCE hInstance = GetModuleHandle(NULL);
	  MessageBox(hwnd, szOpenFile,
		     "Failed to save to:", MB_OK | MB_ICONEXCLAMATION);
	} else {
	  HINSTANCE hInstance = GetModuleHandle(NULL);
	  MessageBox(hwnd, szOpenFile,
		     "Successfully saved to:", MB_OK | MB_ICONINFORMATION);
	}
      }
      break;
    }
    case ID_FILE_EXIT:
      PostMessage(hwnd, WM_CLOSE, 0, 0);
      break;
    }
    break;
  case WM_PAINT: {
    // draw the solarized image
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    // TODO: Figure out GDI & how to use BitBlt here to draw the bitmap
    EndPaint(hwnd, &ps);
    break;
  }
  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
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
