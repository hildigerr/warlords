/* $Id: warlord_ai.h,v 1.1 2013/12/01 20:51:31 moonsdad Exp $*/

/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      warlord_ai.h    --  Header file for AI stuff.                         *
 ******************************************************************************/

#pragma once

#include "client.h"

/* Enums, Typedefs, and Structs */
enum { SINGLES, PAIRS, TRIPPLES, QUADS, GROUP_QT };

enum { THIS, WHOLE };

typedef struct {
    int cardi;
    int size[2]; //THIS,WHOLE  (groups)
//    int value;
} POSIBLE_PLAY;

/* Function Prototypes */
void ai_swap( IODAT* data );
void ai_play( IODAT* data );
