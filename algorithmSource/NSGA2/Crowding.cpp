#include"nsga2.h"
using namespace std;

void Population::calu_crowding_distance(int i)
{
	int n = len[i];
	double m_max, m_min;
	int j;
	for (j = 0; j < n; j++)
		Ft[i][j].crowding_distance = 0;
	Ft[i][0].crowding_distance = Ft[i][n - 1].crowding_distance = 0xffffff;
	qsort(Ft[i], n, sizeof(Individual), cmp1);
	m_max = -0xfffff;
	m_min = 0xfffff;
	if (m_max < Ft[i][n - 1].Cost[0])
		m_max = Ft[i][n - 1].Cost[0];
	if (m_min > Ft[i][0].Cost[0])
		m_min = Ft[i][0].Cost[0];
	for (j = 1; j < n - 1; j++) {
		Ft[i][j].crowding_distance += (Ft[i][j + 1].Cost[0] - Ft[i][j - 1].Cost[0]) / (m_max - m_min);
	}
		
	qsort(Ft[i], n, sizeof(Individual), cmp2);
	m_max = -0xfffff;
	m_min = 0xfffff;
	if (m_max < Ft[i][n - 1].Cost[1])
		m_max = Ft[i][n - 1].Cost[1];
	if (m_min > Ft[i][0].Cost[1])
		m_min = Ft[i][0].Cost[1];
	for (j = 1; j < n - 1; j++)
		Ft[i][j].crowding_distance += (Ft[i][j + 1].Cost[1] - Ft[i][j - 1].Cost[1]) / (m_max - m_min);

	qsort(Ft[i], n, sizeof(Individual), cmp3);
	m_max = -0xfffff;
	m_min = 0xfffff;
	if (m_max < Ft[i][n - 1].Cost[2])
		m_max = Ft[i][n - 1].Cost[2];
	if (m_min > Ft[i][0].Cost[2])
		m_min = Ft[i][0].Cost[2];
	for (j = 1; j < n - 1; j++)
		Ft[i][j].crowding_distance += (Ft[i][j + 1].Cost[2] - Ft[i][j - 1].Cost[2]) / (m_max - m_min);
}
void Population::crowding_sort(int i)
{
	int n;
	n = len[i];
	qsort(Ft[i], n, sizeof(Individual), cmp_c_d);
}

/********************ÅÅÐòº¯Êý************************/
int cmp1(const void *a, const void *b)
//Ä¿±êº¯Êýf1µÄÉýÐòÅÅÐò
{
	const Individual *e = (const Individual *)a;
	const Individual *f = (const Individual *)b;
	if (e->Cost[0] == f->Cost[0])
		return 0;
	else if (e->Cost[0] < f->Cost[0])
		return -1;
	else return 1;
}

int cmp2(const void *a, const void *b)
//Ä¿±êº¯Êýf2µÄÉýÐòÅÅÐò
{
	const Individual *e = (const Individual *)a;
	const Individual *f = (const Individual *)b;
	if (e->Cost[1] == f->Cost[1])
		return 0;
	else if (e->Cost[1] < f->Cost[1])
		return -1;
	else return 1;
}

int cmp3(const void *a, const void *b)
//Ä¿±êº¯Êýf3µÄÉýÐòÅÅÐò
{
	const Individual *e = (const Individual *)a;
	const Individual *f = (const Individual *)b;
	if (e->Cost[2] == f->Cost[2])
		return 0;
	else if (e->Cost[2] < f->Cost[2])
		return -1;
	else return 1;
}
int cmp_c_d(const void *a, const void *b)
//¶ÔÓµ¼·¾àÀë½µÐòÅÅÐò
{
	const Individual *e = (const Individual *)a;
	const Individual *f = (const Individual *)b;
	if (e->crowding_distance == f->crowding_distance)
		return 0;
	else if (e->crowding_distance < f->crowding_distance)
		return 1;
	else
		return -1;
}