#include "sim.h"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

char * lastState=0;
char seedState[256];
char unseededState[256];

static unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
	a=a-b;  a=a-c;  a=a^(c >> 13);
	b=b-c;  b=b-a;  b=b^(a << 8);
	c=c-a;  c=c-b;  c=c^(b >> 13);
	a=a-b;  a=a-c;  a=a^(c >> 12);
	b=b-c;  b=b-a;  b=b^(a << 16);
	c=c-a;  c=c-b;  c=c^(b >> 5);
	a=a-b;  a=a-c;  a=a^(c >> 3);
	b=b-c;  b=b-a;  b=b^(a << 10);
	c=c-a;  c=c-b;  c=c^(b >> 15);

	return c;
}

void randinit(unsigned int seed)
{
	struct timeval 		now;
	gettimeofday(&now, 0);

	initstate(seed, seedState, 256);
	initstate(mix(now.tv_usec, clock(), getpid()), unseededState, 256);
}

long int randrange(long int min, long int max)
{
	if(lastState!=seedState)
	{
		setstate(seedState);
		lastState = seedState;
	}
	return (long int)(min + random() / (RAND_MAX / ((max+1)-min) + 1));
}

long int randrangeNoSeed(long int min, long int max)
{
	if(lastState!=unseededState)
	{
		setstate(unseededState);
		lastState = unseededState;
	}
	return (long int)(min + random() / (RAND_MAX / ((max+1)-min) + 1));
}
