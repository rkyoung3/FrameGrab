/* *************************************************************************************************
* FrameGrab  - Created: 08/11/2016  8/11/2016 3:25:59 PM
* Creator Robert K Young - rkyoung@sonic.net
* ChangeLog:
* 0.0.1 - 8/11/2016 3:32:33 PM - Initial Version
* 0.0.2 - 8/12/2016 5:22:19 PM - Filled in skeleton, corrected typos in code copied, and made added 
*		  code Unicode  complaint.
* 0.0.3 - 8/14/2016 2:34:38 PM - Failed attempt to add functionality
* 0.0.4 - 8/15/2016 4:22:15 PM - Added capture file name based on date/time
* 0.1.0 - 8/16/2016 3:38:58 PM - First beta version with basic functionality
* 0.2.0 - 8/23/2016 1:36:01 PM - Incorporated OpenCV
//**************************************************************************************************/

#include "stdafx.h"

// Debug
#using <system.dll>
#using <mscorlib.dll>
// #using <Mf.dll>


// OpenCV  del: opencv_calib3d310d.lib, opencv_ts310d.lib add: comsupp.lib
#include "opencv2\core\core.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\imgcodecs\imgcodecs.hpp"
#include "opencv2\highgui\highgui.hpp"

using namespace cv;
using namespace std;
using namespace System::Diagnostics;
using namespace System;
using namespace System::Runtime::InteropServices;

#include <cstdlib>

typedef std::wstring String;

typedef std::vector<int> vectorNum;

typedef std::map<std::string, vectorNum> SUBTYPEMap;

typedef std::map<UINT64, SUBTYPEMap> FrameRateMap;

std::map<UINT64, FrameRateMap> vd_CaptureFormats;
// map<UINT64, FrameRateMap> vd_CaptureFormats;

std::vector<MediaType> vd_CurrentFormats;

IMFMediaSource *vd_pSource;

wchar_t *vd_pFriendlyName;

// std::auto_ptr<ImageGrabberThread> vd_pImGrTh;

CamParametrs vd_PrevParametrs;

unsigned int vd_Width;

unsigned int vd_Height;

unsigned int vd_CurrentNumber;

bool vd_IsSetuped;





#define BLUE    0x0001
#define GREEN   0x0002
#define RED     0x0004
#define GRAY    0x0007

#define MAX_LOADSTRING 100
#define MAX_CAMERAS 4

//Remember to Link to vfw32 Library, gdi32 Library  
// Contributing Source Used: http://www.dreamincode.net/forums/topic/193519-win32-webcam-program/


// Global Variables:
HWND mfg_MainHwnd;
HWND mfg_CameraHwnd;
int mfg_DesktopX_Dim;
int mfg_DesktopY_Dim;
int mfg_CameraCount = 0;
wchar_t** mfg_CameraNames;
int mfg_CurrentCamera = 0;
HWND mfg_hWindow;
HDC mfg_hdc;
HDC mfg_hdcMem;
PAINTSTRUCT mfg_ps;
HBITMAP mfg_hbm;
RECT mfg_rc;
HINSTANCE mfg_hInst;                                // current instance
WCHAR mfg_szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR mfg_szWindowClass[MAX_LOADSTRING];            // the main window class name
Mat mfg_m_Image;
// char mfg_FrameSaveDirectory[MAX_PATH];
wchar_t mfg_FrameSaveDirectory[MAX_PATH];


// Forward declarations of functions included in this code module:
int enum_devices();
long initDevice();
long setDeviceFormat(IMFMediaSource *pSource, unsigned long  dwFormatIndex);
bool setupDevice(unsigned int id);
bool setupDevice(unsigned int w, unsigned int h, unsigned int idealFramerate);
long checkDevice(IMFAttributes *pAttributes, IMFActivate **pDevice);
int findType(unsigned int size, unsigned int frameRate);
CamParametrs getParametrs();

static void setcolor(unsigned int color);
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);
bool CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);
LPCTSTR szAppName = L"FrameGrab";
ATOM FGRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void process_filter(IBaseFilter *pBaseFilter, int iIndex);
void _FreeMediaType(AM_MEDIA_TYPE& mt);
HRESULT CamCaps(IBaseFilter *pBaseFilter, int iIndex);
std::string BrowseFolder(std::string saved_path);

// System::Diagnostics::WriteLine((char *) mesg);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	mfg_CameraNames = new wchar_t*[MAX_CAMERAS];
	// Get the number of cameras
	// [ enum_devices() will record their 
	//     Names to an array of strings   ]
	mfg_CameraCount = enum_devices();

	if(mfg_CameraCount <= 0)
		return FALSE;

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, mfg_szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_FRAMEGRAB, mfg_szWindowClass, MAX_LOADSTRING);
	FGRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	// HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABELINSPECTION));

	MSG msg;
	// Main message loop:
	while (GetMessage(&msg, 0, 0, 0))
	{
		if (!IsDialogMessage(mfg_hWindow, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

/*
* Enumerate all video devices
*
* See also:
*
* Using the System Device Enumerator:
*     http://msdn2.microsoft.com/en-us/library/ms787871.aspx
*/
int enum_devices()
{
	HRESULT hr;
	int NumCamerasFound = 0;
	// char mesg[MAX_PATH];
	wchar_t mesg[MAX_PATH];
	setcolor(GRAY);
	// printf("Enumeraring videoinput devices ...\n");

	OutputDebugString(L"Enumeraring videoinput devices ...\n");

	CoInitialize(NULL);
	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		OutputDebugString(L"Error. Can't create enumerator.\n");
		return hr;
	}

	// Obtain a class enumerator for the video input device category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK)
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;

		while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			NumCamerasFound++; // We found a camera
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// Retrieve the device's friendly name
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0); // Read the device's name
				if (SUCCEEDED(hr))
				{
					if (NumCamerasFound < MAX_CAMERAS)
					{
						// Record the name, indexed by order found
						mfg_CameraNames[NumCamerasFound - 1] = new wchar_t[MAX_LOADSTRING];
						wcsncpy_s(mfg_CameraNames[NumCamerasFound - 1], MAX_LOADSTRING, varName.bstrVal, _TRUNCATE);
						if ((swprintf_s(mesg, (size_t)MAX_PATH, L"Found Camera: %ls\n", mfg_CameraNames[NumCamerasFound - 1]) >= 1))
							OutputDebugString(mesg);
					}
				}
				VariantClear(&varName);
				// To create an instance of the filter, do the following:
				IBaseFilter *pFilter;
				hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pFilter);
				process_filter(pFilter, (NumCamerasFound - 1));
				//Remember to release pFilter later.
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	OleUninitialize();
	return NumCamerasFound;
}

static void setcolor(unsigned int color)
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, color | FOREGROUND_INTENSITY);
}

/*
* Do something with the filter. In this sample we just test the pan/tilt properties.
*/
void process_filter(IBaseFilter *pBaseFilter, int iIndex)
{
	CamCaps(pBaseFilter, iIndex);
}

HRESULT CamCaps(IBaseFilter *pBaseFilter, int iIndex)
{
	HRESULT hr = 0;
	// char buffer[MAX_PATH];
	// char mesg[MAX_PATH];
	wchar_t mesg[MAX_PATH];
	vector <IPin*> pins;
	IEnumPins *EnumPins;
	IEnumMediaTypes *emt = NULL;
	AM_MEDIA_TYPE *pmt;
	vector<SIZE> modes;

	pBaseFilter->EnumPins(&EnumPins);
	pins.clear();
	for (;;)
	{
		IPin *pin;
		hr = EnumPins->Next(1, &pin, NULL);
		if (hr != S_OK) { break; }
		pins.push_back(pin);
		pin->Release();
	}
	EnumPins->Release();

	if ((swprintf_s(mesg, (size_t)MAX_PATH, L"Number of Device pins: %zd\n", pins.size()) > 0))
		OutputDebugString(mesg);

	PIN_INFO pInfo;
	for (int i = 0; i<pins.size(); i++)
	{
		pins[i]->QueryPinInfo(&pInfo);
		if ((swprintf_s(mesg, (size_t)MAX_PATH, L"Pin name: %s \n", pInfo.achName) > 0))
			OutputDebugString(mesg);
		
		pins[i]->EnumMediaTypes(&emt);
		if ((swprintf_s(mesg, (size_t)MAX_PATH, L"Avialable resolutions for: %ls \n", mfg_CameraNames[iIndex]) >= 1))
			OutputDebugString(mesg);

		for (;;)
		{
			hr = emt->Next(1, &pmt, NULL);
			if (hr != S_OK) { break; }

			if ((pmt->formattype == FORMAT_VideoInfo) && (pmt->cbFormat >= sizeof(VIDEOINFOHEADER)) && (pmt->pbFormat != NULL))
			{
				VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)pmt->pbFormat;
				SIZE s;
				// Get frame size
				s.cy = pVIH->bmiHeader.biHeight;
				s.cx = pVIH->bmiHeader.biWidth;
				unsigned int bitrate = pVIH->dwBitRate;
				modes.push_back(s);
				// Bits per pixel
				unsigned int bitcount = pVIH->bmiHeader.biBitCount;
				REFERENCE_TIME t = pVIH->AvgTimePerFrame; // blocks (100ns) per frame
				int FPS = (int)(floor(10000000.0 / static_cast<double>(t)));
				if ((swprintf_s(mesg, (size_t)MAX_PATH, L"Size: x=%d\ty=%d\tFPS: %d\t bitrate: %ld\tbit/pixel:%ld\n", s.cx, s.cy, FPS, bitrate, bitcount) >= 1))
					OutputDebugString(mesg);
			}
			_FreeMediaType(*pmt);
		}

		modes.clear();
		emt->Release();
	}
	pins.clear();
	return S_OK;
}// End CamCaps()

void _FreeMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL)
	{
		// pUnk should not be used.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}





//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM FGRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(GetModuleHandle(NULL), IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = mfg_szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
	HRESULT hResult;
	wchar_t szPath[MAX_PATH];

	mfg_hInst = hInstance; // Store instance handle in our global variable
	// Get the desktop dims and take a little off all four sides
	mfg_DesktopX_Dim = (GetSystemMetrics(SM_CXSCREEN) - (GetSystemMetrics(SM_CXSCREEN) >> 5));
	mfg_DesktopY_Dim = (GetSystemMetrics(SM_CYSCREEN) - (GetSystemMetrics(SM_CYSCREEN) >> 5));

	mfg_hWindow = CreateWindowW(mfg_szWindowClass, mfg_szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, mfg_DesktopX_Dim, mfg_DesktopY_Dim, nullptr, nullptr, hInstance, nullptr);

	if (!mfg_hWindow)
		return FALSE;

	ShowWindow(mfg_hWindow, nCmdShow);
	UpdateWindow(mfg_hWindow);
	hResult = SHGetFolderPath(NULL, CSIDL_MYPICTURES | CSIDL_FLAG_CREATE, NULL, 0, szPath); // mfg_FrameSaveDirectory

	// TODO: Read "Capture Directory" 
	// from config storage if available
	if (SUCCEEDED(hResult))
	{
		swprintf_s(mfg_FrameSaveDirectory, (size_t)MAX_PATH, L"%ls", szPath);
		if ((swprintf_s(szPath, (size_t)MAX_PATH, L"Current Save Dir: %s \n", mfg_FrameSaveDirectory) > 0))
			OutputDebugString(szPath);

	}
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	//some buttons
	HWND hButtonStartCam;
	HWND hButtonStopCam;
	HWND hButtonChangeCam;
	HWND hButtonGrabFrame;
	HWND hButtonSetDir;

	switch (message)
	{
	case WM_CTLCOLORSTATIC:
		SetBkMode(mfg_hdc, TRANSPARENT);
		return (LRESULT)CreateSolidBrush(0xFFFFFF);

	case WM_CREATE:
	{
		hButtonStartCam = CreateWindowEx(0, L"BUTTON", L"Start Camera", WS_CHILD | WS_VISIBLE, 0, 0, 300, 60, hWnd, (HMENU) MENU_BUTTON_STARTCAM, hInstance, 0);
		hButtonStopCam = CreateWindowEx(0, L"BUTTON", L"Stop Camera", WS_CHILD | WS_VISIBLE, 0, 75, 300, 60, hWnd, (HMENU) MENU_BUTTON_STOPCAM, hInstance, 0);
		hButtonSetDir = CreateWindowEx(0, L"BUTTON", L"Set Capture Dir", WS_CHILD | WS_VISIBLE, 0, 150, 300, 60, hWnd, (HMENU) MENU_BUTTON_SAVE_DIR, hInstance, 0);


		// If we have more than one camera
		// add a change camera button
		if (mfg_CameraCount > 1)
		{
			hButtonChangeCam = CreateWindowEx(0, L"BUTTON", L"Change Camera", WS_CHILD | WS_VISIBLE, 0, 225, 300, 60, hWnd, (HMENU) MENU_BUTTON_CHANGECAM, hInstance, 0);
			hButtonGrabFrame = CreateWindowEx(0, L"BUTTON", L"Grab Frame", WS_CHILD | WS_VISIBLE, 0, 300, 300, 60, hWnd, (HMENU) MENU_BUTTON_CAPTURE_FRAME, hInstance, 0);
		}else{
			hButtonGrabFrame = CreateWindowEx(0, L"BUTTON", L"Grab Frame", WS_CHILD | WS_VISIBLE, 0, 225, 300, 60, hWnd, (HMENU) MENU_BUTTON_CAPTURE_FRAME, hInstance, 0);
		}

		mfg_CameraHwnd = capCreateCaptureWindow(mfg_CameraNames[0], WS_CHILD, 301, 25, CANERA_WIDTH, CANERA_HEIGHT, hWnd, 0);
		// SendMessage(camhwnd, WM_CAP_DLG_VIDEOSOURCE, 0, 0); // Microsoft is determined to put this dialog up even if not called to
		SendMessage(mfg_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
		SendMessage(mfg_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
		SendMessage(mfg_CameraHwnd, WM_CAP_DRIVER_CONNECT, 0, 0);
		// SendMessage(mfg_CameraHwnd, WM_CAP_DRIVER_CONNECT, 0, 0);
		break;
	}


	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
			case IDM_ABOUT:
				DialogBox(mfg_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;

				// *******************************************************
			case MENU_BUTTON_STARTCAM:
			{

				// SendMessage(camhwnd, WM_CAP_DLG_VIDEOSOURCE, 0, 0);
				SendMessage(mfg_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
				SendMessage(mfg_CameraHwnd, WM_CAP_DRIVER_CONNECT, mfg_CurrentCamera, 0);
				SendMessage(mfg_CameraHwnd, WM_CAP_SET_SCALE, true, 0);
				SendMessage(mfg_CameraHwnd, WM_CAP_SET_PREVIEWRATE, 66, 0);
				SendMessage(mfg_CameraHwnd, WM_CAP_SET_PREVIEW, true, 0);
				ShowWindow(mfg_CameraHwnd, SW_SHOW);
				break;
			}

			case MENU_BUTTON_STOPCAM:
			{
				ShowWindow(mfg_CameraHwnd, SW_HIDE);
				SendMessage(mfg_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
				break;
			}

			case MENU_BUTTON_CAPTURE_FRAME:
			{
				//Grab a Frame
				SendMessage(mfg_CameraHwnd, WM_CAP_GRAB_FRAME, 0, 0);
				//Copy the frame we have just grabbed to the clipboard
				SendMessage(mfg_CameraHwnd, WM_CAP_EDIT_COPY, 0, 0);
				//Copy the clipboard image data to a HBITMAP object called hbm
				mfg_hdc = BeginPaint(mfg_CameraHwnd, &mfg_ps);
				mfg_hdcMem = CreateCompatibleDC(mfg_hdc);

				if (mfg_hdcMem != NULL)
				{
					if (OpenClipboard(mfg_CameraHwnd))
					{
						mfg_hbm = (HBITMAP)GetClipboardData(CF_BITMAP);
						SelectObject(mfg_hdcMem, mfg_hbm);
						GetClientRect(mfg_CameraHwnd, &mfg_rc);
						CloseClipboard();
					}
				}

				//Save hbm to a .bmp file with date/time based name
				PBITMAPINFO pbi = CreateBitmapInfoStruct(hWnd, mfg_hbm);

				__time64_t long_time;
				struct tm newtime;
				wchar_t buffer[MAX_PATH];
				wchar_t mesg[MAX_PATH];

				_time64(&long_time);
				_localtime64_s(&newtime, &long_time); // Convert to local time.
				int len = swprintf_s(buffer, MAX_PATH, L"FG_%04d-%02d-%02d_%02d%02d.bmp", (newtime.tm_year + 1900), (newtime.tm_mon + 1), newtime.tm_mday, newtime.tm_hour, newtime.tm_min);
				if ((swprintf_s(mesg, (size_t)MAX_PATH, L"Creating file: %ls\n", buffer) >= 1))
					OutputDebugString(mesg);

				CreateBMPFile(hWnd, (LPTSTR) buffer, pbi, mfg_hbm, mfg_hdcMem);
				SendMessage(mfg_CameraHwnd, WM_CAP_DRIVER_CONNECT, 0, 0);
				SendMessage(mfg_CameraHwnd, WM_CAP_SET_SCALE, true, 0);
				SendMessage(mfg_CameraHwnd, WM_CAP_SET_PREVIEWRATE, 66, 0);
				SendMessage(mfg_CameraHwnd, WM_CAP_SET_PREVIEW, true, 0);
				ProcessImage((wchar_t*) buffer);
				break;
			} // End case 3:

			case MENU_BUTTON_CHANGECAM:
			{
				mfg_CurrentCamera++;
				if (mfg_CurrentCamera >= mfg_CameraCount)
					mfg_CurrentCamera = 0;
				SendMessage(mfg_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
				SendMessage(mfg_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
				SendMessage(mfg_CameraHwnd, WM_CAP_DRIVER_CONNECT, mfg_CurrentCamera, 0);
				SendMessage(mfg_CameraHwnd, WM_CAP_SET_SCALE, true, 0);
				SendMessage(mfg_CameraHwnd, WM_CAP_SET_PREVIEWRATE, 66, 0);
				SendMessage(mfg_CameraHwnd, WM_CAP_SET_PREVIEW, true, 0);
				ShowWindow(mfg_CameraHwnd, SW_SHOW);
				break;
			} // End case MENU_BUTTON_CHANGECAM:

			case MENU_BUTTON_SAVE_DIR:
			{
				HRESULT hResult;
				wchar_t szPath[MAX_PATH];
				wchar_t mesg[MAX_PATH];
			
				// Microsoft API: https://msdn.microsoft.com/en-us/library/windows/desktop/bb762188(v=vs.85).aspx
				// HRESULT SHGetKnownFolderPath(_In_ REFKNOWNFOLDERID rfid, _In_ DWORD dwFlags, _In_opt_ HANDLE hToken, _Out_  PWSTR *ppszPath);
				hResult = SHGetFolderPath(NULL, CSIDL_MYPICTURES | CSIDL_FLAG_CREATE,NULL,0,szPath); // mfg_FrameSaveDirectory
				// TODO: Default to User's pictures directory or last used
				if (SUCCEEDED(hResult))
				{
					if ((swprintf_s(mfg_FrameSaveDirectory, (size_t)MAX_PATH, L"%ls", szPath) >= 1))
					{
						if ((swprintf_s(mesg, (size_t) MAX_PATH, L"Current Save Directory: %ls\n", mfg_FrameSaveDirectory) >= 1))
							OutputDebugString(mesg);
					}
				}   // End if (SUCCEEDED(hResult))
				break;
			}   // End case MENU_BUTTON_SAVE_DIR:
		}   // End switch (wmId)
	}   // End case WM_COMMAND:
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

bool CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
{

	HANDLE hf;                  // file handle
	BITMAPFILEHEADER hdr;       // bitmap file-header
	PBITMAPINFOHEADER pbih;     // bitmap info-header
	LPBYTE lpBits;              // memory pointer
	DWORD dwTotal;              // total count of bytes
	DWORD cb;                   // incremental count of bytes
	BYTE *hp;                   // byte pointer
	BYTE *rgbPixels;
	DWORD dwTmp;

	pbih = (PBITMAPINFOHEADER)pbi;

	if (pbih->biSizeImage)
		lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);
	else
		return false;

	if (!lpBits)
	{
		MessageBox(hwnd, L"GlobalAlloc", L"Error", MB_OK);
		return false;
	}
	// Save a pointer to the raw pixel data
	rgbPixels = &lpBits[sizeof(BITMAPFILEHEADER)];

	// Retrieve the color table (RGBQUAD array) and the bits
	// (array of palette indices) from the DIB.
	if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS))
	{
		MessageBox(hwnd, L"GetDIBits", L"Error", MB_OK);
		GlobalFree((HGLOBAL)lpBits);
		return false;
	}

	// Create the .BMP file.
	hf = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE, (DWORD)0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

	if (hf == INVALID_HANDLE_VALUE)
	{
		MessageBox(hwnd, L"CreateFile", L"Error", MB_OK);
		GlobalFree((HGLOBAL)lpBits);
		return false;
	}

	hdr.bfType = 0x4d42;  // File type designator "BM" 0x42 = "B" 0x4d = "M"
						  // Compute the size of the entire file.
	hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;
	// Compute the offset to the array of color indices.
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD);


	// Copy the BITMAPFILEHEADER into the .BMP file.
	if (!WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER), (LPDWORD)&dwTmp, NULL))
	{
		MessageBox(hwnd, L"WriteFileHeader", L"Error", MB_OK);
		GlobalFree((HGLOBAL)lpBits);
		return false;
	}

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
	if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD), (LPDWORD)&dwTmp, NULL))
	{
		MessageBox(hwnd, L"WriteInfoHeader", L"Error", MB_OK);
		GlobalFree((HGLOBAL)lpBits);
		return false;

	}

	// Copy the array of color indices into the .BMP file.
	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;

	if (!WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL))
	{
		MessageBox(hwnd, L"WriteFile", L"Error", MB_OK);
		GlobalFree((HGLOBAL)lpBits);
		return false;
	}

	// Close the .BMP file.
	if (!CloseHandle(hf))
	{
		MessageBox(hwnd, L"CloseHandle", L"Error", MB_OK);
		GlobalFree((HGLOBAL)lpBits);
		return false;
	}
	// We have an image
	ProcessImage ProcessImageInst = ProcessImage(pbih, rgbPixels);
	// Free memory.
	GlobalFree((HGLOBAL)lpBits);
	return true;
}

PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
	BITMAP bmp;
	PBITMAPINFO pbmi;
	WORD cClrBits;

	// Retrieve the bitmap color format, width, and height.
	if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
	{
		MessageBox(hwnd, L"GetObject", L"Error", MB_OK);
	}

	// Convert the color format to a count of bits.
	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);

	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else cClrBits = 32;

	// Allocate memory for the BITMAPINFO structure. (This structure
	// contains a BITMAPINFOHEADER structure and an array of RGBQUAD
	// data structures.)
	if (cClrBits != 24)
	{
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * ((int) (1 << cClrBits)));
	}
	else // There is no RGBQUAD array for the 24-bit-per-pixel format.
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));

	// Initialize the fields in the BITMAPINFO structure.
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;

	if (cClrBits < 24)
	{
		pbmi->bmiHeader.biClrUsed = (1 << cClrBits);
	}

	// If the bitmap is not compressed, set the BI_RGB flag.
	pbmi->bmiHeader.biCompression = BI_RGB;

	// Compute the number of bytes in the array of color
	// indices and store the result in biSizeImage.
	// For Windows NT, the width must be DWORD aligned unless
	// the bitmap is RLE compressed. This example shows this.
	// For Windows 95/98/Me, the width must be WORD aligned unless the
	// bitmap is RLE compressed.
	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8 * pbmi->bmiHeader.biHeight;

	// Set biClrImportant to 0, indicating that all of the
	// device colors are important.
	pbmi->bmiHeader.biClrImportant = 0;

	return pbmi; //return BITMAPINFO
}

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{

	if (uMsg == BFFM_INITIALIZED)
	{
		std::string tmp = (const char *)lpData;
		std::cout << "path: " << tmp << std::endl;
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}

std::string BrowseFolder(std::string saved_path)
{
	TCHAR path[MAX_PATH];

	wchar_t szPath[MAX_PATH];
	const char * path_param = saved_path.c_str();

	BROWSEINFO bi = { 0 };
	bi.lpszTitle = L"Browse for folder...";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)path_param;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		//get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, path);

		//free memory used
		IMalloc * imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			
			imalloc->Free(pidl);
			imalloc->Release();
		}

		size_t origsize = wcslen(szPath) + 1;
		size_t convertedChars = 0;
		const size_t newsize = origsize * 2;
		char *nstring = new char[newsize];
		wcstombs_s(&convertedChars, nstring, newsize, szPath, _TRUNCATE);

//		_sprintf_s_l(&buffer[0], MAX_PATH, "%s", (_locale_t)LOCALE_NAME_SYSTEM_DEFAULT, path);
		// return buffer;
		return nstring;
	}

	return "";
}

// **************************************************************************************************************
// Contributing Source: http://www.codeproject.com/Tips/559437/Capturing-Video-from-Web-camera-on-Windows-and-by
// **************************************************************************************************************
long setDeviceFormat(IMFMediaSource *pSource, unsigned long  dwFormatIndex)
{
	IMFPresentationDescriptor *pPD = NULL;
	IMFStreamDescriptor *pSD = NULL;
	IMFMediaTypeHandler *pHandler = NULL;
	IMFMediaType *pType = NULL;

	HRESULT hr = pSource->CreatePresentationDescriptor(&pPD);
	if (FAILED(hr))
	{
		goto done;
	}

	BOOL fSelected;
	hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pSD->GetMediaTypeHandler(&pHandler);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pHandler->GetMediaTypeByIndex((DWORD)dwFormatIndex, &pType);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pHandler->SetCurrentMediaType(pType);

done:
	SafeReleaseAllCount(&pPD);
	SafeRelease(&pSD);
	SafeRelease(&pHandler);
	SafeRelease(&pType);
	return hr;
}

bool setupDevice(unsigned int id)
{
	// DebugPrintOut *DPO = &DebugPrintOut::getInstance();

	if (!vd_IsSetuped)
	{
		HRESULT hr = -1;

		hr = initDevice();

		if (SUCCEEDED(hr))
		{
			vd_Width = vd_CurrentFormats[id].width;

			vd_Height = vd_CurrentFormats[id].height;

			hr = setDeviceFormat(vd_pSource, (DWORD)id);

			vd_IsSetuped = (SUCCEEDED(hr));

			// if (vd_IsSetuped)
			// 	DPO->printOut(L"\n\nVIDEODEVICE %i: Device is setuped \n", vd_CurrentNumber);

			vd_PrevParametrs = getParametrs();

			return vd_IsSetuped;
		}
		else
		{
			// DPO->printOut(L"VIDEODEVICE %i: Interface IMFMediaSource cannot be got \n", vd_CurrentNumber);

			return false;
		}
	}
	else
	{
		// DPO->printOut(L"VIDEODEVICE %i: Device is setuped already \n", vd_CurrentNumber);

		return false;
	}
}

bool setupDevice(unsigned int w, unsigned int h, unsigned int idealFramerate)
{
	unsigned int id = findType(w * h, idealFramerate);

	return setupDevice(id);
}

long initDevice()
{
	HRESULT hr = -1;

	IMFAttributes *pAttributes = NULL;

	IMFActivate * vd_pActivate = NULL;

	 // DebugPrintOut *DPO = &DebugPrintOut::getInstance();

	CoInitialize(NULL);

	hr = MFCreateAttributes(&pAttributes, 1);

	if (SUCCEEDED(hr))
	{
		hr = pAttributes->SetGUID(
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
		);
	}

	if (SUCCEEDED(hr))
	{
		hr = checkDevice(pAttributes, &vd_pActivate);

		if (SUCCEEDED(hr) && vd_pActivate)
		{
			SafeReleaseAllCount(&vd_pSource);

			hr = vd_pActivate->ActivateObject(
				__uuidof(IMFMediaSource),
				(void**)&vd_pSource
			);

			if (SUCCEEDED(hr))
			{

			}

			SafeReleaseAllCount(&vd_pActivate);
		}
		else
		{
			// DPO->printOut(L"VIDEODEVICE %i: Device there is not \n", vd_CurrentNumber);
		}
	}
	else
	{

		// DPO->printOut(L"VIDEODEVICE %i: The attribute of video cameras cannot be getting \n", vd_CurrentNumber);

	}

	SafeReleaseAllCount(&pAttributes);

	return hr;
}

long checkDevice(IMFAttributes *pAttributes, IMFActivate **pDevice)
{
	HRESULT hr = S_OK;

	IMFActivate **ppDevices = NULL;

	// DebugPrintOut *DPO = &DebugPrintOut::getInstance();

	UINT32 count;

	wchar_t *newFriendlyName = NULL;

	hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);

	if (SUCCEEDED(hr))
	{
		if (count > 0)
		{
			if (count > vd_CurrentNumber)
			{
				hr = ppDevices[vd_CurrentNumber]->GetAllocatedString(
					MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
					&newFriendlyName,
					NULL
				);

				if (SUCCEEDED(hr))
				{
					if (wcscmp(newFriendlyName, vd_pFriendlyName) != 0)
					{
						// DPO->printOut(L"VIDEODEVICE %i: Chosen device cannot be found \n", vd_CurrentNumber);

						hr = -1;

						pDevice = NULL;
					}
					else
					{
						*pDevice = ppDevices[vd_CurrentNumber];

						(*pDevice)->AddRef();
					}
				}
				else
				{
					// DPO->printOut(L"VIDEODEVICE %i: Name of device cannot be gotten \n", vd_CurrentNumber);
				}

			}
			else
			{
				// DPO->printOut(L"VIDEODEVICE %i: Number of devices more than corrent number of the device \n", vd_CurrentNumber);

				hr = -1;
			}

			for (UINT32 i = 0; i < count; i++)
			{
				SafeRelease(&ppDevices[i]);
			}

			SafeReleaseAllCount(ppDevices);
		}
		else
			hr = -1;
	}
	else
	{
		// DPO->printOut(L"VIDEODEVICE %i: List of DeviceSources cannot be enumerated \n", vd_CurrentNumber);
	}

	return hr;
}

int findType(unsigned int size, unsigned int frameRate)
{
	if (vd_CaptureFormats.size() == 0)
		return 0;

	FrameRateMap FRM = vd_CaptureFormats[size];

	if (FRM.size() == 0)
		return 0;

	unsigned int frameRateMax = 0;  SUBTYPEMap STMMax;

	if (frameRate == 0)
	{
		std::map<UINT64, SUBTYPEMap>::iterator f = FRM.begin();

		for (; f != FRM.end(); f++)
		{
			if ((*f).first >= frameRateMax)
			{
				frameRateMax = (*f).first;

				STMMax = (*f).second;
			}
		}

	}
	else
	{
		std::map<UINT64, SUBTYPEMap>::iterator f = FRM.begin();

		for (; f != FRM.end(); f++)
		{
			if ((*f).first >= frameRateMax)
			{
				if (frameRate > (*f).first)
				{
					frameRateMax = (*f).first;

					STMMax = (*f).second;
				}
			}
		}
	}

	if (STMMax.size() == 0)
		return 0;


	std::map<std::string, vectorNum>::iterator S = STMMax.begin();

	vectorNum VN = (*S).second;

	if (VN.size() == 0)
		return 0;

	return VN[0];

}

CamParametrs getParametrs()
{
	CamParametrs out;

	if (vd_IsSetuped)
	{
		if (vd_pSource)
		{
			unsigned int shift = sizeof(Parametr);

			Parametr *pParametr = (Parametr *)(&out);

			IAMVideoProcAmp *pProcAmp = NULL;
			HRESULT hr = vd_pSource->QueryInterface(IID_PPV_ARGS(&pProcAmp));

			if (SUCCEEDED(hr))
			{
				for (unsigned int i = 0; i < 10; i++)
				{
					Parametr temp;

					hr = pProcAmp->GetRange(VideoProcAmp_Brightness + i, &temp.Min, &temp.Max, &temp.Step, &temp.Default, &temp.Flag);

					if (SUCCEEDED(hr))
					{
						long currentValue = temp.Default;

						hr = pProcAmp->Get(VideoProcAmp_Brightness + i, &currentValue, &temp.Flag);

						temp.CurrentValue = currentValue;

						pParametr[i] = temp;
					}
				}

				pProcAmp->Release();
			}

			IAMCameraControl *pProcControl = NULL;
			hr = vd_pSource->QueryInterface(IID_PPV_ARGS(&pProcControl));

			if (SUCCEEDED(hr))
			{
				for (unsigned int i = 0; i < 7; i++)
				{
					Parametr temp;

					hr = pProcControl->GetRange(CameraControl_Pan + i, &temp.Min, &temp.Max, &temp.Step, &temp.Default, &temp.Flag);

					if (SUCCEEDED(hr))
					{
						long currentValue = temp.Default;

						hr = pProcAmp->Get(CameraControl_Pan + i, &currentValue, &temp.Flag);

						temp.CurrentValue = currentValue;

						pParametr[10 + i] = temp;
					}
				}

				pProcControl->Release();
			}
		}
	}

	return out;
}

Parametr::Parametr()
{
	CurrentValue = 0;

	Min = 0;

	Max = 0;

	Step = 0;

	Default = 0;

	Flag = 0;
}

MediaType::MediaType()
{
	pMF_MT_AM_FORMAT_TYPEName = NULL;

	pMF_MT_MAJOR_TYPEName = NULL;

	pMF_MT_SUBTYPEName = NULL;

	Clear();
}

MediaType::~MediaType()
{
	Clear();
}

void MediaType::Clear()
{

	MF_MT_FRAME_SIZE = 0;

	height = 0;

	width = 0;

	MF_MT_YUV_MATRIX = 0;

	MF_MT_VIDEO_LIGHTING = 0;

	MF_MT_DEFAULT_STRIDE = 0;

	MF_MT_VIDEO_CHROMA_SITING = 0;

	MF_MT_FIXED_SIZE_SAMPLES = 0;

	MF_MT_VIDEO_NOMINAL_RANGE = 0;

	MF_MT_FRAME_RATE = 0;

	MF_MT_FRAME_RATE_low = 0;

	MF_MT_PIXEL_ASPECT_RATIO = 0;

	MF_MT_PIXEL_ASPECT_RATIO_low = 0;

	MF_MT_ALL_SAMPLES_INDEPENDENT = 0;

	MF_MT_FRAME_RATE_RANGE_MIN = 0;

	MF_MT_FRAME_RATE_RANGE_MIN_low = 0;

	MF_MT_SAMPLE_SIZE = 0;

	MF_MT_VIDEO_PRIMARIES = 0;

	MF_MT_INTERLACE_MODE = 0;

	MF_MT_FRAME_RATE_RANGE_MAX = 0;

	MF_MT_FRAME_RATE_RANGE_MAX_low = 0;

	memset(&MF_MT_MAJOR_TYPE, 0, sizeof(GUID));

	memset(&MF_MT_AM_FORMAT_TYPE, 0, sizeof(GUID));

	memset(&MF_MT_SUBTYPE, 0, sizeof(GUID));
}

