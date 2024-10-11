/* $Id: cardplay.c,v 1.40 2013/12/09 15:38:34 hildigerr Exp $ */

#include "sgui.h" /* #includes "server.h" */

/******************************************************************************/
/* Function:        player_ownes                                              */
/* Parameters:      unsigned    me      Index of player claiming ownership    */
/*                  int[]       cards   Array of cards being verified         */
/* Returns:         bool                True if player owns all cards and all */
/*                                      cards are in valid range of existence */
/* This is the first thing to check, since other validations expect this to   */
/* have passed.                                                               */
/******************************************************************************/
bool player_ownes( unsigned me, int cards[] )
{
    int i;

    assert( me < MAX_PLAYER_QT );

    for( i = 0; i < 4; i++ ) {
        if( cards[i] == 52 ) continue;
        if(( (unsigned)cards[i] > 52 )||( &player[me] != deck[cards[i]] ))
            return false;
    }/* End cards For */

    return true;

}/* End player_ownes Func */


/******************************************************************************/
/* Function:        valid_play                                                */
/* Parameters:      int[]       card    Array of cards to be validated        */
/* Returns:         int                 0 if cards are all in same bin-range  */
/*                                      and not dups. Else a strike code.     */
/* WARNING: Assumes player_ownes was true.                                    */
/*          Assumes first card is !52  -- This is enforced by checking only   */
/* the first card for passing. (Thus Playing 52 as first card forces a pass)  */
/* IF this is to be altered, then sort card array in player_ownes to fix.     */
/******************************************************************************/
#define PLAY_IS_VALID 0
int valid_play( int card[] )
{
    int i, k[2] = { -3, 3 }; /* Delimits The Value Range of the First Card    *
                              * k[1] is upper bound, k[0] is lower.           */

    /* Find Bin Range */
    while( card[0] > k[1] ) k[1] += 4;
    k[0] += k[1]; /* == k[1] - 3 */

    /* Validate Cards */
    for( i = 1; i < 4; i++ ) {
        if( card[i] == 52 ) continue;
        else if( card[i] == card[0] ) return PLAY_DUP; /* Covers 1/2 Dups */
        else if(( card[i] > k[1] )||( card[i] < k[0] )) return PLAY_MISMATCH;
    } /* End cards For */

    /* Check for Other Dups */
    if( ( card[1] != 52 )&&((card[1] == card[2] )||( card[1] == card[3] )) )
        return PLAY_DUP;
    if( ( card[2] != 52 )&&(card[2] == card[3]) ) return PLAY_DUP;

    return PLAY_IS_VALID;

}/* End valid_play Func */


/******************************************************************************/
/* Function:        play_beats_table                                          */
/* Parameters:      int[]       cards       Array of cards being played       */
/* Returns:         int                    */
/* WARNING: Assumes valid_play was true.                                      */
/*          Assums table_top[0] is not 52 if cards have been played.          */
/******************************************************************************/
#define PLAYED 0
#define ONTABL 1
int play_beats_table( int card[] )
{
    int i,                  /* iteration counter */
        k = 3,              /* played card bin range */
        t = 3,              /* tabled card bin range */
        qt[2] = { 0, 0 };   /* qt of cards played and on table */


    if( table_top[0] == 52 ) return PLAY_BEATS_TABLE;
    /* Two on top assumed to have reset tabletop but still want to show card */
    if( table_top[0] > ACE_OF_SPADES ) return PLAY_BEATS_TABLE;

    /* Find Bin Ranges */
    while( card[0] > k ) k += 4;
    k -= 3;
    while( table_top[0] > t ) t += 4;
    t -= 3;

    /* Single 2 Beats Anything */
    if( k > ACE_OF_SPADES ) return PLAYED_TRUMP;

    /* Count Cards */
    for( i = 0; i < 4; i++ ) {
        if( table_top[i] != 52 ) ++qt[ONTABL];
        if( card[i] != 52 ) ++qt[PLAYED];
    } /* End Count Cards For */

    if( k == t ) {
        if( qt[PLAYED] == qt[ONTABL] ) return PLAYED_MATCH;
        if( qt[PLAYED] > qt[ONTABL] ) return PLAY_BEATS_TABLE;
        return PLAY_TOO_FEW;
    } else if( k > t ) {
        if( qt[PLAYED] >= qt[ONTABL] ) return PLAY_BEATS_TABLE;
        return PLAY_TOO_FEW;
    } else return PLAY_TOO_LOW;
}/* End play_beats_table Func */


/******************************************************************************/
/* Function:        played_club3                                              */
/* Parameters:      int[]       card       Cards being checked for 3 of clubs */
/* Returns:         bool                                                      */
/* NOTE: the 3 of clubs is equal to 0                                         */
/******************************************************************************/
bool played_club3( int card[] )
{
    int i;
    for( i = 0; i < 4; i++ ) if( !card[i] ) return true;
    return false;
}/* End played_club3 Func */


/******************************************************************************/
/* Function:        deal                                                      */
/* Parameters:      unsigned    seed        For RNG                           */
/* Returns:         bool        failed      false if !failed -- debug 9 & 16  */
/* The deck is an array of pointers to players, which point to the owning     */
/*  player for each card. We deal out most of the cards randomly for each     */
/*  player. Then beginning with the first player, we hand out the remainder.  */
/*  The remainder is handed out in order, but beginning at random location in */
/*  the deck. Finally, the cards are put into player's hand in order, so the  */
/*  hand data in the player's structure is in ascending order to begin.       */
/* The return value was used for tracing down bug 16 which turned out to be a */
/*  duplicate of bug 9. See bug.db                                            */
/******************************************************************************/
bool deal( unsigned seed )
{
    int i, j, k, handsize = 0;

    srand( seed );

    for( i = 0; i < 52; i++ ) deck[i] = NULL; /* TODO: memset instead? */

    assert( p_in_tab <= MAX_PPPLAYING );
    assert( p_in_tab >= DEF_MIN_PLAYERS );

    if( p_in_tab ) handsize = (int)(52 / p_in_tab);
    else ERROR( "deal", "Nobody at table.", p_in_tab );

    /* Initialize Some Player Data *//* Deal Most Cards */
    for( i = 0; i < p_in_tab; i++ ) {
        table[i]->status = WAITINGP;
        /* ASSERT Empty Hand */
        memset( table[i]->hand, 52, MAX_PHAND_LEN );
        table[i]->cardqt = 0;
        if( use_gui ) for( j = 0; j < 13; j++ ) /* SGUI -- Reset Count */
            table[i]->count[j] = 0;
        for( j = 0; j < handsize; j++ ){
            do{ k = rand() % 52; }while( deck[k] );
            deck[k] = table[i];
        }/* End handsize For */
    }/* End p_in_tab For */

    i = 0;
    k = rand() % 52;
    for( j = 52 % p_in_tab; j > 0; j-- ) {
        do{ handsize = ++k%52; } while( deck[handsize] ); /* REUSING handsize */
        deck[handsize] = table[i];
        ++i;
    }/* End Remaining Cards For */

    /* Set Player's Hand Data in Order */
    for( i = 0; i < 52; i++ ) {
        /* HACK: If player got put into table twice--which happens when *
         *  a player at the table disconnects. (intermitently) I could  *
         *  not trace down the cause without more time. If we notice    *
         *  this is happening, reset the table and deal again.          *
         *  Ocasionally but not consistantly this may displace the      *
         *  warlord. But, at least there wont be a buffer overflow      */
        /* NOTE: This has been fixed. The problem was that when the     *
         *  player that disconnected had ran out of cards in the        *
         *  previous hand, and so had a WAITNEXT status, they would     *
         *  fall through to the default remove_player switch !          *
         *  This should be fixed, but just in case I am leaving this in *
         *  for now. TODO: verify and cleanup.                          */
        if( deck[i]->cardqt == MAX_PHAND_LEN ) {
            PLAYER* nu_table[MAX_PPPLAYING]; /* Local Table for Reseatting */
            p_in_tab = 0; /* Everyone Stand Up */
            for( j = 0; j < MAX_PPPLAYING; j++ )
                if( table[j] ) table[j]->status = IN_LOBBY; /* TEMP STATUS */
            for( j = 0; j < MAX_PPPLAYING; j++ ) {
                if(( table[j] )&&( table[j]->status == IN_LOBBY )) {
                    table[j]->status = WAITINGP;
                    nu_table[p_in_tab++] = table[j];
                }/* End Player !Seated Yet If */
            }/* End MAX_PPPLAYING For */
            for( j = p_in_tab; j < MAX_PPPLAYING; j++ ) nu_table[j] = NULL;
            memcpy( table, nu_table, MAX_PPPLAYING*sizeof(PLAYER*) );
            c_in_tab = p_in_tab;
            if( p_in_tab >= min_ppplaying )
                return( deal( seed ) );
            else return true;
        } else deck[i]->hand[(int)deck[i]->cardqt++] = i; /* Table Ok */
        if( use_gui ) ++deck[i]->count[(i-(i%4))/4]; /* SGUI -- Count */
    }/* End For */

    /* Set Who Goes First */
    if( hand_status == HAND_1 ) deck[0]->status = ACTIVE_P;
    else table[0]->status = ACTIVE_P;
    return false;
}/* End deal Func */


/******************************************************************************/
/* Function:        set_next_turn                                             */
/* Parameters:      unsigned    me*/
/*                  int         cards[]*/
/*                  char        nu_status*/
/* WARNING: Potential for inf loop if not handled correctly.                  */
/******************************************************************************/
void set_next_turn( unsigned me , int cards[], char nu_status )
{
    static int zap = 0;         /* BARF - Current Hand */
    int i,
        activated = -1;         /* BUG 16 & 9 */
    bool all_passed = true,     /* Assume All Players Have Passed */
         skip_nextp = false;    /* Assume Next Player Not Forced to Pass */

    assert( me < MAX_PLAYER_QT );

    if( nu_status == SKIPNEXT ) { skip_nextp = true; nu_status = WAITINGP; }

    /* If active set passed players to active and not activate next player */
    if( (player[me].status = nu_status) == ACTIVE_P ) {
        for( i = 0; i < p_in_tab; i++ )
            if( table[i]->status == PASSED_P ) table[i]->status = WAITINGP;
    } else { /* Trump !Played */
        /* Find Current Player */
        for( i = 0; i < p_in_tab; i++ )
            if( table[i] == &player[me] ) break;

        /* Find Next Player */
        if( ++i == p_in_tab ) i = 0;
        while( table[i]->status != WAITINGP ) // WARNING: Potential for inf loop if
            if( ++i == p_in_tab ) i = 0;      //          not handled correctly.

        if( skip_nextp ) {
            table[i]->status = PASSED_P;
            /* Find Next Player */
            if( ++i == p_in_tab ) i = 0;
            while( table[i]->status != WAITINGP ) // WARNING: Potential for inf loop if
                if( ++i == p_in_tab ) i = 0;      //          not handled correctly.
        }/* End Skip If */

        table[i]->status = ACTIVE_P;
        activated = i;

    }/* End Activate Next Player Else */

    /* Is There Anyone Who Hasn't Passed? */
    for( i = 0; i < p_in_tab; i++ ) {
        if( table[i]->status == WAITINGP ) {
            if( table[i]->cardqt < 1 ) {
                table[i]->status = WAITNEXT;
                continue;
            }/* End Rediscover Discard Rval If */
            all_passed = false;
            break;
        }/* End WAITINGP If */
    }/* End p_in_tab For */

    if( all_passed ) {
            table_top[0] =
            table_top[1] =
            table_top[2] =
            table_top[3] = 52;

        if( c_in_tab < 2 ) { /* 1 or 0 Players Remain */
            #ifdef DEBUG_BARF
                BARF( "Not enough players.", c_in_tab );
            #endif
            for( i = 0; i < p_in_tab; i++ ) {
                if( table[i]->status != DIS_CONN ) { /* DC Players already in scumbag list */
                    if( table[i]->status == WAITNEXT ) continue; /* Warlords already in list */
                    #ifdef DEBUG_BARF
                        BARF( "Moving player to scumbag list.", i );
                    #endif
                    if( table[i]->cardqt > 0 ) { /* BUG 16 & 9 FIX */
                        scumbag->who = table[i];
                        scumbag = scumbag->next = CALLOC( 1, PQUEUE );
                        /* WARNING: assumes calloc is successfull */
                    }/* End HACK If *//* Warlords already in list */
                }/* End !DIS_CONN If */
            }/* End Clear Table For */
            server_status = SHUFFL;
            hand_status = HAND_1;
            timer_handler( 1 );
        } else { /* Enough Players to Continue */
            /* Re-Set player Status */
            for( i = 0; i < p_in_tab; i++ )
                if( table[i]->status == PASSED_P )
                    table[i]->status = WAITINGP;

            /* Verify !End of Hand */
            for( i = 0; i < p_in_tab; i++ )
                if( table[i]->status == WAITINGP ) return;
            #ifdef DEUBUG_BARF
                BARF(" End of Hand", ++zap );
            #else
                ++zap;
            #endif
            if( use_gui ) { /* Display Increased Total Hand Count */
                assert(COLS-SGUI_HEADING_LEN > 0);
                char quant[COLS-SGUI_HEADING_LEN];
                snprintf( quant, COLS-SGUI_HEADING_LEN, "%d", zap );
                servergui( SGUI_ZAPH, quant ); /* SGUI Zap Hand */
            }/* End Use Gui If */
            for( i = 0; i < p_in_tab; i++ ) {
                if( table[i]->status == ACTIVE_P ) { /* Other Players already in scumbag list */
                    #ifdef DEBUG_BARF
                        BARF( "Puting scumbag into scumbag list.", i );
                    #endif
                    table[i]->status = WAITINGP;///TODO: Why did this circumvent bug09 at first? Is it needed///
                    if( table[i]->cardqt > 0 ) { /* BUG 16 & 9 // played a skip, ran out of cards, next player was self */
                        scumbag->who = table[i];
                        scumbag = scumbag->next = CALLOC( 1, PQUEUE ); /* WARNING: assumes calloc is successfull */
                    } break;
                }/* End ACTIVE_P If */
            }/* End Clear Table For */
            server_status = SHUFFL; hand_status = NHAND1;
            timer.it_value.tv_sec = lbtimeout;
            setitimer( ITIMER_REAL, &timer, NULL );
        }/* End Enough Players Else */
    } else { /* Still Players Waiting */
        if(( activated >= 0 )&&( table[activated]->cardqt <= 0 ))/* BUG 16 & 9 */
            set_next_turn( table[activated] - &player[0] , NULL, WAITNEXT );
        if( !cards ) return;
        table_top[0] = cards[0];
        table_top[1] = cards[1];
        table_top[2] = cards[2];
        table_top[3] = cards[3];
    }/* End Set Table Else */

}/* End set_next_turn Func */


/******************************************************************************/
/* Function:        discard                                                   */
/* Parameters:      unsigned    me,         The player who has cards in hand  */
/*                  int         cards[]     The cards to be removed from hand */
/* Returns:         char                    WAITNEXT || WAITINGP              */
/* Removes cards from hand & hand from cards.                                 */
/******************************************************************************/
char discard( unsigned me, int cards[] )
{
    int i,j;

    assert( me < MAX_PLAYER_QT );

    for( i = 0; i < 4; i++ ) {
        if( cards[i] == 52 ) continue;
        j = 0;
        while( player[me].hand[j] != cards[i] ) ++j;
        player[me].hand[j] = 52;
        if( use_gui )/* Decrement Quantity -- SGUI */
            --deck[cards[i]]->count[(cards[i]-(cards[i]%4))/4];
            /* cards[i] is the numeric value for the current card being discarded
             * count[] is the array holding the quantity of each rank of card
             * owned by the player. Rank is from card 3 to 10, J, Q, K, A, 2.
             *  corrosponding indexes are [0,13).
             * The (cards[i]-(cards[i]%4))/4 gives the index to count[] for any
             * real card.
             * The deck[] is an array the size of the deck of pointers to players.
             */
        deck[cards[i]] = NULL;
        --player[me].cardqt;
    }/* End cards For */

    if( player[me].cardqt > 0 ) {
        if( cards[0] > ACE_OF_SPADES ) return ACTIVE_P;
        else return WAITINGP;
    }/* End Still Have Cards If */

    /* Out of Cards */
    #ifdef DEBUG_BARF
        BARF( "Player out of cards.", me );
    #endif
    scumbag->who = &player[me];
    scumbag = scumbag->next = CALLOC( 1, PQUEUE ); /* WARNING: assumes calloc is successfull */
    return WAITNEXT;
}/* End discard Func */


/************************************EOF***************************************/
