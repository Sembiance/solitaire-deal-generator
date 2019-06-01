#include "../sim.h"

#include <math.h>

Board * tripeaks_boardCreate(Card ** cards, unsigned long cardsCount)
{
	Board * board = board_create();
	unsigned char i;

	board_spotAdd(board, spot_create(STOCK, 0));
	board_spotAdd(board, spot_create(FOUNDATION, 0));

	for(i=0;i<28;i++)
	{
		board_spotAdd(board, spot_create(TABLEAU, i));
	}

	for(i=0;i<28;i++)
	{
		board_cardAdd(board, TABLEAU, i, cards[i]);
	}
	for(;i<51;i++)
	{
		board_cardAdd(board, STOCK, 0, cards[i]);
	}

	board_cardAdd(board, FOUNDATION, 0, cards[i]);

	return board;
}

MovePack * tripeaks_movePackGet(Board * board)
{
	MovePack * movePack = movePack_create();
	Spot * stock = board_spotGet(board, STOCK, 0);
	Spot * foundation = board_spotGet(board, FOUNDATION, 0);
	unsigned char foundationCardRank = foundation->cardsCount>0 ? foundation->cards[foundation->cardsCount-1]->rank : 0;
	unsigned char tableauCardRanks[28];
	Spot * spot;
	unsigned char i, y, tableauCardRank;

	// Get our tableau cards
	for(i=0;i<28;i++)
	{
		spot = board_spotGet(board, TABLEAU, i);
		if(spot->cardsCount==1)
			tableauCardRanks[i] = spot->cards[0]->rank;
		else
			tableauCardRanks[i] = 0;
	}

	// Remove those that are covered up by others
	for(i=0;i<=17;i++)
	{
		if(tableauCardRanks[i]==0)
			continue;

		if(i<=2)
			y = i+i+3;
		else if(i<=8)
			y = (i+(unsigned char)ceil((double)i/(double)2)+4);
		else
			y = i+9;

		if(tableauCardRanks[y]!=0 || tableauCardRanks[y+1]!=0)
		{
			tableauCardRanks[i] = 0;
			continue;
		}
	}

	// Tableau->Foundation
	for(i=0;i<28;i++)
	{
		tableauCardRank = tableauCardRanks[i];
		if(tableauCardRank==0)
			continue;

		if(tableauCardRank==foundationCardRank+1 || tableauCardRank==foundationCardRank-1 ||
		   (gConfig.game->id!=TRIPEAKSSTRICT && gConfig.game->id!=TRIPEAKSSTRICTHIDDEN && ((tableauCardRank==13 && foundationCardRank==1) || (tableauCardRank==1 && foundationCardRank==13))))
			movePack_add(movePack, move_create(MOVE, TABLEAU, i, -1, FOUNDATION, 0, -1));
	}

	// Stock->Foundation
	if(stock->cardsCount>0)
		movePack_add(movePack, move_create(MOVE, STOCK, 0, -1, FOUNDATION, 0, -1));

	return movePack;
}

bool tripeaks_isWon(Board * board)
{
	return board_spotGet(board, FOUNDATION, 0)->cardsCount==52;
}
Game tripeaks[] =
{
	{ TRIPEAKS,             "TriPeaks",             MAX_DEPTH_NA,  50000, 1, 0, &tripeaks_boardCreate, &tripeaks_movePackGet, &tripeaks_isWon },
	{ TRIPEAKSHIDDEN,       "TriPeaksHidden",       MAX_DEPTH_NA,  50000, 1, 0, &tripeaks_boardCreate, &tripeaks_movePackGet, &tripeaks_isWon },
	{ TRIPEAKSSTRICT,       "TriPeaksStrict",       MAX_DEPTH_NA, 100000, 1, 0, &tripeaks_boardCreate, &tripeaks_movePackGet, &tripeaks_isWon },
	{ TRIPEAKSSTRICTHIDDEN, "TriPeaksStrictHidden", MAX_DEPTH_NA, 100000, 1, 0, &tripeaks_boardCreate, &tripeaks_movePackGet, &tripeaks_isWon },
	{ GAMEID_END }
};

LOADGAMES(tripeaks)