#ifndef __SIM_H
#define __SIM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "version.h"

/*-----------------------------------------------------------------------------
 * Constants
 *----------------------------------------------------------------------------*/
#define MAX_NAME		   255

#define MAX_DEPTH_DEFAULT  700
#define MAX_DEPTH_MEDIUM   4000
#define MAX_DEPTH_HIGH     10000
#define MAX_DEPTH_NA       999999

#define ITERATIONS_DEFAULT 100000

#define C_NORMAL  "\x1B[0m"
#define C_RED     "\x1B[31m"
#define C_GREEN   "\x1B[32m"
#define C_YELLOW  "\x1B[33m"
#define C_BLUE    "\x1B[34m"
#define C_MAGENTA "\x1B[35m"
#define C_CYAN    "\x1B[36m"
#define C_WHITE   "\x1B[37m"
#define C_RESET   "\033[0m"

/*-----------------------------------------------------------------------------
 * Data types
 *----------------------------------------------------------------------------*/
typedef enum
{
	RED,
	BLACK
} COLOR;

typedef enum
{
	CLUB,
	DIAMOND,
	HEART,
	SPADE
} SUIT;

typedef enum
{
	MOVE,
	MOVEREVERSED,
	MOVEDISTRIBUTE,
	MULTIPLE
} MOVETYPE;

typedef enum
{
	SPOT_NA,
	STOCK,
	WASTE,
	TABLEAU,
	FOUNDATION,
	CELL
	// ALERT!! IF ADDING:   UPDATE spot.c SPOTTYPE_NAMES[]
} SPOTTYPE;

typedef enum
{
	ACESUP,
	BAKERSGAME,
	BAKERSGAMEKINGSONLY,
	DOUBLEEASTHAVEN,
	DOUBLEEASTHAVENKINGSONLY,
	DOUBLEKLONDIKETURN1,
	DOUBLEKLONDIKETURN3,
	EASTHAVEN,
	EASTHAVENKINGSONLY,
	FREECELL,
	KLONDIKETURN1,
	KLONDIKETURN3,
	PYRAMID,
	PYRAMIDEASY,
	PYRAMIDRELAXED,
	PYRAMIDRELAXEDEASY,
	SEAHAVENTOWERS,
	TRIPEAKS,
	TRIPEAKSHIDDEN,
	TRIPEAKSSTRICT,
	TRIPEAKSSTRICTHIDDEN,
	TRIPLEEASTHAVEN,
	TRIPLEEASTHAVENKINGSONLY,
	TRIPLEKLONDIKETURN1,
	TRIPLEKLONDIKETURN3,
	GAMEID_END
} GAMEID;

typedef struct
{
	unsigned char 	rank;
	SUIT 			suit;
	COLOR			color;
	bool			faceUp;
} Card;

typedef struct
{
	SPOTTYPE		type;
	unsigned char	index;
	unsigned char	cardsCount;
	Card **			cards;
} Spot;

typedef struct
{
	unsigned char 	spotsCount;
	Spot **			spots;
	void *          gameState;
} Board;

typedef struct
{
	MOVETYPE		type;
	SPOTTYPE		fromSpotType;
	unsigned char	fromSpotIndex;
	int				fromSpotCardIndex;
	SPOTTYPE		toSpotType;
	unsigned char	toSpotIndex;
	int				toSpotCardIndex;
	struct _MovePack * movePack;
} Move;

typedef struct _MovePack
{
	unsigned long 	movesCount;
	Move **			moves;
} MovePack;

typedef struct MoveLink
{
	Move * move;
	struct MoveLink * prev;
} MoveLink;

typedef struct
{
	GAMEID id;
	const char	idname[MAX_NAME];
	unsigned long defaultMaxDepth;
	unsigned long defaultIterationsCount;
	unsigned char numDecks;
	Card ** (*deckCreate)(unsigned long * cardsCount);
	Board * (*boardCreate)(Card ** cards, unsigned long cardsCount);
	MovePack *  (*movePackGet)(Board * board);
	bool	(*isWon)(Board * board);
	void (*updateBoard)(Board * board);
	void (*freeGameState)(Board * board);
	void * (*dupGameState)(Board * board);
} Game;

typedef struct
{
	unsigned long winCount;
} Results;

typedef struct
{
	bool json;
	bool printSolution;
	unsigned long maxDepth;
	unsigned long iterationCount;
	unsigned long threadCount;
	unsigned int seed;
	Game * game;
} Config;

/*-----------------------------------------------------------------------------
 * Globals
 *----------------------------------------------------------------------------*/
extern Config gConfig;

/*-----------------------------------------------------------------------------
 * Macros
 *----------------------------------------------------------------------------*/
#define LOADGAMES(__id) \
 	void __id ## _load(void) \
 	{ \
 		unsigned char i; \
 		for(i=0;__id[i].id!=GAMEID_END;i++) \
 		{ \
 			add_game(&__id[i]); \
 		}; \
 		return; \
 	}

/*-----------------------------------------------------------------------------
 * Functions prototypes
 *----------------------------------------------------------------------------*/

// game.c
void games_load(void);
void add_game(Game * game);
Game * find_game(const char * idname);
void print_games_list(void);
Card ** game_deckCreate(unsigned long * cardsCount);

// card.c
Card * card_create(unsigned char rank, SUIT suit, bool faceUp);
Card * card_createFromId(char * id);
char * card_print(Card * card, bool showDownCard);
Card * card_dup(Card * card);
void card_free(Card * card);
Card ** cards_createDeck(unsigned char deckCount);
Card ** cards_createFromIds(char * idAsString);
void cards_shuffle(Card ** cards, unsigned char cardsCount);
char * cards_print(Card ** cards, unsigned char cardsCount, bool showDownCards);
void cards_printAsDeck(Card ** cards, unsigned char cardsCount);
void cards_reverse(Card ** cards, unsigned char cardsCount);

// spot.c
Spot * spot_create(SPOTTYPE spotType, unsigned char spotIndex);
void spot_cardAdd(Spot * spot, Card * card);
void spot_moveCards(Spot * fromSpot, unsigned char fromCardsIndex, unsigned char cardsToMoveCount, Spot * toSpot, unsigned char toCardsIndex, bool reverseCards);
Spot * spot_dup(Spot * spot);
void spot_print(Spot * spot, bool showDownCards);
const char * spot_name(SPOTTYPE spotType);
void spot_free(Spot * spot);

// board.c
Board * board_create(void);
void board_spotAdd(Board * board, Spot * spot);
Spot * board_spotGet(Board *, SPOTTYPE spotType, unsigned char spotIndex);
void board_cardAdd(Board * board, SPOTTYPE spotType, unsigned char spotIndex, Card * card);
void board_print(Board * board, bool showDownCards);
void board_moveMake(Board * board, Move * move);
void board_moveUndo(Board * board, Move * move);
Board * board_dup(Board * board);
void board_free(Board * board);
void boards_free(Board ** boards, unsigned long boardsCount);
Board ** board_boardAddDups(Board ** boards, unsigned long boardsCount, Board * board, unsigned long boardNumToAdd);
void board_moveLinkMake(Board * board, MoveLink * lastLink);
void board_printKlondikeSolverDeck(Board * board);

// move.c
Move * move_create(MOVETYPE type, SPOTTYPE fromSpotType, unsigned char fromSpotIndex, int fromSpotCardIndex, SPOTTYPE toSpotType, unsigned char toSpotIndex, int toSpotCardIndex);
Move * move_createMultiple(MovePack * movePack);
void move_print(Board * board, Move * move, bool indent);
void move_free(Move * move);
Move * move_dup(Move * move);
void movePack_print(Board * board, MovePack * movePack, bool indent);
void movePack_free(MovePack * movePack, bool freeMoves);
void movePack_add(MovePack * movePack, Move * move);
void movePack_dropLast(MovePack * movePack);
MovePack * movePack_dup(MovePack * movePack);
void movePack_addMany(MovePack * movePack, MovePack * movePackMore);
MovePack * movePack_create(void);
void move_print_json(Move * move);

// results.c
Results * results_create(void);
void results_add(Results * results, Results * totalResults);
void results_free(Results * results);

// rand.c
void randinit(unsigned int seed);
long int randrange(long int min, long int max);
long int randrangeNoSeed(long int min, long int max);

// util.c
double getTicks(void);
char * comma_print(unsigned long n);
unsigned long min(unsigned long a, unsigned long b);
unsigned long max(unsigned long a, unsigned long b);

// string_util.c
char *  strfree(char * str);
unsigned long strchrcount(char * haystack, char needle);
char * strchrrep(char * haystack, char needle, char replacement);
char ** strchrexplode(char * strP, char sep);

// sim.c
void sim(void);

#endif
