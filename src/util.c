#include <sys/time.h>
#include <locale.h>

double getTicks(void)
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (double)((((double)tv.tv_sec)*1000) + (((double)tv.tv_usec)/1000));
}

char commaPrintBufs[16][30];
unsigned char commaPrintBufIndex=0;

char * comma_print(unsigned long n)
{
	int comma = '\0';
	char *p;
	int i = 0;

	if(commaPrintBufIndex>=16)
		commaPrintBufIndex = 0;

	p = &commaPrintBufs[commaPrintBufIndex][sizeof(commaPrintBufs[commaPrintBufIndex])-1];

	if(comma == '\0')
	{
		struct lconv *lcp = localeconv();
		if(lcp != NULL)
		{
			if(lcp->thousands_sep != NULL && *lcp->thousands_sep != '\0')
				comma = *lcp->thousands_sep;
			else
				comma = ',';
		}
	}

	*p = '\0';

	do
	{
		if(i%3 == 0 && i != 0)
			*--p = comma;

		*--p = '0' + n % 10;
		n /= 10;
		i++;
	} while(n != 0);

	commaPrintBufIndex++;
	return p;
}

unsigned long min(unsigned long a, unsigned long b)
{
	return (a<b ? a : b);
}

unsigned long max(unsigned long a, unsigned long b)
{
	return (a>b ? a : b);
}
