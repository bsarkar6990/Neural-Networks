// NNBinaryDecoder.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "NeuralNet.h"
using namespace std;
int main()
{
	srand((double)time(NULL));

	NeuralNet* neuralnet = new NeuralNet(3), *tnet;
	cout << "Provide Number of Hidden Layers: ";
	int hLayers;
	cin >> hLayers;
	cout << "Initializing hidden layers..";
	neuralnet->initlayer(hLayers);

	cout << "Setting Input:\n\n";
	neuralnet->in[0] = 0.0f;
	neuralnet->in[1] = 1.0f;
	neuralnet->in[2] = 0.0f;

	cout << "forwardfeed:\n";
	neuralnet->forwardfeed(1);

	cout << "Training:\n";
	double target[3] = {0.0f,1.0f,0.0f};
	double r = 0.0f;
	for (int i = 0; i <= 50; i++) {
		neuralnet->player->backpropagation(target);

		cout << "forwardfeed:\n";
		neuralnet->forwardfeed(0);
	} 

}
