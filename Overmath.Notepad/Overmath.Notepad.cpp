// Overmath.Notepad.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../Overmath/overmath.h"
#include "Overmath.Notepad.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;
HWND hInput, hOutput;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(hInstance, IDC_OVERMATHNOTEPAD, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  if (!InitInstance(hInstance, nCmdShow))
  {
    return FALSE;
  }

  HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OVERMATHNOTEPAD));

  MSG msg;

  while (GetMessage(&msg, nullptr, 0, 0))
  {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return static_cast<int>(msg.wParam);
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
  WNDCLASSEXW wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OVERMATHNOTEPAD));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_OVERMATHNOTEPAD);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

  return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  hInst = hInstance; // Store instance handle in our global variable

  HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

  if (!hWnd)
  {
    return FALSE;
  }

  RECT rcClient;
  GetClientRect(hWnd, &rcClient);

  hInput = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""),
    WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_LEFT,
    0, 0, rcClient.right / 2, rcClient.bottom,
    hWnd, nullptr, hInstance, nullptr);

  if (!hInput) return FALSE;

  hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""),
    WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_LEFT,
    rcClient.right / 2, 0, rcClient.right / 2, rcClient.bottom,
    hWnd, nullptr, hInstance, nullptr);

  if (!hOutput) return FALSE;

  auto hFont = CreateFont(-24, 0, 0, 0, FW_DONTCARE, 0,
    0, 0, ANSI_CHARSET, 0, 0, 0, FIXED_PITCH, TEXT("Consolas"));
  SendMessage(hInput, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), 0);
  SendMessage(hOutput, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), 0);

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_COMMAND:
  {
    int wmId = LOWORD(wParam);
    auto hi = HIWORD(wParam);

    if (hi == EN_CHANGE && lParam == reinterpret_cast<LPARAM>(hInput))
    {
      auto len = GetWindowTextLength(hInput) + 1;
      std::wstring input(len, 0);
      GetWindowText(hInput, &input[0], len);
      auto result = overmath::parse(begin(input), end(input));
      SetWindowText(hOutput, result.c_str());
    }

    // Parse the menu selections:
    switch (wmId)
    {
    case IDM_ABOUT:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
      break;
    case IDM_EXIT:
      DestroyWindow(hWnd);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  }
  break;
  case WM_SIZE:
  {
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);
    
    SetWindowPos(hInput, nullptr, 0, 0, rcClient.right / 2, rcClient.bottom, 0);
    SetWindowPos(hOutput, nullptr, rcClient.right / 2, 0, rcClient.right / 2, rcClient.bottom, 0);
  }
  break;
  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    // TODO: Add any drawing code that uses hdc here...
    EndPaint(hWnd, &ps);
  }
  break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(lParam);
  switch (message)
  {
  case WM_INITDIALOG:
    return static_cast<INT_PTR>(TRUE);

  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
    {
      EndDialog(hDlg, LOWORD(wParam));
      return static_cast<INT_PTR>(TRUE);
    }
    break;
  }
  return static_cast<INT_PTR>(FALSE);
}
