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
#define URAND (rand()/(RAND_MAX+1.0))//���������

using namespace std;

/***********************����������*************************/
class Individual
{
public:
	vector<vector<int>> jobDecode;
	double Cost[numObjectives];//object
	int jobCmax[jobsize];
	double idleTime[factorysize];
	int sp[2 * popsize];//��֧����弯��SP�������ǿ��н�ռ������б�����p֧��ĸ�����ɵļ��ϡ�
	int is_dominated;//����sp�ĸ���
	int np;//֧�����np���������ڿ��н�ռ��п���֧�����p�����и����������		
	int rank;//���ȼ���Pareto����Ϊ��ǰ��߼�
	double crowding_distance;//ӵ������
	void costFunction();//����Ŀ��ֵ
	void NR2();//����
};

/***********************��Ⱥ������*************************/
class Population
{
public:
	Population();//���ʼ��
	Individual Pt[popsize];
	Individual Qt[popsize];
	Individual Rt[2 * popsize];
	//�������һ����ʼ����P
	//����ͱ�����������Ӵ�Qt
	//��Pt��Qt���뵽Rt�У���ʼʱt=0������Rt���п��ٷ�֧�������
	int Rnum;
	int Pnum;
	int Qnum;
	//P,Q,R��Ԫ�صĸ���
	void crossover_mutation();//�����µ��Ӵ�
	void fast_nondominated_sort();//���ٷ�֧������
	void calu_crowding_distance(int i);//ӵ���������
	void crowding_sort(int i);//��ӵ�����뽵������
	//�����������ڲ�ͬ�ȼ��ķ�֧��⼯�����ȿ��ǵȼ���Ž�С��
	//��������������ͬһ�ȼ��ķ�֧��⼯�����ȿ���ӵ������ϴ��
	int len[2 * popsize];//�������콻����Ⱥ��Fi�ĳ��ȵļ���
	int len_f;//����Ⱥ��rankֵ
};

/***********************ȫ�ֱ�������*************************/
extern int Time[][machinesize];//processing time
extern int setupTime[][jobsize + 1][jobsize];
extern int Due[];
extern Individual Ft[][2 * popsize];
int rand_int(int low, int high);//�����������
double rand_real(double low, double high);//�������ʵ��
int cmp1(const void *a, const void *b);
int cmp2(const void *a, const void *b);
int cmp3(const void *a, const void *b);
int cmp_c_d(const void *a, const void *b);
bool e_is_dominated(const Individual &a, const Individual &b);
void FactoryC(vector<int>&Pi, int(&C)[jobsize][machinesize], int(&SP)[machinesize]);//���㹤��f������깤ʱ��
void crossover(vector<vector<int>> x1, vector<vector<int>> x2, vector<vector<int>> &y);
void mutation(vector<vector<int>> x, vector<vector<int>> &y);
void ReadFile();
#endif
