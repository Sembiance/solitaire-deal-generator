#include "../sim.h"

Board * klondike_boardCreate(Card ** cards, unsigned long cardsCount)
{
	Board * board = board_create();
	unsigned char i, spotIndex, spotCount;
	unsigned char foundationCount = (gConfig.game->id==TRIPLEKLONDIKETURN1 || gConfig.game->id==TRIPLEKLONDIKETURN3 ? 12 : (gConfig.game->id==DOUBLEKLONDIKETURN1 || gConfig.game->id==DOUBLEKLONDIKETURN3 ? 8 : 4));
	unsigned char tableauCount = (gConfig.game->id==TRIPLEKLONDIKETURN1 || gConfig.game->id==TRIPLEKLONDIKETURN3 ? 13 : (gConfig.game->id==DOUBLEKLONDIKETURN1 || gConfig.game->id==DOUBLEKLONDIKETURN3 ? 9 : 7));

	board_spotAdd(board, spot_create(STOCK, 0));
	board_spotAdd(board, spot_create(WASTE, 0));

	for(i=0;i<tableauCount;i++)
	{
		board_spotAdd(board, spot_create(TABLEAU, i));
	}
	for(i=0;i<foundationCount;i++)
	{
		board_spotAdd(board, spot_create(FOUNDATION, i));
	}

	i=0;
	for(spotIndex=0;spotIndex<tableauCount;spotIndex++)
	{
		for(spotCount=0;spotCount<(spotIndex+1);spotCount++)
		{
			board_cardAdd(board, TABLEAU, spotIndex, cards[i++]);
			if(spotCount<spotIndex)
				cards[i-1]->faceUp = false;
		}
	}
	for(;i<cardsCount;i++)
	{
		board_cardAdd(board, STOCK, 0, cards[i]);
	}

	return board;
}

MovePack * klondike_movePackGet(Board * board)
{
	unsigned char i, j, k;
	unsigned char foundationCount = (gConfig.game->id==TRIPLEKLONDIKETURN1 || gConfig.game->id==TRIPLEKLONDIKETURN3 ? 12 : (gConfig.game->id==DOUBLEKLONDIKETURN1 || gConfig.game->id==DOUBLEKLONDIKETURN3 ? 8 : 4));
	unsigned char tableauCount = (gConfig.game->id==TRIPLEKLONDIKETURN1 || gConfig.game->id==TRIPLEKLONDIKETURN3 ? 13 : (gConfig.game->id==DOUBLEKLONDIKETURN1 || gConfig.game->id==DOUBLEKLONDIKETURN3 ? 9 : 7));
	MovePack * movePack = movePack_create();
	Spot * stock = board_spotGet(board, STOCK, 0);
	Spot * waste = board_spotGet(board, WASTE, 0);
	Spot * foundations[foundationCount];
	Spot * tableaus[tableauCount];
	Card * card;

	for(i=0;i<tableauCount;i++)
	{
		if(i<foundationCount)
			foundations[i] = board_spotGet(board, FOUNDATION, i);
		tableaus[i] = board_spotGet(board, TABLEAU, i);
	}

	// Waste to foundation or tableau
	if(waste->cardsCount>0)
	{
		card = waste->cards[waste->cardsCount-1];
		for(i=0;i<foundationCount;i++)
		{
			if(foundations[i]->cardsCount==0 && card->rank==1)
			{
				movePack_add(movePack, move_create(MOVE, WASTE, 0, -1, FOUNDATION, i, -1));
				// All empty foundations are created equal, if moving an Ace there, no need to entertain moving to the other foundations
				break;
			}
			else if(foundations[i]->cardsCount>0 && card->rank==foundations[i]->cards[foundations[i]->cardsCount-1]->rank+1 && card->suit==foundations[i]->cards[foundations[i]->cardsCount-1]->suit)
			{
				movePack_add(movePack, move_create(MOVE, WASTE, 0, -1, FOUNDATION, i, -1));
			}
		}

		for(i=0;i<tableauCount;i++)
		{
			if(tableaus[i]->cardsCount==0 && card->rank==13)
			{
				movePack_add(movePack, move_create(MOVE, WASTE, 0, -1, TABLEAU, i, -1));
				// All empty tableaus are created equal, if moving a King there, no need to entertain moving to the other empty tableuas
				break;
			}
			else if(tableaus[i]->cardsCount>0 && card->rank+1==tableaus[i]->cards[tableaus[i]->cardsCount-1]->rank && card->color!=tableaus[i]->cards[tableaus[i]->cardsCount-1]->color)
			{
				movePack_add(movePack, move_create(MOVE, WASTE, 0, -1, TABLEAU, i, -1));
			}
		}
	}

	// Tableau to foundation
	for(i=0;i<tableauCount;i++)
	{
		if(tableaus[i]->cardsCount==0)
			continue;

		card = tableaus[i]->cards[tableaus[i]->cardsCount-1];

		for(j=0;j<foundationCount;j++)
		{
			if(foundations[j]->cardsCount==0 && card->rank==1)
			{
				movePack_add(movePack, move_create(MOVE, TABLEAU, i, -1, FOUNDATION, j, -1));
				// All empty foundations are created equal, if moving an Ace, there is no need to entertain moving to the other foundations
				break;
			   
			}
			else if(foundations[j]->cardsCount>0 && card->rank==foundations[j]->cards[foundations[j]->cardsCount-1]->rank+1 && card->suit==foundations[j]->cards[foundations[j]->cardsCount-1]->suit)
			{
				movePack_add(movePack, move_create(MOVE, TABLEAU, i, -1, FOUNDATION, j, -1));
			}
		}
	}

	// Tableau to tableau
	for(i=0;i<tableauCount;i++)
	{
		if(tableaus[i]->cardsCount==0)
			continue;

		for(j=0;j<tableaus[i]->cardsCount;j++)
		{
			card = tableaus[i]->cards[j];
			if(!card->faceUp)
				continue;

			for(k=0;k<tableauCount;k++)
			{
				if(k==i)
					continue;

				if(tableaus[k]->cardsCount==0 && card->rank==13 && j>0)
				{
					movePack_add(movePack, move_create(MOVE, TABLEAU, i, j, TABLEAU, k, -1));
					// All empty tableaus are created equal, if moving a King there, no need to entertain moving to the other empty tableuas
					break;
				}
				else if(tableaus[k]->cardsCount>0 && card->rank+1==tableaus[k]->cards[tableaus[k]->cardsCount-1]->rank && card->color!=tableaus[k]->cards[tableaus[k]->cardsCount-1]->color)
				{
					movePack_add(movePack, move_create(MOVE, TABLEAU, i, j, TABLEAU, k, -1));
				}
			}
		}
	}

	// Waste to stock
	if(stock->cardsCount==0 && waste->cardsCount>0)
		movePack_add(movePack, move_create(MOVEREVERSED, WASTE, 0, 0, STOCK, 0, -1));

	// Stock to waste
	if(stock->cardsCount>0)
		movePack_add(movePack, move_create(((gConfig.game->id==KLONDIKETURN3 || gConfig.game->id==DOUBLEKLONDIKETURN3 || gConfig.game->id==TRIPLEKLONDIKETURN3) ? MOVEREVERSED : MOVE), STOCK, 0, (stock->cardsCount-min(stock->cardsCount, ((gConfig.game->id==KLONDIKETURN1 || gConfig.game->id==DOUBLEKLONDIKETURN1 || gConfig.game->id==TRIPLEKLONDIKETURN1) ? 1 : 3))), WASTE, 0, -1));

	return movePack;
}

bool klondike_isWon(Board * board)
{
	unsigned char i;
	unsigned char foundationCount = (gConfig.game->id==TRIPLEKLONDIKETURN1 || gConfig.game->id==TRIPLEKLONDIKETURN3 ? 12 : (gConfig.game->id==DOUBLEKLONDIKETURN1 || gConfig.game->id==DOUBLEKLONDIKETURN3 ? 8 : 4));
	Spot * foundation;
	for(i=0;i<foundationCount;i++)
	{
		foundation = board_spotGet(board, FOUNDATION, i);
		if(foundation->cardsCount!=13)
			return false;
	}

	return true;
}

void klondike_updateBoard(Board * board)
{
	unsigned char i;
	unsigned char tableauCount = (gConfig.game->id==TRIPLEKLONDIKETURN1 || gConfig.game->id==TRIPLEKLONDIKETURN3 ? 13 : (gConfig.game->id==DOUBLEKLONDIKETURN1 || gConfig.game->id==DOUBLEKLONDIKETURN3 ? 9 : 7));
	Spot * tableau;

	for(i=0;i<tableauCount;i++)
	{
		tableau = board_spotGet(board, TABLEAU, i);

		if(tableau->cardsCount==0)
			continue;

		if(!tableau->cards[tableau->cardsCount-1]->faceUp)
			tableau->cards[tableau->cardsCount-1]->faceUp = true;
	}
}

Game klondike[] =
{
	{ DOUBLEKLONDIKETURN1,      "DoubleKlondikeTurn1",      MAX_DEPTH_HIGH,       5000, 2, 0, &klondike_boardCreate, &klondike_movePackGet, &klondike_isWon, &klondike_updateBoard },
	{ DOUBLEKLONDIKETURN3,      "DoubleKlondikeTurn3",      MAX_DEPTH_HIGH,       3000, 2, 0, &klondike_boardCreate, &klondike_movePackGet, &klondike_isWon, &klondike_updateBoard },
	{ KLONDIKETURN1,            "KlondikeTurn1",            MAX_DEPTH_DEFAULT,   50000, 1, 0, &klondike_boardCreate, &klondike_movePackGet, &klondike_isWon, &klondike_updateBoard },
	{ KLONDIKETURN3,            "KlondikeTurn3",            MAX_DEPTH_DEFAULT,   60000, 1, 0, &klondike_boardCreate, &klondike_movePackGet, &klondike_isWon, &klondike_updateBoard },
	{ TRIPLEKLONDIKETURN1,      "TripleKlondikeTurn1",      MAX_DEPTH_HIGH,       3000, 3, 0, &klondike_boardCreate, &klondike_movePackGet, &klondike_isWon, &klondike_updateBoard },
	{ TRIPLEKLONDIKETURN3,      "TripleKlondikeTurn3",      MAX_DEPTH_HIGH,       2500, 3, 0, &klondike_boardCreate, &klondike_movePackGet, &klondike_isWon, &klondike_updateBoard },
	{ GAMEID_END }
};

LOADGAMES(klondike)