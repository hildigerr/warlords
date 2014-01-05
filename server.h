/* $Id: server.h,v 1.34 2013/12/09 15:38:34 moonsdad Exp $ */

/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      server.h    --  Header File for the Server                            *
 ******************************************************************************/

#pragma once

#include "game.h"
#include <sys/time.h>    /* for setitimer */
#include <time.h>

/* SGUI FIX To Use Curses */
#ifdef DEBUG_BARF
    #ifdef Write
        #undef Write
        #define Write write
    #endif
    #undef DEBUG_BARF
#endif

/* Default Constants */
#define DEF_ERTIMEOUT   15  /* seconds */
#define MAX_ERTIMEOUT   180 /* XXX 3 minutes XXX */
#define DEF_LBTIMEOUT   15  /* seconds */
#define MAX_LBTIMEOUT   180 /* XXX 3 minutes XXX */
#define DEF_MIN_PLAYERS 3
#define MAX_STRIKES    '3'
#define LISTENQ         MAX_PLAYER_QT /* TODO: Is this Optimum? */

/* Server and Game Status */
#define ENQING 'Q'
#define ACTIVE 'A'
#define SHUFFL 'S'
#define SWAPIN 'W'
#define HAND_1 '1'
#define NHAND1 '0'

/* Player Status */
#define ACTIVE_P 'a'
#define PASSED_P 'p'
#define WAITINGP 'w'
#define DIS_CONN 'd'
#define IS_EMPTY 'e'
#define IN_LOBBY 'l'
#define UNJOINED 'u'
#define WAITNEXT 'W'
#define PASSCARD 'P'
#define SKIPNEXT 's'

/* Play Beats Table Return Values */
#define PLAY_BEATS_TABLE 0
#define PLAYED_TRUMP 0
#define PLAYED_MATCH 1

/* Structures and Typedefs */
typedef struct {
    BUFFER mesg;
    BUFFER input;
    char hand[MAX_PHAND_LEN];
    char name[MAX_PNAME_LEN+1];
    int  socketfd;
    int  cardqt;
    char status;
    char strikqt;
    /* SGUI Stuff: */
    int count[13];
    unsigned strikelog[3];
    unsigned hands;
    int score;
    bool ban;
} PLAYER;

typedef struct pq {
    PLAYER* who;
    struct pq* next;
} PQUEUE;

/* Global Variables */
EXTERN struct itimerval timer;                      /* Timer using Signals */
EXTERN time_t ertimeout     INIT(DEF_ERTIMEOUT);    /* timeout before strike */
EXTERN time_t lbtimeout     INIT(DEF_LBTIMEOUT);    /* timeout before game begins */
EXTERN char server_status   INIT(ENQING),           /* Server State: 'Q' 'A' 'S' */
            hand_status     INIT(HAND_1),           /* Ranked or NonRanked Hand */
            table_top[4];                           /* The Cards Last Played */
EXTERN PLAYER player[MAX_PLAYER_QT],                /* connected player data */
            * table[MAX_PPPLAYING],                 /* players playing at table */
            * deck[52];
EXTERN PQUEUE lobby_head,                           /* The Front of The Lobby Queue */
            * lobby_last    INIT(&lobby_head),      /* The Rear  of The Lobby Queue */
              warlord,                              /* The Queue of Players, lead by winner */
            * scumbag       INIT(&warlord);         /*     so far and tailed by loosers */
EXTERN unsigned c_in_tab    INIT(0),                /* quantity of clients at table */
                p_in_tab    INIT(0),                /* quantity of players at table */
                p_in_lob    INIT(0);                /* quantity of players in lobby */
EXTERN unsigned min_ppplaying INIT(DEF_MIN_PLAYERS);/* min qt of players for a game */
EXTERN int maxci            INIT(-1);               /* Max Client Index for player array */
EXTERN bool need_club3      INIT(true);             /* 3of clubs must be used first */
EXTERN int swapped_cd;
EXTERN fd_set* gfds         INIT(NULL);
EXTERN bool use_gui         INIT(false);            /* SGUI */

/* Function Prototypes */
/* TODO: Refactor paramaters: Eliminate duplicated effort due to globalization */
inline void init_player( unsigned i );
// void init_p( PLAYER* who ); /* Deprecated */
inline bool mangle( unsigned i );
inline void broadcast( char* buf, size_t count );
bool strike( unsigned i, unsigned error );
void remove_player( unsigned i, char* buf, fd_set* set );
#define Remove_player(i) remove_player( i, buf, &allset )
void send_tabl_mesg( void );
void send_slobb_mesg( void );
/* servsig.c */
void sigpipe_handler( int signum );
void timer_handler( int signum );
/* cardpaly.c */
bool player_ownes( unsigned me, int cards[] );
int valid_play( int cards[] );
int play_beats_table( int cards[] );
bool played_club3( int card[] );
bool deal( unsigned seed );
void set_next_turn( unsigned me , int cards[], char nu_status );
char discard( unsigned me, int cards[] );
#define Discard(x) discard( i, x )


/************************************EOF***************************************/
