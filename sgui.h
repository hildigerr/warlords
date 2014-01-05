/* $Id: sgui.h,v 1.1 2013/12/09 15:38:34 moonsdad Exp $ */

/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      sgui.h    --  Server GUI Header                                       *
 ******************************************************************************/
#pragma once

#include "server.h"
#include <curses.h>

/* t Cases *//* DONT USE: h,H,j,J,p,P,q,Q,s,S */
#define SGUI_INIT 'i'
#define SGUI_TABL 't'
#define SGUI_LOBB 'l'
#define SGUI_CHAT 'c'
#define SGUI_ZAPH 'z'
#define SGUI_SCMD 'x'
#define SGUI_CCMD 'X'

/* WINDOW* Array Index */
#define SGUI_TABLE    0
#define SGUI_LOBBY    1
#define SGUI_CHATW    2
#define SGUI_CMD_W    3
#define SUB_WINDOW_QT 4

#define SGUI_HEADING_LEN 72

#define GenSubwin(a,b) subwin( stdscr, a, COLS, b, 0 )

void sig_quit( int );
void curse_cleanup( void );
bool servergui( char, char* );
bool cmd_exec( BUFFER );


/************************************EOF***************************************/
