#include "../sim.h"

Board * freecell_boardCreate(Card ** cards, unsigned long cardsCount)
{
	Board * board = board_create();
	unsigned char i;
	unsigned char tableauCount = gConfig.game->id==SEAHAVENTOWERS ? 10 : 8;

	for(i=0;i<tableauCount;i++)
	{
		if(i<4)
		{
			board_spotAdd(board, spot_create(FOUNDATION, i));
			board_spotAdd(board, spot_create(CELL, i));
		}

		board_spotAdd(board, spot_create(TABLEAU, i));
	}

	for(i=0;i<52;i++)
	{
		if(i>=50 && gConfig.game->id==SEAHAVENTOWERS)
			board_cardAdd(board, CELL, (i-49), cards[i]);
		else
			board_cardAdd(board, TABLEAU, (i%tableauCount), cards[i]);
	}

	return board;
}

bool freecell_validCardTableauDrop(Card * from, Card * to)
{
	if(from->rank+1!=to->rank)
		return false;

	if(gConfig.game->id==BAKERSGAME || gConfig.game->id==BAKERSGAMEKINGSONLY || gConfig.game->id==SEAHAVENTOWERS)
		return from->suit==to->suit;
	else
		return from->color!=to->color;
}

MovePack * freecell_movePackGet(Board * board)
{
	MovePack * movePack = movePack_create();
	unsigned char i, j, k;
	unsigned char openCellCount=0;
	unsigned char tableauCount = gConfig.game->id==SEAHAVENTOWERS ? 10 : 8;
	bool correctlyStacked;
	Spot * cells[4];
	Spot * foundations[4];
	Spot * tableaus[tableauCount];
	Card * card;

	for(i=0;i<tableauCount;i++)
	{
		if(i<4)
		{
			foundations[i] = board_spotGet(board, FOUNDATION, i);
			cells[i] = board_spotGet(board, CELL, i);
			if(cells[i]->cardsCount==0)
				openCellCount++;
		}
		tableaus[i] = board_spotGet(board, TABLEAU, i);
	}

	// Cell
	for(i=0;i<4;i++)
	{
		if(cells[i]->cardsCount==0)
			continue;

		card = cells[i]->cards[cells[i]->cardsCount-1];

		// Cell->Tableau
		for(j=0;j<tableauCount;j++)
		{
			if((tableaus[j]->cardsCount==0 && ((gConfig.game->id!=BAKERSGAMEKINGSONLY && gConfig.game->id!=SEAHAVENTOWERS) || card->rank==13)) || (tableaus[j]->cardsCount>0 && freecell_validCardTableauDrop(card, tableaus[j]->cards[tableaus[j]->cardsCount-1])))
				movePack_add(movePack, move_create(MOVE, CELL, i, -1, TABLEAU, j, -1));
		}

		// Cell->Foundation
		for(j=0;j<4;j++)
		{
			if(foundations[j]->cardsCount==0 && card->rank==1)
			{
				movePack_add(movePack, move_create(MOVE, CELL, i, -1, FOUNDATION, j, -1));
				// All empty foundations are created equal, if moving an Ace, there is no need to entertain moving to the other foundations
				break;
			}
			else if(foundations[j]->cardsCount>0 && card->rank==foundations[j]->cards[foundations[j]->cardsCount-1]->rank+1 && card->suit==foundations[j]->cards[foundations[j]->cardsCount-1]->suit)
			{
				movePack_add(movePack, move_create(MOVE, CELL, i, -1, FOUNDATION, j, -1));
			}
		}
	}

	// Tableau->Tableau
	for(i=0;i<tableauCount;i++)
	{
		if(tableaus[i]->cardsCount==0)
			continue;

		for(j=0;j<tableaus[i]->cardsCount;j++)
		{
			card = tableaus[i]->cards[j];

			// Ensure stack not too long
			if((tableaus[i]->cardsCount-j)>(openCellCount+1))
				continue;

			// And is correctly stacked
			for(correctlyStacked=true,k=(j+1);k<tableaus[i]->cardsCount;k++)
			{
				if(!freecell_validCardTableauDrop(tableaus[i]->cards[k], tableaus[i]->cards[(k-1)]))
				{
					correctlyStacked = false;
					break;
				}
			}

			if(!correctlyStacked)
				continue;

			//printf("tableau %d at index %d has correctly ranked card: %s\n", i, j, card_print(card, false));

			for(k=0;k<tableauCount;k++)
			{
				if(k==i)
					continue;

				if((tableaus[k]->cardsCount==0 && j>0 && ((gConfig.game->id!=BAKERSGAMEKINGSONLY && gConfig.game->id!=SEAHAVENTOWERS) || card->rank==13)) ||
				   (tableaus[k]->cardsCount>0 && freecell_validCardTableauDrop(card, tableaus[k]->cards[tableaus[k]->cardsCount-1])))
				{
					movePack_add(movePack, move_create(MOVE, TABLEAU, i, j, TABLEAU, k, -1));
				}
			}
		}
	}

	// Tableau->Cell
	for(i=0;i<4;i++)
	{
		if(cells[i]->cardsCount>0)
			continue;

		for(j=0;j<tableauCount;j++)
		{
			if(tableaus[j]->cardsCount==0)
				continue;

			movePack_add(movePack, move_create(MOVE, TABLEAU, j, -1, CELL, i, -1));
		}

		break;
	}

	// Tableau->Foundation
	for(i=0;i<tableauCount;i++)
	{
		if(tableaus[i]->cardsCount==0)
			continue;

		card = tableaus[i]->cards[tableaus[i]->cardsCount-1];

		for(j=0;j<4;j++)
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

	return movePack;
}

bool freecell_isWon(Board * board)
{
	unsigned char i;
	Spot * foundation;
	for(i=0;i<4;i++)
	{
		foundation = board_spotGet(board, FOUNDATION, i);
		if(foundation->cardsCount!=13)
			return false;
	}

	return true;
}

Game freecell[] =
{
	{ BAKERSGAME,          "BakersGame",          MAX_DEPTH_DEFAULT, 1000000, 1, 0, &freecell_boardCreate, &freecell_movePackGet, &freecell_isWon },
	{ BAKERSGAMEKINGSONLY, "BakersGameKingsOnly", MAX_DEPTH_DEFAULT,  350000, 1, 0, &freecell_boardCreate, &freecell_movePackGet, &freecell_isWon },
	{ FREECELL,            "FreeCell",            MAX_DEPTH_DEFAULT,   50000, 1, 0, &freecell_boardCreate, &freecell_movePackGet, &freecell_isWon },
	{ SEAHAVENTOWERS,      "SeahavenTowers",      MAX_DEPTH_DEFAULT,  150000, 1, 0, &freecell_boardCreate, &freecell_movePackGet, &freecell_isWon },
	{ GAMEID_END }
};

LOADGAMES(freecell)