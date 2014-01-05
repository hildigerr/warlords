/* $Id: server.c,v 1.70 2013/12/09 15:38:34 moonsdad Exp $ */

/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      server.c    --  The Server main file.                                 *
 ******************************************************************************/

#define MAIN
#include "sgui.h" /* #includes "server.h" */
#include <pcre.h>

/********************************************************************** MAIN: */
int main( int argc, char* argv[] )
{
    struct protoent * ptrp;     /* pointer to a protocol table entry */
    struct sockaddr_in sad;     /* structure to hold server's address */
    fd_set rset, allset;                      /* fd sets for select */
    int listensd, maxfd, sd;                  /* socket descriptors */
    int nready;                               /* qt of sd ready on select */
    int port = PROTOPORT;                     /* protocol port number */

    struct sigaction timer_sa, cdie_sa;       /* Signal Catching Handles */

    char buf[MAX_SMESG_LEN+1], * x;           /* buffer for outgoing messages */
    int i, j;                                 /* iterating and indexing */
    int sop = 1;

    /* Parse Command Line Options */
    opterr = 0;
    while( (i = getopt( argc, argv, "t:m:l:g" )) != -1 ) {
        switch( i ) {
            case 't':
                ertimeout = atoi(optarg);
                if( !ertimeout ) ++ertimeout;//TODO: Fix so that 0 disables ertime
                if( ertimeout > MAX_ERTIMEOUT ) {
                    ertimeout = DEF_ERTIMEOUT;
                    fprintf( stderr, "Invalid -t argument: Reverting to default.\n" );
                } break;
            case 'm':
                min_ppplaying = atoi(optarg);
                if(( min_ppplaying < DEF_MIN_PLAYERS )||( min_ppplaying > MAX_PLAYER_QT )) {
                    min_ppplaying = DEF_MIN_PLAYERS;
                    fprintf( stderr, "Invalid -m argument: Reverting to default.\n" );
                } break;
            case 'l':
                lbtimeout = atoi(optarg);
                if( !lbtimeout ) ++lbtimeout;
                if( lbtimeout > MAX_LBTIMEOUT ) {
                    lbtimeout = DEF_LBTIMEOUT;
                    fprintf( stderr, "Invalid -l argument: Reverting to default.\n" );
                } break;
            case 'g':
                use_gui = true; /* Assume It Will Work *//* done last */
                break;
            case '?':
                fprintf( stderr, "Ignoring " );
                switch( optopt ){
                case('t'): case('m'): case('l'):
                    fprintf( stderr, "Option -%c : requires an argument.\n", optopt);
                    break;
                default:
                    if( isprint (optopt) )
                        fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                    #ifdef DEBUG_BARF
                    else fprintf (stderr, "Unknown option character `0x%x'.\n", optopt );
                    #endif
                }/* End optopt Switch */
                break;
            /* NO DEFAULT */
        }/* End getopt rval Switch */
    }/* End getopt While */

    /* Initialize Data */
    for( i = 0; i < MAX_PLAYER_QT; i++ ) init_player( i );
    for( i = 0; i < 4; i++ ) table_top[i] = 52; /* No Cards on Table */
    memset( (char *)&sad, 0, sizeof(sad) ); /* clear sockaddr structure */
    sad.sin_family = AF_INET;               /* set family to Internet */
    sad.sin_addr.s_addr = INADDR_ANY;       /* set the local IP address */
    sad.sin_port = htons((u_short)port);    /* set tcp port to default value */

    /* Catch SIGPIPE */
    memset( &cdie_sa, 0, sizeof(cdie_sa) );
    cdie_sa.sa_handler = SIG_IGN;//&sigpipe_handler;
    sigaction( SIGPIPE, &cdie_sa, NULL );

    /* Catch SIGVTALRM  */
    memset( &timer_sa, 0, sizeof(timer_sa) );
    timer_sa.sa_handler = &timer_handler;
    sigaction( SIGALRM, &timer_sa, NULL );
    timer.it_value.tv_sec = lbtimeout;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    /* TODO: maintain a strikelog file. */

    /* Map TCP transport protocol name to protocol number */
    if( !(ptrp = getprotobyname("tcp")) )
        exit( Error( "Cannot map \"tcp\" to protocol number!", __LINE__ ) );

    /* Create a socket *//* init maxfd & allset */
    /* NOTE: Windows sockets don't set errno. If porting use: "Socket creation failed." */
    if( (maxfd = listensd = socket( PF_INET, SOCK_STREAM, ptrp->p_proto )) < 0 )
        exit( Error( strerror(errno) , listensd ) );
    /* Initialize FD Set for Select */
    gfds = &allset; /*HACK - global fd set */
    FD_ZERO(&allset); FD_SET(listensd, &allset);
    if( use_gui ) FD_SET( 0, &allset );

#define TODO //When Working
#ifdef TODO
    /* Set Socket to Reuseable *//* Must be Done Before Bind */
    setsockopt( listensd, SOL_SOCKET, SO_REUSEADDR, (const char *)&sop, sizeof(int) );// NULL, 0 );
#undef TODO
#endif

    /* Bind a local address to the socket */
    if( bind( listensd, (struct sockaddr *)&sad, sizeof(sad) ) < 0 )
        exit( ERROR( "bind", strerror(errno), listensd ) );

    /* Set socket to non-blocking */
    fcntl_setf( listensd, O_NONBLOCK );

    /* Specify size of request queue */
    if( listen( listensd, LISTENQ ) < 0 )
        exit( ERROR( "listen", strerror(errno), listensd ) );

    /* Initialize Gui Last Thing Before Main Loop *//* May Abort */
    if( use_gui ) use_gui = servergui( SGUI_INIT, NULL );

    /* Main server loop */
    while( TRUE ) {
        rset = allset;
        nready = select( maxfd+1, &rset, NULL, NULL, NULL );

        /* Accept new Client Requests */
        if( FD_ISSET(listensd, &rset) ) {
            #ifdef DEBUG_BARF
                BARF("Incomming Connection", 1);
            #endif
            for( i = 0; i < MAX_PLAYER_QT; i++ ) {
                sd = player[i].socketfd;
                if( sd < 0 ) {
                    if( (sd = player[i].socketfd = accept( listensd, NULL, NULL )) < 0 )
                    #ifdef DEBUG_BARF
                        ERROR( "accept", strerror(errno), i )
                    #endif
                    ;
                    else player[i].status = UNJOINED;
                    break;
                }/* End accept player[i].socketfd If */
            }/* End Find Empty Player For */
            if( i == MAX_PLAYER_QT ) {
                #ifdef DEBUG_BARF
                    ERROR( "client connect", "Maximum players reached.", MAX_PLAYER_QT );
                #endif
                int bob = accept( listensd, NULL, NULL );
                if( bob < 0 ) continue;
                Write( bob, "[strik|81|3]", 12 ); /* 81 (SERVER_FULL) */
                close( bob );
                // TODO //
                // Kill any clients who havn't joined the lobby yet
                // if they have been idle ? too ? long
                // then accept the newbie
            } else { /* Room for More Clients */
                if( sd < 0 ) continue; /* False Connection: Accept WOULDBLOCK */
                #ifdef DEBUG_BARF
                    BARF( "Connecting", sd );
                #endif
                FD_SET( sd, &allset ); /* Listen to new Client */
                if( maxfd < sd ) maxfd = sd;
                if( maxci < i ) maxci = i;
            }/* End New Client Connected Else */
            if( --nready <= 0 ) continue; /* no more ready sockets */
        }/* End New Client If */

        if( FD_ISSET(0, &rset) ) { /* Server Admin -- Standard In */
            if( use_gui ){
                BUFFER scmdl;
                getnstr( scmdl.buf, MAX_SMESG_LEN );
                scmdl.n = strlen( scmdl.buf ); // TODO: Better Way /////////////////WORKING
//                 scmdl.n = wgetnstr( curse_input, scmdl.buf, MAX_SMESG_LEN );
                cmd_exec( scmdl );
            } else fflush( stdin );//TODO: allow commands without gui
            if( --nready <= 0 ) continue; /* no more ready sockets */
        }/* End STDIN If */

        /* Check all Clients for Data */
        for( i = 0; i <= maxci; i++ ) {
            if( player[i].status == DIS_CONN ) continue; /* Skip DC Players */
            if( (sd = player[i].socketfd) < 0 ) continue; /* Skip Unused Entries */
            if( FD_ISSET(sd, &rset) ) {
                #ifdef DEBUG_BARF
                    BARF( "Incomming Data ", sd );
                #endif
                if( !(player[i].input.n = recv( sd, player[i].input.buf, MAX_CMESG_LEN+2, 0 )) ) {
                    Remove_player( i );
                } else { /* Read in some Data */
                    int pos = 0;
                    if( player[i].input.n < 0 ) { /* recv Error */
                        #ifdef DEBUG_BARF
                            ERROR( "recv", strerror(errno), i );
                        #endif
                        Remove_player(i); /* Assume Connection Reset */
                        continue;
                    }/* End Failed recv If */
                    assert(  player[i].input.n <= MAX_CMESG_LEN+2 );
                    while( (pos = buf_copy2( &player[i].mesg, &player[i].input, pos, ']', TRUE ))  < player[i].input.n )
                    if( pos++ < 0 ) {
                        if( player[i].mesg.n > MAX_CMESG_LEN ) {
                            player[i].mesg.n = 0;
                            if( strike( i, MALFORMED_MESG ) ) Remove_player(i);
                        } break;
                    } else { /* Have Complete Mesg */
                        #ifdef DEBUG_BARF
                            fprintf( stderr, " TESTING-BARF: %s ", player[i].mesg.buf );//TODO: put in a logfile
                        #endif
                        /* Handle Client Request */
                        switch( player[i].mesg.buf[2] ) { /* HACK: Looking only at expected first unique char */
                            case 'c': case 'C': /* [cchat|...] */
                                /* Verify mesg Length */
                                if( (( player[i].mesg.n < 9 )&&( strike( i, MALFORMED_MESG )) )
                                ||( ( player[i].mesg.n > MAX_CHAT_MESG+8 )&&( strike( i, MESG_TOO_LONG ) ) )
                                ||( ( player[i].ban )&&( strike( i, CHAT_BAN ) ) ) )
                                    Remove_player(i);
                                else { /* Forward Message to Players */
                                    if( player[i].ban ) break;
                                    /* TODO: padd with space XXX */
                                    sprintf( buf, "[schat|%s|%s", player[i].name , &player[i].mesg.buf[7] );
                                    broadcast( buf, player[i].mesg.n+9 );
                                } break;
                            case 'h': case 'H': /* [chand] */
                                sprintf( buf, "[shand|" );
                                x = &buf[7];
                                for( j = 0; j < MAX_PHAND_LEN; j++ ) {
                                    *x++ = '0'+ (int)( player[i].hand[j] / 10 );
                                    *x++ = '0'+ ( player[i].hand[j] % 10 );
                                    *x++ = ',';
                                }/* End card in hand For */
                                *(x-1) = ']'; *x = 0;
                                Write( sd, buf, 61 );
                                break;
                            case 'j': case 'J': /* [cjoin|name] */
                                if( player[i].status != UNJOINED ) break; //TODO: allow renaming instead
                                /* Verify mesg length */
                                if( player[i].mesg.n < 8+MAX_PNAME_LEN ) strike( i, MALFORMED_MESG );
                                else if( mangle(i) ) strike( i, MALFORMED_MESG );
                                sprintf( buf, "[sjoin|%s]", player[i].name );
                                Write( sd, buf, 16 );
                                /* Queue Player in Lobby */
                                player[i].status = IN_LOBBY;
                                lobby_last->who = &player[i];
                                lobby_last = lobby_last->next = CALLOC( 1, PQUEUE ); /* WARNING: assumes calloc is successfull */
                                ++p_in_lob;
                                send_slobb_mesg();
                                /* Set Countdown To Play if Enough Players */
                                if( ( server_status == ENQING )&&( p_in_lob == min_ppplaying ) ) {
                                    server_status = SHUFFL;
                                    timer.it_value.tv_sec = lbtimeout;
                                    setitimer( ITIMER_REAL, &timer, NULL );
                                }/* End min_ppplaying if */
                                break;
                            case 'p': case 'P': /* [cplay|c1,c3,c3,c4] */
                                if( player[i].mesg.n != 19 ) {
                                    if( strike( i, MALFORMED_MESG ) )
                                        Remove_player(i);
                                } else if( player[i].status == IN_LOBBY ) {
                                    if( strike( i, LOBBY_PLAY ) )
                                        Remove_player(i);
                                } else if( player[i].status != ACTIVE_P ) {
                                    if( strike( i, PLAY_OUT_OF_TURN ) )
                                        Remove_player(i);
                                } else { /* mesg is correct length & is player's turn */
                                    int cards[4] = { ( c2d(player[i].mesg.buf[7])  * 10 ) + c2d(player[i].mesg.buf[8]),
                                                     ( c2d(player[i].mesg.buf[10]) * 10 ) + c2d(player[i].mesg.buf[11]),
                                                     ( c2d(player[i].mesg.buf[13]) * 10 ) + c2d(player[i].mesg.buf[14]),
                                                     ( c2d(player[i].mesg.buf[16]) * 10 ) + c2d(player[i].mesg.buf[17]) },
                                        flav;
                                    if( cards[0] == 52 ) { //Enforce PLAYER_IS_PASSING
//                                         broadcast( "[schat|SERVER  |Player Passed]" , 30 );//TESTING_BARF
                                        set_next_turn( i , NULL, PASSED_P );
                                        if( need_club3 ) need_club3 = false;//Allow's player with 3of clubs to pass//XXX//IS REDUNDANT
                                    } else if( !player_ownes( i, cards ) ) {
                                        if( strike( i, PLAY_UNOWNED ) )
                                            Remove_player(i);
                                        else//send hand
                                            break;
                                    } else if( (flav = valid_play( cards )) ) {
                                        if( strike( i, flav ) )
                                            Remove_player(i);
                                        else//send hand
                                             break;
                                    } else if(( need_club3 )&&( !played_club3( cards ) )) {
                                        if( strike( i, IPLAY_NO_3CLUB ) )
                                            Remove_player(i);
                                        else//send hand
                                             break;
                                    } else if( (flav = play_beats_table( cards )) ) {
                                        if( flav == PLAYED_MATCH ) {
                                            Discard( cards ); /* WARNING: RVAL IGNORED */
                                            set_next_turn( i , cards, SKIPNEXT );
                                        } else { /* Played Too Low Or Few */
                                            if( strike( i, flav ) )
                                                Remove_player(i);
                                            else break;
                                        }/* End Strike Else */
                                    } else { /* Good Non Match nor Trump Play */
                                        set_next_turn( i , cards, Discard( cards ) );
                                    }/* End Good Play Else */
                                    if( need_club3 ) need_club3 = false;
                                    send_tabl_mesg();
                                    timer.it_value.tv_sec = ertimeout;
                                    setitimer( ITIMER_REAL, &timer, NULL );
                                } break;
                            case 'q': case 'Q': /* [cquit] */
                                Remove_player(i); break;
                            case 's': case 'S': /* [cswap] */
                                if( server_status != SWAPIN ) {
                                    if( strike( i, SWAP_BAD_TIME ) )
                                        Remove_player(i);
                                    break;
                                }/* End SWAP_BAD_TIME If */
                                if( &player[i] != table[0] ) {
                                    if( strike( i, SWAP_BAD_WHO ) )
                                        Remove_player(i);
                                    break;
                                }/* End SWAP_BAD_WHO If */
                                if( player[i].mesg.n < 10 ) {
                                    if( strike( i, MALFORMED_MESG ) )
                                        Remove_player(i);
                                    break;
                                }/* End MALFORMED_MESG If */
                                else {
                                    unsigned nu_cd;
                                    player[i].mesg.buf[9] = '\0';
                                    nu_cd = atoi( &player[i].mesg.buf[7] );
                                    if(( nu_cd >= 52 )||( deck[nu_cd] != table[0] )) {
                                        if( strike( i, SWAP_UNOWNED ) )
                                            Remove_player(i);
                                        break;
                                    }/* End SWAP_UNOWNED If */
                                    deck[nu_cd] = table[p_in_tab-1];
                                    if( use_gui ) { /* SGUI -- Update Count */
                                        int l = (nu_cd-(nu_cd%4))/4;
                                        ++deck[nu_cd]->count[l];
                                        --player[i].count[l];
                                    }/* End Use Gui If */
                                    sprintf( buf, "[swaps|%s|", &player[i].mesg.buf[7] );
                                    ( swapped_cd < 10 ) ?
                                        sprintf( buf+10, "%c%d]", '0', swapped_cd ):
                                        sprintf( buf+10, "%d]", swapped_cd );
                                    if( Write( table[p_in_tab-1]->socketfd, buf, 13 ) < 0 )
                                        ERROR( "write", strerror(errno), p_in_tab-1 );
                                    for( j = 0; j < p_in_tab; j++ ) {
                                        memset( table[j]->hand, 52, MAX_PHAND_LEN );
                                        table[j]->cardqt = 0;
                                    }/* End Reset Hand For */
                                    for( j = 0; j < 52; j++ ) {
                                        assert( deck[j]->cardqt != MAX_PHAND_LEN ); /* TODO: Verify Fix */
//                                         if( deck[j]->cardqt == MAX_PHAND_LEN ) continue; /* HACK: TODO-DONE: Really Fix */
//                                         else
                                            deck[j]->hand[(int)deck[j]->cardqt++] = j;
                                    }/* End Set Hand For */
                                    sprintf( buf, "[shand|" );
                                    x = &buf[7];
                                    /* Send Scumbag The New Hand */
                                    for( j = 0; j < MAX_PHAND_LEN; j++ ) {
                                        *x++ = '0'+ (int)( table[p_in_tab-1]->hand[j] / 10 );
                                        *x++ = '0'+ ( table[p_in_tab-1]->hand[j] % 10 );
                                        *x++ = ',';
                                    }/* End card in hand For */
                                    *(x-1) = ']'; *x = 0;
                                    if( Write( table[p_in_tab-1]->socketfd, buf, 61 ) < 0 )
                                        ERROR( "write", strerror(errno), p_in_tab-1 );
                                    /* Begin Play */
                                    server_status = ACTIVE;
                                    send_tabl_mesg();
                                    timer.it_value.tv_sec = ertimeout;
                                    setitimer( ITIMER_REAL, &timer, NULL );
                                } break;
                            default: /* send strike */
                                if( strike( i, UNKOWN_MESG_TYPE ) )
                                    Remove_player(i);
                        } /* End MESG switch */
                        player[i].mesg.n = 0; /* Reset */
                    }/* End recv complete mesg Else */
                }/* End recv mesg Else */
                if( --nready <= 0 ) continue; /* no more ready sockets */
            } /* End Current Player Speaking If */
        }/* End maxci clients For */
    }/* End inf While */

    return 0;
}/* End Main Func */


/******************************************************************************/
/* Function:    init_player                                                   */
/* Parameters:  unsigned        i       index of player to initialize         */
/******************************************************************************/
inline void init_player( unsigned i )
{
    assert( i < MAX_PLAYER_QT );
    memset( &player[i].input.buf, 0, MAX_SMESG_LEN+1 );
    memset( &player[i].mesg.buf, 0, MAX_SMESG_LEN+1 );
    memset( &player[i].hand, 52, MAX_PHAND_LEN );
    sprintf( player[i].name, "Player%d", i );
    if( i < 10 ) sprintf( &player[i].name[7], "%c", ' ' );

    player[i].socketfd = -1; /* available */
    player[i].cardqt = 0;
    player[i].input.n = 0;
    player[i].mesg.n = 0;
    player[i].status = DIS_CONN;
    player[i].strikqt = '0';

    /* SGUI */
    player[i].hands = 0;
    player[i].score = 0;
    player[i].ban = false;

    if( use_gui ) {
        player[i].strikelog[0] =
        player[i].strikelog[1] =
        player[i].strikelog[2] = 0;
        player[i].count[0] = player[i].count[1] =
        player[i].count[2] = player[i].count[3] =
        player[i].count[4] = player[i].count[5] =
        player[i].count[6] = player[i].count[7] =
        player[i].count[8] = player[i].count[9] =
        player[i].count[10] = player[i].count[11] =
        player[i].count[12] = 0;
    }/* End SGUI If */
}/* End init_player Func */


// /******************************************************************************/
// /* Function:    init_p                 ***DEPRECATED***                       */
// /* Parameters:  PLAYER*         who     Address of player to reinitialize     */
// /* Reinitializes the player structure, for when we don't know its index.      */
// /******************************************************************************/
// inline void init_p( PLAYER* who )
// {
//     unsigned i = (who-player);///sizeof(PLAYER);
//     #ifdef DEBUG_BARF
//         BARF( "Testing init_p", i );
//     #endif
//     assert( i <  MAX_PLAYER_QT );
//
//     if( who ) {
//         memset( &who->mesg.buf, 0, MAX_SMESG_LEN+1 );
//         memset( &who->input.buf, 0, MAX_SMESG_LEN+1 );
//         memset( &who->hand, 52, MAX_PHAND_LEN );
//         sprintf( who->name, "Player%d", i );
//         if( i < 10 ) sprintf( &player[i].name[7], "%c", ' ' );
//
//         who->socketfd = -1; /* available */
//         who->cardqt = 0;
//         who->mesg.n = 0;
//         who->input.n = 0;
//         who->status = DIS_CONN;
//         who->strikqt = '0';
//     }/* End !NULL If */
// }/* End init_p Func */


/******************************************************************************/
/* Function:    mangle                                                        */
/* Parameters:  unsigned        i       index of player to initialize         */
/* Returns:     bool                    True if invalid name requested        */
/* WARNING:     TODO*/
/******************************************************************************/
#define DEFAULT_OPTIONS 0
#define OVECCOUNT 6     /* should be a multiple of 3 */
bool mangle( unsigned i )
{
    bool retval = false; /* Assume !invalid Name Requested */
    const char *error;
    int erroffset , rc, ovector[OVECCOUNT];

    pcre* regex = pcre_compile( "^[A-Za-z_][A-Za-z_0-9]*",
                                DEFAULT_OPTIONS, &error, &erroffset, NULL );

    assert( i < MAX_PLAYER_QT );

    player[i].mesg.buf[7+MAX_PNAME_LEN] = 0;

    if( !regex )
        ERROR( "PCRE compilation", error, erroffset );
        /* Use Default Names */
    else {
        rc = pcre_exec( regex, NULL, &player[i].mesg.buf[7], MAX_PNAME_LEN,
          0,                    /* start at offset 0 in the subject */
          DEFAULT_OPTIONS,
          ovector,              /* output vector for substring information */
          OVECCOUNT );          /* number of elements in the output vector */

        if( rc < 0 ){ /*  pcre_exec Failed */
            if( rc == PCRE_ERROR_NOMATCH ) retval = true; /* Get Strike */
            else ERROR( "PCRE", "matching error", rc );
        } else { /* Success */
            int j;
            for( j = 0; j < MAX_PLAYER_QT; j++ )
                if( !strcmp( &player[i].mesg.buf[7], player[j].name ) ) /* Found Dup */
                    break;
            if( j == MAX_PLAYER_QT ) sprintf( player[i].name, "%s", &player[i].mesg.buf[7] );
        }/* End pcre_exec Success Else */

        pcre_free( regex );
    } return retval;
}/* End mangle Func */


/******************************************************************************/
/* Function:    broadcast                                                     */
/* Parameters:  const char*     buf     Container of mesg to send.            */
/*              size_t          count   Quantity of chars in buf.             */
/******************************************************************************/
inline void broadcast( char* buf, size_t count )
{
    int i;
    if( use_gui ) servergui( buf[2], buf ); /* SGUI */
    for( i = 0; i < MAX_PLAYER_QT; i++ )
        if(( player[i].socketfd > 0 )&&( player[i].status != DIS_CONN ))
            if( Write( player[i].socketfd, buf, count ) < 0 )
                #ifdef DEBUG_BARF
                    ERROR( "broadcast-write", strerror(errno), i )
                #endif
                ;
}/* End broadcast Func */


/******************************************************************************/
/* Function:   strike                                                         */
/* Parameters:  unsigned        i       index of player to initialize         */
/*              unsigned        error   code indicating reason for strike     */
/* Returns:     bool                    TRUE if player striked out            */
/******************************************************************************/
bool strike( unsigned i, unsigned error )
{
    char buf[13];

    assert( i < MAX_PLAYER_QT ); assert( error < 100 );

    if( use_gui ) player[i].strikelog[(int)(player[i].strikqt-'0')] = error; /* SGUI */
    sprintf( buf, "[strik|%d|%c]", error, ++player[i].strikqt );
    Write( player[i].socketfd, buf, 12 );

    if( player[i].strikqt >= MAX_STRIKES ) return TRUE;
    else return FALSE;
}/* End strike Func */


/******************************************************************************/
/* Function:    remove_player                                                 */
/* Parameters:  unsigned        i       index of player to remove             */
/*              char*           buf     pass in main buffer for use           */
/*              fd_set*         set     set of file descriptors selecting on  */
/* Closes player's socket and clears it from listening set.                   */
/* Moves player to scumbag list if is at table, to be removed after hand.     */
/* Reinitializes player if unconnected or in lobby. Also, removes player from */
/* lobby and reports that change.                                             */
/******************************************************************************/
void remove_player( unsigned i, char* buf, fd_set* set )//TODO: Pause Timer?
{
    PQUEUE *temp, *r;
    char* x = &buf[7];

    assert( i < MAX_PLAYER_QT );

    close( player[i].socketfd );
    FD_CLR( player[i].socketfd, set );

    switch( player[i].status ) {
        case ACTIVE_P:
            if( server_status == SWAPIN ) {
                server_status = ACTIVE;
                deck[swapped_cd] = table[p_in_tab-1]; /* Return Scumbag's Card */
            }/* End SWAPIN If */
            if( deck[0] == &player[i] ) need_club3 = false;
            --c_in_tab; /* NEEDED before set_next_turn */
            set_next_turn( i, NULL, DIS_CONN );
            /* NO BREAK */
            ++c_in_tab;/* HACK: Gets decremented again, but needed for set_next_turn */
////////////////////////////////////////////////////////////////////////////////////////////////////
        case PASSED_P: case WAITINGP:
            /* Move Player to Warlords Queue for Now */
            #ifdef DEBUG_BARF
                BARF( "Moving DC Player to warQ.", i );
            #endif
            scumbag->who = &player[i];
            scumbag = scumbag->next = CALLOC( 1, PQUEUE ); /* WARNING: assumes calloc is successfull */
            /* NO BREAK */
        case WAITNEXT:
            player[i].status = DIS_CONN;
            sprintf( buf, "[schat|SERVER  |%s has disconnected.]", player[i].name );
            broadcast( buf, 4+5+8+8+18 );

            if( --c_in_tab < 2 ) { /* Solo Player Remains */
                for( i = 0; i < p_in_tab; i++ ) { /* WARNING: Reusing i */
                    if( table[i]->status != DIS_CONN ) { /* DC Players already in scumbag list */
                        if( table[i]->status == WAITNEXT ) continue;
                        table[i]->status = IN_LOBBY;
                        scumbag->who = table[i];
                        scumbag = scumbag->next = CALLOC( 1, PQUEUE ); /* WARNING: assumes calloc is successfull */
                    } //else init_p( table[i] ); //Saved 'til later to retain data for current hand
                }/* End Clear Table For */
                server_status = SHUFFL;
                timer_handler( 1 );
                return;
            } /* End ! At Least 2 Players If */
            if( server_status == ACTIVE ) send_tabl_mesg();
            break;
////////////////////////////////////////////////////////////////////////////////////////////////////
//         case PASSED_P: case WAITINGP: case WAITNEXT:
//             player[i].status = DIS_CONN;
//             sprintf( buf, "[schat|SERVER  |%s has disconnected.]", player[i].name );
//             broadcast( buf, 4+5+8+8+18 );
//
//             /* Move Player to Warlords Queue for Now */
//             #ifdef DEBUG_BARF
//                 BARF( "Moving DC Player to warQ.", i );
//             #endif
//             scumbag->who = &player[i];
//             scumbag = scumbag->next = CALLOC( 1, PQUEUE ); /* WARNING: assumes calloc is successfull */
// //             /* Move Player to DC List */
// //             BARF( "Moving DC Player to list.", i );
// //             dc_last->who = &player[i];
// //             dc_last = dc_last->next = CALLOC( 1, PQUEUE ); /* WARNING: assumes calloc is successfull */
//
//             if( --c_in_tab < 2 ) { /* Solo Player Remains */
//                 for( i = 0; i < p_in_tab; i++ ) { /* WARNING: Reusing i */
//                     if( table[i]->status != DIS_CONN ) { /* DC Players already in scumbag list */////TODO !scumbag but dc list
//                         if( table[i]->status == WAITNEXT ) continue;
//                         table[i]->status = IN_LOBBY;
//                         scumbag->who = table[i];
//                         scumbag = scumbag->next = CALLOC( 1, PQUEUE ); /* WARNING: assumes calloc is successfull */
//                     } //else init_p( table[i] ); //Saved 'til later to retain data for current hand
//                 }/* End Clear Table For */
//                 server_status = SHUFFL;
//                 timer_handler( 1 );
//             } /* End ! At Least 2 Players If */
//                 if( server_status == ACTIVE ) send_tabl_mesg();
//             break;
////////////////////////////////////////////////////////////////////////////////////////////////////
        case IN_LOBBY:
            sprintf( buf, "[slobb|" );
            *x++ = '0'+ (int)( --p_in_lob / 10 ); /* SIDE EFFECT: decremented p_in_lob */
            *x++ = '0'+ ( p_in_lob % 10 );
            *x++ = '|';
            r = &lobby_head;
            while( r ) {
                if( r->who == player + i ) {
                    #ifdef DEBUG_BARF
                        BARF( "Removing Player From Queue", i );
                    #endif
                    r->who = r->next->who;
                    temp = r->next;
                    r->next = r->next->next;
                    if( temp ) free( temp );
                }/* End Remove Player If */
                if( r->who ) {
                    sprintf( x, "%s,", r->who->name );
                    x += 9;
                }/* End Current Player Exists If */
                lobby_last = r;
                r = r->next;
            }/* End r While */
            *(x-1) = ']'; *x = 0;
            broadcast( buf, 10+(9*p_in_lob) );
            /* NO BREAK */
        default:
            init_player( i );
    }/* End player status Case */

}/* End remove_player Func */


/******************************************************************************/
/* Function:    send_tabl_mesg                                                */
/******************************************************************************/
void send_tabl_mesg( void )
{
    char buf[126], * x = &buf[7];
    int i = 0;

    sprintf( buf, "[stabl|" );
    while( table[i] ) {
        assert( isalpha(table[i]->status) );
        assert( isdigit(table[i]->strikqt) );
        sprintf( x, "%c%c:%s:%c%d,",    tolower(table[i]->status),
                                        table[i]->strikqt,
                                        table[i]->name,
                                       (table[i]->cardqt > 9)? '1':'0',
                                        table[i]->cardqt % 10         );
        x += 3+8+3+1; ++i;
    }/* End table[i] While */
    while( i++ < MAX_PPPLAYING ) {
        sprintf( x, "%c0:        :00,", IS_EMPTY );
        x += 3+8+3+1;
    } /* Empty Seat While */
    sprintf( x-1, "|%c%c,%c%c,%c%c,%c%c|%c]", //TODO: faster using x?
        '0' + (int)( table_top[0] / 10 ), '0' + ( table_top[0] % 10 ),
        '0' + (int)( table_top[1] / 10 ), '0' + ( table_top[1] % 10 ),
        '0' + (int)( table_top[2] / 10 ), '0' + ( table_top[2] % 10 ),
        '0' + (int)( table_top[3] / 10 ), '0' + ( table_top[3] % 10 ),
        hand_status
    );

    broadcast( buf, 126 );
}/* End send_tabl_mesg Func */


/******************************************************************************/
/* Function:    send_slobb_mesg                                               */
/******************************************************************************/
void send_slobb_mesg( void )
{
    char buf[MAX_SMESG_LEN+1], * x = &buf[7]; /* Buffer for Outgoing Messages */
    PQUEUE* r = &lobby_head;                  /* Lobby Queue Traversal Ptr */

    sprintf( buf, "[slobb|" );
    *x++ = '0'+ (int)( p_in_lob / 10 );
    *x++ = '0'+ ( p_in_lob % 10 );
    *x++ = '|';

    while( r->who ) {
        sprintf( x, "%s,", r->who->name );
        x += 9;
        r = r->next;
    }/* End who in lobby For */
    *(x-1) = ']'; *x = 0;
    broadcast( buf, ( x - buf ) );//broadcast( buf, 10+(9*p_in_lob) );

}/* End send_slobb_mesg Func */



/************************************EOF***************************************/
