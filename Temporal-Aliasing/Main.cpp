//*****************************************************************************
//
// Main.cpp : Defines the entry point for the application.
// Creates a white RGB image with a black line and displays it.
// Two images are displayed on the screen.
// Left Pane: Input Image, Right Pane: Modified Image
//
//*****************************************************************************


// Include class files
#include "Image.h"
#include <iostream>

#include <mmsystem.h>

#define PI 3.14159265358979323846

#define ORIGIN_FPS 30

#define MAX_LOADSTRING 100

// Global Variables:
MyImage			inImage, outImage;				// image objects
HINSTANCE		hInst;							// current instance
HWND            gWnd;                           // Main window handle
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// The title bar text

MMRESULT timerID;
MMRESULT samplingTimerID;
float currentAngle = 0.0;
float unitAngle = 0.0;
int numOfLine = 0;
float speedOfRotation = 0.0;
int currentSec = 0;
float fps = 0.0;
int delay = 0;
int samplingDelay = 0;


// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void 				ColorPixel(char* imgBuf, int w, int h, int x, int y, byte r = 0, byte g = 0, byte b = 0);
void				DrawLine(char* imgBuf, int w, int h, int x1, int y1, int x2, int y2, byte r = 0, byte g = 0, byte b = 0);
void				DrawRadial(char* imgData, int width, int height, float currentAngle, int numOfLine);
void CALLBACK		TimeProcForOrigin(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
void CALLBACK		TimeProcForSampling(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);


// Main entry point for a windows application
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	// Read in a parameter from the command line
	sscanf(lpCmdLine, "%d %f %f", &numOfLine, &speedOfRotation, &fps);
	
	// Create a separate console window to display output to stdout
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	printf("The first parameter was: %d\nThe second parameter was: %f\nThe third parameter was: %f\n", numOfLine, speedOfRotation, fps);
	
	// Set up the images
	int w = 512;
	int h = 512;
	inImage.setWidth(w);
	inImage.setHeight(h);

	outImage.setWidth(w);
	outImage.setHeight(h);

	char* imgData = new char[w * h * 4];
	memset(imgData, 0xFF, sizeof(char) * w * h * 4);
	
	inImage.setImageData(imgData);

	char* imgData2 = new char[w * h * 4];
	memcpy(imgData2, imgData, sizeof(char) * w * h * 4);
	outImage.setImageData(imgData2);

	//timeBeginPeriod(1);
	delay = (int)(1000 / ORIGIN_FPS);
	unitAngle = ((360.0 * speedOfRotation) / ORIGIN_FPS);
	samplingDelay = (int)(1000 / fps);
	timerID = timeSetEvent(delay, 1, TimeProcForOrigin, 0, TIME_PERIODIC);
	samplingTimerID = timeSetEvent(samplingDelay, 1, TimeProcForSampling, 0, TIME_PERIODIC);
	 
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_IMAGE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_IMAGE);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

// Colors a pixel at the given (x, y) position black.
void ColorPixel(char* imgBuf, int w, int h, int x, int y, byte r, byte g, byte b)
{
	imgBuf[(4 * y * w) +  4 * x] = b;
	imgBuf[(4 * y * w) +  4 * x + 1] = g;
	imgBuf[(4 * y * w) +  4 * x + 2] = r;
}

// Draws a line using Bresenham's algorithm.
void DrawLine(char* imgBuf, int w, int h, int x1, int y1, int x2, int y2, byte r, byte g, byte b)
{
	const bool steep = (abs(y2 - y1) > abs(x2 - x1));
	if (steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}

	if (x1 > x2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	const float dx = x2 - x1;
	const float dy = fabs((float)(y2 - y1));

	float error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	int y = (int)y1;

	const int maxX = (int)x2;

	for (int x = (int)x1; x<maxX; x++)
	{
		if (steep)
		{
			ColorPixel(imgBuf, w, h, y, x, r, g, b);
		}
		else
		{
			ColorPixel(imgBuf, w, h, x, y, r, g, b);
		}

		error -= dy;
		if (error < 0)
		{
			y += ystep;
			error += dx;
		}
	}
}

void DrawRadial(char* imgData, int width, int height, float currentAngle, int numOfLine) {
	int x1 = width / 2, y1 = height / 2;
	int x2 = width - 1, y2 = y1;
	for (int i = 0; i < numOfLine; i++) {
		float angle = currentAngle + (360.0 / numOfLine) * (float)i;
		if (360.0 < angle) {
			currentAngle -= 360.0;
		}

		float degree = angle * (PI / 180.0);

		int x = ceil(cos(degree) * (x2 - x1) - sin(degree) * (y2 - y1) + x1);
		int y = ceil(sin(degree) * (x2 - x1) + cos(degree) * (y2 - y1) + y1);

		if (0 < i) {
			DrawLine(imgData, width, height, x1, y1, x, y);
		}
		else {
			DrawLine(imgData, width, height, x1, y1, x, y, 0xFF, 0, 0xFF);
		}
	}
}

void CALLBACK TimeProcForOrigin(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2) {
	
	char* imgData = inImage.getImageData();
	memset(imgData, 0xFF, sizeof(char) * inImage.getWidth() * inImage.getHeight() * 4);
	DrawRadial(inImage.getImageData(), inImage.getWidth(), inImage.getHeight(), currentAngle, numOfLine);

	currentAngle += unitAngle;
	currentAngle -= (360.0 < currentAngle ? 360.0 : 0.0);

	RECT rt;
	rt.top = 100;
	rt.left = 100;
	rt.bottom = rt.top + inImage.getHeight();
	rt.right = rt.left + inImage.getWidth();

	//GetClientRect(gWnd, &rt);
	InvalidateRect(gWnd, &rt, false);
}

void CALLBACK TimeProcForSampling(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2) {
	char* imgData = inImage.getImageData();
	char* imgBuf = outImage.getImageData();
	memcpy(imgBuf, imgData, sizeof(char) * inImage.getWidth() * inImage.getHeight() * 4);

	RECT rt;
	rt.top = 100;
	rt.left = 150 + inImage.getWidth();
	rt.bottom = rt.top + inImage.getHeight();
	rt.right = rt.left + inImage.getWidth();

	//GetClientRect(gWnd, &rt);
	InvalidateRect(gWnd, &rt, false);
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_IMAGE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_IMAGE;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}


//
//   FUNCTION: InitInstance(HANDLE, int)
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
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   gWnd = hWnd;
   
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
// TO DO: part useful to render video frames, may place your own code here in this function
// You are free to change the following code in any way in order to display the video

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR szHello[MAX_LOADSTRING];
	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);
	RECT rt;
	GetClientRect(hWnd, &rt);
	
	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case ID_MODIFY_IMAGE:
				   outImage.Modify();
				   InvalidateRect(hWnd, &rt, false); 
				   break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			{
				hdc = BeginPaint(hWnd, &ps);
				// TO DO: Add any drawing code here...
				if (100 == ps.rcPaint.left) {

					BITMAPINFO bmi;
					// CBitmap bitmap;
					memset(&bmi, 0, sizeof(bmi));
					bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
					bmi.bmiHeader.biWidth = inImage.getWidth();
					bmi.bmiHeader.biHeight = -inImage.getHeight();  // Use negative height.  DIB is top-down.
					bmi.bmiHeader.biPlanes = 1;
					bmi.bmiHeader.biBitCount = 32;
					bmi.bmiHeader.biCompression = BI_RGB;
					bmi.bmiHeader.biSizeImage = inImage.getWidth()*inImage.getHeight();

					SetDIBitsToDevice(hdc,
						100, 100, inImage.getWidth(), inImage.getHeight(),
						0, 0, 0, inImage.getHeight(),
						inImage.getImageData(), &bmi, DIB_RGB_COLORS);
				} else if (inImage.getWidth() + 150 == ps.rcPaint.left) {
					BITMAPINFO bmi;
					// CBitmap bitmap;

					memset(&bmi, 0, sizeof(bmi));
					bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
					bmi.bmiHeader.biWidth = outImage.getWidth();
					bmi.bmiHeader.biHeight = -outImage.getHeight();  // Use negative height.  DIB is top-down.
					bmi.bmiHeader.biPlanes = 1;
					bmi.bmiHeader.biBitCount = 32;
					bmi.bmiHeader.biCompression = BI_RGB;
					bmi.bmiHeader.biSizeImage = outImage.getWidth()*outImage.getHeight();

					SetDIBitsToDevice(hdc,
						inImage.getWidth() + 150, 100, outImage.getWidth(), outImage.getHeight(),
						0, 0, 0, outImage.getHeight(),
						outImage.getImageData(), &bmi, DIB_RGB_COLORS);
				}

				EndPaint(hWnd, &ps);
			}
			break;
		case WM_DESTROY:
			timeKillEvent(timerID);
			timeKillEvent(samplingTimerID);
			timeEndPeriod(1);
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}




// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}


