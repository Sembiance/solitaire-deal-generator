#include "../sim.h"

Card ** acesup_deckCreateWin(unsigned long * cardsCount)
{
	char * cardString = strdup("2d,3d,4d,5d,13d,13h,1s,13s,2s,3s,4s,5s,6s,7s,8s,9s,10s,11s,12s,1c,2c,3c,4c,5c,6c,7c,8c,9c,10c,11c,12c,13c,2h,3h,4h,5h,6h,7h,8h,9h,10h,11h,12h,12d,6d,7d,8d,9d,10d,11d,1h,1d");
	Card ** cards;
	*cardsCount = 52;
	cards = cards_createFromIds(cardString);
	free(cardString);
	return cards;
}

Board * acesup_boardCreate(Card ** cards, unsigned long cardsCount)
{
	Board * board = board_create();
	unsigned char i;

	board_spotAdd(board, spot_create(STOCK, 0));
	board_spotAdd(board, spot_create(FOUNDATION, 0));

	for(i=0;i<4;i++)
	{
		board_spotAdd(board, spot_create(TABLEAU, i));
	}

	for(i=0;i<4;i++)
	{
		board_cardAdd(board, TABLEAU, i, cards[i]);
	}
	for(;i<52;i++)
	{
		board_cardAdd(board, STOCK, 0, cards[i]);
	}

	return board;
}

MovePack * acesup_movePackGet(Board * board)
{
	MovePack * movePack = movePack_create();
	unsigned char i, j;
	Spot * stock = board_spotGet(board, STOCK, 0);
	Spot * tableaus[4];
	unsigned char suitTopRanks[4] = {0,0,0,0};
	Card * card;

	for(i=0;i<4;i++)
	{
		tableaus[i] = board_spotGet(board, TABLEAU, i);
		if(tableaus[i]->cardsCount<1)
			continue;

		card = tableaus[i]->cards[tableaus[i]->cardsCount-1];
		for(j=0;j<4;j++)
		{
			if(card->suit==j)
				suitTopRanks[j] = max(suitTopRanks[j], (card->rank==1 ? 14 : card->rank));
		}
	}

	// Play card to foundation
	for(i=0;i<4;i++)
	{
		if(tableaus[i]->cardsCount<1)
			continue;

		card = tableaus[i]->cards[tableaus[i]->cardsCount-1];
		for(j=0;j<4;j++)
		{
			if(card->rank!=1 && card->suit==j && card->rank<suitTopRanks[j])
				movePack_add(movePack, move_create(MOVE, TABLEAU, i, -1, FOUNDATION, 0, -1));
		}
	}

	// Tableau to Tableau
	for(i=0;i<4;i++)
	{
		if(tableaus[i]->cardsCount<1)
			continue;

		for(j=0;j<4;j++)
		{
			if(i==j || tableaus[j]->cardsCount>0)
				continue;

			movePack_add(movePack, move_create(MOVE, TABLEAU, i, -1, TABLEAU, j, -1));
		}
	}

	// Play stock to tableau
	if(stock->cardsCount>0)
		movePack_add(movePack, move_create(MOVEDISTRIBUTE, STOCK, 0, -1, TABLEAU, 4, -1));

	return movePack;
}

bool	acesup_isWon(Board * board)
{
	return board_spotGet(board, FOUNDATION, 0)->cardsCount==48;
}

Game acesup[] =
{
	{ ACESUP, "AcesUp", MAX_DEPTH_NA, 1000000, 1, 0, &acesup_boardCreate, &acesup_movePackGet, &acesup_isWon },
	{ GAMEID_END }
};

LOADGAMES(acesup)