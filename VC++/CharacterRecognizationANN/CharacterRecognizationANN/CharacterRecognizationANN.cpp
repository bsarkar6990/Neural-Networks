
#include "util.h"

struct ImageSet {
	char character;
	string sampleDir;
	vector<string> samples;
};

ID2D1HwndRenderTarget* m_pRT;
IWICFormatConverter* m_pConvertedSourceBitmap;
string trainingSamplePath;
vector<ImageSet> TrainingSample;
NeuralNet* nNet;
char config[] = "charRecNN.config";

template <typename T>
inline void SafeRelease(T*& p)
{
	if (nullptr != p)
	{
		p->Release();
		p = nullptr;
	}
}
int main() {
	HWND hInstance = NULL;
	GetWindowLong(
		hInstance,
		GWL_HINSTANCE
	);

	return ProcessANN((HINSTANCE)hInstance);
}
int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR pszCmdLine,
	_In_ int nCmdShow)
{

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pszCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	return ProcessANN((HINSTANCE)hInstance);
}
int ProcessANN(HINSTANCE hInstance){
	srand((float)time(NULL));
	HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		hr= CreatePreview(hInstance);
		if (SUCCEEDED(hr))
		{
			MSG msg = { 0 };
			while (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		CoUninitialize();
	}

	return 0;
}


HRESULT CreatePreview(HINSTANCE hInstance) {
	HRESULT hr = S_OK;
	WNDCLASSEX wc;
	HWND hWnd;
	ZeroMemory(&wc, sizeof wc);
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_NO);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wc.lpszClassName = "CharacterRecognizationANN";
	wc.hIconSm = LoadIcon(wc.hInstance, IDI_APPLICATION);
	hr = RegisterClassEx(&wc) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		// Create window
		hWnd = CreateWindow(
			"CharacterRecognizationANN",
			"Image Preview",
			WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_CLIPCHILDREN | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			DEFAULT_WIN_WIDTH,
			DEFAULT_WIN_HEIGHT,
			nullptr,
			nullptr,
			hInstance,
			0
		);
		if (hWnd!=0) {
			SetWindowLong(hWnd, GWL_STYLE,
				GetWindowLong(hWnd, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
		}
		hr = hWnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			hr = CreateDeviceResources(hWnd);
		}
	}
	//initNeuralNet();
	return hr;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		// Parse the menu selections:
		switch (LOWORD(wParam))
		{
		case ID_RECOGNIZE:
		{
			Recognize(hWnd);
			break;
		}
		case ID_TRAIN_ANN:
		{
			TrainANN(hWnd);
			break;
		}
		}
		break;
	}
	case WM_PAINT:
	{
		return OnPaint(hWnd);
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}
LRESULT OnPaint(HWND hWnd)
{
	HRESULT hr = S_OK;
	PAINTSTRUCT ps;
	ID2D1Bitmap* m_pD2DBitmap=nullptr;
	if (BeginPaint(hWnd, &ps))
	{
		// Create render target if not yet created
		//hr = CreateDeviceResources(hWnd);
		if (SUCCEEDED(hr) && !(m_pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
		{
			m_pRT->BeginDraw();
			m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

			// Clear the background
			m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

			auto rtSize = m_pRT->GetSize();

			// Create a rectangle with size of current window
			auto rectangle = D2D1::RectF(0.0f, 0.0f, rtSize.width, rtSize.height);

			// D2DBitmap may have been released due to device loss. 
			// If so, re-create it from the source bitmap
			if (m_pConvertedSourceBitmap && !m_pD2DBitmap)
			{
				m_pRT->CreateBitmapFromWicBitmap(m_pConvertedSourceBitmap, nullptr, &m_pD2DBitmap);
			}

			// Draws an image and scales it to the current window size
			if (m_pD2DBitmap!=nullptr)
			{
				m_pRT->DrawBitmap(m_pD2DBitmap, rectangle);
			}

			hr = m_pRT->EndDraw();
			SafeRelease(m_pD2DBitmap);
			SafeRelease(m_pConvertedSourceBitmap);
			// In case of device loss, discard D2D render target and D2DBitmap
			// They will be re-created in the next rendering pass
			if (hr == D2DERR_RECREATE_TARGET)
			{
				SafeRelease(m_pD2DBitmap);
				SafeRelease(m_pRT);
				// Force a re-render
				hr = InvalidateRect(hWnd, nullptr, TRUE) ? S_OK : E_FAIL;
			}
		}

		EndPaint(hWnd, &ps);
	}

	return SUCCEEDED(hr) ? 0 : 1;
}


HRESULT Recognize(HWND hWnd) {
	HRESULT hr = S_OK;
	TCHAR szFileName[MAX_PATH];
	if (LocateImageFile(hWnd, szFileName, ARRAYSIZE(szFileName)) == true) {
		string path= szFileName;
		wstring stemp = wstring(path.begin(), path.end());
		if (SUCCEEDED(CreateD2DBitmapFromFile(hWnd, stemp.c_str())))
		{
			InvalidateRect(hWnd, nullptr, TRUE);
		}
	}
	return hr;
}
HRESULT TrainANN(HWND hWnd) {
	HRESULT hr = S_OK;
	TCHAR szFileName[MAX_PATH]; 
	float* trainingOutput=nullptr;
	if (LocateTrainingSet(hWnd, szFileName) == true)
	{
		if (!TrainingSample.empty()) {
			TrainingSample.clear();
		}
		trainingSamplePath= szFileName;
		vector<string> samples = getFilesFolders(trainingSamplePath, false);
		trainingOutput = (float*)malloc(sizeof(float) * TRAINING_SAMPLES);
		for (int i = 0; i < TRAINING_SAMPLES; i++) {
			ImageSet tv;
			tv.character = (char)samples[i].substr(0, samples[i].find("_")).c_str()[0];
			tv.sampleDir = samples[i];
			vector<string> fl = getFilesFolders((trainingSamplePath + "\\" + samples[i]).c_str(), true);
			tv.samples = fl;
			TrainingSample.push_back(tv);
			trainingOutput[i] = 0.0f;
		}
		initNeuralNet(TRAINING_SAMPLES);
		if (!samples.empty()) {
			samples.clear();
		}
		int prevSample = 0,curSample=0,j;
		nNet->enabletrain = true;
		for (int i = 0; i < TRAINING_ITERATION; i++) {
			string path = "\0";
			nNet->setSamples((float)i + 1);
			j = rand() % TRAINING_SAMPLES* TRAINING_SAMPLES_SET;
			char character = getTrainingSample(&path, j,&curSample);
			wstring stemp = wstring(path.begin(), path.end());
			trainingOutput[prevSample] = 0.0f;
			trainingOutput[curSample] = 1.0f;
			cout << i <<"            ";
			if (SUCCEEDED(CreateD2DBitmapFromFile(hWnd, stemp.c_str(), trainingOutput)))
			{
				
				prevSample = curSample;
				//InvalidateRect(hWnd, nullptr, TRUE);
			}
		}

		nNet->enabletrain = false;
		nNet->saveconfig(config);
	}
	if (!TrainingSample.empty()) {
		TrainingSample.clear();
	}
	return hr;
}

bool LocateImageFile(HWND hWnd, LPSTR pszFileName, DWORD cchFileName)
{
	pszFileName[0] = L'\0';

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter ="All Image Files\0"              "*.bmp;*.dib;*.wdp;*.mdp;*.hdp;*.gif;*.png;*.jpg;*.jpeg;*.tif;*.ico\0"
		"Windows Bitmap\0"               "*.bmp;*.dib\0"
		"High Definition Photo\0"        "*.wdp;*.mdp;*.hdp\0"
		"Graphics Interchange Format\0"  "*.gif\0"
		"Portable Network Graphics\0"    "*.png\0"
		"JPEG File Interchange Format\0" "*.jpg;*.jpeg\0"
		"Tiff File\0"                    "*.tif\0"
		"Icon\0"                         "*.ico\0"
		"All Files\0"                    "*.*\0"
		"\0";
	ofn.lpstrFile = pszFileName;
	ofn.nMaxFile = cchFileName;
	TCHAR path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, path);
	ofn.lpstrInitialDir = path;
	ofn.lpstrTitle = "Select Image";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	return (GetOpenFileName(&ofn) == TRUE);
}

bool LocateTrainingSet(HWND hWnd, LPSTR pszFileName)
{
	bool retVal = false;
	pszFileName[0] = '\0';
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	TCHAR path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, path);
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_EDITBOX;
	bi.hwndOwner = hWnd;
	bi.lpszTitle = "Select Training Set Folder";
	bi.lParam = (LPARAM)strcat(path,"\\lib");
	bi.lpfn = BrowseCallbackProc;
	LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

	if (pIDL != NULL)
	{
		if (::SHGetPathFromIDList(pIDL, pszFileName) != 0)
		{
			retVal = true;
		}
		CoTaskMemFree(pIDL);
	}
	return retVal;
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
			if (NULL != lpData)
			{
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
			}
		break;
	}


	return 0; // The function should always return 0.
}

vector<string> getFilesFolders(string path, bool isFile) {
	vector<string> filesFolders;
	string  searchpath = path+ "/*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(searchpath.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..") && ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !isFile)) {
				filesFolders.push_back(fd.cFileName);
			}
			else if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..") && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && isFile) {
				filesFolders.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return filesFolders;
}

char getTrainingSample(string *path, int i, int* curSample) {
	char character = '\0';
	int k=0, l=0, ssize = TrainingSample[0].samples.size(),size= TrainingSample.size();
	if (i >= 0 && i< TRAINING_SAMPLES_SET * TRAINING_SAMPLES) {
		k = floor(i / TRAINING_SAMPLES_SET);
		l = i - k * TRAINING_SAMPLES_SET;
		*path = trainingSamplePath+"\\" + TrainingSample[k].sampleDir+"\\"+TrainingSample[k].samples.at(l);
		character = TrainingSample[k].character;
		*curSample = k;
	}
	return character;
}


HRESULT CreateD2DBitmapFromFile(HWND hWnd, LPCWSTR szFileName, float* trainingOutput)
{
	HRESULT hr = S_OK;
	IWICImagingFactory* m_pIWICFactory;
	IWICBitmapScaler* pScaler = NULL;
	IWICBitmap* pIBitmap = NULL;
	IWICBitmapLock* pILock = NULL;
	UINT originalWidth, originalHeight;
	UINT cbBufferSize = 0;
	unsigned int size;
	unsigned int stride;
	IWICFormatConverter* pConverter = NULL;
	unsigned char* pv = nullptr;
	hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&m_pIWICFactory)
	);

	if (SUCCEEDED(hr))
	{
		IWICBitmapDecoder* pDecoder = nullptr;

		hr = m_pIWICFactory->CreateDecoderFromFilename(
			szFileName,                      // Image to be decoded
			nullptr,                         // Do not prefer a particular vendor
			GENERIC_READ,                    // Desired read access to the file
			WICDecodeMetadataCacheOnDemand,  // Cache metadata when needed
			&pDecoder                        // Pointer to the decoder
		);

		// Retrieve the first frame of the image from the decoder
		IWICBitmapFrameDecode* pFrame = nullptr;

		if (SUCCEEDED(hr))
		{
			hr = pDecoder->GetFrame(0, &pFrame);
		}
		SafeRelease(pDecoder);
		
		hr = m_pIWICFactory->CreateFormatConverter(&pConverter);
		if (SUCCEEDED(hr))
		{
			hr = pConverter->Initialize(
				pFrame,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL,
				0.f,
				WICBitmapPaletteTypeMedianCut
			);
		}
		SafeRelease(pFrame);

		RECT rc;
		hr = GetClientRect(hWnd, &rc) ? S_OK : E_FAIL;
		originalWidth = rc.right - rc.left;
		originalHeight = rc.bottom - rc.top;
		WICRect rcLock = { 0, 0, PROCESS_IMG_WIDTH, PROCESS_IMG_HEIGHT };

		hr = m_pIWICFactory->CreateBitmapScaler(&pScaler);
		if (SUCCEEDED(hr))
		{
			hr = pScaler->Initialize(
				pConverter,
				PROCESS_IMG_WIDTH,
				PROCESS_IMG_HEIGHT,
				WICBitmapInterpolationModeCubic
			);
		}
		hr = m_pIWICFactory->CreateBitmapFromSource(
			pScaler,          // Create a bitmap from the image frame
			WICBitmapCacheOnDemand,  // Cache metadata when needed
			&pIBitmap);
		SafeRelease(pScaler);
		if (SUCCEEDED(hr))
		{
			hr = pIBitmap->Lock(&rcLock, WICBitmapLockWrite, &pILock);
			if (SUCCEEDED(hr))
			{

				// Retrieve a pointer to the pixel data.
				if (SUCCEEDED(hr))
				{
					
					hr = pILock->GetDataPointer(&cbBufferSize, &pv);
					pILock->GetStride(&stride);

				}
				/*Image Processing Starts*/
				if (trainingOutput != nullptr)
				{
					float a, r, b, g, gray = 0.0f;
					for (unsigned int i = 0; i < (unsigned int)PROCESS_IMG_HEIGHT; i++)
					{
						for (unsigned int j = 0; j < (unsigned int)PROCESS_IMG_WIDTH; j++)
						{
							b = (float)pv[(i * PROCESS_IMG_WIDTH + j) * 4 + 0] / 255.0f;
							g = (float)pv[(i * PROCESS_IMG_WIDTH + j) * 4 + 1] / 255.0f;
							r = (float)pv[(i * PROCESS_IMG_WIDTH + j) * 4 + 2] / 255.0f;
							a = (float)pv[(i * PROCESS_IMG_WIDTH + j) * 4 + 3] / 255.0f;
							gray = (0.2125 * r + 0.7154 * b + 0.0721 * g);
							nNet->in[i * PROCESS_IMG_WIDTH + j] = 1-gray;
						}
					}
					nNet->trainPreload(trainingOutput);
					nNet->feedforward();
					cout << nNet->getError() << "\n";
				}
				else {
					nNet->setImgin(pv, PROCESS_IMG_WIDTH, PROCESS_IMG_HEIGHT);
					nNet->feedforward();
					nNet->getfilteroutput();
					nNet->showoutput();
				}
				
				/*Image Processing Ends*/
				SafeRelease(pILock);
			}
		}
		SafeRelease(pIBitmap);
		if (trainingOutput == nullptr) {
			if (SUCCEEDED(hr))
			{
				SafeRelease(m_pConvertedSourceBitmap);
				hr = m_pIWICFactory->CreateFormatConverter(&m_pConvertedSourceBitmap);
			}
			if (SUCCEEDED(hr))
			{

				hr = m_pConvertedSourceBitmap->Initialize(
					pConverter,                          // Input bitmap to convert
					GUID_WICPixelFormat32bppPBGRA,   // Destination pixel format
					WICBitmapDitherTypeNone,         // Specified dither pattern
					nullptr,                         // Specify a particular palette 
					0.f,                             // Alpha threshold
					WICBitmapPaletteTypeCustom       // Palette translation type
				);

			}
		}
		SafeRelease(pConverter);
	}
	SafeRelease(m_pIWICFactory);

	return S_OK;
}
HRESULT CreateDeviceResources(HWND hWnd)
{
	HRESULT hr = S_OK;
	ID2D1Factory* m_pD2DFactory=nullptr;

	if (!m_pRT)
	{
		RECT rc;
		hr = GetClientRect(hWnd, &rc) ? S_OK : E_FAIL;

		if (SUCCEEDED(hr))
		{
			auto renderTargetProperties = D2D1::RenderTargetProperties();

			// Set the DPI to be the default system DPI to allow direct mapping
			// between image pixels and desktop pixels in different system DPI settings
			renderTargetProperties.dpiX = DEFAULT_DPI;
			renderTargetProperties.dpiY = DEFAULT_DPI;

			auto size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
			hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

			if (SUCCEEDED(hr))
			{
				hr = m_pD2DFactory->CreateHwndRenderTarget(
					renderTargetProperties,
					D2D1::HwndRenderTargetProperties(hWnd, size),
					&m_pRT
				);
			}
			SafeRelease(m_pD2DFactory);
		}
	}

	return hr;
}
void initNeuralNet(int sampleCount) {
	int imagedim = PROCESS_IMG_WIDTH * PROCESS_IMG_HEIGHT;
	int topology[] = { imagedim,100,100,100,sampleCount };
	if (nNet != NULL) {
		nNet->~NeuralNet();
	}
	nNet = new NeuralNet(topology, sizeof(topology) / sizeof(int));
	//nNet->loadconfig(config);
}