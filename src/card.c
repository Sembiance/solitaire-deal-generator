#include "sim.h"

static const char SUIT_CHAR[] = { 'c', 'd', 'h', 's' };

Card * card_create(unsigned char rank, SUIT suit, bool faceUp)
{
	Card * card = (Card *)malloc(sizeof(Card));
	card->rank = rank;
	card->suit = suit;
	card->color = suit==HEART || suit==DIAMOND ? RED : BLACK;
	card->faceUp = faceUp;

	return card;
}

Card * card_createFromId(char * id)
{
	char rank[3] = {0, 0, 0};
	rank[0] = id[0];
	char suit = id[1];

	if(strlen(id)==3)
	{
		rank[1] = id[1];
		suit = id[2];
	}

	return card_create(atoi(rank), (suit=='s' ? SPADE : (suit=='c' ? CLUB : (suit=='h' ? HEART : DIAMOND))), true);
}

void card_free(Card * card)
{
	free(card);
}

Card * card_dup(Card * card)
{
	Card * dupCard = (Card *)malloc(sizeof(Card));
	memcpy(dupCard, card, sizeof(Card));
	return dupCard;
}

void cards_reverse(Card ** cards, unsigned char cardsCount)
{
	unsigned char start=0;
	unsigned char end=cardsCount-1;
	Card * card;
	while(start<end)
	{
		card = cards[start];
		cards[start] = cards[end];
		cards[end] = card;
		start++;
		end--;
	}
}

Card ** cards_createFromIds(char * idAsString)
{
	unsigned long cardCount = strchrcount(idAsString, ',')+1;
	char ** ids = strchrexplode(idAsString, ',');
	char ** id;
	unsigned long i;
	Card ** cards = (Card **)malloc(sizeof(Card *)*cardCount);

	for(i=0,id=ids;*id;id++,i++)
	{
		cards[i] = card_createFromId(*id);
		free(*id);
	}
	free(ids);

	return cards;
}

Card ** cards_createDeck(unsigned char decksCount)
{
	unsigned char i, d;
	Card ** cards = (Card **)malloc(sizeof(Card *)*(52*decksCount));

	for(d=0;d<decksCount;d++)
	{
		for(i=1;i<=13;i++)
		{
			cards[((i-1)*4)+(d*52)] = card_create(i, CLUB, true);
			cards[((i-1)*4)+1+(d*52)] = card_create(i, DIAMOND, true);
			cards[((i-1)*4)+2+(d*52)] = card_create(i, HEART, true);
			cards[((i-1)*4)+3+(d*52)] = card_create(i, SPADE, true);
		}
	}

	return cards;
}

char cardPrintBufs[255][5];
unsigned char cardPrintBufIndex=0;

char * card_print(Card * card, bool showDownCard)
{
	if(cardPrintBufIndex>=255)
		cardPrintBufIndex = 0;

	if(!card->faceUp && !showDownCard)
		sprintf(cardPrintBufs[cardPrintBufIndex], "BACK");
	else
		sprintf(cardPrintBufs[cardPrintBufIndex], "%3d%c", card->rank, SUIT_CHAR[card->suit]);

	cardPrintBufIndex++;
	return cardPrintBufs[(cardPrintBufIndex-1)];
}

void cards_printAsDeck(Card ** cards, unsigned char cardsCount)
{
	unsigned char i;
	printf("[");
	for(i=0;i<cardsCount;i++)
	{
		if(i>0)
			printf(",");

		printf("\"%d%c\"", cards[i]->rank, SUIT_CHAR[cards[i]->suit]);
	}
	printf("]");
}

char cardsPrintBufs[255][255];
unsigned char cardsPrintBufIndex=0;

char * cards_print(Card ** cards, unsigned char cardsCount, bool showDownCards)
{
	unsigned char i;
	if(cardsPrintBufIndex>=255)
		cardsPrintBufIndex = 0;

	memset(cardsPrintBufs[cardsPrintBufIndex], 0, 255);

	for(i=0;i<cardsCount;i++)
	{
		if(i>0)
			strcat(cardsPrintBufs[cardsPrintBufIndex], " ");

		strcat(cardsPrintBufs[cardsPrintBufIndex], card_print(cards[i], showDownCards));
	}

	cardsPrintBufIndex++;
	return cardsPrintBufs[(cardsPrintBufIndex-1)];
}

void cards_shuffle(Card ** cards, unsigned char cardsCount)
{
	Card * t;
	unsigned char i;
	unsigned char m=cardsCount;
	while(m)
	{
		i = randrange(0, --m);
		t = cards[m];
		cards[m] = cards[i];
		cards[i] = t;
	}
}
