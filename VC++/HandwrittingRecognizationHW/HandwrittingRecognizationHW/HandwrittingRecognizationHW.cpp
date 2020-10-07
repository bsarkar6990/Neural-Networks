// HandwrittingRecognizationHW.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include <comdef.h>
#include <msinkaut.h>
#include <msinkaut_i.c>
#include "HandwrittingRecognizationHW.h"
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <math.h>
#include <sstream>
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

ID2D1Factory* m_pD2DFactory;
IWICImagingFactory* m_pWICFactory;
ID2D1HwndRenderTarget* m_pRenderTarget;
ID2D1Bitmap* m_pBitmap;
HWND *phWnd;
// Declare all necessary global interface pointers here
IInkCollector* g_pIInkCollector = NULL;
IInkDisp* g_pIInkDisp = NULL;
IInkRecognizerContext* g_pIInkRecoContext = NULL;
// Global constants and variables.
const TCHAR* gc_szAppName = TEXT("Basic Recognition");

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HRESULT LoadBitmapFromFile(
	ID2D1RenderTarget* pRenderTarget,
	IWICImagingFactory* pIWICFactory,
	PCWSTR uri,
	UINT destinationWidth,
	UINT destinationHeight,
	ID2D1Bitmap** ppBitmap
);

void CleanUp()  // Release all objects
{
	if (g_pIInkRecoContext != NULL)
	{
		g_pIInkRecoContext->Release();
		g_pIInkRecoContext = NULL;
	}

	if (g_pIInkDisp != NULL)
	{
		g_pIInkDisp->Release();
		g_pIInkDisp = NULL;
	}

	if (g_pIInkCollector != NULL)
	{
		g_pIInkCollector->Release();
		g_pIInkCollector = NULL;
	}
}

HRESULT CreateDeviceIndependentResources() {
	HRESULT hr;
	// Create a Direct2D factory.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	if (SUCCEEDED(hr))
	{
		// Create WIC factory.
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void**>(&m_pWICFactory)
		);
	}
	return hr;	
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_HANDWRITTINGRECOGNIZATIONHW, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
	CoInitialize(NULL);
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HANDWRITTINGRECOGNIZATIONHW));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HANDWRITTINGRECOGNIZATIONHW));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HANDWRITTINGRECOGNIZATIONHW);
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
   CreateDeviceIndependentResources();
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
HRESULT CreateDeviceResources(HWND hWnd) {
	HRESULT hr = S_OK;
	phWnd = &hWnd;
	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(hWnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
		);

		// Create a Direct2D render target.
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hWnd, size),
			&m_pRenderTarget
		);
		if (SUCCEEDED(hr))
		{
			// Create a bitmap by loading it from a file.
			hr = LoadBitmapFromFile(
				m_pRenderTarget,
				m_pWICFactory,
				L".\\sampleImage.jpg",
				640,
				480,
				&m_pBitmap
			);
		}
	}
	return hr;
}
HRESULT OnRender(HWND hWnd) {
	HRESULT hr;
	hr = CreateDeviceResources(hWnd);
	if (SUCCEEDED(hr) && !(m_pRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
	{

		m_pRenderTarget->BeginDraw();
		D2D1_SIZE_F size = m_pBitmap->GetSize();
		// Draw a bitmap in the upper-left corner of the window.
		m_pRenderTarget->DrawBitmap(
			m_pBitmap,
			D2D1::RectF(0.0f, 0.0f, size.width, size.height)
		);

		hr = m_pRenderTarget->EndDraw();

		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			SafeRelease(&m_pRenderTarget);
			SafeRelease(&m_pBitmap);

		}
	}
	return hr;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
	{
		HRESULT hr;


		// Create a recognition context that uses the default recognizer.
		// The single context will be used for all the recognition.
		hr = CoCreateInstance(CLSID_InkRecognizerContext,
			NULL, CLSCTX_INPROC_SERVER,
			IID_IInkRecognizerContext,
			(void**)& g_pIInkRecoContext);
		if (FAILED(hr))
		{
			::MessageBox(NULL, TEXT("There are no handwriting recognizers installed.\n"
				TEXT("You need to have at least one in order to run this sample.\nExiting.")),
				gc_szAppName, MB_ICONERROR);
			return -1;
		}

		// Create the InkCollector object.
		hr = CoCreateInstance(CLSID_InkCollector,
			NULL, CLSCTX_INPROC_SERVER,
			IID_IInkCollector,
			(void**)& g_pIInkCollector);
		if (FAILED(hr))
			return -1;

		// Get a pointer to the Ink object
		hr = g_pIInkCollector->get_Ink(&g_pIInkDisp);
		if (FAILED(hr))
			return -1;

		// Tell InkCollector the window to collect ink in
		hr = g_pIInkCollector->put_hWnd((long)hWnd);
		if (FAILED(hr))
			return -1;

		// Enable ink input in the window
		hr = g_pIInkCollector->put_Enabled(VARIANT_TRUE);
		if (FAILED(hr))
			return -1;

		break;
	}

    case WM_COMMAND:
		if (wParam == ID_CLEAR)
		{
			// Delete all strokes from the Ink
			g_pIInkDisp->DeleteStrokes(0);

			// Update the window
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == ID_RECOGNIZE)
		{
			// change cursor to the system's Hourglass
			HCURSOR hCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
			// Get a pointer to the ink stroke collection
			// This collection is a snapshot of the entire ink object
			IInkStrokes* pIInkStrokes = NULL;
			HRESULT hr = g_pIInkDisp->get_Strokes(&pIInkStrokes);
			if (SUCCEEDED(hr))
			{
				// Pass the stroke collection to the recognition context
				hr = g_pIInkRecoContext->putref_Strokes(pIInkStrokes);
				if (SUCCEEDED(hr))
				{
					// Recognize
					IInkRecognitionResult* pIInkRecoResult = NULL;
					InkRecognitionStatus RecognitionStatus;
					hr = g_pIInkRecoContext->Recognize(&RecognitionStatus, &pIInkRecoResult);
					if (SUCCEEDED(hr) && (pIInkRecoResult != NULL))
					{
						// Get the best result of the recognition
						BSTR bstrBestResult = NULL;
						hr = pIInkRecoResult->get_TopString(&bstrBestResult);
						pIInkRecoResult->Release();
						pIInkRecoResult = NULL;

						// Show the result string
						if (SUCCEEDED(hr) && bstrBestResult)
						{
							MessageBoxW(hWnd, bstrBestResult,
								L"Recognition Results", MB_OK);
							SysFreeString(bstrBestResult);
						}
					}
					// Reset the recognition context
					g_pIInkRecoContext->putref_Strokes(NULL);
				}
				pIInkStrokes->Release();
			}
			// restore the cursor
			::SetCursor(hCursor);
		}
		else
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
        break;
    case WM_PAINT:
	case WM_DISPLAYCHANGE:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			OnRender(hWnd); 
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



HRESULT LoadBitmapFromFile(
	ID2D1RenderTarget* pRenderTarget,
	IWICImagingFactory* pIWICFactory,
	PCWSTR uri,
	UINT destinationWidth,
	UINT destinationHeight,
	ID2D1Bitmap** ppBitmap
)
{
	HRESULT hr = S_OK;

	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICStream* pStream = NULL;
	IWICFormatConverter* pConverter = NULL;
	IWICBitmapScaler* pScaler = NULL;
	void* pvImageBits = nullptr;
	hr = pIWICFactory->CreateDecoderFromFilename(
		uri,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);
	if (SUCCEEDED(hr))
	{

		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}

	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}
	hr = pConverter->Initialize(
		pSource,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		NULL,
		0.f,
		WICBitmapPaletteTypeMedianCut
	);
	UINT originalWidth, originalHeight;
	hr = pConverter->GetSize(&originalWidth, &originalHeight);
	if (SUCCEEDED(hr))
	{
	
		// If a new width or height was specified, create an
		// IWICBitmapScaler and use it to resize the image.
		if (destinationWidth != 0 || destinationHeight != 0)
		{
			
			if (SUCCEEDED(hr))
			{
				if (destinationWidth == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
					destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
				}
				else if (destinationHeight == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
					destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
				}

				hr = pIWICFactory->CreateBitmapScaler(&pScaler);
				
				if (SUCCEEDED(hr))
				{
					IWICBitmap* pIBitmap = NULL;
	IWICBitmapLock* pILock = NULL;
	WICRect rcLock = { 0, 0, originalWidth, originalHeight };
	UINT cbBufferSize = 0;
	unsigned char* pv = NULL;
	unsigned int size;
	unsigned int stride;
	hr = pIWICFactory->CreateBitmapFromSource(
		pConverter,          // Create a bitmap from the image frame
		WICBitmapCacheOnDemand,  // Cache metadata when needed
		&pIBitmap);
	if (SUCCEEDED(hr))
	{
		// Obtain a bitmap lock for exclusive write.
		// The lock is for a 10x10 rectangle starting at the top left of the
		// bitmap.
		hr = pIBitmap->Lock(&rcLock, WICBitmapLockWrite, &pILock);
		if (SUCCEEDED(hr))
		{
			
			// Retrieve a pointer to the pixel data.
			if (SUCCEEDED(hr))
			{
				hr = pILock->GetDataPointer(&cbBufferSize, &pv);
				pILock->GetStride(&stride);

			}
			std::wstringstream wss;
			std::wstring ws;
			wss << stride;
			wss >> ws;
			//MessageBoxW(*phWnd, ws.c_str(),
				//L"Recognition Results", MB_OK);
			unsigned char r, b, g, a,gray;
			for (unsigned int i = 0; i < (unsigned int)originalHeight; i++)
			{
				for (unsigned int j = 0; j < (unsigned int)originalWidth; j++)
				{
					a=pv[(i * originalWidth + j) * 4 + 3];
					r=pv[(i * originalWidth + j) * 4 + 2];
					g=pv[(i * originalWidth + j) * 4 + 1] ;
					b=pv[(i * originalWidth + j) * 4 + 0] ;
/*
					r = r * 0.299;
					g = g * 0.587;
					b = b * 0.144;*/
					gray = (r+b+g)/3;

					pv[(i * originalWidth + j) * 4 + 3] = a;
					pv[(i * originalWidth + j) * 4 + 2] = gray;
					pv[(i * originalWidth + j) * 4 + 1] = gray;
					pv[(i * originalWidth + j) * 4 + 0] = gray;
				}
			}
			// Pixel manipulation using the image data pointer pv.
			// ...

			// Release the bitmap lock.
			SafeRelease(&pILock);
		}
		
	}
	if (SUCCEEDED(hr))
	{
		hr = pScaler->Initialize(
			pIBitmap,
			destinationWidth,
			destinationHeight,
			WICBitmapInterpolationModeCubic
		);
	}
					
				}
			}
		}
		else // Don't scale the image.
		{
			IWICBitmap* pIBitmap = NULL;
			IWICBitmapLock* pILock = NULL;
			WICRect rcLock = { 0, 0, originalWidth, originalHeight };
			UINT cbBufferSize = 0;
			unsigned char* pv = NULL;
			unsigned int size;
			unsigned int stride;
			hr = pIWICFactory->CreateBitmapFromSource(
				pSource,          // Create a bitmap from the image frame
				WICBitmapCacheOnDemand,  // Cache metadata when needed
				&pIBitmap);
			if (SUCCEEDED(hr))
			{
				// Obtain a bitmap lock for exclusive write.
				// The lock is for a 10x10 rectangle starting at the top left of the
				// bitmap.
				hr = pIBitmap->Lock(&rcLock, WICBitmapLockWrite, &pILock);
				if (SUCCEEDED(hr))
				{

					// Retrieve a pointer to the pixel data.
					if (SUCCEEDED(hr))
					{
						hr = pILock->GetDataPointer(&cbBufferSize, &pv);
						pILock->GetStride(&stride);

					}

					for (int i = 0; i < (int)originalHeight; i++)
					{
						for (int j = 0; j < (int)originalWidth; j++)
						{
							pv[j] = 255 * ((i / 100) % 2) * ((j / 100) % 2); // 100x100 squares
						}
						pv += stride;
					}
					// Pixel manipulation using the image data pointer pv.
					// ...

					// Release the bitmap lock.
					SafeRelease(&pILock);
				}

			}
			hr = pConverter->Initialize(
				pIBitmap,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL,
				0.f,
				WICBitmapPaletteTypeMedianCut
			);
		}
	}
	

	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pScaler,
			NULL,
			ppBitmap
		);
		
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pStream);
	SafeRelease(&pConverter);
	SafeRelease(&pScaler);

	return hr;
}