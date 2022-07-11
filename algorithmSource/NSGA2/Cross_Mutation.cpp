#include"nsga2.h"
using namespace std;

void Population::crossover_mutation()
{
	for (int i = 0; i < popsize; i++) {
        int n1 = rand_int(0, popsize - 1);
		if (rand_real(0, 1) <= pCrossover ) {
            int n2 = rand_int(0, popsize - 1);
			crossover(Pt[n1].jobDecode, Pt[n2].jobDecode, Qt[i].jobDecode);
		}
		else {
			mutation(Pt[n1].jobDecode, Qt[i].jobDecode);
		}
		Qt[i].costFunction();
	}
	Rnum = 0;
	Qnum = popsize;
	int i;
	for (i = 0; i < Pnum; i++) {
		Rt[Rnum++] = Pt[i];
	}
	for (i = 0; i < Qnum; i++) {
		Rt[Rnum++] = Qt[i];
	}
}

void crossover(vector<vector<int>> x1, vector<vector<int>> x2, vector<vector<int>> &y)
{
	y.clear();
	y.resize(factorysize);
	vector<int>sub;
	for (int i = 0; i < factorysize; i++) {
		int p1 = rand_int(0, x1[i].size() - 1);
		int p2 = rand_int(0, x1[i].size() - 1);
		if (p1 <= p2) {
			for (int j = p1; j <= p2; j++) {
				sub.push_back(*(x1[i].begin() + j));
				y[i].push_back(*(x1[i].begin() + j));
			}
		}
		else {
			for (int j = p2; j <= p1; j++) {
				sub.push_back(*(x1[i].begin() + j));
				y[i].push_back(*(x1[i].begin() + j));
			}
		}
	}
	for (int i = 0; i < factorysize; i++) {
		for (vector<int> ::iterator it = x2[i].begin(); it != x2[i].end(); it++) {
			if (find(sub.begin(), sub.end(), *it) == sub.end()) {//*it±£¡Ù
				y[i].push_back(*it);
			}
		}
	}
}
void mutation(vector<vector<int>> x, vector<vector<int>> &y)
{
	int f1 = rand_int(0, factorysize - 1);
	int f2 = rand_int(0, factorysize - 1);
	int p1 = rand_int(0, x[f1].size() - 1);
	int p2;
	int job;
	y = x;
	job = *(y[f1].begin() + p1);
	y[f1].erase(y[f1].begin() + p1);
	p2 = rand_int(0, y[f2].size());
	y[f2].insert(y[f2].begin() + p2, job);
}
