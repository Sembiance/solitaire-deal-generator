#include "../sim.h"

Board * easthaven_boardCreate(Card ** cards, unsigned long cardsCount)
{
	Board * board = board_create();
	unsigned char i, spotIndex, spotCount;
	unsigned char foundationCount = (gConfig.game->id==TRIPLEEASTHAVEN ? 12 : (gConfig.game->id==DOUBLEEASTHAVEN ? 8 : 4));
	unsigned char tableauCount = (gConfig.game->id==TRIPLEEASTHAVEN ? 12 : (gConfig.game->id==DOUBLEEASTHAVEN ? 8 : 7));

	board_spotAdd(board, spot_create(STOCK, 0));

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
		for(spotCount=0;spotCount<3;spotCount++)
		{
			board_cardAdd(board, TABLEAU, spotIndex, cards[i++]);
			if(spotCount<2)
				cards[i-1]->faceUp = false;
		}
	}
	for(;i<cardsCount;i++)
	{
		board_cardAdd(board, STOCK, 0, cards[i]);
	}

	return board;
}

bool easthaven_validCardTableauDrop(Card * from, Card * to)
{
	if(from->rank+1!=to->rank)
		return false;

	return from->color!=to->color;
}

MovePack * easthaven_movePackGet(Board * board)
{
	unsigned char i, j, k;
	bool stackedCorrectly;
	unsigned char foundationCount = (gConfig.game->id==TRIPLEEASTHAVEN ? 12 : (gConfig.game->id==DOUBLEEASTHAVEN ? 8 : 4));
	unsigned char tableauCount = (gConfig.game->id==TRIPLEEASTHAVEN ? 12 : (gConfig.game->id==DOUBLEEASTHAVEN ? 8 : 7));
	MovePack * movePack = movePack_create();
	Spot * stock = board_spotGet(board, STOCK, 0);
	Spot * foundations[foundationCount];
	Spot * tableaus[tableauCount];
	Card * card;

	for(i=0;i<tableauCount;i++)
	{
		if(i<foundationCount)
			foundations[i] = board_spotGet(board, FOUNDATION, i);
		tableaus[i] = board_spotGet(board, TABLEAU, i);
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
				// All foundations are created equal, if moving an Ace, there is no need to entertain moving to the other foundations
				break;
			}
			else if(foundations[j]->cardsCount>0 && card->rank==foundations[j]->cards[foundations[j]->cardsCount-1]->rank+1 && card->suit==foundations[j]->cards[foundations[j]->cardsCount-1]->suit)
			{
				movePack_add(movePack, move_create(MOVE, TABLEAU, i, -1, FOUNDATION, j, -1));
				// All foundations are created equal, if moving an Ace, there is no need to entertain moving to the other foundations
				break;
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

			stackedCorrectly = true;
			for(k=j+1;stackedCorrectly && k<tableaus[i]->cardsCount;k++)
			{
				stackedCorrectly = easthaven_validCardTableauDrop(tableaus[i]->cards[k], tableaus[i]->cards[k-1]);
			}
			if(!stackedCorrectly)
				continue;

			for(k=0;k<tableauCount;k++)
			{
				if(k==i)
					continue;

				if(tableaus[k]->cardsCount==0 && j>0 && (card->rank==13 || (gConfig.game->id!=EASTHAVENKINGSONLY && gConfig.game->id!=DOUBLEEASTHAVENKINGSONLY && gConfig.game->id!=TRIPLEEASTHAVENKINGSONLY)))
				{
					movePack_add(movePack, move_create(MOVE, TABLEAU, i, j, TABLEAU, k, -1));
					// If moving to an empty tableau, no need to consider moving a card higher up in the stack
					j=tableaus[i]->cardsCount-1;
					// All empty tableaus are created equal, if moving a there, no need to entertain moving to the other empty tableuas
					break;
				}
				else if(tableaus[k]->cardsCount>0 && easthaven_validCardTableauDrop(card, tableaus[k]->cards[tableaus[k]->cardsCount-1]))
				{
					movePack_add(movePack, move_create(MOVE, TABLEAU, i, j, TABLEAU, k, -1));
				}
			}
		}
	}

	// Stock to tableau
	if(stock->cardsCount>0)
		movePack_add(movePack, move_create(MOVEDISTRIBUTE, STOCK, 0, -1, TABLEAU, min(tableauCount, stock->cardsCount), -1));

	return movePack;
}

bool easthaven_isWon(Board * board)
{
	unsigned char i;
	unsigned char foundationCount = (gConfig.game->id==TRIPLEEASTHAVEN ? 12 : (gConfig.game->id==DOUBLEEASTHAVEN ? 8 : 4));
	Spot * foundation;
	for(i=0;i<foundationCount;i++)
	{
		foundation = board_spotGet(board, FOUNDATION, i);
		if(foundation->cardsCount!=13)
			return false;
	}

	return true;
}

void easthaven_updateBoard(Board * board)
{
	unsigned char i;
	unsigned char tableauCount = (gConfig.game->id==TRIPLEEASTHAVEN ? 12 : (gConfig.game->id==DOUBLEEASTHAVEN ? 8 : 7));
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

Game easthaven[] =
{
	{ DOUBLEEASTHAVEN,          "DoubleEasthaven",          MAX_DEPTH_MEDIUM, 4000, 2, 0, &easthaven_boardCreate, &easthaven_movePackGet, &easthaven_isWon, &easthaven_updateBoard },
	{ DOUBLEEASTHAVENKINGSONLY, "DoubleEasthavenKingsOnly", MAX_DEPTH_MEDIUM, 4000, 2, 0, &easthaven_boardCreate, &easthaven_movePackGet, &easthaven_isWon, &easthaven_updateBoard },
	{ EASTHAVEN,                "Easthaven",                MAX_DEPTH_MEDIUM, 4000, 1, 0, &easthaven_boardCreate, &easthaven_movePackGet, &easthaven_isWon, &easthaven_updateBoard },
	{ EASTHAVENKINGSONLY,       "EasthavenKingsOnly",       MAX_DEPTH_MEDIUM, 4000, 1, 0, &easthaven_boardCreate, &easthaven_movePackGet, &easthaven_isWon, &easthaven_updateBoard },
	{ TRIPLEEASTHAVEN,          "TripleEasthaven",          MAX_DEPTH_HIGH,   4000, 3, 0, &easthaven_boardCreate, &easthaven_movePackGet, &easthaven_isWon, &easthaven_updateBoard },
	{ TRIPLEEASTHAVENKINGSONLY, "TripleEasthavenKingsOnly", MAX_DEPTH_HIGH,   4000, 3, 0, &easthaven_boardCreate, &easthaven_movePackGet, &easthaven_isWon, &easthaven_updateBoard },
	{ GAMEID_END }
};

LOADGAMES(easthaven)