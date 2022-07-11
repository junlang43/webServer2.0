#include"nsga2.h"
using namespace std;

void Individual::costFunction()
{
	int Cmax[factorysize] = { 0 };
	int sumP[machinesize];
	double Tradiness[jobsize];
	int count = 0;
	jobCmax[jobsize] = { 0 };//工件最大完工
	for (vector<vector<int>>::iterator Jit = jobDecode.begin(); Jit != jobDecode.end(); Jit++) {
		int compelet[jobsize][machinesize] = { 0 };
		int subjSize;
		if ((*Jit).size() != 0) {
			memset(sumP, 0, sizeof(sumP));
			idleTime[count] = 0;
			subjSize = (*Jit).size();
			FactoryC(*Jit, compelet, sumP);
			Cmax[count] = compelet[subjSize - 1][machinesize - 1];
			for (int i = 0; i < machinesize; i++) {
				idleTime[count] += compelet[subjSize - 1][i] - sumP[i];
			}
			idleTime[count] = idleTime[count] / double(machinesize);
			for (vector<int>::reverse_iterator it = (*Jit).rbegin(); it != (*Jit).rend(); it++) {
				jobCmax[*it - 1] = compelet[--subjSize][machinesize - 1];
			}
			count++;
		}
	}
	for (int i = 0; i < jobsize; i++) {
		Tradiness[i] = max((jobCmax[i] - Due[i]), 0);
	}
	Cost[0] = *max_element(Cmax, Cmax + factorysize);
	Cost[1] = accumulate(Tradiness, Tradiness + jobsize, 0) / double(jobsize);
	Cost[2] = accumulate(idleTime, idleTime + factorysize, 0) / double(factorysize);
}
void FactoryC(vector<int>&Pi, int(&C)[jobsize][machinesize], int (&SP)[machinesize])
{
	int subjSize = Pi.size();
	int ii = 0;
	int NewT[jobsize][machinesize] = { 0 };
	int S[jobsize][machinesize] = { 0 };
	if (subjSize < 1) {
		cout << "wrong" << endl;
	}
	for (vector<int>::iterator it = Pi.begin(); it != Pi.end(); it++) {
		int n = *it;
		for (int j = 0; j < machinesize; j++) {
			NewT[ii][j] = Time[n - 1][j];
			if (ii == 0) {
				S[ii][j] = setupTime[j][0][n - 1];
			}
			else {
				S[ii][j] = setupTime[j][*(it - 1)][n - 1];
			}
		}
		ii++;
	}
	for (int i = 0; i < machinesize; i++) {
		for (int j = 0; j < ii; j++) {
			SP[i] += NewT[j][i];
		}
	}
	C[0][0] = S[0][0] + NewT[0][0];
	for (int i = 1; i < subjSize; i++) {
		C[i][0] = C[i - 1][0] + S[i][0] + NewT[i][0];
	}
	for (int i = 0; i < subjSize; i++) {
		for (int j = 1; j < machinesize; j++) {
			if (i == 0) {
				C[i][j] = max(C[i][j - 1], S[i][j]) + NewT[i][j];
			}
			else {
				C[i][j] = max(C[i][j - 1], S[i][j] + C[i - 1][j]) + NewT[i][j];
			}
		}
	}
}
