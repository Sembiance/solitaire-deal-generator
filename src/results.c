#include "sim.h"

Results * results_create(void)
{
	Results * results;
	results = (Results *)malloc(sizeof(Results));
	results->winCount = 0;

	return results;
}

void results_add(Results * results, Results * totalResults)
{
	totalResults->winCount += results->winCount;
}

void results_free(Results * results)
{
	free(results);
}