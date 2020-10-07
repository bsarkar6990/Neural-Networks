// ArtificialNeuralNet.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <windows.h>
#include <iostream>
#include "NeuralNet.h"
using namespace std;

int main()
{
	int topology[] = {3,8,8,3}, topology1[] = { 3,8,8,8 };
	char config[] = "counterNN.config", config1[] = "decoderNN.config";
	NeuralNet* nNet = new NeuralNet(topology, sizeof(topology) / sizeof(int));
	NeuralNet* nNet1 = new NeuralNet(topology1, sizeof(topology1) / sizeof(int));
	if (!nNet1->loadconfig(config1)) {
		float ti[][3] = { {0.0f,0.0f,0.0f},   //decoder 3,8,8,8
								{0.0f,0.0f,1.0f},
								{0.0f,1.0f,0.0f},
								{0.0f,1.0f,1.0f},
								{1.0f,0.0f,0.0f},
								{1.0f,0.0f,1.0f},
								{1.0f,1.0f,0.0f},
								{1.0f,1.0f,1.0f} },
			to[][8] = { {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f},
						{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f},
						{0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f},
						{0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f},
						{0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f},
						{0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,0.0f},
						{0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f},
						{1.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f} }, * tit, * tot;
		int j = 0,size= (sizeof(ti) / sizeof(float)) / (sizeof(ti[0]) / sizeof(float));
		nNet1->resetError();
		nNet1->enabletrain = true;
		for (int i = 0; i < 25000; i++) {
			j = rand() %size;
			nNet1->setSamples((float)i + 1);
			tit = ti[j];
			tot = to[j];
			nNet1->train(tit, tot);
			nNet1->feedforward();
			//cout << "Output: ";
			//nNet->showoutput();
			cout << nNet1->getError() << "\n";
		}
		nNet1->enabletrain = false;
		nNet1->saveconfig(config1);
	}
	if (!nNet->loadconfig(config)) {
		float ti[][3] = { {0.0f,0.0f,0.0f},
									{0.0f,0.0f,1.0f},
									{0.0f,1.0f,0.0f},
									{0.0f,1.0f,1.0f},
									{1.0f,0.0f,0.0f},
									{1.0f,0.0f,1.0f},
									{1.0f,1.0f,0.0f},
									{1.0f,1.0f,1.0f} },
				to[][3] = { {0.0f,0.0f,1.0f},
								 {0.0f,1.0f,0.0f},
								 {0.0f,1.0f,1.0f},
								 {1.0f,0.0f,0.0f},
								 {1.0f,0.0f,1.0f},
								 {1.0f,1.0f,0.0f},
								 {1.0f,1.0f,1.0f},
								 {0.0f,0.0f,0.0f} }, * tit, * tot;
		int j = 0, size = (sizeof(ti) / sizeof(float)) / (sizeof(ti[0]) / sizeof(float));
		nNet->resetError();
		nNet->enabletrain = true;
		for (int i = 0; i < 25000; i++) {
			j = rand() % size;
			nNet->setSamples((float)i + 1);
			tit = ti[j];
			tot = to[j];
			nNet->train(tit, tot);
			nNet->feedforward();
			//cout << "Output: ";
			//nNet->showoutput();
			cout << nNet->getError() << "\n";
		}
		nNet->enabletrain = false;
		nNet->saveconfig(config);
	}

	float ch = 9;
	nNet->in[0] = 0;
	nNet->in[1] = 0;
	nNet->in[2] = 0;
	while(true) {
		nNet->feedforward();
		//cout << "\nOutput: ";
		//nNet->showoutput();
		nNet->fuse();
		nNet1->setin(nNet->getfilteroutput());
		nNet1->feedforward();
		nNet1->showoutput();
		Sleep(300);
	}
}
