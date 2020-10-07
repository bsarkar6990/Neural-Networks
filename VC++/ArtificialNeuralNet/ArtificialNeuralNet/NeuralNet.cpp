#include "NeuralNet.h"

float NeuralNet::eta = 0.05f; // overall net learning rate
float NeuralNet::alpha = 0.4f; // momentum, multiplier of last deltaWeight, [0.0..n]
float NeuralNet::avgSmtFactor = 15000.0f; // Number of training samples to average over
int NeuralNet::TLayers = 1;
float NeuralNet::errorsum=0;
float NeuralNet::samples=0;
bool NeuralNet::enabletrain = false;
int* NeuralNet::ntopology = nullptr;
NeuralNet::NeuralNet(int* topology,int size) {
	TLayers = size - 1;
	ntopology = topology;
	inCnt = topology[0]+1;
	outCnt = topology[0];
	nLayer = nullptr;
	pLayer = nullptr;
	layerno = 0;
	weight = nullptr;
	dweight = nullptr;
	pdweight = nullptr;
	in = new float[inCnt];
	bias = 1.0f;
	error = 0.0f;
	avgError = 0.0f;
	gradient = nullptr;
	out = in;
	tout = out;
	for (int i = 0; i < inCnt; i++) {
		in[i] = 0.0f;
	}
	in[inCnt - 1] = 1.0f;
	NeuralNet* nNet=this;
	for (int i = 0; i < size - 1; i++) {
		NeuralNet* hLayer = new NeuralNet(topology[i], topology[i+1]);
		hLayer->layerno = i + 1;
		nNet->nLayer = hLayer;
		hLayer->pLayer = nNet;
		nNet = hLayer;
		//cout << "Layer no: " << hLayer->layerno << "\n";
		//nNet->showweights();
	}
	this->pLayer = nNet;
}
NeuralNet::NeuralNet(int m, int n) {
	inCnt = m+1;
	outCnt = n;
	pLayer = nullptr;
	nLayer = nullptr;
	layerno = 0;
	in = new float[inCnt];
	out = new float[outCnt];
	tout = new float[outCnt];
	weight = new float* [inCnt];
	dweight = new float* [inCnt];
	pdweight = new float* [inCnt];
	error = 0.0f;
	avgError = 0.0f;
	bias = 1.0f;
	gradient = new float[outCnt];
	int flag = 0;
	for (int i = 0; i < inCnt; i++) {
		in[i] = 0.0f;
		weight[i] = new float [outCnt];
		dweight[i] = new float [outCnt];
		pdweight[i] = new float [outCnt];
		for (int j = 0; j < outCnt; j++) {
			weight[i][j] = (float)rand() / float(RAND_MAX) ;
			dweight[i][j] = 0.0f;
			pdweight[i][j] = 0.0f;
			if (flag == 0) {
				out[j] = 0.0f;
				gradient[j] = 0.0f;
				flag = 1;
			}
		}
	}
	in[inCnt - 1] = 1.0f;
}
int NeuralNet::thrashold(float x) {
	if (x > 0.4f) {
		return 1;
	}
	return 0;
}
void NeuralNet::showweights() {
	for (int i = 0; i < inCnt; i++) {
		for (int j = 0; j < outCnt; j++) {
			cout << weight[i][j] << " ";
		}
		cout << "\n";
	}
}
void NeuralNet::showoutput() {
	NeuralNet* nNet = this;
	while (nNet->layerno != TLayers) {
		nNet = nNet->pLayer;
	}
	for (int j = 0; j < nNet->outCnt; j++) {
		cout << thrashold(nNet->out[j] )<< " ";
	}
	cout << "\n";
}
float* NeuralNet::getoutput() {
	return pLayer->out;
}
int* NeuralNet::getfilteroutput() {
	int* o = new int[pLayer->outCnt];
		for (int i = 0; i < pLayer->outCnt; i++) {
			o[i] = thrashold(pLayer->out[i]);
		}
	return o;
}
void  NeuralNet::setin(int* i) {
	for (int j = 0; j < inCnt; j++) {
		in[j]=i[j];
	}
}
void NeuralNet::showin() {
	cout << "Layer " << layerno << " in: ";
	for (int j = 0; j < inCnt; j++) {
		cout << in[j] << " ";
	}
	cout << "\n";
}
void NeuralNet::showout() {
	cout << "Layer " << layerno << " out: ";
	for (int j = 0; j < outCnt; j++) {
		cout << out[j] << " ";
	}
	cout << "\n";
}
float NeuralNet::activation(float x) {
	return tanh(x);
}
float NeuralNet::activationderivaative(float x) {
	return  4 / pow((exp(x) + exp(-x)), 2);
}
void NeuralNet::feedforward() {
	//showin();
	if (layerno> 0) {
		float sum = 0.0f;
		for (int j = 0; j < outCnt; j++) {
			sum = 0.0f;
			for (int i = 0; i < inCnt; i++) {
				
				sum += weight[i][j] * in[i];
			}
			out[j] = activation(sum);
		}
	//	if(!enabletrain)
		//showweights();
	}
	//showout();
	if (nLayer == nullptr) {
		return;
	}
	for (int i = 0; i < outCnt; i++) {
		nLayer->in[i] = out[i];
	}
	nLayer->feedforward();
}

void NeuralNet::train(float *ti,float* to) {
	for (int i = 0; i < inCnt-1; i++) {
		in[i] = ti[i];
	}
	for (int i = 0; i < pLayer->outCnt; i++) {
		pLayer->tout[i] = to[i];
	}
	feedforward();
	pLayer->backpropagation();
}
void NeuralNet::backpropagation() {
	if (layerno == 0) {
		return;
	}
	if (layerno == TLayers) {
		error = 0.0f;
		for (int i = 0; i < outCnt; i++) {
			float delta = (tout[i] - out[i]);
			gradient[i] = delta * activationderivaative(out[i]) ;
			error += delta * delta/2;
		}
		error = sqrt(error) /sqrt((float)outCnt);
		//avgError = (avgError * avgSmtFactor + error) / (avgSmtFactor + 1.0f);
		avgError = (errorsum + error) / samples;
	}
	else {
		float sum = 0.0f;
		for (int i = 0; i < outCnt; i++) {
			sum = 0.0f;
			for (int j = 0; j < nLayer->outCnt; j++) {
				sum += nLayer->gradient[j] * nLayer->weight[i][j];
			}
			gradient[i] = sum * activationderivaative(out[i]);
		}
	}
	for (int j = 0; j < outCnt; j++) {
		for (int i = 0; i < inCnt; i++) {
			pdweight[i][j] = dweight[i][j];
			dweight[i][j] = eta * in[i] * gradient[j] + alpha * pdweight[i][j];
			weight[i][j] += dweight[i][j];
		}
	}
	pLayer->backpropagation();
}
float NeuralNet::getError() {
	return pLayer->avgError;
}
void NeuralNet::resetError() {
	pLayer->samples = 0;
	pLayer->errorsum = 0;
}
void NeuralNet::fuse() {
	in[0] = thrashold(pLayer->out[0]);
	in[1] = thrashold(pLayer->out[1]);
	in[2] = thrashold(pLayer->out[2]);
}
void NeuralNet::setSamples(float i) {
	pLayer->samples = i;
}
void  NeuralNet::saveweights(fstream& fhandle) {
	for (int i = 0; i < inCnt; i++) {
		for (int j = 0; j < outCnt; j++) {
			fhandle <<  weight[i][j] ;
			if (j < outCnt - 1) fhandle << ",";
			else fhandle << "$";
		}
		fhandle << "\n";
	}
	fhandle << "9192939$";
	if (nLayer == nullptr) {
		return;
	}
	else {
		fhandle << "\n";
	}
	nLayer->saveweights(fhandle);
}
void NeuralNet::saveconfig(char* config) {
	fstream fhandle(config, fstream::out | fstream::trunc);
	assert(fhandle.is_open());
	nLayer->saveweights(fhandle);
	fhandle.close();
}
bool NeuralNet::loadconfig(char* config) {
	fstream fhandle(config, fstream::in );
	if (!fhandle.is_open()) {
		return false;
	}
	nLayer->loadweights(fhandle);
	fhandle.close();
	return true;
}
void NeuralNet::loadweights(fstream& fhandle) {
	char ch;
	float w;
	int i = 0, j = 0,b=0;
	while (!fhandle.eof()) {
		fhandle >> w >> ch;
		if (w == 9192939 && ch == '$') {
			if (fhandle.eof()||nLayer==nullptr) {
				return;
			}
			nLayer->loadweights(fhandle);
			return;
		}
		weight[i][j] = w;
		if (ch == '$') {
			i++;
			j = 0;
		}
		else if (ch == ',') {
			j++;
		}
	}
}
