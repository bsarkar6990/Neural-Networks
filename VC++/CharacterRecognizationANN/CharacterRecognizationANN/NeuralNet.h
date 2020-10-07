#pragma once
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <cmath>
#include <fstream>
#include <sstream>
#include <assert.h>
using namespace std;
class NeuralNet
{
private:
	static int TLayers;
	static float eta;		// [0.0...1.0] overall net training rate
	static float alpha;   // [0.0...n] multiplier of last weight change [momentum]
	static float avgSmtFactor;
	static float errorsum, samples;
	static int* ntopology;
	int layerno;
	float** weight;
	float** dweight;
	float** pdweight;
	float bias;
	int inCnt;
	int outCnt;
	float error;
	float avgError;
	NeuralNet* nLayer, * pLayer;
	NeuralNet(int m, int n);
	int thrashold(float x);
	float activation(float x);
	float activationderivaative(float x);
	void backpropagation();
	void saveweights(fstream& fhandle);
	void loadweights(fstream& fhandle, int lno);
	void destroy();
public:
	float* in;
	float* out;
	float* tout;
	float *gradient;
	bool  init;
	static bool enabletrain;
	NeuralNet(int *topology, int size);
	~NeuralNet();
	void showweights();
	void showoutput();
	float* getoutput();
	int* getfilteroutput();
	void setin(int *i);
	void setImgin(unsigned char* img, unsigned int w, unsigned int h);
	void showin();
	void showout();
	void feedforward();
	void train(float* ti,float* to);
	void trainPreload(float* to);
	float getError();
	void resetError();
	void setSamples(float i);
	void saveconfig(char *config);
	bool loadconfig(char* config);
	void fuse();
};

