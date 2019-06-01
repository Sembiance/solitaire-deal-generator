#include "sim.h"

Game ** 		games=0;
unsigned long 	gamesCount=0;

void acesup_load(void);
void easthaven_load(void);
void freecell_load(void);
void klondike_load(void);
void pyramid_load(void);
void tripeaks_load(void);

void games_load(void)
{
	acesup_load();
	easthaven_load();
	freecell_load();
	klondike_load();
	pyramid_load();
	tripeaks_load();
}

void add_game(Game * game)
{
	gamesCount++;
	// cppcheck-suppress memleakOnRealloc
	games = (Game **)realloc(games, sizeof(Game *)*gamesCount); 
	games[(gamesCount-1)] = game;
}

Game * find_game(const char * idname)
{
	for(unsigned long i=0;i<gamesCount;i++)
	{
		if(!strcasecmp(games[i]->idname, idname))
			return games[i];
	}

	return 0;
}

void print_games_list(void)
{
    for(unsigned long i=0;i<gamesCount;i++)
    {
		fprintf(stderr, "%s\n", games[i]->idname);
    }
}


// Generic functions
Card ** game_deckCreate(unsigned long * cardsCount)
{
	Card ** cards = cards_createDeck(gConfig.game->numDecks);
	*cardsCount = (gConfig.game->numDecks*52);

	cards_shuffle(cards, *cardsCount);

	return cards;
}
