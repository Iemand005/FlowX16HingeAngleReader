// FlowX16HingeAngleReader.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "FlowX16HingeAngleReader.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hMainWnd;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


HingeSensorReader* pHingeSensorReader;            // Hinge sensor reader instance
int hingeAngle = 0, lidAngle = 0, bodyAngle = 0;


// Forward declarations of functions included in this code module:
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

    // TODO: Place code here.
	pHingeSensorReader = new HingeSensorReader();

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FLOWX16HINGEANGLEREADER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FLOWX16HINGEANGLEREADER));

    MSG msg;

    // Main message loop:
    bool quitRequested = false;
    while (!quitRequested) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) {
                quitRequested = true;
                break;
            }
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        // Get the hinge angle from the sensor
        HRESULT hr = pHingeSensorReader->GetHingeAngle(&hingeAngle, &lidAngle, &bodyAngle);
        if (SUCCEEDED(hr)) {
            std::wcout << L"Hinge Angle: " << hingeAngle << L" Lid Angle: " << lidAngle << L" Body Angle: " << bodyAngle << std::endl;

            InvalidateRect(hMainWnd, NULL, TRUE);
            UpdateWindow(hMainWnd);

        } else {
            std::wcerr << L"Failed to get hinge angle. HRESULT: " << hr << std::endl;
        }
    }

    return (int) msg.wParam;
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

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FLOWX16HINGEANGLEREADER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_FLOWX16HINGEANGLEREADER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP hbmMem = NULL;
    static HBITMAP hbmOld = NULL;
    static HDC hdcMem;
	static int width, height;

    switch (message)
    {
    case WM_CREATE:
        {
            hMainWnd = hWnd;
            HDC hdc = GetDC(hWnd);

            hdcMem = CreateCompatibleDC(hdc);

            RECT rc;
            GetClientRect(hWnd, &rc);

            hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);

            hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

            ReleaseDC(hWnd, hdc);
        }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
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
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            if (!hbmMem) {
                RECT rcClient;
                GetClientRect(hWnd, &rcClient);
                width = rcClient.right - rcClient.left;
                height = rcClient.bottom - rcClient.top;
            
                hbmMem = CreateCompatibleBitmap(hdc, width, height);
            }

            SelectObject(hdcMem, hbmMem);

            
            static WCHAR labelBuffer[32];
            wsprintfW(labelBuffer, L"%d", hingeAngle);

            RECT rect = { 10, 10, 200, 30 };
            DrawText(hdc, labelBuffer, -1, &rect, DT_LEFT | DT_VCENTER);

            DOUBLE degrees = hingeAngle;
            DOUBLE radians = degrees * (M_PI / 180.0);

            INT xStart = width / 4;
            INT yStart = height / 2;

            INT length = min(width / 2, height) / 2;

            // Calculate end point coordinates
            INT xEnd = xStart + (INT)(length * cos(radians));
            INT yEnd = yStart - (INT)(length * sin(radians)); // Subtract because Y increases downward

            // Draw the line
            MoveToEx(hdc, xStart, yStart, NULL);

            POINT points[] = { {xStart + length, yStart}, {xStart, yStart}, {xEnd, yEnd} };
            Polyline(hdc, points, ARRAYSIZE(points));

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_SIZE:
        {
            width = LOWORD(lParam);
            height = HIWORD(lParam);

            // Delete old bitmap if exists
            if (hbmMem) DeleteObject(hbmMem);

            HDC hdc = GetDC(hWnd);
            hbmMem = CreateCompatibleBitmap(hdc, width, height);
            ReleaseDC(hWnd, hdc);

            InvalidateRect(hWnd, NULL, FALSE);
            break;
        }
    case WM_DESTROY:
        DeleteObject(hbmMem);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
