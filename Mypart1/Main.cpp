//*****************************************************************************
//
// Main.cpp : Defines the entry point for the application.
// Creates a white RGB image with a black line and displays it.
// Two images are displayed on the screen.
// Left Pane: Input Image, Right Pane: Modified Image
//
//
//*****************************************************************************


// Include class files
#include "Image.h"
#include <iostream>

#define MAX_LOADSTRING 100

// Global Variables:
MyImage			inImage, outImage;				// image objects
HINSTANCE		hInst;							// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// The title bar text

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void 				ColorPixel(char* imgBuf, int w, int h, int x, int y);
void				DrawLine(char* imgBuf, int w, int h, int x1, int y1, int x2, int y2);

void CreateImage(byte* imgData, const int w, const int h, const int n) {
	const double PI = 3.14159265358979323846;
	int x1 = w / 2, y1 = h / 2;
	int x2 = w - 1, y2 = y1;
	for (int i = 0; i < n; i++) {
		float angle = (360.0 / n) * (float)i;
		float degree = angle * (PI / 180.0);

		float tempX = cos(degree) * (x2 - x1) - sin(degree) * (y2 - y1) + x1;
		float tempY = sin(degree) * (x2 - x1) + cos(degree) * (y2 - y1) + y1;

		/*float slope = (tempY - y1) / (tempX - x1);
		int x = ceil(tempX) < x1 ? 0 : w - 1;
		int y = ceil(y1 + slope * (x - x1));
		if (y < 0 || h - 1 < y) {
		slope = (tempX - x1) / (tempY - y1);
		y = (ceil(tempY) < y1 ? 0 : h - 1);
		x = ceil(x1 + slope * (y - y1));
		}*/

		float slope = (tempX - x1) / (tempY - y1);
		int y = (ceil(tempY) < y1 ? 0 : h - 1);
		int x = ceil(x1 + slope * (y - y1));
		if (x < 0 || w - 1 < x) {
			slope = (tempY - y1) / (tempX - x1);
			x = ceil(tempX) < x1 ? 0 : w - 1;
			y = ceil(y1 + slope * (x - x1));
		}

		DrawLine((char*)imgData, w, h, x1, y1, x, y);
	}
}

void ScaleDownImage(byte* imgData, int w, int h, float scale, bool isAntiAliasing) {
	if (isAntiAliasing) {
		for (int i = 0; (i * scale) < h; i++) {
			int y = i * scale;
			for (int j = 0; (j * scale) < w; j++) {
				int x = j * scale;
				long b = 0, g = 0, r = 0, a = 0, cnt = 0;;
				for (int yy = max(0, y - 1); yy <= min(y + 1, h - 1); yy++) {
					for (int xx = max(0, x - 1); xx <= min(x + 1, w - 1); xx++) {
						b += imgData[(xx * 4) + (yy * w * 4) + 0];
						g += imgData[(xx * 4) + (yy * w * 4) + 1];
						r += imgData[(xx * 4) + (yy * w * 4) + 2];
						a += imgData[(xx * 4) + (yy * w * 4) + 3];
						cnt++;
					}
				}

				/*imgData[(x * 4) + (y * w * 4) + 0] = ((byte)(b / cnt) <= 198 ? 0 : 0xFF);
				imgData[(x * 4) + (y * w * 4) + 1] = ((byte)(g / cnt) <= 198 ? 0 : 0xFF);
				imgData[(x * 4) + (y * w * 4) + 2] = ((byte)(r / cnt) <= 198 ? 0 : 0xFF);
				imgData[(x * 4) + (y * w * 4) + 3] = ((byte)(a / cnt) <= 198 ? 0 : 0xFF);*/
				imgData[(x * 4) + (y * w * 4) + 0] = (byte)(b / cnt);
				imgData[(x * 4) + (y * w * 4) + 1] = (byte)(g / cnt);
				imgData[(x * 4) + (y * w * 4) + 2] = (byte)(r / cnt);
				imgData[(x * 4) + (y * w * 4) + 3] = (byte)(a / cnt);
			}
		}
	}

	int scaledWidth = w / scale;
	int scaledHeight = h / scale;

	byte* imgBuf = new byte[scaledWidth * scaledHeight * 4];
	for (int i = 0; i < scaledHeight; i++) {
		int y = i * scale;
		for (int j = 0; j < scaledWidth; j++) {
			int x = j * scale;
			imgBuf[(j * 4) + (i * scaledWidth * 4) + 0] = imgData[(x * 4) + (y * w * 4) + 0];
			imgBuf[(j * 4) + (i * scaledWidth * 4) + 1] = imgData[(x * 4) + (y * w * 4) + 1];
			imgBuf[(j * 4) + (i * scaledWidth * 4) + 2] = imgData[(x * 4) + (y * w * 4) + 2];
			imgBuf[(j * 4) + (i * scaledWidth * 4) + 3] = imgData[(x * 4) + (y * w * 4) + 3];
		}
	}

	DrawLine((char*)imgBuf, scaledWidth, scaledHeight, 0, 0, 0, scaledHeight - 1);			// left
	DrawLine((char*)imgBuf, scaledWidth, scaledHeight, 0, 0, scaledWidth - 1, 0);			// top
	DrawLine((char*)imgBuf, scaledWidth, scaledHeight, scaledWidth - 1, 0, scaledWidth - 1, scaledHeight - 1);	// right
	DrawLine((char*)imgBuf, scaledWidth, scaledHeight, scaledWidth - 1, scaledWidth - 1, 0, scaledHeight - 1);		// bottom

	memset(imgData, NULL, sizeof(byte) * w * h * 4);
	memcpy(imgData, imgBuf, sizeof(byte) * scaledWidth * scaledHeight * 4);
	delete[] imgBuf;
}


// Main entry point for a windows application
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	// Read in a parameter from the command line
	int n;
	float scale = 0.0;
	int aliasing = 0;
	bool isAntiAliasing = false;
	sscanf(lpCmdLine, "%d %f %d", &n, &scale, &aliasing);
	isAntiAliasing = (1 == aliasing);
	
	// Create a separate console window to display output to stdout
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	printf("The first parameter was: %d\nThe second parameter was: %f\nThe third parameter was: %d\n", n, scale, aliasing);
	
	// Set up the images
	const int w = 512;
	const int h = 512;
	inImage.setWidth(w);
	inImage.setHeight(h);

	outImage.setWidth(w / scale);
	outImage.setHeight(h / scale);
	
	char* imgData = new char[w * h * 4];
	memset(imgData, 0xFF, sizeof(char) * w * h * 4);

	// Draws a boundary
	DrawLine(imgData, w, h, 0, 0, 0, h - 1);			// left
	DrawLine(imgData, w, h, 0, 0, w - 1, 0);			// top
	DrawLine(imgData, w, h, w - 1, 0, w - 1, h - 1);	// right
	DrawLine(imgData, w, h, w-1, h - 1, 0, h - 1);		// bottom

	// Create original 512x512 image
	CreateImage((byte*)imgData, w, h, n);
	inImage.setImageData(imgData);

	char* imgData2 = new char[w * h * 4];
	memcpy(imgData2, imgData, sizeof(char) * w * h * 4);

	// Careate scaled image
	ScaleDownImage((byte*)imgData2, w, h, scale, isAntiAliasing);
	outImage.setImageData(imgData2);

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
void ColorPixel(char* imgBuf, int w, int h, int x, int y)
{
	imgBuf[(4 * y * w) +  4 * x] = 0;
	imgBuf[(4 * y * w) +  4 * x + 1] = 0;
	imgBuf[(4 * y * w) +  4 * x + 2] = 0;
}

// Draws a line using Bresenham's algorithm.
void DrawLine(char* imgBuf, int w, int h, int x1, int y1, int x2, int y2)
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
			ColorPixel(imgBuf, w, h, y, x);
		}
		else
		{
			ColorPixel(imgBuf, w, h, x, y);
		}

		error -= dy;
		if (error < 0)
		{
			y += ystep;
			error += dx;
		}
	}
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
				char text[1000];
				strcpy(text, "Original image (Left)  Image after modification (Right)\n");
				DrawText(hdc, text, strlen(text), &rt, DT_LEFT);
				strcpy(text, "\nUpdate program with your code to modify input image");
				DrawText(hdc, text, strlen(text), &rt, DT_LEFT);

				BITMAPINFO bmi;
				// CBitmap bitmap;
				memset(&bmi,0,sizeof(bmi));
				bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
				bmi.bmiHeader.biWidth = inImage.getWidth();
				bmi.bmiHeader.biHeight = -inImage.getHeight();  // Use negative height.  DIB is top-down.
				bmi.bmiHeader.biPlanes = 1;
				bmi.bmiHeader.biBitCount = 32;
				bmi.bmiHeader.biCompression = BI_RGB;
				bmi.bmiHeader.biSizeImage = inImage.getWidth()*inImage.getHeight();

				SetDIBitsToDevice(hdc,
								  100,100,inImage.getWidth(),inImage.getHeight(),
								  0,0,0,inImage.getHeight(),
								  inImage.getImageData(),&bmi,DIB_RGB_COLORS);

				memset(&bmi, 0, sizeof(bmi));
				bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
				bmi.bmiHeader.biWidth = outImage.getWidth();
				bmi.bmiHeader.biHeight = -outImage.getHeight();  // Use negative height.  DIB is top-down.
				bmi.bmiHeader.biPlanes = 1;
				bmi.bmiHeader.biBitCount = 32;
				bmi.bmiHeader.biCompression = BI_RGB;
				bmi.bmiHeader.biSizeImage = outImage.getWidth()*outImage.getHeight();

				SetDIBitsToDevice(hdc,
								  inImage.getWidth()+150,100,outImage.getWidth(),outImage.getHeight(),
								  0,0,0,outImage.getHeight(),
								  outImage.getImageData(),&bmi,DIB_RGB_COLORS);


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


