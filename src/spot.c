#include "sim.h"

const char * SPOTTYPE_NAMES[] =
{
	"n/a",
	"stock",
	"waste",
	"tableau",
	"foundation",
	"cell"
};

Spot * spot_create(SPOTTYPE spotType, unsigned char spotIndex)
{
	Spot * spot = (Spot *)malloc(sizeof(Spot));
	spot->type = spotType;
	spot->index = spotIndex;
	spot->cardsCount = 0;
	spot->cards = 0;

	return spot;
}

void spot_free(Spot * spot)
{
	unsigned char i;
	for(i=0;i<spot->cardsCount;i++)
	{
		card_free(spot->cards[i]);
	}
	free(spot->cards);
	free(spot);
}

const char * spot_name(SPOTTYPE spotType)
{
	return SPOTTYPE_NAMES[spotType];
}

void spot_cardAdd(Spot * spot, Card * card)
{
	spot->cardsCount++;
	spot->cards = (Card **)realloc(spot->cards, sizeof(Card *)*spot->cardsCount);
	spot->cards[spot->cardsCount-1] = card;
}

void spot_print(Spot * spot, bool showDownCards)
{
	printf("%10s%d: %s\n", spot_name(spot->type), spot->index, cards_print(spot->cards, spot->cardsCount, showDownCards));
}

Spot * spot_dup(Spot * spot)
{
	unsigned char i;
	Spot * dupSpot = spot_create(spot->type, spot->index);
	dupSpot->cards = (Card **)malloc(sizeof(Card *)*spot->cardsCount);
	dupSpot->cardsCount = spot->cardsCount;
	for(i=0;i<spot->cardsCount;i++)
	{
		dupSpot->cards[i] = card_dup(spot->cards[i]);
	}
	return dupSpot;
}

void spot_moveCards(Spot * fromSpot, unsigned char fromCardsIndex, unsigned char cardsToMoveCount, Spot * toSpot, unsigned char toCardsIndex, bool reverseCards)
{
	unsigned char displacedCardsCount = toSpot->cardsCount>0 && toCardsIndex<toSpot->cardsCount ? toSpot->cardsCount-toCardsIndex : 0;
	//printf("%d from %s%d %d to %s%d %d\n", cardsToMoveCount, fromSpot->name, fromSpot->index, fromCardsIndex, toSpot->name, toSpot->index, toCardsIndex);

	toSpot->cards = (Card **)realloc(toSpot->cards, sizeof(Card *)*(cardsToMoveCount+toSpot->cardsCount));
	if(displacedCardsCount>0)
	{
		printf("TODO!! INSERTING cards, so displacing %d cards in destination\n", displacedCardsCount);
		exit(EXIT_FAILURE);
		//memcpy(toSpot->cards+(sizeof(Card *)*((toSpot->cardsCount-toCardsIndex)+cardsToMoveCount)), toSpot->cards+(sizeof(Card *)*(toSpot->cardsCount-toCardsIndex)));
	}

	memcpy(&toSpot->cards[toCardsIndex], &fromSpot->cards[fromCardsIndex], sizeof(Card *)*cardsToMoveCount);
	if(reverseCards)
		cards_reverse(&toSpot->cards[toCardsIndex], cardsToMoveCount);
	toSpot->cardsCount+=cardsToMoveCount;

	fromSpot->cardsCount-=cardsToMoveCount;
	fromSpot->cards = (Card **)realloc(fromSpot->cards, sizeof(Card *)*fromSpot->cardsCount);
}