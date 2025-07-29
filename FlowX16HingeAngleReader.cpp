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
        if (FAILED(hr)) return hr;
        
        InvalidateRect(hMainWnd, NULL, TRUE);
        UpdateWindow(hMainWnd);
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

POINT CreateClockHandWithDegrees(INT degrees, POINT start, INT length)
{
    DOUBLE radians = degrees * (M_PI / 180.0);

    return { start.x + (INT)(length * cos(radians)), start.y - (INT)(length * sin(radians)) };
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

            RECT rc;
            GetClientRect(hWnd, &rc);

            FillRect(hdcMem, &rc, (HBRUSH)(COLOR_WINDOW + 1));

            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
            HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));

            HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);

            static WCHAR labelBuffer[32];
            wsprintfW(labelBuffer, L"Hinge: %d, body: %d, lid: %d", hingeAngle, bodyAngle, lidAngle);

            RECT rect = { 10, 10, 230, 30 };
            DrawText(hdcMem, labelBuffer, -1, &rect, DT_LEFT | DT_VCENTER);

            POINT hingeStart = { width / 4, height / 2 };
            POINT bodyLidStart = { width / 4 + width / 2, height / 2 };

            INT margin = 10;
            INT length = min(width / 2, height) / 2 - margin;

			POINT hingeEnd = CreateClockHandWithDegrees(hingeAngle, hingeStart, length);

            POINT hingePoints[] = { {hingeStart.x + length, hingeStart.y}, {hingeStart.x, hingeStart.y}, {hingeEnd.x, hingeEnd.y} };
            Polyline(hdcMem, hingePoints, ARRAYSIZE(hingePoints));

            POINT lidEnd = CreateClockHandWithDegrees(lidAngle, bodyLidStart, length);
            POINT bodyEnd = CreateClockHandWithDegrees(bodyAngle, bodyLidStart, length);

            POINT lidBodyPoints[] = { {bodyEnd.x, bodyEnd.y}, {bodyLidStart.x, bodyLidStart.y}, {lidEnd.x, lidEnd.y} };
            Polyline(hdcMem, lidBodyPoints, ARRAYSIZE(lidBodyPoints));


            SelectObject(hdcMem, hOldPen);
            SelectObject(hdcMem, hOldBrush);
            DeleteObject(hPen);
            DeleteObject(hBrush);

            BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_SIZE:
        {
            width = LOWORD(lParam);
            height = HIWORD(lParam);

            if (hbmMem) {
                SelectObject(hdcMem, hbmOld);
                DeleteObject(hbmMem);
            }

            HDC hdc = GetDC(hWnd);
            hbmMem = CreateCompatibleBitmap(hdc, width, height);
            hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
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
