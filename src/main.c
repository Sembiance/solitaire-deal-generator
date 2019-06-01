#include "sim.h"

#include <unistd.h>

Config gConfig;

static void usage(void)
{
	fprintf(stderr,
			"sim %s\n"
			"\n"
			"Usage: sim [OPTIONS] GAMEID\n"
			"      --seed <n>          Seed  to use for random number generator\n"
			"      --iterations <n>    Number of iterations to perform.\n"
			"      --maxDepth <n>      Maximum depth to randomly crawl into a move tree before giving up\n"
			"      --threads <n>       Number of threads to use. Default is CPU core count\n"
			"      --json              Output winCount, numIterations, shortestWin and deck as JSON\n"
			"      --printSolution     Output a winning solution of moves in JSON\n"
			"  -h, --help              Output this help and exit\n"
			"  -V, --version           Output version and exit\n"
			"\n", SIM_VERSION);
	fprintf(stderr, "Valid GAMEIDs:\n");
	print_games_list();
	exit(EXIT_FAILURE);
}

static void parse_options(int argc, char **argv)
{
	int i;

	gConfig.json = false;
	gConfig.iterationCount = 0;
	gConfig.threadCount = sysconf(_SC_NPROCESSORS_ONLN);
	gConfig.seed = 0;
	gConfig.maxDepth = 0;
	gConfig.game = 0;
	gConfig.printSolution = false;

	for(i=1;i<argc;i++)
	{
		int lastarg = i==argc-1;

		if(!strcmp(argv[i],"-h") || !strcmp(argv[i], "--help"))
		{
			usage();
		}
		else if(!strcmp(argv[i],"-V") || !strcmp(argv[i], "--version"))
		{
			printf("sim %s\n", SIM_VERSION);
			exit(EXIT_SUCCESS);
		}
		else if(!strcmp(argv[i], "--json"))
		{
			gConfig.json = true;
		}
		else if(!strcmp(argv[i], "--printSolution"))
		{
			gConfig.printSolution = true;
		}
		else if(!strcmp(argv[i], "--iterations") && !lastarg)
		{
			gConfig.iterationCount = atol(argv[++i]);
		}
		else if(!strcmp(argv[i], "--maxDepth") && !lastarg)
		{
			gConfig.maxDepth = atol(argv[++i]);
		}
		else if(!strcmp(argv[i], "--threads") && !lastarg)
		{
			gConfig.threadCount = atol(argv[++i]);
		}
		else if(!strcmp(argv[i], "--seed") && !lastarg)
		{
			gConfig.seed = atoi(argv[++i]);
		}
		else
		{
			break;
		}
	}

	argc -= i;
	argv += i;

	if(argc==0)
		usage();

	gConfig.game = find_game(argv[0]);
	if(gConfig.game==0)
		usage();

	if(gConfig.iterationCount==0)
		gConfig.iterationCount = gConfig.game->defaultIterationsCount;

	if(gConfig.iterationCount<gConfig.threadCount)
		gConfig.threadCount = 1;

	if(gConfig.maxDepth==0)
		gConfig.maxDepth = gConfig.game->defaultMaxDepth;
}

int main(int argc, char ** argv)
{
	double startTime = getTicks();

	games_load();

	parse_options(argc, argv);
	randinit(gConfig.seed);

	sim();

	if(!gConfig.json)
		printf(C_WHITE "        <<<===   " C_GREEN "DONE " C_RED "%0.2f seconds" C_WHITE "  ===>>>" C_RESET "\n", (getTicks()-startTime)/1000);

	return EXIT_SUCCESS;
}
