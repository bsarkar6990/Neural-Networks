#include "NeuralNet.h"
#include <iostream>
using namespace std;
NeuralNet::NeuralNet(int m,int n){
	int i, j,flag=0;
	inCnt = m;
	outCnt = n;
	nlayer = NULL;
	player = NULL;
	layer = 0;
	nLayers = 0;
	in = new double[inCnt];
	out = new double[outCnt];
	bias = new double[outCnt];
	axon = new double[outCnt];
	weight = new double* [inCnt];
	pweight = new double* [inCnt];
	dweight = new double* [inCnt];
	for (i = 0; i < inCnt; i++) {
		in[i] = 0.0f;
		weight[i] = new double [outCnt];
		pweight[i] = new double[outCnt];
		dweight[i] = new double[outCnt];
		for (j = 0; j < outCnt; j++) {
			weight[i][j] = (double)rand() / double(RAND_MAX);
			pweight[i][j] = weight[i][j];
			dweight[i][j] = 0.0f;
			if (flag == 0) {
				bias[j] = 0.0f;
				axon[j] = 0.0f;
				out[j] = 0.0f;
				flag = 1;
			}

		}
	}
}
NeuralNet::NeuralNet(int m) {
	int i;
	inCnt = m;
	outCnt = 0;
	nlayer = NULL;
	nLayers = 1;
	player = NULL;
	layer = 0;
	in = new double[inCnt];
	out = NULL;
	bias = NULL;
	axon = NULL;
	weight = NULL;
	dweight = NULL;
	pweight = NULL;
	for (i = 0; i < inCnt; i++) {
		in[i] = 0.0f;
	}
}

void NeuralNet::showweights() {
	for (int i = 0; i < inCnt; i++) {
		for (int j = 0; j < outCnt; j++) {
			cout << weight[i][j] << " ";
		}
		cout << "\n";
	}
}
void NeuralNet::showpweights() {
	for (int i = 0; i < inCnt; i++) {
		for (int j = 0; j < outCnt; j++) {
			cout << pweight[i][j] << " ";
		}
		cout << "\n";
	}
}
void NeuralNet::showin() {
	cout << "Layer " << layer << " in: ";
	for (int j = 0; j < inCnt; j++) {
		cout << in[j] << " ";
	}
	cout << "\n";
}
void NeuralNet::showout() {
	cout << "Layer "<<layer<<" out: ";
		for (int j = 0; j < outCnt; j++) {
			cout << out[j] << " ";
		}
		cout << "\n";
}
double sigmoid(double x) {
	return (1 / (1 + exp(-x)));
}
double dsigmoid(double x) {
	return (sigmoid(x) * (1 - sigmoid(x)));
}
double relu(double x) {
	if (x > 0.0f) {
		return x;
	}
	return 0.0f;
}

double activation(double in) {
	double sigma = 0.0f;
	sigma = in;
	return sigma;
}

void NeuralNet::initlayer(int hL) {
	NeuralNet * tnet;
	tnet = this;
	nLayers=hL+1;
	for (int i = 0; i < hL; i++) {
		cout << "\n";
		cout << "hidden Neuron layer " << i << " :\n";
		NeuralNet* hlayer = new NeuralNet(3, 3);
		hlayer->showweights();
		hlayer->layer = i + 1;
		tnet->nlayer = hlayer;
		tnet->nLayers = hL + 1;
		hlayer->player = tnet;
		tnet = hlayer;
	}
	cout << "\n";
	cout << "Output Neuron layer 2:\n";
	NeuralNet* olayer = new NeuralNet(3, 3);
	olayer->showweights();
	tnet->nlayer = olayer;
	olayer->layer = tnet->layer + 1;
	olayer->player = tnet;
	olayer->nLayers = hL + 1;
	this->player = olayer;
}
void NeuralNet::forwardfeed(int c) {
	if(c==1||layer==0)
	showin();
	if (out != NULL) {
		double sum = 0.0f;
		for (int i = 0; i < outCnt; i++) {
			sum = 0.0f;
			for (int j = 0; j < inCnt; j++) {
				sum += (in[j] * weight[i][j]);
			}
			axon[i] = sum;
			out[i] = sigmoid(sum);
			if (nlayer != NULL) {
				nlayer->in[i] = out[i];
			}
		}
		if (nlayer != NULL) {
			if(c==1)
			showout();
			nlayer->forwardfeed(c);
		}
		else {
			showout();
		}
		
	}
	else {
		if (nlayer != NULL) {
		//	showin();
			for (int i = 0; i < inCnt; i++) {
				if (nlayer != NULL) {
					nlayer->in[i] = in[i];
				}
			}
			nlayer->forwardfeed(c);
		}
	}
}
void NeuralNet::backpropagation(double* pdrv) {
	if (layer == nLayers) {
		cout << "\n------------------------------------------------\n";
	}
	 if (layer == 0) {
		 return;
	}
	 double* cldrv = new double[inCnt];
	 double sum = 0.0f, d = 0.0f, sig = 0.0f, e = 0.0f,te=0.0f;
	 if (layer == nLayers)
		 cout << "Error: ";
	for (int i = 0; i < inCnt; i++) {
		sum = 0.0f, d = 0.0f, sig = 0.0f, e = 0.0f;
		if (layer == nLayers)
		te +=  (double)pow((pdrv[i] - out[i]), 2)/2;
		for (int j = 0; j < outCnt; j++) {
			sig= dsigmoid(out[j]);
			if (layer == nLayers) {
				e = (pdrv[j] - out[j]);
				d = e * sig;
				
			}
			else {
				d = pdrv[i] * sig;
			}
			sum += d * weight[i][j];
			pweight[i][j] = weight[i][j];
			dweight[i][j] = (double)(d * in[i]);
			weight[i][j] = weight[i][j] + dweight[i][j];
		}
		cldrv[i] = sum;
		if (layer == nLayers) cout << e<< " ";
	}
	 if (layer == nLayers)
		 cout <<"\nTotal Error: "<< te << "\n";
	cout << "\n\nPrevious Weigths:\n";
	showpweights();
	cout << "\nNew Weigths:\n";
	showweights();
	cout << "\n";
	player->backpropagation(cldrv);
	delete[] cldrv;
}

