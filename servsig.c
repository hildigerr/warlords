/* $Id: servsig.c,v 1.34 2013/12/09 15:38:34 moonsdad Exp $ */

/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      servsig.c    --  Server Signal Handlers                               *
 ******************************************************************************/

#include "server.h"

/******************************************************************************/
/* Function:    timer_handler                                                 */
/* Parameters:  int     signum                                                */
/******************************************************************************/
void timer_handler( int signum )
{
    char buf[MAX_SMESG_LEN+1], * x = &buf[7]; /* buffer for outgoing messages */
    PQUEUE *temp, *r;
    int i, j;

    #ifdef DEBUG_BARF
        BARF( "timer expired", signum );
    #endif

    switch( server_status ) {
        case SHUFFL://TODO: remove DC players before here?
            if( (c_in_tab+p_in_lob) < min_ppplaying ) {/* !Enough Players */
                #ifdef DEBUG_BARF
                    BARF( "!enough players", c_in_tab+p_in_lob );
                #endif
                if( p_in_tab ) { /* Re-Enqueue Table Players Into Lobby */
                    c_in_tab = p_in_tab = 0; /* Stand Up */
                    while( (temp = warlord.next) ) { //TODO: just append remainder of warlord list to lobby and reset scumbag
                        /* Remove DC Players */
                        if( warlord.who->status != DIS_CONN ) {
                            ++p_in_lob;
                            warlord.who->status = IN_LOBBY;
                            lobby_last->who = warlord.who;
                            lobby_last = lobby_last->next = CALLOC( 1, PQUEUE ); /* WARNING: assumes calloc is successfull */
                        } else init_player( warlord.who-player );//init_p( warlord.who );
                        warlord.who = warlord.next->who;
                        warlord.next = warlord.next->next;
                        free( temp );
                    } /* End de-queue While */
//TODO: WORKING: append warlord to lobby and hold dc players in another ll
//                     lobby_last->who = warlord.who;
//                     lobby_last->next = warlord.next;
//                     lobby_last = scumbag;
//                     warlord.who = NULL; warlord.next = NULL;
                    scumbag = &warlord;
//                     //TODO: remove dc players from appended section, set status
                    send_slobb_mesg();
                }/* End Moved Players to Lobby If */
                server_status = ENQING;
                hand_status = HAND_1; /* Reset Hand For Next Players */
                return;
            } /* End !Enough Players Remain to Play If */

            if( hand_status == HAND_1 ) need_club3 = true;
            else {
                server_status = SWAPIN;
                #ifdef DEBUG_BARF
                    BARF( "PLAYING ANOTHER HAND", hand_status );
                #endif
            }/* End !HAND_1 Else */

            /* Move to Table */
            p_in_tab = 0; /* Everyone Stand Up */
            while( (temp = warlord.next) ) {
                /* Remove DC Players */
                if( warlord.who->status != DIS_CONN ) {
                    table[p_in_tab] = warlord.who;
                    ++table[p_in_tab++]->hands; /* SGUI  - Track Qt Played Hands */
                } else init_player( warlord.who-player );//init_p( warlord.who );
                warlord.who = warlord.next->who;
                warlord.next = warlord.next->next;
                free( temp );
            } /* End de-queue While */
            scumbag = &warlord;

            /* Move in From Lobby if Room */
            if(( p_in_tab < MAX_PPPLAYING )&&( p_in_lob )) {
                sprintf( buf, "[slobb|" ); /* Correct qt Filled in Later */

                /* Sit at Table */
                while( p_in_tab < MAX_PPPLAYING ) {
                    if( !lobby_head.who ) break;
                    table[p_in_tab++] = lobby_head.who;
                    lobby_head.who = lobby_head.next->who;
                    temp = lobby_head.next;
                    lobby_head.next = lobby_head.next->next;
                    free( temp ); --p_in_lob;
                }/* End Room@Table While */

                *x++ = '0'+ (int)( p_in_lob / 10 );
                *x++ = '0'+ ( p_in_lob % 10 );
                *x++ = '|';

                lobby_last = r = &lobby_head;
                while( r->who ) {
                    sprintf( x, "%s,", r->who->name );
                    x += 9;
                    lobby_last = r;
                    r = r->next;
                }/* End Current Player Exists Else */
                *(x-1) = ']'; *x = 0;
                broadcast( buf, 10+(9*p_in_lob) );
            }/* End New Players Join If */
            c_in_tab = p_in_tab;
            for( i = c_in_tab; i < MAX_PPPLAYING; i++ ) table[i] = NULL;

            /* Deal */
            assert( p_in_tab >= DEF_MIN_PLAYERS );
//             deal( time( NULL ) );
            if( deal( time( NULL ) ) ) { /* Returns True if Failed */
                /* If Got Here then !Enough Players Anymore */
                for( i = 0; i < p_in_tab; i++ ) {
                    lobby_last->who = table[i];
                    lobby_last = lobby_last->next = CALLOC( 1, PQUEUE ); /* WARNING: assumes calloc is successfull */
                    ++p_in_lob;
                } /* End Re-Enqueue For */
                send_slobb_mesg();
                server_status = ENQING;
                return;
            }/* End Failed Deal If */

            sprintf( buf, "[shand|" );
            for( i = 0; i < c_in_tab; i++ ) {
                x = &buf[7];
                for( j = 0; j < MAX_PHAND_LEN; j++ ) {
                    *x++ = '0'+ (int)( table[i]->hand[j] / 10 );
                    *x++ = '0'+ ( table[i]->hand[j] % 10 );
                    *x++ = ',';
                }/* End card in hand For */
                 *(x-1) = ']'; *x = 0;
                if( Write( table[i]->socketfd, buf, 61 ) < 0 )
                    ERROR( "write", strerror(errno), i );
            }/* End table[i] For */

            if( server_status != SWAPIN ) {
                send_tabl_mesg();
                server_status = ACTIVE;
            } else { /* Request Swap */
                char* cd = &(table[p_in_tab-1]->hand[MAX_PHAND_LEN-1]);
                while( *cd == 52 ) --cd; /* Find Highest Card */
                swapped_cd = *cd;
                assert( (unsigned)swapped_cd < 52 );/* ERROR CHECK */
                if( use_gui ) /* Decrement Count -- SGUI */
                    --deck[swapped_cd]->count[(swapped_cd-(swapped_cd%4))/4];
                deck[swapped_cd] = table[0]; /* Give Warlord Swappeed Card */
                if( use_gui ) /* Increment Count -- SGUI */
                    ++deck[swapped_cd]->count[(swapped_cd-(swapped_cd%4))/4];
                /* Update Scores For SGUI */
                ++table[0]->score; --table[p_in_tab-1]->score;
                /* Notify Clients of Change */
                ( swapped_cd < 10 ) ?
                    sprintf( buf, "[swapw|%c%d]", '0', swapped_cd ):
                    sprintf( buf, "[swapw|%d]", swapped_cd );
                if( Write( table[0]->socketfd, buf, 10 ) < 0 )
                    ERROR( "write", strerror(errno), table[0]->socketfd );
            }/* End SWAPIN Else */

            timer.it_value.tv_sec = ertimeout;
            setitimer( ITIMER_REAL, &timer, NULL );
            break;
        case ACTIVE:
            #ifdef DEBUG_BARF
                BARF( "Testing: server_status is ACTIVE.", ACTIVE );
            #endif
            for( i = 0; i < MAX_PPPLAYING; i++ )
                if( player[i].status == ACTIVE_P ) break;
            /* No Break */
        case SWAPIN:
            #ifdef DEBUG_BARF
                if( server_status != ACTIVE )
                    BARF( "Testing: server_status is SWAPIN", SWAPIN );
            #endif
            if( server_status != ACTIVE ) i = table[0] - &player[0];
            if( strike( i, PLAY_TIMEOUT ) )
                remove_player( i, buf, gfds );
            else if( server_status == ACTIVE ) send_tabl_mesg();
            timer.it_value.tv_sec = ertimeout;
            setitimer( ITIMER_REAL, &timer, NULL );
            break;
        default: /* ENQING */
            #ifdef DEBUG_BARF
                ERROR( "timer_h", "Timer went off while ENQING", signum );
            #endif
            break;
    }/* End server_status Switch */
    return;
}/* End timer_handler Func */


/******************************************************************************/
/* Function:    sigpipe_handler                                               */
/* Parameters:  int     signum                                                */
/******************************************************************************/
void sigpipe_handler( int signum )
{
    #ifdef DEBUG_BARF
        BARF( "a client died :^(", signum );
    #endif
    return;
}/* End sigpipe_handler Func */


/******************************************************************************/
/* FUNCTION: sig_quit                                                         */
/* Parameters:  int     sign                                                  */
/* Catch SIGINT and cause to exit. So curses can endwin before dieing.        */
/******************************************************************************/
void sig_quit( int sig )
{
    signal( SIGINT, SIG_IGN );
    exit(0);
}/* End sig_quit Func */


/************************************EOF***************************************/
