#include "sim.h"

#include <pthread.h>

MovePack * winningMoves=0;

void depthFirst(Game * game, Board * board, unsigned long depth, MoveLink * prevMoveLink, Results * results)
{
	MovePack * movePack=0;
	MoveLink * moveLink;

	if(game->isWon(board))
	{
		if(gConfig.printSolution && winningMoves==0)
		{
			winningMoves = movePack_create();

			while(depth>1)
			{
				movePack_add(winningMoves, move_dup(prevMoveLink->move));
				depth--;
				if(depth>1)
					prevMoveLink = prevMoveLink->prev;
			}
		}

		results->winCount++;

		return;
	}

	if(depth<gConfig.maxDepth)
		movePack = game->movePackGet(board);

	if(movePack!=0 && movePack->movesCount>0)
	{
		moveLink = (MoveLink *)malloc(sizeof(MoveLink));
		moveLink->prev = prevMoveLink ? prevMoveLink : 0;
		moveLink->move = movePack->moves[randrangeNoSeed(0, movePack->movesCount-1)];

		board_moveMake(board, moveLink->move);

		depthFirst(game, board, depth+1, moveLink, results);

		free(moveLink);
	}

	movePack_free(movePack, true);
}

void * depthFirstThread(void * _startBoard)
{
	unsigned long i, numToDo=gConfig.iterationCount/gConfig.threadCount;
	Results * results = results_create();
	Board * startBoard = (Board *)_startBoard;
	Board * board;

	for(i=0;i<numToDo;i++)
	{
		board = board_dup(startBoard);
		depthFirst(gConfig.game, board, 1, 0, results);
		board_free(board);
	}

	return results;
}

void sim(void)
{
	unsigned long i;
	pthread_t threads[gConfig.threadCount];
	Board * startBoard;
	Board * winningStartBoard;
	Results * results;
	Results * totalResults = results_create();
	Card ** cards;
	MovePack * movePack;
	unsigned long cardsCount=0;

	if(gConfig.game->deckCreate==0)
		cards = game_deckCreate(&cardsCount);
	else
		cards = gConfig.game->deckCreate(&cardsCount);

	if(gConfig.json)
	{
		printf("{\"deck\":");
		cards_printAsDeck(cards, cardsCount);
	}

	startBoard = gConfig.game->boardCreate(cards, cardsCount);
	winningStartBoard = board_dup(startBoard);
	free(cards);
	
	// Debug section for new game
	/*board_print(startBoard, false);
	printf("====\n");
	unsigned char movesToMake[] = {0,0,1,1,0,1,1,0,0,0,0,0,0,1};
	for(i=0;i<sizeof(movesToMake);i++)
	{
		board_moveMake(startBoard, gConfig.game->movePackGet(startBoard)->moves[movesToMake[i]]);
	}
	board_print(startBoard, false);
	movePack_print(startBoard, gConfig.game->movePackGet(startBoard), false);
	exit(0);*/

	if(!gConfig.json && !gConfig.printSolution)
	{
		printf(C_MAGENTA "Total Iterations: " C_CYAN "%s" C_RESET "\n", comma_print(gConfig.iterationCount));
		printf(C_MAGENTA "   Maximum Depth: " C_CYAN "%s" C_RESET "\n", comma_print(gConfig.maxDepth));
		printf(C_MAGENTA "    Thread Count: " C_CYAN "%s" C_RESET "\n", comma_print(gConfig.threadCount));
		printf("\n");
		printf(C_MAGENTA "            Game: " C_CYAN "%s" C_RESET "\n", gConfig.game->idname);
		printf(C_MAGENTA "            Seed: " C_CYAN "%u" C_RESET "\n", gConfig.seed);
		printf("\n");
		printf(C_MAGENTA "           Board: " C_RESET "\n");
		board_print(startBoard, false);
		printf("\n");
		printf(C_MAGENTA "  Starting Moves:" C_RESET "\n");
		movePack = gConfig.game->movePackGet(startBoard);
		movePack_print(startBoard, movePack, false);
		movePack_free(movePack, true);
		printf("\n");

		printf(C_WHITE "        <<<=== " C_GREEN "RUNNING" C_WHITE " ===>>>" C_RESET "\n");
	}

	if(gConfig.threadCount>1)
	{
		for(i=0;i<gConfig.threadCount;i++)
		{
			if(pthread_create(&threads[i], 0, depthFirstThread, startBoard)!=0)
			{
				fprintf(stderr, "Error creating thread %lu!\n", i);
				exit(EXIT_FAILURE);
			}
		}
		
		for(i=0;i<gConfig.threadCount;i++)
		{
			if(pthread_join(threads[i], (void **)&results)!=0)
			{
				fprintf(stderr, "Error joining thread %lu!\n", i);
				exit(EXIT_FAILURE);
			}

			results_add(results, totalResults);
			results_free(results);
		}
	}
	else
	{
		totalResults = depthFirstThread(startBoard);
	}


	if(!gConfig.json)
	{
		printf(C_MAGENTA "       Win Count: " C_YELLOW "%s" C_MAGENTA " out of " C_YELLOW "%s" C_RESET "\n", comma_print(totalResults->winCount), comma_print(gConfig.iterationCount));
		printf(C_MAGENTA "  Win Percentage: " C_YELLOW "%0.4f%%" C_RESET "\n", ((double)totalResults->winCount/(double)gConfig.iterationCount)*100);
	}
	else
	{
		printf(",\"seed\":%u,\"wins\":%lu,\"games\":%lu", gConfig.seed, totalResults->winCount, gConfig.iterationCount);
		printf("}\n");
	}

	if(gConfig.printSolution && winningMoves!=0)
	{
		printf("\nWinning moves:\n");
		i = winningMoves->movesCount-1;
		do
		{
			//board_print(winningStartBoard, false);
			move_print(winningStartBoard, winningMoves->moves[i], true);
			board_moveMake(winningStartBoard, winningMoves->moves[i]);
			if(i==0)
				break;

			i--;
		} while(true);

		movePack_free(winningMoves, true);
	}

	board_free(startBoard);
	board_free(winningStartBoard);
	results_free(totalResults);
}