// NNHelloWorld.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <math.h>
using namespace std;

double relu(double x) {
	if (x > 0.0f) {
		return x;
	}
	return 0.0f;
}

double drelu(double x) {
	if (x > 0.0f) {
		return 1.0f;
	}
	return 0.0f;
}

double sigmoid(double x) {
	return (1 / (1 + exp(-x)));
}

double dsigmoid(double x) {
	return (sigmoid(x) *(1- sigmoid(x)));
}

double softmax(int c,double* i) {
	return (exp(i[c])/(exp(i[0])+ exp(i[1])+ exp(i[2])));
}

double dsoftmax(int c, double* i) {
	return ((exp(i[c])*(exp(i[(int)((1 + c) % 3)])+exp(i[(int)((2 + c) % 3)])))/pow((exp(i[0])+exp(i[1]) + exp(i[2])),2.0f));
}

void forwardfeed(double* i, double w1[][3], double* hi1, double* ho1, double w2[][3], double* hi2, double* ho2, double w3[][3], double* oi, double *o) {
	double sum = 0.0f;
	double bias = 0.0f;
	//calculate hidden layer
	for (int c = 0; c < 3; c++) {
		sum = 0.0f;
		for (int d = 0; d < 3; d++) {
			sum += (i[d] * w1[d][c]);
		}
		hi1[c] = sum + bias;
		ho1[c] = relu(hi1[c]);
		//cout << ho1[c] << " ";
	}
	//cout << "\n";
	for (int c = 0; c < 3; c++) {
		sum = 0.0f;
		for (int d = 0; d < 3; d++) {
			sum += (ho1[d] * w2[d][c]);
		}
		hi2[c]=sum + bias;
		ho2[c] = sigmoid(hi2[c]);
		//cout << ho2[c] << " ";
	}
	//cout << "\n";
	for (int c = 0; c < 3; c++) {
		sum = 0.0f;
		for (int d = 0; d < 3; d++) {
			sum += (ho2[d] * w3[d][c]);
		}
		oi[c] = (sum + bias);
//		cout << oi[c] << " ";
	}
//	cout << "\n";
	for (int c = 0; c < 3; c++) {
		o[c] = softmax(c, oi);
		//cout << o[c] << " ";
	}
}

//Derivative of Cross-Entropy
double derror(double y, double o) {
	return -1*((y/o)+((1-y)/((1-o))));
}

void backpropagation(double* i, double w1[][3], double* hi1, double* ho1, double w2[][3], double* hi2, double* ho2, double w3[][3], double* oi, double* o,double *y,double lr) {
	
	//back propagation
	//cout << "values of derivative of cross-entropy wrt output.\n";
	double deo[3];
	for (int c = 0; c < 3; c++) {
		deo[c] = derror(y[c], o[c]);
		//cout << deo[c] << " ";
	}
	///cout << "\n\nvalues of derivative of softmax wrt output layer input \n";
	//Matrix of Derivative of softmax wrt output layer input
	double doi[3];
	for (int c = 0; c < 3; c++) {
		doi[c] = dsoftmax(c, oi);
		//cout << doi[c] << " ";
	}
	//cout << "\n\n";
	double delw3[3][3];
	for (int c = 0; c < 3; c++) {
		for (int d = 0; d < 3; d++) {
			delw3[c][d] = deo[c] * doi[c] * ho2[d];
			//cout << delw3[c][d] << " ";
		}
		//cout << "\n";
	}
	//cout << "\nModified weights of kl neurons after backprop\n";
	double nw3[3][3];
	for (int c = 0; c < 3; c++) {
		for (int d = 0; d < 3; d++) {
			nw3[c][d] = w3[c][d] - (lr * delw3[c][d]);
			//cout << nw3[c][d] << " ";
		}
		//cout << "\n";
	}
	//cout << "\nValues of derivative of output of layer-2 wrt input of layer1\n";
	//hidden layer 2

	double dh2[3];
	for (int c = 0; c < 3; c++) {
		dh2[c] = dsigmoid(hi2[c]);
		//cout << dh2[c] << "\n";
	}

	//cout << "\nderivative of total error wrt output of hidden layer-2\n";

	double dteh2[3];
	double sum = 0.0f;
	for (int c = 0; c < 3; c++) {
		sum = 0.0f;
		for (int d = 0; d < 3; d++) {
			sum += deo[d] * doi[d] * w3[c][d];
		}
		dteh2[c] = sum;
		//cout << sum << " ";
	}
	//cout << "\n\ndelta weights for hidden layer 1 and 2\n";
	double delw2[3][3];
	for (int c = 0; c < 3; c++) {
		for (int d = 0; d < 3; d++) {
			delw2[c][d] = dteh2[c] * dh2[c] * ho1[d];
			//cout << delw2[c][d] << " ";
		}
		//cout << "\n";
	}
	//cout << "\nModified weights of kl neurons after backprop\n";
	double nw2[3][3];
	for (int c = 0; c < 3; c++) {
		for (int d = 0; d < 3; d++) {
			nw2[c][d] = w2[c][d] + (lr * delw2[c][d]);
			//cout << nw2[c][d] << " ";
		}
		//cout << "\n";
	}

	//cout << "\nderivative of total error wrt output of hidden layer-1\n";

	double dteh1[3];
	sum = 0.0f;
	for (int c = 0; c < 3; c++) {
		sum = 0.0f;
		for (int d = 0; d < 3; d++) {
			sum += dteh2[d] * dh2[d] * w2[c][d];
		}
		dteh1[c] = sum;
	//	cout << sum << " ";
	}
	//cout << "\n\ndelta weights for input and hidden layer 1\n";
	double delw1[3][3];
	for (int c = 0; c < 3; c++) {
		for (int d = 0; d < 3; d++) {
			delw1[c][d] = dteh1[c] * 1 * i[d];
//			cout << delw1[c][d] << " ";
		}
	//	cout << "\n";
	}
//	cout << "\nModified weights of kl neurons after backprop\n";
	double nw1[3][3];
	for (int c = 0; c < 3; c++) {
		for (int d = 0; d < 3; d++) {
			nw1[c][d] = w1[c][d] - (lr * delw1[c][d]);
	//		cout << nw1[c][d] << " ";
		}
	//	cout << "\n";
	}
	for (int c = 0; c < 3; c++) {
		for (int d = 0; d < 3; d++) {
			w1[c][d] = nw1[c][d];
			w2[c][d] = nw2[c][d];
			w3[c][d] = nw3[c][d];
		}
	}
}

int main()
{

	double i[3] = {0.1,0.2,0.7};
	double hi1[3],hi2[3],ho1[3], ho2[3],oi[3];
	double w1[3][3] = { {0.1,0.2,0.3},{0.3,0.2,0.7},{0.4,0.3,0.9} };
	double w2[3][3] = { {0.2,0.3,0.5},{0.3,0.5,0.7},{0.6,0.4,0.8} };
	double w3[3][3] = { {0.1,0.4,0.8},{0.3,0.7,0.2},{0.5,0.2,0.9} };
	double y[3] = {1.0,0.0,0.0},o[3];
	long itr=0;
	double error = 0.0f;
	double lr = 0.1f;
	forwardfeed(i, w1, hi1, ho1, w2, hi2, ho2, w3, oi, o);
	do {
		error = 0.0f;
		for (int c = 0; c < 3; c++) {
			error += ((y[c] * log(o[c])) + ((1 - y[c] * (log(1 - o[c])))));
		}
		error = -error;
		cout << error<<" "<<itr <<" "<< abs(error) - 0.755f<<" -- ";
		for (int c = 0; c < 3; c++) {
			cout << o[c] << " ";
		}
		cout << "\n";
		backpropagation(i, w1, hi1, ho1, w2, hi2, ho2, w3, oi, o,y,lr);
		forwardfeed(i, w1, hi1, ho1, w2, hi2, ho2, w3, oi, o);
		if(itr%20000==0)
			lr =lr/10 ;
		itr++;

	}while (abs(error)- 0.755f>0.00283f );
	cout << "Final: \n";
	for (int c = 0; c < 3; c++) {
		cout << o[c] << " ";
	}
	return 0;
}
