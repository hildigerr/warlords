/* $Id: warlords.c,v 1.46 2014/02/05 21:53:19 moonsdad Exp $ */


/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      warlords.c    --  The Client main file.                               *
 ******************************************************************************/
#define MAIN
#include "client.h"

char localhost[] = "localhost"; /* default host name */

/********************************************************************** MAIN: */
int main( int argc, char **argv )
{
    struct hostent  * ptrh;     /* pointer to a host table entry */
    struct protoent * ptrp;     /* pointer to a protocol table entry */
    struct sockaddr_in sad;     /* structure to hold an IP address */
    int sd;                     /* socket descriptor */
    int port = PROTOPORT;       /* protocol port number */
    char* host = localhost;     /* pointer to host name */
    IODAT io_data;              /* BUFFERs holding Game Data and IO */
    int i;

    /* GTK Variables */
    const gchar* title = "Warlords and Skumbags";
    const gchar* icon = //"./dat/img/13031717912126086382istockphoto_6922629-wild-joker-in-a-deck-of-cards-th.png";
        "/usr/share/icons/warscum.png";
    gchar* tplist_coltitle[LIST_COL_QT] = { "STATUS", "NAME", "CARDS" };
    GIOChannel* Sd;
    GtkWidget* widget[MAX_WIDGETS]; /* WARNING: Beware the difference between *
                                     *  widget[] and Widget[]. The former has *
                                     *  global scope and a different range of *
                                     *  valid indexes.                        */

    /* Parse Command Line Options */
    opterr = 0;
    while( (i = getopt( argc, argv, "s:p:n:m" )) != -1 ) {
        switch( i ) {
            case 's':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                if( port < 36700 ) port += 36700;
                break;
            case 'n':
                if( strlen( optarg ) >= MAX_PNAME_LEN )
                    strncpy( my_name, optarg, MAX_PNAME_LEN );
                my_name[MAX_PNAME_LEN] = '\0'; /* Assert null-termniated String */
                break;
            case 'm':
                auto_mode = false;
                BARF( "auto_mode disabled.", 0 );
            case '?':
                fprintf( stderr, "Ignoring " );
                switch( optopt ){
                    case('s'): case('p'): case('n'):
                        fprintf( stderr, "Option -%c : requires an argument.\n", optopt);
                        break;
                    default:
                        if( isprint (optopt) )
                            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                        else fprintf (stderr, "Unknown option character `0x%x'.\n", optopt );
                }/* End optopt Switch */
        }/* End getopt rval Switch */
    }/* End getopt While */

    /* Init IODAT */
    io_data.input.n = io_data.output.n =
    io_data.slobb.n = io_data.stabl.n  = io_data.shand.n = 0;

    swap_cd[1][0] = swap_cd[0][0] = '5';
    swap_cd[1][1] = swap_cd[0][1] = '2';
    swap_cd[1][2] = swap_cd[0][2] = '\0';


    /************************* Set up Connection ***************************/
    memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET; /* set family to Internet */

    if( port > 0 ) /* test for legal value */
        sad.sin_port = htons((u_short)port);
    else exit( Error( "bad port number:", port ) );

    /* Convert host name to equivalent IP address and copy to sad. */
    if( (ptrh = gethostbyname( host )) )
        memcpy( &sad.sin_addr, ptrh->h_addr, ptrh->h_length );
    else exit( ERROR( host, "invalid host", 1 ));

    /* Map TCP transport protocol name to protocol number. */
    if( !(ptrp = getprotobyname("tcp")) )
        exit( Error( "Cannot map \"tcp\" to protocol number!", 1 ) );

    /* Create a socket. */
    if( (sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto)) < 0 )
        exit( ERROR( "socket", strerror(errno), sd ) );
    gsd = sd; //HACK: make sd global?

    /* Connect the socket to the specified server. */
    if( connect( sd, (struct sockaddr *)&sad, sizeof(sad) ) < 0 )
        exit( ERROR( "connect", strerror(errno), sd ) );

    /* Set socket to nonblocking */
    fcntl_setf( sd, O_NONBLOCK );

    /* Setup IO Channel for GTK Main Loop */
    if( !(Sd = g_io_channel_unix_new( sd )) )
        exit( ERROR("g_io_channel_unix_new", "failed", sd) );

    if( auto_mode ) click_pbtn(NULL, NULL);
    /*************************** Set up GUI *********************************/
    gtk_init( &argc, &argv );

    /* Add Channel to GTK Main Loop */
//     g_io_add_watch_full( Sd, G_PRIORITY_HIGH, G_IO_IN, getmsg, (gpointer)&io_data, NULL/*?_dtor*/ );
    g_io_add_watch( Sd, G_IO_IN, getmsg, (gpointer)&io_data );

    /* Setup Main Window */
    widget[MAIN_WINDOW] = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW (widget[MAIN_WINDOW]), title );
    gtk_window_set_icon( GTK_WINDOW (widget[MAIN_WINDOW]), load_pixbuf(icon));
    g_signal_connect( widget[MAIN_WINDOW], "destroy", G_CALLBACK (gtk_main_quit), NULL);
    gtk_container_set_border_width( GTK_CONTAINER (widget[MAIN_WINDOW]), 5 );
    gtk_window_set_default_size( GTK_WINDOW (widget[MAIN_WINDOW]), DEFAULT_WINDOW_SIZE );

    widget[MAIN_FRAME] = gtk_table_new( 6, 3, FALSE ); //gtk_vbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER (widget[MAIN_WINDOW]), widget[MAIN_FRAME] );


    /* Setup Table Frame */
    widget[TABLE_FRAME] = gtk_vbox_new( FALSE, 0 );
    gtk_table_attach_defaults( GTK_TABLE (widget[MAIN_FRAME]), widget[TABLE_FRAME], 0, 2, 0, 5 );
    Widget[ADD_PLAYERS] = gtk_hbox_new( FALSE, 0 );
    gtk_box_pack_start( GTK_BOX (widget[TABLE_FRAME]), Widget[ADD_PLAYERS], TRUE, TRUE, 0 );
    Widget[TABLE_TOP] = gtk_hbox_new( FALSE, 0 );
    gtk_box_pack_start( GTK_BOX (widget[TABLE_FRAME]), Widget[TABLE_TOP], TRUE, TRUE, 0 );
    gtk_widget_show( Widget[TABLE_TOP] );
    gtk_widget_show( Widget[ADD_PLAYERS] );
    gtk_widget_show( widget[TABLE_FRAME] );

    /* Pack Empty Seats */
    for( i = 0; i < 4; i++ )
        gtk_box_pack_start( GTK_BOX (Widget[ADD_PLAYERS]), create_image( "./dat/img/empty_seat.png",
                NULL, NULL ), TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX (Widget[TABLE_TOP]), create_image( "./dat/img/empty_seat.png",
                NULL, NULL ), TRUE, TRUE, 0 );
    for( i = 0; i < 4; i++ )
        gtk_box_pack_start( GTK_BOX (Widget[TABLE_TOP]),
            create_image( "./dat/img/cards/52.jpeg",
                NULL, NULL ), TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX (Widget[TABLE_TOP]), create_image( "./dat/img/empty_seat.png",
                NULL, NULL ), TRUE, TRUE, 0 );

    /* Create Hand Window */
    widget[HAND_WINDOW] = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW (widget[HAND_WINDOW]), "Hand" );
    gtk_window_set_icon( GTK_WINDOW (widget[HAND_WINDOW]), load_pixbuf(icon));
    gtk_container_set_border_width( GTK_CONTAINER (widget[HAND_WINDOW]), 10 );
//     gtk_window_set_default_size( GTK_WINDOW (widget[HAND_WINDOW]), WINDOW_SIZE_HAND );
    g_signal_connect( widget[HAND_WINDOW], "destroy", G_CALLBACK (gtk_main_quit), NULL);//TODO: make recreate
    Widget[HAND] = gtk_hbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER (widget[HAND_WINDOW]), Widget[HAND] );
    gtk_widget_show( Widget[HAND] );
    /* Pack Empty Hand into Hand Box */
    for( i = 0; i < MAX_PHAND_LEN; i++ )
        gtk_box_pack_start( GTK_BOX (Widget[HAND]),
            create_image( "./dat/img/cards/52.jpeg",
                G_CALLBACK(click_hcard), "52" ), TRUE, TRUE, 0 );
    gtk_widget_show( widget[HAND_WINDOW] );

    /* Create Play Window */
    widget[PLAY_WINDOW] = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW (widget[PLAY_WINDOW]), "Play" );
    gtk_window_set_icon( GTK_WINDOW (widget[PLAY_WINDOW]), load_pixbuf(icon));
    gtk_container_set_border_width( GTK_CONTAINER (widget[PLAY_WINDOW]), 10 );
//     gtk_window_set_default_size( GTK_WINDOW (widget[PLAY_WINDOW]), WINDOW_SIZE_PLAY );
    g_signal_connect( widget[PLAY_WINDOW], "destroy", G_CALLBACK (gtk_main_quit), NULL);//TODO: make recreate
    if( !auto_mode ) gtk_widget_show( widget[PLAY_WINDOW] );

    widget[PLAY_FRAME] =  gtk_vbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER (widget[PLAY_WINDOW]), widget[PLAY_FRAME] );
    gtk_widget_show( widget[PLAY_FRAME] );
    widget[BTN_FRAME] = gtk_hbox_new( FALSE, 0 );
    gtk_box_pack_start( GTK_BOX (widget[PLAY_FRAME]), widget[BTN_FRAME], TRUE, TRUE, 0 );
    gtk_widget_show( widget[BTN_FRAME] );

    widget[PLAY_BUTTON] = gtk_button_new_with_label("PLAY");
    g_signal_connect( widget[PLAY_BUTTON], "clicked", G_CALLBACK (click_pbtn), "pl" );
    gtk_box_pack_start( GTK_BOX (widget[BTN_FRAME]), widget[PLAY_BUTTON], FALSE, FALSE, 4);
    gtk_widget_show( widget[PLAY_BUTTON] );

    widget[PASS_BUTTON] = gtk_button_new_with_label("PASS");
    g_signal_connect( widget[PASS_BUTTON], "clicked", G_CALLBACK (click_pbtn), "ps" );
    gtk_box_pack_start( GTK_BOX (widget[BTN_FRAME]), widget[PASS_BUTTON], FALSE, FALSE, 4);
    gtk_widget_show( widget[PASS_BUTTON] );

    widget[CLR_BUTTON] = gtk_button_new_with_label("CLEAR");
    g_signal_connect( widget[CLR_BUTTON], "clicked", G_CALLBACK (click_pbtn), "cl" );
    gtk_box_pack_start( GTK_BOX (widget[BTN_FRAME]), widget[CLR_BUTTON], FALSE, FALSE, 4);
    gtk_widget_show( widget[CLR_BUTTON] );

    Widget[PLAY] = gtk_vbox_new( FALSE, 0 );
    gtk_box_pack_start( GTK_BOX (widget[PLAY_FRAME]), Widget[PLAY], TRUE, TRUE, 0 );
    gtk_widget_show( Widget[PLAY] );
    for( i = 0; i < 4; i++ )
        gtk_box_pack_start( GTK_BOX (Widget[PLAY]), create_image( "./dat/img/cards/52.jpeg",
                NULL, NULL ), TRUE, TRUE, 0 );

    /* Setup Player List View */
    widget[STAT_VIEW] = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW (widget[STAT_VIEW]), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_table_attach_defaults( GTK_TABLE(widget[MAIN_FRAME]), widget[STAT_VIEW], 2, 3, 0, 6 );
    widget[STAT_FRAME] = gtk_vbox_new( FALSE, 0 );

    /* Setup Table Player List */
    playerlist[TABBLL] = gtk_list_store_new( LIST_COL_QT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );
    widget[TABLE_TREE] = gtk_tree_view_new_with_model( GTK_TREE_MODEL (playerlist[TABBLL]) );
    for( i = 0; i < LIST_COL_QT; i++ )
        gtk_tree_view_append_column( GTK_TREE_VIEW (widget[TABLE_TREE]),
            gtk_tree_view_column_new_with_attributes( tplist_coltitle[i],
                gtk_cell_renderer_text_new(), "text", i, NULL ) );
    gtk_widget_show( widget[TABLE_TREE] );
    gtk_box_pack_start( GTK_BOX (widget[STAT_FRAME]), widget[TABLE_TREE], TRUE, TRUE, 0 );
    gtk_widget_show( widget[STAT_FRAME] );

    /* Setup Lobby Player List */
    playerlist[LOBBYL] = gtk_list_store_new( 1, G_TYPE_STRING );
    widget[LOBBY_TREE] = gtk_tree_view_new_with_model( GTK_TREE_MODEL (playerlist[LOBBYL]) );
    {   GtkTreeViewColumn* l_col = gtk_tree_view_column_new_with_attributes( "The Lobby",
            gtk_cell_renderer_text_new(), "text", 0, NULL );
        gtk_tree_view_column_set_alignment( l_col, 0.5 );
        gtk_tree_view_append_column( GTK_TREE_VIEW (widget[LOBBY_TREE]), l_col);
    } gtk_widget_show( widget[LOBBY_TREE] );
    gtk_box_pack_start( GTK_BOX (widget[STAT_FRAME]), widget[LOBBY_TREE], TRUE, TRUE, 0 );
    gtk_widget_show( widget[STAT_FRAME] );

    gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW (widget[STAT_VIEW]), widget[STAT_FRAME] );
    gtk_widget_show( widget[STAT_VIEW] );

    /* Setup Text Chat/Msg Frame */
    widget[COM_FRAME] = gtk_vbox_new( FALSE, 0 );
    gtk_table_attach_defaults( GTK_TABLE(widget[MAIN_FRAME]), widget[COM_FRAME], 0, 2, 5, 6 );
    gtk_widget_show( widget[COM_FRAME] );

    widget[MESG_BOX] = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW (widget[MESG_BOX]), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start( GTK_BOX (widget[COM_FRAME]), widget[MESG_BOX] , TRUE, TRUE, 4);
    gtk_widget_show( widget[MESG_BOX] );

    widget[MESG_FIELD] = gtk_text_view_new();
    display_buf = gtk_text_view_get_buffer( GTK_TEXT_VIEW (widget[MESG_FIELD]) );
    gtk_text_view_set_editable( GTK_TEXT_VIEW (widget[MESG_FIELD]), FALSE );
    gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW (widget[MESG_FIELD]), FALSE );
    gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW (widget[MESG_FIELD]), GTK_WRAP_WORD );
    gtk_widget_show( widget[MESG_FIELD] );
    gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW (widget[MESG_BOX]), widget[MESG_FIELD] );

    widget[CHAT_FIELD] = gtk_text_view_new();
    gtk_text_view_set_editable( GTK_TEXT_VIEW (widget[CHAT_FIELD]), TRUE );
    gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW (widget[CHAT_FIELD]), TRUE );
    gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW (widget[CHAT_FIELD]), GTK_WRAP_WORD );
    g_signal_connect( widget[CHAT_FIELD], "key_press_event", G_CALLBACK(chat_key_press), (gpointer)widget[CHAT_FIELD] );
    widget[CHAT_BOX] = gtk_hbox_new( FALSE, 0 );
    gtk_box_pack_start( GTK_BOX (widget[COM_FRAME]),  widget[CHAT_BOX] , FALSE, FALSE, 4);
    widget[SAY_BTN] = gtk_button_new_with_label("SAY:");
    g_signal_connect( widget[SAY_BTN], "clicked", G_CALLBACK (parse_chat), (gpointer)widget[CHAT_FIELD] );
    g_signal_connect_after( widget[SAY_BTN], "clicked", G_CALLBACK (send_mesg), (gpointer)Sd );
    gtk_box_pack_start( GTK_BOX (widget[CHAT_BOX]), widget[SAY_BTN], FALSE, FALSE, 4);
    gtk_box_pack_start( GTK_BOX (widget[CHAT_BOX]), widget[CHAT_FIELD], TRUE, TRUE, 4);
    gtk_widget_show( widget[SAY_BTN] );
    gtk_widget_show( widget[CHAT_BOX] );
    gtk_widget_show( widget[CHAT_FIELD] );

    /* Display GUI */
    gtk_widget_show( widget[MAIN_FRAME] );
    gtk_widget_show( widget[MAIN_WINDOW] );

    gtk_main();

    close(sd);
    return 0;
}/* End main Func */


/******************************************************************************/
/* Function:    click_hcard             G_CALLBACK                            */
/* Parameters:  GtkWidget*      event_box                                     */
/*              GdkEventButton* event                                         */
/*              gpointer        data                                          */
/* Returns:     gboolean                TRUE if event handling completed.     */
/*                                      FALSE to continue invoking callbacks. */
/* This is the handler for when cards are clicked. It directs the call to     */
/*      click_cswap if iam_warlord -- The card is selected for swaping.       */
/* Otherwise, The data is either a string with:                               */
/*      "ps"    -- Player is Passing.                                         */
/*      "pl"    -- Player is Playing.                                         */
/*      "cl"    -- Player is Clearing Current Prepared Play.                  */
/*      card    -- Number representing Card Face.                             */
/******************************************************************************/
gboolean click_hcard( GtkWidget *event_box, GdkEventButton *event, gpointer data )
{
    static char playmsg[20] = "[cplay|52,52,52,52]";
    static int cards_played = 0;
    char fp[24] = "./dat/img/cards/52.jpeg",
        *Data   = (char*)data; /* Get Rid of Warning */

    assert( data != NULL );

    if( iam_warlord ) return click_cswap( NULL, NULL, data );

#ifdef DEBUG_BARF
    /* BARF */
    if( event )
        g_print( "Event box clicked at coordinates %f,%f", event->x, event->y );
    g_print( "== Card # %s\n", (char*)data );
#endif

    switch( Data[0] ) {
        case 'p':
            if( Data[1] == 's' ) memcpy( playmsg+7, "52,52,52,52", 11 );
            if( Write( gsd, playmsg, 19 ) < 0 )
                ERROR( "write", strerror(errno), gsd );
            #ifdef DEBUG_BARF
                g_print( "PLAYED: %s", playmsg );//BARF
            #endif
            is_myturn = false;
            if( Write( gsd, "[chand]", 7 ) < 0 )
                ERROR( "write", strerror(errno), gsd );
            /* NO BREAK */
        case 'c' :
            cards_played = 0;
            memcpy( playmsg+7, "52,52,52,52", 11 );
            break;
        default:
            if( !isdigit( Data[0] ) ) break; //Assert no Garbage From GTK
            memcpy( playmsg+7+(3*cards_played), data, 2 );
            if( ++cards_played == 4 ) cards_played = 0; /* Loop Around */
            //TODO: have preference setting for other behaviour
    }/* End Data Switch */

    /* Display Play So Far */
    gtk_container_foreach( GTK_CONTAINER (Widget[PLAY]), widget_destroy, NULL );
    fp[16] = playmsg[7]; fp[17] = playmsg[8];
    gtk_box_pack_start( GTK_BOX (Widget[PLAY]),
        create_image( fp, NULL, NULL ), TRUE, TRUE, 0 );
    fp[16] = playmsg[10]; fp[17] = playmsg[11];
    gtk_box_pack_start( GTK_BOX (Widget[PLAY]),
        create_image( fp, NULL, NULL ), TRUE, TRUE, 0 );
    fp[16] = playmsg[13]; fp[17] = playmsg[14];
    gtk_box_pack_start( GTK_BOX (Widget[PLAY]),
        create_image( fp, NULL, NULL ), TRUE, TRUE, 0 );
    fp[16] = playmsg[16]; fp[17] = playmsg[17];
    gtk_box_pack_start( GTK_BOX (Widget[PLAY]),
        create_image( fp, NULL, NULL ), TRUE, TRUE, 0 );

    return TRUE;
}/* End click_hcard Func */


/******************************************************************************/
/* Function:    click_cswap             G_CALLBACK                            */
/* Parameters:  GtkWidget*      event_box                                     */
/*              GdkEventButton* event                                         */
/*              gpointer        data                                          */
/* Returns:     gboolean                TRUE if event handling completed.     */
/*                                      FALSE to continue invoking callbacks. */
/* This is the handler for when cards are clicked for swaping.                */
/* data is a character array beginning with an 's' for SWAP, or containing    */
/*      a card number for the card being selected for swapping.               */
/******************************************************************************/
gboolean click_cswap( GtkWidget *event_box, GdkEventButton *event, gpointer data )
{
    static char playmsg[11] = "[cswap|52]";
    char fp[24] = "./dat/img/cards/52.jpeg",
        *Data   = (char*)data; /* Get Rid of Warning */

    assert( data != NULL );

    if( !iam_warlord ) return TRUE;

    if( Data[0] == 's' ) {
        if( Write( gsd, playmsg, 10 ) < 0 )
                ERROR( "write", strerror(errno), gsd );
        if( Write( gsd, "[chand]", 7 ) < 0 )
                ERROR( "write", strerror(errno), gsd );
        iam_warlord = false;
    } else {
        memcpy( playmsg+7, data, 2 );
        gtk_container_foreach( GTK_CONTAINER (Widget[SWAP]), widget_destroy, NULL );
        fp[16] = swap_cd[0][0]; fp[17] = swap_cd[0][1];
        gtk_box_pack_start( GTK_BOX (Widget[SWAP]),
            create_image( fp, G_CALLBACK(click_cswap), swap_cd[0] ), TRUE, TRUE, 0 );
        fp[16] = Data[0]; fp[17] = Data[1];
        gtk_box_pack_start( GTK_BOX (Widget[SWAP]),
            create_image( fp, NULL, swap_cd[0] ), TRUE, TRUE, 0 );
    }/* End Else */
    return TRUE;
}/* End click_cswap Func */


/******************************************************************************/
/* Function:    click_pbtn              G_CALLBACK                            */
/* Parameters:  GtkWidget*      button                                        */
/*              gpointer        data                                          */
/* Returns:     gboolean                TRUE if event handling completed.     */
/*                                      FALSE to continue invoking callbacks. */
/* This is the handler wrapper for when the "play" button is clicked.         */
/*  If player has joined, pass data on to click_hcard. Else send a cjoin.     */
/* TODO: have popup menu for last minute configuration if non auto_mode.      */
/******************************************************************************/
gboolean click_pbtn( GtkWidget *button, gpointer data ) {
    char cjoin[16];

    if( iam_joined ) return click_hcard( NULL, NULL, data );

    sprintf( cjoin, "[cjoin|%s]", my_name );
    if( Write( gsd, cjoin, 16 ) < 0 )
        ERROR( "write", strerror(errno), gsd );
    return TRUE;
}/* End click_pbtn Func */


/******************************************************************************/
/* Function:    chat_key_press              G_CALLBACK                        */
/* Parameters:  GtkWidget*      widget                                        */
/*              GdkEventKey*    key                                           */
/*              gpointer        data                                          */
/* Returns:     gboolean                TRUE if event handling completed.     */
/*                                      FALSE to continue invoking callbacks. */
/* This is the handler for chat message box input. It detects an 'enter' key  */
/* press as the signal to parse the chat message and send it on over.         */
/* I'm unsure of the gtk constant which matches the hex keyval.               */
/******************************************************************************/
gboolean chat_key_press( GtkWidget* widget, GdkEventKey* key, gpointer data )
{
    if(( key->type == GDK_KEY_PRESS )&&( key->keyval == 0xff0d )) {//HACK
        parse_chat( NULL, data );
        if( Write( gsd, transmit.buf, transmit.n ) < 0 )
                    ERROR( "write", strerror(errno), -1 );
        transmit.n = 0;
        return true;
    } else return false;
}/* End chat_key_press Func */


/************************************EOF***************************************/
