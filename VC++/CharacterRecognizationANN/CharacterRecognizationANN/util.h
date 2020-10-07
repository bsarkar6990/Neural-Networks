#pragma once

#include <Windows.h>
#include <d2d1.h>
#include "resource.h"
#include <shlobj.h>
#include <string>
#include <vector>
#include <wincodec.h>
#include <commdlg.h>
#include "NeuralNet.h"
#include <iostream>

using namespace std;
#define DEFAULT_WIN_WIDTH     400
#define DEFAULT_WIN_HEIGHT    400
#define PROCESS_IMG_WIDTH     20
#define PROCESS_IMG_HEIGHT    20
#define TRAINING_SAMPLES	2
#define TRAINING_SAMPLES_SET 3
#define TRAINING_ITERATION	  TRAINING_SAMPLES*TRAINING_SAMPLES_SET*5000
const float DEFAULT_DPI = 96.f;

int ProcessANN(HINSTANCE hInstance);
HRESULT CreatePreview(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT OnPaint(HWND hWnd);
HRESULT Recognize(HWND hWnd);
HRESULT TrainANN(HWND hWnd);

bool LocateImageFile(HWND hWnd, LPSTR pszFileName, DWORD cchFileName);
bool LocateTrainingSet(HWND hWnd, LPSTR pszFileName);
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

vector<string> getFilesFolders(string path, bool isFile);
char getTrainingSample(string * path,int i,int *curSample);

HRESULT CreateD2DBitmapFromFile(HWND hWnd, LPCWSTR pszFileName,float* trainingOutput=nullptr);
HRESULT CreateDeviceResources(HWND hWnd);

void initNeuralNet(int sampleCount=1);