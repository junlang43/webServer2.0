#include"nsga2.h"
using namespace std;

void  Individual::NR2()
{
	int jobEncode[jobsize];
	for (int i = 0; i < jobsize; i++) {
		jobEncode[i] = i + 1;
	}
	random_shuffle(jobEncode, jobEncode + jobsize);
	jobDecode.clear();
	jobDecode.resize(factorysize);
	for (int i = 0; i < jobsize; i++) {
		vector<int> tempjs;
		int C[jobsize][machinesize];
		int sumP[machinesize];
		int complete = 100000;
		int tempcomplete;
		int factory = 0;
		for (int k = 0; k < factorysize; k++) {
			tempjs.assign(jobDecode[k].begin(), jobDecode[k].end());
			tempjs.push_back(jobEncode[i]);
			FactoryC(tempjs, C, sumP);
			tempcomplete = C[tempjs.size() - 1][machinesize - 1];
			if (tempcomplete < complete) {
				complete = tempcomplete;
				factory = k;
			}
			tempjs.clear();
		}
		jobDecode[factory].push_back(jobEncode[i]);//需要提前清空
	}
}