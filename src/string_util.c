#include <string.h>
#include <stdlib.h>

char *  strfree(char * str)
{
	if(str)
		free(str);

	return 0;
}
unsigned long strchrcount(char * haystack, char needle)
{
	unsigned long       i=0;

	if(!haystack)
		return 0;

	while(*haystack)
	{
		if(*haystack==needle)
			i++;

		haystack++;
	}

	return i;
}

char * strchrrep(char * haystack, char needle, char replacement)
{
	char *  c=0;
	int     length=0;

	if(!haystack)
		return haystack;

	for(c=haystack,length=strlen(haystack);length;length--,c++)
	{
		if(*c==needle)
			*c = replacement;
	}

	return haystack;
}

char ** strchrexplode(char * strP, char sep)
{
	unsigned long   count, i;
	char **         array;
	char *          str;
	char *          originalstr;

	if(!strP)
		return 0;

	str = strdup(strP);
	originalstr = str;

	if(strlen(str)==0)
	{
		strfree(originalstr);
		return 0;
	}

	count = strchrcount(str, sep)+1;

	array = (char **)malloc(sizeof(char *)*(count+1));
	memset(array, 0, (sizeof(char *)*(count+1)));

	strchrrep(str, sep, '\0');

	for(i=0;i<count;i++)
	{
		if(!strlen(str))
		{
			array[i] = (char *)malloc(1);
			array[i][0] = '\0';
		}
		else
		   array[i] = strdup(str);

		str = str+strlen(str)+1;
	}

	free(originalstr);

	return array;
}
