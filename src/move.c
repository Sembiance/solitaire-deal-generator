#include "sim.h"

Move * move_create(MOVETYPE type, SPOTTYPE fromSpotType, unsigned char fromSpotIndex, int fromSpotCardIndex, SPOTTYPE toSpotType, unsigned char toSpotIndex, int toSpotCardIndex)
{
	Move * move = (Move *)malloc(sizeof(Move));
	move->type = type;
	move->fromSpotType = fromSpotType;
	move->fromSpotIndex = fromSpotIndex;
	move->fromSpotCardIndex = fromSpotCardIndex;
	move->toSpotType = toSpotType;
	move->toSpotIndex = toSpotIndex;
	move->toSpotCardIndex = toSpotCardIndex;
	move->movePack = 0;

	return move;
}

Move * move_createMultiple(MovePack * movePack)
{
	Move * move = move_create(MULTIPLE, SPOT_NA, 0, 0, SPOT_NA, 0, 0);
	move->movePack = movePack;
	return move;
}

Move * move_dup(Move * move)
{
	Move * dupMove = move_create(move->type, move->fromSpotType, move->fromSpotIndex, move->fromSpotCardIndex, move->toSpotType, move->toSpotIndex, move->toSpotCardIndex);
	if(move->movePack!=0)
		dupMove->movePack = movePack_dup(move->movePack);
	return dupMove;
}

void movePack_print_json(MovePack * movePack)
{
	unsigned char i;
	printf("[");
	for(i=0;i<movePack->movesCount;i++)
	{
		if(i>0)
			printf(",");
		move_print_json(movePack->moves[i]);
	}
	printf("]");
}

void move_print_json(Move * move)
{
	if(move->type==MULTIPLE)
		movePack_print_json(move->movePack);
	else
		printf("[%d,\"%s\",%d,%d,\"%s\",%d,%d]", move->type, spot_name(move->fromSpotType), move->fromSpotIndex, move->fromSpotCardIndex, spot_name(move->toSpotType), move->toSpotIndex, move->toSpotCardIndex);
}

void move_print(Board * board, Move * move, bool indent)
{
	char buf[1024];
	Spot * fromSpot = (move->type==MULTIPLE ? 0 : board_spotGet(board, move->fromSpotType, move->fromSpotIndex));
	Spot * toSpot = (move->type==MOVE || move->type==MOVEREVERSED ? board_spotGet(board, move->toSpotType, move->toSpotIndex) : 0);
	unsigned char fromSpotCardIndex = (fromSpot==0 ? 0 : (move->fromSpotCardIndex==-1 ? (fromSpot->cardsCount-1) : move->fromSpotCardIndex));
	unsigned char toSpotCardIndex = (toSpot==0 ? 0 : (move->toSpotCardIndex==-1 ? (toSpot->cardsCount-1) : move->toSpotCardIndex));
	unsigned char i=0;
	Card * toCard = (toSpot==0 ? 0 : (toSpot->cardsCount==0 ? 0 : toSpot->cards[toSpotCardIndex]));

	if((move->type==MOVE || move->type==MOVEREVERSED) && fromSpot)
	{
		sprintf(buf, "%s => %s", cards_print(&fromSpot->cards[fromSpotCardIndex], fromSpot->cardsCount-fromSpotCardIndex, false), toCard==0 ? "TOP" : cards_print(&toSpot->cards[toSpotCardIndex], toSpot->cardsCount-toSpotCardIndex, false));
		printf("%c          MOVE %10s%02d %2d => %10s%02d   %2d %s (%s)\n", (indent ? '\t' : ' '), spot_name(move->fromSpotType), move->fromSpotIndex, move->fromSpotCardIndex, spot_name(move->toSpotType), move->toSpotIndex, move->toSpotCardIndex, move->type==MOVEREVERSED ? "(reversed)" : "", buf);	
	}
	else if(move->type==MOVEDISTRIBUTE && fromSpot)
	{
		printf("%cMOVEDISTRIBUTE %10s%02d %2d => %10s0-%02d %2d (", (indent ? '\t' : ' '), spot_name(move->fromSpotType), move->fromSpotIndex, move->fromSpotCardIndex, spot_name(move->toSpotType), move->toSpotIndex-1, move->toSpotCardIndex);

		buf[0] = 0;
		for(i=0;i<move->toSpotIndex;i++)
		{
			if(i>0)
				printf(", ");

			toSpot = board_spotGet(board, move->toSpotType, i);
			printf("%s => %s", card_print(fromSpot->cards[(fromSpotCardIndex-i)], false), toSpot->cardsCount==0 ? "TOP" : (card_print(toSpot->cards[(toSpot->cardsCount-1)], false)));
		}
		printf(")\n");
	}
	else if(move->type==MULTIPLE)
	{
		printf("%c      MULTIPLE\n", (indent ? '\t' : ' '));
		movePack_print(board, move->movePack, true);
	}
	else
	{
		printf("Unknown or invalid formatted move %d %s-%d:%d %s-%d:%d\n", move->type, spot_name(move->fromSpotType), move->fromSpotIndex, move->fromSpotCardIndex, spot_name(move->toSpotType), move->toSpotIndex, move->toSpotCardIndex);
	}
}

void move_free(Move * move)
{
	if(move->movePack!=0)
		movePack_free(move->movePack, true);

	free(move);
}

MovePack * movePack_create(void)
{
	MovePack * movePack = (MovePack *)malloc(sizeof(MovePack));
	movePack->movesCount = 0;
	movePack->moves = 0;

	return movePack;
}

MovePack * movePack_dup(MovePack * movePack)
{
	unsigned long i;
	MovePack * dupMovePack = movePack_create();
	dupMovePack->moves = (Move **)malloc(sizeof(Move *)*movePack->movesCount);
	dupMovePack->movesCount = movePack->movesCount;
	for(i=0;i<movePack->movesCount;i++)
	{
		dupMovePack->moves[i] = move_dup(movePack->moves[i]);
	}

	return dupMovePack;
}

void movePack_addMany(MovePack * movePack, MovePack * movePackMore)
{
	unsigned long i;

	movePack->moves = (Move **)realloc(movePack->moves, sizeof(Move *)*(movePack->movesCount+movePackMore->movesCount));
	for(i=0;i<movePackMore->movesCount;i++)
	{
		movePack->moves[movePack->movesCount+i] = movePackMore->moves[i];
	}
	movePack->movesCount+=movePackMore->movesCount;
}

void movePack_add(MovePack * movePack, Move * move)
{
	movePack->movesCount++;
	movePack->moves = (Move **)realloc(movePack->moves, sizeof(Move *)*movePack->movesCount);
	movePack->moves[(movePack->movesCount-1)] = move;
}

void movePack_dropLast(MovePack * movePack)
{
	movePack->movesCount--;
	move_free(movePack->moves[movePack->movesCount]);
	movePack->moves = (Move **)realloc(movePack->moves, sizeof(Move *)*movePack->movesCount);
}

void movePack_print(Board * board, MovePack * movePack, bool indent)
{
	unsigned long i;

	for(i=0;i<movePack->movesCount;i++)
	{
		printf("%c%2ld: ", (indent ? '\t' : ' '), i);
		move_print(board, movePack->moves[i], indent);
	}
}

void movePack_free(MovePack * movePack, bool freeMoves)
{
	unsigned long i;

	if(!movePack)
		return;

	if(freeMoves)
	{
		for(i=0;i<movePack->movesCount;i++)
		{
			move_free(movePack->moves[i]);
		}
	}
	
	free(movePack->moves);
	free(movePack);
}
