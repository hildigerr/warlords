/* $Id: warlord_ai.c,v 1.5 2013/12/03 05:47:23 moonsdad Exp $*/

/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      warlord_ai.c    --  AI stuff.                                         *
 ******************************************************************************/
#include "warlord_ai.h"


/******************************************************************************/
/*  Function:   ai_swap                                                       */
/*  Parameter:  IODAT*      data                                              */
/*  Selects card to send to the scumbag, and does so.                         */
/* TODO: Find first single-meld card, etc. Make More Interesting.             */
/******************************************************************************/
void ai_swap( IODAT* data )
{
//     int i,j;
    char playmsg[12];

    assert( iam_warlord );


//     for( i = 0, j = 1; i < data->shand.n; ) {
//         int cd = (c2d( data->cards[i][0] ) * 10) + c2d( data->cards[i][1] );
//         while( cd % 4 ) --cd; /* Min Bin Range */
//
//     } /* End Cards in Hand For */

    sprintf( playmsg, "[cswap|%s]", data->cards[0] );

    if( Write( gsd, playmsg, 10 ) < 0 )
        ERROR( "write", strerror(errno), gsd );

    iam_warlord = false;
    used_hand = true;

    if( Write( gsd, "[chand]", 7) < 0 )
        ERROR( "write", strerror(errno), gsd );

}/* End ai_swap Func */


/******************************************************************************/
/*  Function:   ai_play                                                       */
/*  Parameter:  IODAT*      data                                              */
/*  Finds a play and sends it.                                                */
/* TODO: Make More Interesting.                                               */
/******************************************************************************/
void ai_play( IODAT* data )
{
    char playmsg[20] = "[cplay|52,52,52,52]", *slice = &playmsg[4];
    bool found = false;
    POSIBLE_PLAY move;
    int needed_size = 4, needed_rank = -1;
    int meld_size = 1, meld_rank, i,j,k;
    int table_top[4] = {    (c2d( data->stabl.buf[112] ) * 10) + c2d( data->stabl.buf[113] ),
                            (c2d( data->stabl.buf[115] ) * 10) + c2d( data->stabl.buf[116] ),
                            (c2d( data->stabl.buf[118] ) * 10) + c2d( data->stabl.buf[119] ),
                            (c2d( data->stabl.buf[121] ) * 10) + c2d( data->stabl.buf[122] ) };

    assert( is_myturn );

    /* Find Needed Size and Rank */
    for( i = 0; i < 4; i++ ) {
        if( table_top[i] <= ACE_OF_SPADES )
            needed_rank = table_top[i];
        else --needed_size;
    }/* End Table Top Cards For */

    /* Get Min Bin Range */
    if( needed_rank > 0 )
        while( needed_rank % 4 )
            --needed_rank;

    /* Serach for a valid Play */
    for( j = i = 0; i < data->shand.n; i = j ) { ++j;

        /* Skip Cards Too Low */
        meld_rank = (c2d( data->cards[i][0] ) * 10) + c2d( data->cards[i][1] );
        if( meld_rank < needed_rank ) continue;

        if( meld_rank > ACE_OF_SPADES ) { /* IS A Trump */
            move.cardi = i;
            move.size[THIS] = move.size[WHOLE] = 1;
//             move.value = 1 - i;
            found = true;
            break;
        }/* End Trump If */

        /* Skip Melds Too Small */
        while( meld_rank % 4 ) --meld_rank; /* Min Bin Range */
        meld_size = 1;
        for( j = i+1; j < data->shand.n; j++ ) {
            k = (c2d( data->cards[j][0] ) * 10) + c2d( data->cards[j][1] );
            if(( k < meld_rank )||( k > meld_rank+3 )) break;
            else ++meld_size;
        }/* End j For */
        if( meld_size < needed_size ) continue;

        move.cardi = i;
        move.size[THIS] = move.size[WHOLE] = meld_size;
        found = true;
        break;
    }/* End Cards in Hand For*/

    if( found ) {
        assert( (unsigned)move.size[THIS] <= 4 );
        #ifdef DEBUG_BARF
            BARF( "Found a play:", 0 );
        #endif
        while( move.size[THIS] ) {
            slice += 3;
            slice[0] =  data->cards[move.cardi][0];
            slice[1] =  data->cards[move.cardi][1];
            ++move.cardi;
            --move.size[THIS];
        }/* End While */
    }
    #ifdef DEBUG_BARF
        BARF( playmsg, 0 );
    #endif
    if( Write( gsd, playmsg, 19 ) < 0 )
        ERROR( "write", strerror(errno), gsd );

    is_myturn = false;
    used_hand = true;

    if( Write( gsd, "[chand]", 7 ) < 0 )
        ERROR( "write", strerror(errno), gsd );


}/* */
// {
//     char playmsg[20] = "[cplay|52,52,52,52]", *slice = &playmsg[4];
//     bool found = false;
//     POSIBLE_PLAY move;
//     int needed_size = 4, needed_rank = -1;
//     int meld_size = 1, meld_rank, i,j,k;
// //     int groupCount[4] = { 0,0,0,0 };
//     int table_top[4] = {    atoi(data->stabl.buf+112),
//                             atoi(data->stabl.buf+115),
//                             atoi(data->stabl.buf+118),
//                             atoi(data->stabl.buf+121)   };
//
//     /* Find Needed Size and Rank */
//     for( i = 0; i < 4; i++ ) {
//         if( table_top[i] < ACE_OF_SPADES ) {
//             while( table_top[i] % 4 ) --table_top[i]; /* Min Bin Range */
//             needed_rank = table_top[i];
//         } else --needed_size;
//     }/* End Table Top Cards For */
//
//     for( i = 0; i < data->shand.n; ) {
//
//         /* Skip Cards Too Low */
//         meld_rank = atoi(data->cards[i]);
//         if( meld_rank < needed_rank ) continue;
//
//         if( meld_rank > ACE_OF_SPADES ) { /* IS A Trump */
//             move.cardi = i;
//             move.size[THIS] = move.size[WHOLE] = 1;
// //             move.value = 1 - i;
//             found = true;
//             break;
//         }/* End Trump If */
//
//         /* Skip Melds Too Small */
//         meld_size = 1;
//         for( j = i+1; j < data->shand.n; j++ ) {
//             k = atoi(data->cards[j]);
//             if(( k < meld_rank )||( k > meld_rank+3 )) break;
//             else ++meld_size;
//         }/* End j For */
//         if( meld_size < needed_size ) continue;
//
//         move.cardi = i;
// //         move.value =
//         move.size[THIS] = move.size[WHOLE] = meld_size;
// //         ++groupCount[meld_size-1];
// //         move.value -= (meld_rank/4)+1;
//         found = true;
//         break;
// // //         /* Enumerate Remaining Possible Plays */
// // //         while( meld_size > needed_size ) {
// // //             moves[++x].cardi = i;
// // //             moves[x].size[WHOLE] = moves[x-1].size[WHOLE];
// // //             moves[x].value = moves[x].size[THIS] = --meld_size;
// // //             moves[x].value -= moves[x].size[WHOLE];
// // //         }/* End Split Melds While */
// //         i += moves[x++].size[WHOLE];
//     }/* End Cards in Hand For*/
//
//     if( found ) {
// //         for( i = 0; i < x; i++ )
// //             if( moves[i].size[THIS] == moves[i].size[WHOLE] )
// //                 moves[i].value += groupCount[moves[i].size[THIS]-1];
// //
// //         /* TODO: adjust value for skipping a player */
// //
// //         for( i = 1, j = 0; i < x; i++ )
// //             if( moves[i].value > moves[j].value ) j = i;
//
//         while( move.size[THIS] ) {
//             slice += 3;
//             slice[0] =  data->cards[move.cardi][0];
//             slice[1] =  data->cards[move.cardi][1];
//             ++move.cardi;
//             --move.size[THIS];
//         }/* End While */
//     }
//     SAY( playmsg, 19 );//BARF-TEST
// }/* */
