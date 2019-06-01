#include "sim.h"

Board * board_create(void)
{
	Board * board = (Board *)malloc(sizeof(Board));
	board->spotsCount = 0;
	board->spots = 0;
	board->gameState = 0;

	return board;
}

void board_spotAdd(Board * board, Spot * spot)
{
	board->spotsCount++;
	board->spots = (Spot **)realloc(board->spots, sizeof(Spot *)*board->spotsCount);
	board->spots[board->spotsCount-1] = spot;
}

void board_free(Board * board)
{
	unsigned char i;
	if(board->gameState!=0 && gConfig.game->freeGameState!=0)
		gConfig.game->freeGameState(board);

	for(i=0;i<board->spotsCount;i++)
	{
		spot_free(board->spots[i]);
	}
	free(board->spots);
	free(board);
}

Spot * board_spotGet(Board * board, SPOTTYPE spotType, unsigned char spotIndex)
{
	unsigned char i;
	Spot * spot;

	for(i=0;i<board->spotsCount;i++)
	{
		spot = board->spots[i];

		if(spot->type==spotType && spot->index==spotIndex)
			return spot;
	}

	return 0;
}

void board_cardAdd(Board * board, SPOTTYPE spotType, unsigned char spotIndex, Card * card)
{
	Spot * spot = board_spotGet(board, spotType, spotIndex);
	if(!spot)
		return;

	spot_cardAdd(spot, card);
}

SPOTTYPE * board_getSpotTypes(Board * board, unsigned char * spotTypesCountResult)
{
	unsigned char i, j, spotTypesCount=0;
	bool hasType;
	SPOTTYPE * spotTypes = 0;
	Spot * spot;

	for(i=0;i<board->spotsCount;i++)
	{
		spot = board->spots[i];
		hasType = false;
		for(j=0;j<spotTypesCount;j++)
		{
			if(spotTypes[j]==spot->type)
			{
				hasType = true;
				break;
			}
		}

		if(!hasType)
		{
			spotTypesCount++;
			// cppcheck-suppress memleakOnRealloc
			spotTypes = (SPOTTYPE *)realloc(spotTypes, sizeof(SPOTTYPE)*spotTypesCount);
			spotTypes[(spotTypesCount-1)] = spot->type;
		}
	}

	*spotTypesCountResult = spotTypesCount;

	return spotTypes;
}

void board_print(Board * board, bool showDownCards)
{
	unsigned char i, j, spotTypesCount;
	Spot * spot;
	SPOTTYPE spotType;
	SPOTTYPE * spotTypes = board_getSpotTypes(board, &spotTypesCount);

	for(i=0;i<spotTypesCount;i++)
	{
		spotType = spotTypes[i];
		for(j=0;j<board->spotsCount;j++)
		{
			spot = board->spots[j];
			if(spot->type==spotType)
				spot_print(spot, showDownCards);
		}
	}

	free(spotTypes);
}

void board_printKlondikeSolverDeck(Board * board)
{
	int i,j;
	Spot * spot;

	for(i=0;i<7;i++)
	{
		for(j=0;j<7;j++)
		{
			spot = board_spotGet(board, TABLEAU, j);
			if(spot->cardsCount>=(i+1))
				printf("%02d%d", spot->cards[i]->rank, spot->cards[i]->suit+1);
		}
	}

	spot = board_spotGet(board, STOCK, 0);
	for(i=spot->cardsCount-1;i>=0;i--)
	{
		printf("%02d%d", spot->cards[i]->rank, spot->cards[i]->suit+1);
	}

	printf("\n");
}

Board * board_dup(Board * board)
{
	unsigned char i;
	Board * dupBoard = board_create();
	if(board->spotsCount>0)
	{
		dupBoard->spots = (Spot **)malloc(sizeof(Spot *)*board->spotsCount);
		dupBoard->spotsCount = board->spotsCount;
		for(i=0;i<board->spotsCount;i++)
		{
			dupBoard->spots[i] = spot_dup(board->spots[i]);
		}
	}
	if(board->gameState!=0 && gConfig.game->dupGameState!=0)
		dupBoard->gameState = gConfig.game->dupGameState(board);

	return dupBoard;
}

void board_moveMake(Board * board, Move * move)
{
	Spot * fromSpot;
	unsigned char fromSpotCardIndex;
	unsigned char cardsToMoveCount;
	unsigned char toSpotCardIndex;
	Spot * toSpot;
	unsigned char i;

	if(move->type==MULTIPLE)
	{
		for(i=0;i<move->movePack->movesCount;i++)
		{
			board_moveMake(board, move->movePack->moves[i]);
		}
	}
	else
	{
		fromSpot = board_spotGet(board, move->fromSpotType, move->fromSpotIndex);
		fromSpotCardIndex = move->fromSpotCardIndex==-1 ? fromSpot->cardsCount-1 : move->fromSpotCardIndex;
		cardsToMoveCount = fromSpot->cardsCount-fromSpotCardIndex;
		
		if(move->type==MOVEDISTRIBUTE)
		{
			for(i=0;i<move->toSpotIndex;i++)
			{
				fromSpotCardIndex = move->fromSpotCardIndex==-1 ? fromSpot->cardsCount-1 : move->fromSpotCardIndex;
				cardsToMoveCount = fromSpot->cardsCount-fromSpotCardIndex;
				
				toSpot = board_spotGet(board, move->toSpotType, i);
				toSpotCardIndex = move->toSpotCardIndex==-1 ? toSpot->cardsCount : move->toSpotCardIndex;

				spot_moveCards(fromSpot, fromSpotCardIndex, cardsToMoveCount, toSpot, toSpotCardIndex, move->type==MOVE);
			}
			goto moveFinish;
		}
		
		if(move->type==MOVE || move->type==MOVEREVERSED)
		{
			fromSpotCardIndex = move->fromSpotCardIndex==-1 ? fromSpot->cardsCount-1 : move->fromSpotCardIndex;
			toSpot = board_spotGet(board, move->toSpotType, move->toSpotIndex);
			toSpotCardIndex = move->toSpotCardIndex==-1 ? toSpot->cardsCount : move->toSpotCardIndex;

			spot_moveCards(fromSpot, fromSpotCardIndex, cardsToMoveCount, toSpot, toSpotCardIndex, move->type==MOVEREVERSED);
			goto moveFinish;
		}
	}

	moveFinish:
	if(gConfig.game->updateBoard)
		gConfig.game->updateBoard(board);
}

void board_moveLinkMake(Board * board, MoveLink * lastLink)
{
	Move * moveLinks[2048];
	long i=0;

	while(lastLink)
	{
		moveLinks[i++] = lastLink->move;
		lastLink = lastLink->prev;
	}

	for(--i;i>=0;i--)
	{
		board_moveMake(board, moveLinks[i]);
	}
}

Board ** board_boardAddDups(Board ** boards, unsigned long boardsCount, Board * board, unsigned long boardNumToAdd)
{
	unsigned long i;

	boards = (Board **)realloc(boards, sizeof(Board *)*(boardsCount+boardNumToAdd));
	for(i=0;i<boardNumToAdd;i++)
	{
		boards[boardsCount+i] = board_dup(board);
	}

	return boards;
}

void boards_free(Board ** boards, unsigned long boardsCount)
{
	unsigned long i;

	for(i=0;i<boardsCount;i++)
	{
		board_free(boards[i]);
	}

	free(boards);
}