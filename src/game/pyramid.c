#include "../sim.h"

Card ** pyramid_deckCreateWin(unsigned long * cardsCount)
{
	char * cardString = strdup("13s,12s,1s,13d,13h,10s,3s,8s,9s,4s,5s,7s,6s,12h,1h,11h,2h,10h,3h,9h,4h,8h,5h,7h,6h,11s,12d,1d,11d,2d,10d,3d,9d,4d,8d,5d,7d,6d,13c,12c,1c,11c,2c,10c,3c,9c,4c,8c,5c,7c,6c,2s");
	Card ** cards;
	*cardsCount = 52;
	cards = cards_createFromIds(cardString);
	free(cardString);
	return cards;
}

void  pyramid_freeGameState(Board * board)
{
	if(gConfig.game->id==PYRAMIDEASY || gConfig.game->id==PYRAMIDRELAXEDEASY)
		free(board->gameState);
}

void * pyramid_dupGameState(Board * board)
{
	unsigned char * gameState = (unsigned char *)malloc(sizeof(unsigned char));
	*gameState = *(unsigned char *)board->gameState;
	return gameState;
}

Board * pyramid_boardCreate(Card ** cards, unsigned long cardsCount)
{
	Board * board = board_create();
	unsigned char i;

	board_spotAdd(board, spot_create(STOCK, 0));
	board_spotAdd(board, spot_create(FOUNDATION, 0));
	board_spotAdd(board, spot_create(WASTE, 0));

	for(i=0;i<28;i++)
	{
		board_spotAdd(board, spot_create(TABLEAU, i));
	}

	for(i=0;i<28;i++)
	{
		board_cardAdd(board, TABLEAU, i, cards[i]);
	}
	for(;i<52;i++)
	{
		board_cardAdd(board, STOCK, 0, cards[i]);
	}

	if(gConfig.game->id==PYRAMIDEASY || gConfig.game->id==PYRAMIDRELAXEDEASY)
	{
		board->gameState = (unsigned char *)malloc(sizeof(unsigned char));
		*(unsigned char *)board->gameState = 0;
	}

	return board;
}

void pyramid_addCombineMove(MovePack * movePack, SPOTTYPE spot1, unsigned char spot1Index, SPOTTYPE spot2, unsigned char spot2Index)
{
	MovePack * subPack = movePack_create();
	movePack_add(subPack, move_create(MOVE, spot1, spot1Index, -1, FOUNDATION, 0, -1));
	movePack_add(subPack, move_create(MOVE, spot2, spot2Index, -1, FOUNDATION, 0, -1));
	movePack_add(movePack, move_createMultiple(subPack));
}

MovePack * pyramid_movePackGet(Board * board)
{
	MovePack * movePack = movePack_create();
	Spot * stock = board_spotGet(board, STOCK, 0);
	unsigned char stockCardRank = stock->cardsCount>0 ? stock->cards[stock->cardsCount-1]->rank : 0;
	Spot * waste = board_spotGet(board, WASTE, 0);
	unsigned char wasteCardRank = waste->cardsCount>0 ? waste->cards[waste->cardsCount-1]->rank : 0;
	unsigned char tableauCardRanks[28];
	Spot * spot;
	unsigned char i, j, y, tableauCardRank;

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
	for(i=0;i<=20;i++)
	{
		if(tableauCardRanks[i]==0)
			continue;

		y = (i==0 ? 1 : (i<=2 ? 2 : (i<=5 ? 3 : (i<=9 ? 4 : (i<=14 ? 5 : 6)))));

		if(tableauCardRanks[i+y]!=0 || tableauCardRanks[i+y+1]!=0)
		{
			tableauCardRanks[i] = 0;
			continue;
		}
	}

	// Stock or Waste Kings
	if(stockCardRank==13)
		movePack_add(movePack, move_create(MOVE, STOCK, 0, -1, FOUNDATION, 0, -1));
	if(wasteCardRank==13)
		movePack_add(movePack, move_create(MOVE, WASTE, 0, -1, FOUNDATION, 0, -1));

	// Waste+Stock
	if(stockCardRank!=0 && wasteCardRank!=0 && stockCardRank+wasteCardRank==13)
		pyramid_addCombineMove(movePack, STOCK, 0, WASTE, 0);

	// Tableau
	for(i=0;i<28;i++)
	{
		if(tableauCardRanks[i]==0)
			continue;

		tableauCardRank = tableauCardRanks[i];

		// Tableau King
		if(tableauCardRank==13)
			movePack_add(movePack, move_create(MOVE, TABLEAU, i, -1, FOUNDATION, 0, -1));

		// Stock+Tableau
		if(stockCardRank!=0 && stockCardRank+tableauCardRank==13)
			pyramid_addCombineMove(movePack, STOCK, 0, TABLEAU, i);

		// Waste+Tableau
		if(wasteCardRank!=0 && wasteCardRank+tableauCardRank==13)
			pyramid_addCombineMove(movePack, WASTE, 0, TABLEAU, i);

		// Tableau+Tableau
		for(j=0;j<28;j++)
		{
			if(i==j || tableauCardRanks[j]==0)
				continue;
		
			if(tableauCardRank+tableauCardRanks[j]==13)
				pyramid_addCombineMove(movePack, TABLEAU, i, TABLEAU, j);
		}
	}

	// Stock->Waste
	if(stockCardRank!=0)
		movePack_add(movePack, move_create(MOVE, STOCK, 0, -1, WASTE, 0, -1));

	if((gConfig.game->id==PYRAMIDEASY || gConfig.game->id==PYRAMIDRELAXEDEASY) && wasteCardRank!=0 && stockCardRank==0 && *(unsigned char *)board->gameState<2)
	{
		*(unsigned char *)board->gameState = (*(unsigned char *)board->gameState)+1;
		movePack_add(movePack, move_create(MOVEREVERSED, WASTE, 0, 0, STOCK, 0, -1));
	}

	return movePack;
}

bool pyramid_isWon(Board * board)
{
	unsigned char i;
	for(i=0;i<28;i++)
	{
		if(board_spotGet(board, TABLEAU, i)->cardsCount!=0)
			return false;
	}

	if(gConfig.game->id==PYRAMIDRELAXED || gConfig.game->id==PYRAMIDRELAXEDEASY)
		return true;

	return board_spotGet(board, FOUNDATION, 0)->cardsCount==52;
}

Game pyramid[] =
{
	{ PYRAMID,            "Pyramid",            MAX_DEPTH_NA, 500000, 1, 0, &pyramid_boardCreate, &pyramid_movePackGet, &pyramid_isWon },
	{ PYRAMIDEASY,        "PyramidEasy",        MAX_DEPTH_NA, 500000, 1, 0, &pyramid_boardCreate, &pyramid_movePackGet, &pyramid_isWon, 0, &pyramid_freeGameState, &pyramid_dupGameState },
	{ PYRAMIDRELAXED,     "PyramidRelaxed",     MAX_DEPTH_NA, 800000, 1, 0, &pyramid_boardCreate, &pyramid_movePackGet, &pyramid_isWon, 0, &pyramid_freeGameState, &pyramid_dupGameState },
	{ PYRAMIDRELAXEDEASY, "PyramidRelaxedEasy", MAX_DEPTH_NA, 600000, 1, 0, &pyramid_boardCreate, &pyramid_movePackGet, &pyramid_isWon, 0, &pyramid_freeGameState, &pyramid_dupGameState },
	{ GAMEID_END }
};

LOADGAMES(pyramid)