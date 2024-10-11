/* $Id: clichan.c,v 1.38 2013/12/10 01:40:16 hildigerr Exp $ */


/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      clichan.c    --  Client GIOChannel Functions                          *
 ******************************************************************************/

#include "warlord_ai.h"

/******************************************************************************/
/* Function:    getmsg                  -- GIOFunc                            */
/* Parameters:  GIOChannel*     source  -- The Source Channel                 */
/*              GIOCondition    condition   -- Which Signal                   */
/*              gpointer        data        IODAT                             */
/* Returns:     gboolean                    false if G_IO_STATUS_EOF          */
/* Reads input, parses into messages, and hands them on. Partial messages are */
/*  saved in an outbut buffer in data for next read.                          */
/******************************************************************************/
#define bufcp(x) buf_copy2( &data->output, &data->input, x, ']', true )
gboolean getmsg( GIOChannel* source, GIOCondition condition, gpointer dat )
{
    gsize r = 0;
    GError* err = NULL; /* Has Members:  GQuark domain; gint code; gchar *message; */
    IODAT* data = (IODAT*)dat;
    int pos = 0;

    assert( source != NULL ); assert( data != NULL );

// //     BARF( "getmsg has been called", 0 );

    switch( g_io_channel_read_chars( source, data->input.buf, MAX_SMESG_LEN, &r, &err ) ) {
        case G_IO_STATUS_NORMAL: /* Success. */
            data->input.n = r;
            while( (pos = bufcp(pos)) <= (data->input.n -1) )
                if( pos++ < 0 ) break;
                else Have_Complete_Mesg( data );
            if(( auto_mode )&&( !used_hand )) {
                if( iam_warlord ) ai_swap( data );
                else if( is_myturn ) ai_play( data );
            } break;
        case G_IO_STATUS_EOF: /* End of file. */
            #ifdef DEBUG_BARF
                BARF( "Recieved EOF from socket", r );
            #endif
            SAY( "Recieved EOF from socket.", 25 );
            g_clear_error( &err );
            if( g_io_channel_shutdown( source, true, &err ) == G_IO_STATUS_ERROR ){ //TODO: Is this correct way to handle this case?
                if( err ) ERROR( "GIOChannel shutdown", err->message, err->code );
                else ERROR("GIOChannel shutdown", "no GError msg", r );
            } return false;
        case G_IO_STATUS_AGAIN: /* Resource temporarily unavailable. */
            BARF( "Recieved G_IO_STATUS_AGAIN from channel read", r );
            break;
        case G_IO_STATUS_ERROR: /* An error occurred. */
            if( err ) ERROR( "GIOChannel read", err->message, err->code );
            else ERROR( "GIOChannel read", "no GError msg", r );
        default:
            ERROR( "GIOChannel read", "default switch occured", r );
    }/* End Read Switch */

    g_clear_error( &err );
// //     BARF("Returning from getmsg func.",0);
    return true;
}/* End getmsg Func */


/******************************************************************************/
/* Function:    adjust_display_buf_view                                       */
/*  Ensures chat window only displays DISPLAY_MAXLINE lines.                  */
/******************************************************************************/
inline void adjust_display_buf_view( void )
{
    static bool broke_max_line = false;

    if(( !broke_max_line )&&( gtk_text_buffer_get_line_count( display_buf ) > DISPLAY_MAXLINE ))
        broke_max_line = true; // Don't Keep Calling gtk for line count

    if( broke_max_line ) {
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter( display_buf, &start );
        gtk_text_buffer_get_iter_at_line( display_buf, &end, 1 );
        gtk_text_buffer_delete( display_buf, &start, &end );
    }/* End View Full If */
}/* End adjust_display_buf_view Func */


/******************************************************************************/
/* Function:    Have_Complete_Mesg                                            */
/* Parameters:  IODAT*      data                                              */
/* Takes the complete message from data->outbut.buf and handles it.           */
/* ASSUMES: the message is complete, and well formed.                         */
/******************************************************************************/
#define WARL_SWAP 1
#define SCUM_SWAP 2
void Have_Complete_Mesg( IODAT* data )
{
    assert( data != NULL );
    #ifdef DEBUG_BARF
        BARF( data->output.buf, 0 );
    #endif
    switch( data->output.buf[5] ) {
    /* HACK: Looking only at expected first unique char */
        case 'b': case 'B': /* [slobb|...] */ update_lobby( data ); break;
        case 'd': case 'D': /* [shand|...] */
            /* WARNING: Requesting a non existent hand will cause client to   *
             * belive it's playing.                                           */
            /* NOTE: a Player should not request hand if not playing.         *
             * but, my server will allow for this by returning an empty hand. */
            /* TODO: Warn or disallow player to request hand if !iam_playing. */
            iam_playing = true;
            used_hand = false;
            update_hand( data );
            break;
        case 'k': case 'K': /* [strik|...] */
            //TODO: Send these also to stderr and/or a log file
            //TODO: update strike quantity in player list
            adjust_display_buf_view();
            if( data->output.buf[7] != 8 ) SAY( "Recieved Strike for ",  20 );
            else SAY( "==SERVER IS FULL==", 18 );
            switch( data->output.buf[7] ) {
                case '1': SAY( "ILLEGAL PLAY: ", 14 );
                    is_myturn = true;
                    switch( data->output.buf[8] ) {
                        case '1': SAY( "face mismatch", 13 ); break;
                        case '2': SAY( "too low", 7 ); break;
                        case '3': SAY( "too few", 7 ); break;
                        case '4': SAY( "unowned", 7 ); break;
                        case '5': SAY( "not your turn", 13 );
                            is_myturn = false;
                            break;
                        case '6': SAY( "must play 3 of clubs", 20 ); break;
                        case '7': SAY( "played duplicate card", 21 ); break;
                        default: SAY( "unknown reason", 14 );
                    }/* End Illigal Play Specific Switch */
//                     if(( auto_mode )&&( is_myturn )) {
//                        Write( gsd, "[cplay|52,52,52,52]", 19 );
//                        is_myturn = false;
//                     } break;
                    break;
                case '2':
                    SAY( "TIMEOUT: taking too long to play", 32 );
                    is_myturn = true;
                    break;
                case '3': SAY( "BAD MESSAGE: ", 13 );
                    switch( data->output.buf[8] ) {
                        case '1': SAY("play from lobby", 15 );
                            iam_playing = false;
                            break;
                        case '2': SAY("overflow", 8 ); break;
                        case '3': SAY("unknown type", 12 ); break;
                        case '4': SAY("malformed known type", 20 ); break;
                        default: SAY( "unknown reason", 14 );
                    }/* End Bad Message Specific Switch */
                    break;
                case '6': SAY("CHAT FLOOD", 10 ); break;
                case '7': SAY("ILLEGAL SWAP: ", 14 );
                    switch( data->output.buf[8] ) {
                        case '0':
                            SAY( "unowned card", 12 );
                            iam_warlord = true;
                            swap_message( WARL_SWAP );
                            /* WARNING: Other Errors will !reopen swap dialog */
                            break;
                        case '1': SAY( "Your not the warlord", 20 ); break;
                        case '2': SAY( "not time for swapping", 21 ); break;
                        default:
                            SAY( "unknown reason", 14 );
                            iam_warlord = true; /* Didn't say !Warlord */
                    }/* End Illigal Play Specific Switch */
                    break;
                case '8':
                    if( data->output.buf[8] == '1' )
                        SAY( "Full Server", 11 );
                    break;
                default: SAY( "unknown reason", 14 );
            }/* End Strike Code Switch*/
            SAY( ".\n", 2 );
            if( data->output.buf[10] == '3' )
                SAY( "You have been kicked out of the game!", 37 );
            break;
        case 'l': case 'L': /* [stabl|...] */
            if( update_table( data ) ) { /* True if my Turn */
                #ifdef DEBUG_BARF_TOO_MUCH
                    if( !auto_mode ) {
                        adjust_display_buf_view();
                        SAY( "It's Your Turn!\n", 16 );
                    }/* End Human If*/
                #endif
            } break;
        case 'n': case 'N': /* [sjoin|...] */
            //TODO: send mesage also to stderr and/or log
            memcpy( my_name, data->output.buf+7, 8 );
            SAY( "You have joined the server as \"", 31 );
            SAY( my_name, 8 );
            SAY( "\".\n", 3 );
            iam_joined = true;
            break;
        case 's': case 'S': /* [swaps|...] */
            if( !auto_mode ) {
                swap_cd[1][0] = data->output.buf[7];
                swap_cd[1][1] = data->output.buf[8];
                swap_cd[0][0] = data->output.buf[10];
                swap_cd[0][1] = data->output.buf[11];
                swap_message( SCUM_SWAP );
                swap_cd[1][0] = swap_cd[0][0] = '5';
                swap_cd[1][1] = swap_cd[0][1] = '2';
            }/* End Human If */
            if( Write( gsd, "[chand]", 7 ) < 0 )
                ERROR( "write", strerror(errno), gsd );
            break;
        case 'w': case 'W': /* [swapw|...] */
//             #ifdef DEBUG_BARF
//                 SAY( data->output.buf, data->output.n );
//             #endif
            iam_warlord = true;
            swap_cd[0][0] = data->output.buf[7];
            swap_cd[0][1] = data->output.buf[8];
            if( !auto_mode )
                swap_message( WARL_SWAP );
            break;
        case 't': case 'T': /* [schat|...] */
            data->output.buf[data->output.n-1] = '\"'; /* Strip Off ']' */
            data->output.buf[7+MAX_PNAME_LEN] = ':'; /* Split Mesg Elements */

            adjust_display_buf_view();
            SAY( &data->output.buf[7], MAX_PNAME_LEN+1 );
            SAY( " \"", 2 );
            SAY( &data->output.buf[8+MAX_PNAME_LEN], data->output.n - (8+MAX_PNAME_LEN) );
            SAY( "\n", 1 );
            break;
    }/* End Mesg Type Switch */

    /* Reset Output Buffer */
    data->output.n = 0;
}/* End Have_Complete_Mesg Func */


/******************************************************************************/
/* Function:    send_mesg                                                     */
/* Parameters:  gpointer        b                                             */
/*              GIOChannel*     Sd                                            */
/* Sends whatever is in the transmit.buf and displays write error messages.   */
/* This was used before the global socket descriptor, gsd, was needed.        */
/* TODO: Find out if this is still used/needed.                               */
/******************************************************************************/
void send_mesg( gpointer b, GIOChannel* Sd )
{
    assert( Sd != NULL );

    if( transmit.n > 0 )
        if( Write( g_io_channel_unix_get_fd(Sd), transmit.buf, transmit.n ) < 0 )
                ERROR( "channel-write", strerror(errno), __LINE__ );
}/* End send_mesg Func */


/******************************************************************************/
/* Function:    swap_message                                                  */
/* Parameters:  unsigned        qt      1 or 2 -- qt of cards in the mesg     */
/*  qt is used to determine if player is the warlord or scumbag. This opens   */
/* the dialog box for swapping.                                               */
/******************************************************************************/
#define DIALOG 0
#define DLABEL 1
#define CONTNT 2
#define CARDBX SWAP /* Descriptive Indexing Alias */
void swap_message( unsigned qt )
{
    GtkWidget* widget[3];
    char* status[2] = {"You Are Warlord!","You Are Scumbag!"},
        * whathap[2] = {" Select a card to swap. ", "You Lost:\tYou Recieved:"},
        * response[2] = {"Swap","Fine"},
          fp[24] = "./dat/img/cards/52.jpeg";

    assert( qt < 3 );

    widget[DIALOG] = gtk_dialog_new( );
    gtk_window_set_default_size( GTK_WINDOW (widget[DIALOG]), 250, 232 );
    widget[CONTNT] = gtk_dialog_get_content_area( GTK_DIALOG (widget[DIALOG]) );
    gtk_window_set_title( GTK_WINDOW (widget[DIALOG]), status[qt-1] );
    gtk_dialog_add_button( GTK_DIALOG (widget[DIALOG]), response[qt-1], qt );

    widget[DLABEL] = gtk_label_new( whathap[qt-1] );
    g_signal_connect( widget[DIALOG], "response", G_CALLBACK (click_cswap), "s" );
    g_signal_connect_after( widget[DIALOG], "response", G_CALLBACK (gtk_widget_destroy), widget[DIALOG] );
    gtk_box_pack_start( GTK_BOX (widget[CONTNT]),  widget[DLABEL] , FALSE, FALSE, 4);
    Widget[CARDBX] =  gtk_hbox_new( FALSE, 0 );
    gtk_box_pack_start( GTK_BOX (widget[CONTNT]),  Widget[CARDBX] , FALSE, FALSE, 4);

    fp[16] = swap_cd[0][0]; fp[17] = swap_cd[0][1];
    gtk_box_pack_start( GTK_BOX (Widget[CARDBX]),
            create_image( fp, G_CALLBACK(click_cswap), swap_cd[0] ), TRUE, TRUE, 0 );
    fp[16] = swap_cd[1][0]; fp[17] = swap_cd[1][1];
    gtk_box_pack_start( GTK_BOX (Widget[CARDBX]),
            create_image( fp, NULL, swap_cd[1] ), TRUE, TRUE, 0 );
    swap_cd[1][0] = '5'; fp[17] = swap_cd[1][1] = '2'; /* Reset Swap */
    gtk_widget_show_all( widget[DIALOG] );
}/* End swap_message Func */


/************************************EOF***************************************/
