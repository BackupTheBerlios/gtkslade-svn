
#include "main.h"

void remove_duplicates_intvector(vector<int> *vec)
{
	vector<int> uniques;

	vector<int>::iterator iter = vec->begin();

	while (iter != vec->end());
	{
		if (vector_exists(uniques, *iter))
			vec->erase(iter);
		else
		{
			uniques.push_back(*iter);
			iter++;
		}
	}
}
