#ifndef NSGA2_H
#define NSGA2_H
#include<vector>
#include <iostream>
#include <fstream>
#include<assert.h>
#include<algorithm>
#include<numeric>
#include <ctime>
#define numObjectives 3
#define PI acos(-1)
#define jobsize 100
#define machinesize 5
#define factorysize 6
#define productsize 30
#define popsize 60
#define Generation 200  
#define pCrossover 0.7
#define URAND (rand()/(RAND_MAX+1.0))//产生随机数

using namespace std;

/***********************个体类声明*************************/
class Individual
{
public:
	vector<vector<int>> jobDecode;
	double Cost[numObjectives];//object
	int jobCmax[jobsize];
	double idleTime[factorysize];
	int sp[2 * popsize];//被支配个体集合SP。该量是可行解空间中所有被个体p支配的个体组成的集合。
	int is_dominated;//集合sp的个数
	int np;//支配个数np。该量是在可行解空间中可以支配个体p的所有个体的数量。		
	int rank;//优先级，Pareto级别为当前最高级
	double crowding_distance;//拥挤距离
	void costFunction();//计算目标值
	void NR2();//解码
};

/***********************种群类声明*************************/
class Population
{
public:
	Population();//类初始化
	Individual Pt[popsize];
	Individual Qt[popsize];
	Individual Rt[2 * popsize];
	//随机产生一个初始父代P
	//交叉和变异操作产生子代Qt
	//将Pt和Qt并入到Rt中（初始时t=0），对Rt进行快速非支配解排序，
	int Rnum;
	int Pnum;
	int Qnum;
	//P,Q,R中元素的个数
	void crossover_mutation();//产生新的子代
	void fast_nondominated_sort();//快速非支配排序
	void calu_crowding_distance(int i);//拥挤距离计算
	void crowding_sort(int i);//对拥挤距离降序排列
	//两个个体属于不同等级的非支配解集，优先考虑等级序号较小的
	//若两个个体属于同一等级的非支配解集，优先考虑拥挤距离较大的
	int len[2 * popsize];//各个变异交叉后的群体Fi的长度的集合
	int len_f;//整个群体rank值
};

/***********************全局变量声明*************************/
extern int Time[][machinesize];//processing time
extern int setupTime[][jobsize + 1][jobsize];
extern int Due[];
extern Individual Ft[][2 * popsize];
int rand_int(int low, int high);//产生随机整数
double rand_real(double low, double high);//产生随机实数
int cmp1(const void *a, const void *b);
int cmp2(const void *a, const void *b);
int cmp3(const void *a, const void *b);
int cmp_c_d(const void *a, const void *b);
bool e_is_dominated(const Individual &a, const Individual &b);
void FactoryC(vector<int>&Pi, int(&C)[jobsize][machinesize], int(&SP)[machinesize]);//计算工厂f的最大完工时间
void crossover(vector<vector<int>> x1, vector<vector<int>> x2, vector<vector<int>> &y);
void mutation(vector<vector<int>> x, vector<vector<int>> &y);
void ReadFile();
#endif
