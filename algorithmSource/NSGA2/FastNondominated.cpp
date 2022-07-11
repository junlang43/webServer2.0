#include"nsga2.h"
using namespace std;

void Population::fast_nondominated_sort()
{
	int i;
	Individual H[2 * popsize];
	int h_len = 0;
	len_f = 0;
	for (i = 0; i < 2 * popsize; i++)
	{
		Rt[i].np = 0;
		Rt[i].is_dominated = 0;
		len[i] = 0;
	}
	for (i = 0; i < 2 * popsize; i++)
	{
		for (int j = 0; j < 2 * popsize; j++)
		{
			if (i != j)
			{
				if (e_is_dominated(Rt[i], Rt[j]))
					Rt[i].sp[Rt[i].is_dominated++] = j;
				else if (e_is_dominated(Rt[j], Rt[i]))
					Rt[i].np += 1;
			}
		}
		if (Rt[i].np == 0)
		{
			len_f = 1;
			Ft[0][len[0]++] = Rt[i];
		}

	}
	i = 0;
	while (len[i] != 0)
	{
		h_len = 0;
		for (int j = 0; j < len[i]; j++)
		{
			for (int k = 0; k < Ft[i][j].is_dominated; k++)
			{
				Rt[Ft[i][j].sp[k]].np--;
				if (Rt[Ft[i][j].sp[k]].np == 0)
				{
					H[h_len++] = Rt[Ft[i][j].sp[k]];
					Rt[Ft[i][j].sp[k]].rank = i + 2;
				}
			}
		}
		i++;
		len[i] = h_len;
		if (h_len != 0)
		{
			len_f++;
			for (int j = 0; j < len[i]; j++) {
				Ft[i][j] = H[j];
			}
		}
	}	
 }
bool e_is_dominated(const Individual &a, const Individual &b)
{
	if ((a.Cost[0] <= b.Cost[0]) && (a.Cost[1] <= b.Cost[1]) && (a.Cost[2] <= b.Cost[2]))
	{
		if ((a.Cost[0] == b.Cost[0]) && a.Cost[1] == b.Cost[1] && a.Cost[2] == b.Cost[2])
			return false;
		else
			return true;
	}
	else
		return false;
}