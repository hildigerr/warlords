/* $Id: clibuf.c,v 1.22 2013/12/05 04:01:58 moonsdad Exp $*/

/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      clibuf.c    --  BUFFER management and manipulation Functions          *
 ******************************************************************************/

#include "client.h"


/******************************************************************************/
/* Function:    PAD                         XXX                               */
/* Parameters:  char    c       -- The character used for padding.            */
/*              int     qt      -- The size of array for a new padd string.   */
/* Returns:     char*           -- an allocated padd string.                  */
/* WARNING: if successfull must free.                                         */
/******************************************************************************/
static char* PAD( char c, int qt )
{
    char* robj = NULL;

    if( qt > 0 ) {
        robj = CALLOC( qt+1, char );
        if( robj ) {
            memset( robj, c, qt );
            robj[qt] = '\0';
        }/* End If */
    } return robj;
}/* End PADD Func */


/******************************************************************************/
/* Function:    parse_chat                                                    */
/* Parameters:  gpointer        b       The attached button                   */
/*              GtkTextView*    view    The chat message bar.                 */
/* Prepares the transmit buffer with a message to send to the server.         */
/*  Player may either format messages directly using correct protocol, or     */
/* else it will be sent as a chat message.                                    */
/******************************************************************************/
void parse_chat( gpointer b, GtkTextView* view )
{
    GtkTextBuffer * buffer = gtk_text_view_get_buffer( view );
    gchar * text;
    GtkTextIter start, end;

    char* space_pad = NULL;
    transmit.n = 0; /* Just In Case */

    gtk_text_buffer_get_bounds( buffer, &start, &end );
    if( (text = gtk_text_buffer_get_text( buffer, &start, &end, FALSE )) ) {
        size_t len = strlen( text );

        if( text[0] == '[' ) { /* Allow Player Direct Communication to Server */
            if( len > MAX_CMESG_LEN ) {
                /* HACK: Send Partial Mesg */
                text[MAX_CMESG_LEN-1] = ']';
                text[MAX_CMESG_LEN] = 0; /* Null-Terminate Though Unnecessary */
                transmit.n = MAX_CMESG_LEN;
            } else transmit.n = len;
            sprintf( transmit.buf, "%s", text );
        } else { /* Player is Chatting */
            if( len > MAX_CHAT_MESG ) {
                /* HACK: Send Partial Mesg */
                text[MAX_CHAT_MESG] = 0;
                transmit.n = MAX_CHAT_MESG+8;
            } else transmit.n = len+8;
            space_pad = PAD(' ', MAX_CHAT_MESG+8 - transmit.n );
            if( space_pad ) {
                transmit.n = MAX_CHAT_MESG+8;
                sprintf( transmit.buf, "[cchat|%s%s]", text, space_pad );
                free( space_pad );
            } else sprintf( transmit.buf, "[cchat|%s]", text );
        }/* End Else */
        g_free( text );
        gtk_text_buffer_delete( buffer, &start, &end );
    }/* End get text If */
// //     BARF( transmit.buf, transmit.n );
}/* End Func */


/******************************************************************************/
/* Function:    rsort_cards                                                   */
/* Parameter:   IODAT*      data                                              */
/* Returns:     int             the Qt of non-empty cards in hand.            */
/* Performs radix-sort using two passes of counting sort on data->cards.      */
/*  data->cards is an array of pointers into data->shand.buf. The pointers    */
/*  are aranged in ascending order based on the two-ascii-digit number they   */
/*  point at.                                                                 */
/******************************************************************************/
static int rsort_cards( IODAT* data )
{
    char* B[MAX_PHAND_LEN];
    int i, C[10] = {0,0,0,0,0,0,0,0,0,0};

    /* Counting Sort -- Pass 1 */
    for( i = 0; i < MAX_PHAND_LEN; i++ )
        C[ data->cards[i][1] - '0' ] += 1;
    for( i = 1; i < 10; i++ )
        C[i] += C[i-1];
    for( i = MAX_PHAND_LEN-1; i >= 0; i-- ) {
        int k = data->cards[i][1] - '0';
        --C[k];
        B[C[k]] = data->cards[i];
    }/* End For */

    /* Counting Sort -- Pass 2 */
    for( i = 0; i < 10; i++ )
        C[i] = 0;
    for( i = 0; i < MAX_PHAND_LEN; i++ )
        C[ B[i][0] - '0' ] += 1;
    for( i = 1; i < 10; i++ )//todo 5
        C[i] += C[i-1];
    for( i = MAX_PHAND_LEN-1; i >= 0; i-- ) {
        int k = B[i][0] - '0';
        --C[k];
        data->cards[C[k]] = B[i];
    }/* End For */

    /* Count Empty Cards */
    for( i = MAX_PHAND_LEN-1; i >= 0; i-- )
        if(( *data->cards[i] != '5' )||( *(data->cards[i]+1) != '2' )) break;

    return i+1;

}/* End rsort_cards Func */


/******************************************************************************/
/* Function:    update_hand                                                   */
/* Parameter:   IODAT*      data                                              */
/* Saves the new hand in the shand.buf and qt of !empty cards in shand.n      */
/* TODO: only pack into hand window !empty cards.                             */
/******************************************************************************/
void update_hand( IODAT* data )
{
    char* slicer = &data->shand.buf[7], /* [shand|@X,xx,xx,..] */
          fp[24] = "./dat/img/cards/52.jpeg" ;
    int i;

    memcpy( data->shand.buf, data->output.buf, data->output.n );

    gtk_container_foreach( GTK_CONTAINER (Widget[HAND]), widget_destroy, NULL );

    for( i = 0; i < MAX_PHAND_LEN; i++ ) {
        data->cards[i] = slicer;
        slicer++; slicer++; *slicer++ = '\0';//TODO: make simpler
    }/* End Hand Card For */

    data->shand.n = rsort_cards( data );/* HACK: using n !for buf len */

    for( i = 0; i < MAX_PHAND_LEN; i++ ) {
        fp[16] = data->cards[i][0]; fp[17] = data->cards[i][1];
        gtk_box_pack_start( GTK_BOX (Widget[HAND]),
            create_image( fp, G_CALLBACK(click_hcard), data->cards[i] ), TRUE, TRUE, 0 );
    }/* End Hand Card For */
}/* End update_hand Func */


/******************************************************************************/
/* Function:    update_table                                                  */
/* Parameter:   IODAT*      data                                              */
/* Returns:     bool                TRUE if player is Activated               */
/* Saves the new table information and updates table list and gui area.       */
/******************************************************************************/
bool update_table( IODAT* data )
{
    char* slicer = &data->output.buf[9-3]; /* [stabl|sn:xxxxxxxx:qt,...] */
    int i, qt = 0, count;
    GtkTreeIter iter;

    /* Null-Terminate Select Elements in Original Buffer, Count Players */
    for( i = 0; i < MAX_PPPLAYING; i++ ) {
        *(slicer += 3) = '\0';
        if( *(++slicer) != ' ' ) ++qt;
        *(slicer += 8) = '\0';
        *(slicer += 3) = '\0';
    }/* End Slicer For */
    *(slicer += 3) = '\0';*(slicer += 3) = '\0';
    *(slicer += 3) = '\0';*(slicer += 3) = '\0';

    gtk_list_store_clear( playerlist[TABBLL] );

    slicer = &data->output.buf[7];
    for( count = qt, i = 0; i < qt; i++ ){
        switch( *slicer ) { /* Check Status */
            case 'a': case 'A':
		        if( !strcmp( my_name, slicer+3) ) is_myturn = true;
		        break;
            case 'w': case 'W': if( atoi( slicer+12 ) ) break;
            default: --count;
        }/* End Status Switch */
        gtk_list_store_append( playerlist[TABBLL], &iter );
        gtk_list_store_set( playerlist[TABBLL], &iter,
            STATUS_COL, slicer, NAME_COL, slicer+3, CARD_QT_COL, slicer+12, -1);
        slicer += 15;
    }/* End Player Qt For */

    /* HACK: XXX Fix needed to work with another student's silly server XXX */
    if(( data->output.buf[97] == 'a' )&&( !strcmp( my_name, &data->output.buf[100] ) )) is_myturn = true;

    /* Set Up Table GUI Area */
    if( iam_playing ) --qt;
    gtk_container_foreach( GTK_CONTAINER (Widget[TABLE_TOP]), widget_destroy, NULL );
    gtk_box_pack_start( GTK_BOX (Widget[TABLE_TOP]),
            create_image( (--qt < 0)? "./dat/img/empty_seat.png":"./dat/img/1194984596564665015card_figure_jack_nicu_r.svg.thumb.png",
                    NULL, NULL ), TRUE, TRUE, 0 );
    slicer = &data->output.buf[112];
    for( i = 0; i < 4; i++ ) {
        char fp[24];
        sprintf( fp, "./dat/img/cards/%s.jpeg", slicer );
        gtk_box_pack_start( GTK_BOX (Widget[TABLE_TOP]),
            create_image( fp, NULL, NULL ), TRUE, TRUE, 0 );
        slicer += 3;
    }/* End Card For */
    gtk_box_pack_start( GTK_BOX (Widget[TABLE_TOP]),
            create_image( (qt < 5)? "./dat/img/empty_seat.png":"./dat/img/1194984596564665015card_figure_jack_nicu_r.svg.thumb.png",
                    NULL, NULL ), TRUE, TRUE, 0 );

    if( data->stabl.n < 2 ) { /* This Is New Deal */
        gtk_container_foreach( GTK_CONTAINER (Widget[ADD_PLAYERS]), widget_destroy, NULL );
        for( i = 0; i < 4; i++ )
            gtk_box_pack_start( GTK_BOX (Widget[ADD_PLAYERS]),
                create_image( (--qt < 0)? "./dat/img/empty_seat.png":"./dat/img/1194984596564665015card_figure_jack_nicu_r.svg.thumb.png",
                    NULL, NULL ), TRUE, TRUE, 0 );
    }/* End New Deal If */

    /* Store History */
    memcpy( data->stabl.buf, data->output.buf, data->output.n );
    data->stabl.n = count; /* HACK: using n !for buf len */

    return is_myturn;
}/* End update_table Func */


/******************************************************************************/
/* Function:    update_lobby                                                  */
/* Parameter:   IODAT*      data                                              */
/* Updates the lobby listing.                                                 */
/******************************************************************************/
void update_lobby( IODAT* data )//TODO: change cols
{
    char* slicer = &data->output.buf[7]; /* [sloby|@...] */
    int i, qt;
    GtkTreeIter iter;

    assert( data != NULL );

    /* Discover New Player Qt */
    *(slicer+2) = 0; /* [slobb|xx|...] --> [slobb|xx0...] */
    qt = atoi( slicer );

    /* Null-Terminate all Player Names in Original Buffer */
    for( i = 0, slicer += 3; i < qt; slicer++, i++ ) /* Init @ buf[10] */
        *(slicer += MAX_PNAME_LEN) = 0;

    /* Update List */
    if( data->slobb.n ) gtk_list_store_clear( playerlist[LOBBYL] );
    for( i = 0, slicer = &data->output.buf[10]; i < qt; i++ ) {
        gtk_list_store_append( playerlist[LOBBYL], &iter );
        gtk_list_store_set( playerlist[LOBBYL], &iter, 0, slicer, -1);
        slicer += MAX_PNAME_LEN+1;
    }/* End Player Qt For */

    /* Store History */
    memcpy( data->slobb.buf, data->output.buf, data->output.n );
    data->slobb.n = qt; /* HACK: using n !for buf len */
}/* End update_lobby Func */


/************************************EOF***************************************/
//char* slicer = &data->output.buf[7]; /* [sxxxx|@...] */
