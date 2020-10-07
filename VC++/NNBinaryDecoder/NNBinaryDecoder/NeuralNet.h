#pragma once
#include <ctime>
#include <cstdlib>
class NeuralNet
{
private:
	double **weight;
	double** pweight;
	double *bias;
	double *axon;
	int nLayers;
	//
	double** dweight;
public:
	int inCnt;
	int outCnt;
	double* in;
	double* out;
	int layer;
	NeuralNet* nlayer, *player;
	NeuralNet(int m, int n);
	NeuralNet(int m);
	void initlayer(int hLayers);
	void forwardfeed(int c);
	void backpropagation(double *pdrv);
	void showweights();
	void showpweights();
	void showout();
	void showin();
};

