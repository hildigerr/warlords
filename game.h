/* $Id: game.h,v 1.11 2013/12/06 18:58:49 moonsdad Exp $ */

/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      game.h  --  Base header used by both client and server                *
 ******************************************************************************/

#pragma once

#include "rmhv_stdlib.h" /* #includes{ <stdio.h>, <stdlib.h>, <string.h>,    *
                          *            <stdbool.h>, <unistd.h>, <errno.h>,   *
                          *            <fcntl.h>                           } */
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

/* Precompiler Constants */
#define PROTOPORT       36737 /* default protocol port number */
#define MAX_PLAYER_QT   35
#define MAX_PNAME_LEN   8
#define MAX_PPPLAYING   7
#define MAX_PHAND_LEN   18
#define MAX_CHAT_MESG   63
#define MAX_CMESG_LEN   MAX_CHAT_MESG+8
#define MAX_SMESG_LEN   325+1

/* Strike Error Codes */
#define PLAY_MISMATCH    11
#define PLAY_TOO_LOW     12
#define PLAY_TOO_FEW     13
#define PLAY_UNOWNED     14
#define PLAY_OUT_OF_TURN 15
#define IPLAY_NO_3CLUB   16
#define PLAY_DUP         17
#define PLAY_TIMEOUT     20
#define LOBBY_PLAY       31
#define MESG_TOO_LONG    32
#define UNKOWN_MESG_TYPE 33
#define MALFORMED_MESG   34
#define CHAT_BAN         60
#define SWAP_UNOWNED     70
#define SWAP_BAD_WHO     71
#define SWAP_BAD_TIME    72
#define SERVER_FULL      81

/* The Deck of Cards */
#define CLUBS    0
#define DIAMONDS 1
#define HEARTS   2
#define SPADES   3
#define ACE_OF_SPADES 47

/* Structures and Typedefs */
typedef struct {
    char buf[MAX_SMESG_LEN+1];  /* Largest Expected Buffer Sized Buffer */
    int n;                      /* Number of Characters Read/Filled */
} BUFFER;

/* Macro Functions */
#define c2d(x) (x - '0')

/* Function Prototypes */
int buf_copy2( BUFFER* dest, BUFFER* source, unsigned pos, char term, bool ignore );

/************************************EOF***************************************/
