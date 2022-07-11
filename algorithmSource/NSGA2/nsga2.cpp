#include"nsga2.h"
using namespace std;

int Time[jobsize][machinesize];
int Due[jobsize];
int setupTime[machinesize][jobsize + 1][jobsize];
Individual Ft[2 * popsize][2 * popsize];
int main()
{
	int *p;
	int x = 10;
	p = &x;
	cout << p << " " << *p;
	memset(Time, 0, sizeof(Time));
	memset(Due, 0, sizeof(Due));
	srand((unsigned int)time(NULL));//随机种子
	ReadFile();
	for (int ii = 0; ii < 1; ii++) {
		Population pop;
		int iteration = Generation;
		while (iteration--) {
			pop.crossover_mutation();
			pop.fast_nondominated_sort();
			pop.Pnum = 0;
			int maxRank = 0;
			while (pop.Pnum + pop.len[maxRank] <= popsize) {
				for (int j = 0; j < pop.len[maxRank]; j++)
					pop.Pt[pop.Pnum++] = Ft[maxRank][j];
				maxRank++;
				if (maxRank >= pop.len_f)break;
			}
			if (maxRank < pop.len_f && pop.Pnum < popsize)
			{
				pop.calu_crowding_distance(maxRank);
				pop.crowding_sort(maxRank);
			}
			int num = pop.Pnum;
			for (int j = 0; j < popsize - num; j++)
				pop.Pt[pop.Pnum++] = Ft[maxRank][j];
		}
		for (int i = 0; i < pop.len[0]; i++)
		{
			cout << pop.Pt[i].Cost[0] << " " << pop.Pt[i].Cost[1] << " " << pop.Pt[i].Cost[2] << endl;
		}
	}
	system("pause");
	return 0;
}
/******************initial**********************/
Population::Population()
{
	int i;
	for (i = 0; i < popsize; i++)
	{
		Pt[i].NR2();
        Pt[i].costFunction();
	}
	Pnum = popsize;
	Qnum = 0;
	Rnum = 0;
}
void ReadFile()
{
	ifstream in;
	in.open("I_3_100_5 _6_30.txt");
	assert(in.is_open());
	int number;
	/*int sumP[jobsize];
	int(*timeP)[machinesize] = Time;*/
	vector<int> Arr;
	vector<int>::iterator itA;
	while (in >> number, !in.eof()) {
		if (in.fail()) {
			in.clear();
			in.ignore();
			continue;
		}
		Arr.push_back(number);
	}
	itA = Arr.begin() + 2;
	for (int i = 0; i < jobsize; i++) {
		for (int j = 0; j < machinesize; j++) {
			itA = itA + 2;
			Time[i][j] = *itA;
		}
		/*sumP[i] = accumulate(*timeP, (*timeP) + machinesize, 0);
		Due[i] = sumP[i] * round(1 + 3 * rand_real(0, 1));
		timeP++;*/
	}
	for (int i = 0; i < jobsize; i++) {
		Due[i] = *(++itA);
	}
	itA++;
	itA = itA + 2 * productsize;
	itA++;//指向第一个工件
	itA = itA + 2 * jobsize;
	for (int k = 0; k < machinesize; k++) {
		itA++;
		for (int i = 0; i <= jobsize; i++) {
			for (int j = 0; j < jobsize; j++) {
				setupTime[k][i][j] = *itA;
				itA++;
			}
		}
	}
}

int rand_int(int low, int high)
{
	return int((high - low + 1)*URAND) + low;
}

double rand_real(double low, double high)
{
	double h;
	h = (high - low)*URAND + low + 0.001;
	if (h >= high)
		h = high - 0.001;
	return h;
}

