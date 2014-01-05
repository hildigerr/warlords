/* $Id: sgui.c,v 1.2 2013/12/10 01:40:16 moonsdad Exp $ */

/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      sgui.c    --  Server GUI Functions                                    *
 ******************************************************************************/
#include "sgui.h"


/******************************************************************************/
/*  FUNCTION: curse_cleanup                                                   */
/*  Wrapper for endwin. To attach at exit. Also moves cursor to lower left.   */
/******************************************************************************/
void curse_cleanup( void )
{
    mvcur( 0, COLS-1, LINES-1, 0 );
    endwin();
}/* End curse_cleanup Func */


/******************************************************************************/
/*  FUNCTION:   servergui                                                     */
/*  PARAMETERS: char        t               -- key for gui aspect switch      */
/*              char*       data            -- buffer of data if needed       */
/*  RETURNS:    bool                        -- assign this to use_gui         */
/*  t:  SGUI_INIT -- Initialize curses and returne success value. May Abort.  */
/*      SGUI_TABL -- Update Table -- No Data Needed                           */
/*      SGUI_LOBB -- Update Lobby -- Data is slobb message before broadcast.  */
/*      SGUI_CHAT -- Display Chat -- Data is schat message before broadcast.  */
/*      SGUI_ZAPH -- Zap Hand     -- Data is string packed with current total */
/*                                   hand count.                              */
/* The zap hand data string's size is regulated externally.                   */
/******************************************************************************/
bool servergui( char t, char* data )
{
    static WINDOW* cwin[SUB_WINDOW_QT];
    int i;//, x, y;
    #define LOBBY_H 4+2
    #define CHATW_H 8


    switch( t ) {
        case SGUI_INIT:
//             if( (initscr() == ERR) ); else
            initscr(); /* Assumed Success */
            if( /* Create SubWins */
              ( !(cwin[SGUI_TABLE] = GenSubwin( MAX_PPPLAYING, 1 )) )
            ||( !(cwin[SGUI_LOBBY] = GenSubwin( LOBBY_H, MAX_PPPLAYING+1 )) )
            ||( !(cwin[SGUI_CHATW] = GenSubwin( CHATW_H, MAX_PPPLAYING+LOBBY_H+1 )) )
            ||( !(cwin[SGUI_CMD_W] = GenSubwin( LINES - (MAX_PPPLAYING+LOBBY_H+CHATW_H+2),
                                                LINES - (MAX_PPPLAYING+LOBBY_H+CHATW_H+2) )) )
            ) endwin();

            else { /* All Good */
                atexit( curse_cleanup ); /* Assumed Success */
                signal( SIGINT, sig_quit );
                scrollok( cwin[SGUI_CHATW], true );
                box( cwin[SGUI_LOBBY], ' ', '=' );
                move(0,0);
                addstr("  NAME   [ix] S: 3 4 5 6 7 8 9 0 J Q K A 2 (TT) Score  Strikes   [Hands:" ); /**SGUI_HEADING_**/
                move(0, COLS-1);
                addch(']');
                move( MAX_PPPLAYING+LOBBY_H+CHATW_H+1, 0 );
                for( i = 0; i < COLS; i++ ) addch('=');
                break;
            }/* End Success Else */

            ERROR( "GUI Initialization", "Reverting to non-GUI mode.", 0 );
            return false; break;
        case SGUI_TABL:
            wmove( cwin[SGUI_TABLE], 0,0 );
            for( i = 0; i < MAX_PPPLAYING; i++ ) {
                if( table[i] )
                    wprintw( cwin[SGUI_TABLE], "%s [%2d] %c:%2d%2d%2d%2d%2d%2d%2d%2d%2d%2d%2d%2d%2d (%2d) %5d  %c:%2d%2d%2d  [%d]\n",
                        table[i]->name, table[i]-player, table[i]->status,
                        table[i]->count[0], table[i]->count[1], table[i]->count[2], table[i]->count[3],
                        table[i]->count[4], table[i]->count[5], table[i]->count[6], table[i]->count[7],
                        table[i]->count[8], table[i]->count[9], table[i]->count[10], table[i]->count[11],
                        table[i]->count[12],
                        table[i]->cardqt, table[i]->score, table[i]->strikqt,
                        table[i]->strikelog[0], table[i]->strikelog[1], table[i]->strikelog[2],
                        table[i]->hands
                    );
                else wprintw( cwin[SGUI_TABLE], "\n" );
            }/* End Table Player For */
            wrefresh( cwin[SGUI_TABLE] );
            break;
        case SGUI_LOBB:
            wclear( cwin[SGUI_LOBBY] );
            box( cwin[SGUI_LOBBY], ' ', '=' );//TODO: Setup aditional subwin
            wmove( cwin[SGUI_LOBBY], 1,1 );
            wprintw( cwin[SGUI_LOBBY], "%s\b ", &data[10] );
            wrefresh( cwin[SGUI_LOBBY] );
            break;
        case SGUI_CHAT:
            wprintw( cwin[SGUI_CHATW], "%s\b \n", &data[7] );
            wrefresh( cwin[SGUI_CHATW] );
            break;
        case SGUI_ZAPH:
            move( 0, 72 );
            addstr(data);
            break;
        case SGUI_CCMD:
            //wclear( cwin[SGUI_CMD_W] );
            clrtoeol();
            wrefresh( cwin[SGUI_CMD_W] );
            break;
        default:
            return false;
    }/* End t Switch */
//     wmove( cwin[SGUI_CMD_W], 0, 0 );
    move( LINES-1, 0 );
    refresh();
    return true;
}/* End servergui Func */


/******************************************************************************/
/*  FUNCTION:   cmd_exec                    Execute Server Command            */
/*  PARAMETER:  BUFFER      cmd             -- Input Buffer for the Command   */
/*  RETURNS:    bool                        -- Validity of Input Command char */
/*  WARNING: No error checking! Expects valid command parameters. TODO        */
/*  Checks the first character for escape character signaling the prescence   */
/*  of a valid server command. The next character is what distinguishes the   */
/*  specific command:   X -- Shut Down the Server     "\X"                    */
/*                      s -- Strike a Player          "\s i"                  */
/*                      b -- Ban a Player from Chat   "\b i"                  */
/*                      k -- Kick a Player            "\k i" or "\K i"        */
/*                      u -- UnBan a Player           "\u i"                  */
/*                      f -- Forgive Player 1 Strike  "\f i"                  */
/******************************************************************************/
bool cmd_exec( BUFFER cmd )
{
    int ix, d;
    char* slicer;
    BUFFER admin_chat;

    if(( cmd.buf[0] != '/' )&&( cmd.buf[0] != '\\' )) return false;

    switch( cmd.buf[1] ){
        case 'X': /* Shut Down Server */
            exit( 0 );
            break;
        case 's': /* Strike a Player */
            if( cmd.n < 7 ) return false; /* TODO: Verify for Real */
            slicer = &cmd.buf[3];// TODO: UNIFY AND MAKE ROBUST
            while( isdigit(*slicer) ) ++slicer;
            *slicer = 0;
            ix = atoi( &cmd.buf[3] );
            *(slicer+3) = 0;
            d = atoi( slicer+1 );
            if( strike( ix, d ) )
                remove_player( ix, cmd.buf, gfds );
            break;
        case 'b': /* Ban From Chat */
            if( cmd.n < 4 ) return false; /* TODO: Verify for Real */
            slicer = &cmd.buf[3];
            while( isdigit(*slicer) ) ++slicer;
            *slicer = 0;
            ix = atoi( &cmd.buf[3] );
            player[ix].ban = true;
            sprintf(cmd.buf, "[schat|SERVER  |%s is banned from chat!]", player[ix].name );
            broadcast( cmd.buf, 46 );
            break;
        case 'k': case 'K': /* Kick Client */
            if( cmd.n < 4 ) return false; /* TODO: Verify for Real */
            slicer = &cmd.buf[3];
            while( isdigit(*slicer) ) ++slicer;
            *slicer = 0;
            ix = atoi( &cmd.buf[3] );
            remove_player( ix, cmd.buf, gfds );
            break;
        case 'u': /* UnBan From Chat */
            if( cmd.n < 4 ) return false; /* TODO: Verify for Real */
            slicer = &cmd.buf[3];
            while( isdigit(*slicer) ) ++slicer;
            *slicer = 0;
            ix = atoi( &cmd.buf[3] );
            player[ix].ban = false;
            sprintf(cmd.buf, "[schat|SERVER  |%s is allowed to chat.]", player[ix].name );
            broadcast( cmd.buf, 45 );
            break;
        case 'f': /* Forgive A Strike */
            if( cmd.n < 4 ) return false; /* TODO: Verify for Real */
            slicer = &cmd.buf[3];
            while( isdigit(*slicer) ) ++slicer;
            *slicer = 0;
            ix = atoi( &cmd.buf[3] );
            if( !isdigit(--player[ix].strikqt) ) player[ix].strikqt = '0';/* TODO: Update List */
            Write( player[ix].socketfd, "[schat|SERVER  |You Are Forgiven 1 Strike.]", 43 );
            break;
        case 'c': /* Chat */
            if( cmd.n < 4 ) return false; /* TODO: Verify for Real */
            snprintf( admin_chat.buf, MAX_SMESG_LEN, "[schat|SERVER  |%s]", cmd.buf+2 );
            admin_chat.n = cmd.n + 17 - 2;
            broadcast( admin_chat.buf, admin_chat.n );
            break;
        case 'w':/* whisper *///w i mesg
            if( cmd.n < 6 ) return false; /* TODO: Verify for Real */
            slicer = &cmd.buf[3];
            while( isdigit(*slicer) ) ++slicer;
            *slicer++ = 0; /* Increment for snprintf */
            ix = atoi( &cmd.buf[3] );
            snprintf( admin_chat.buf, MAX_SMESG_LEN, "[schat|SERVER  |%s]", slicer );
            admin_chat.n = cmd.n + 17 - 2;
            Write( player[ix].socketfd, admin_chat.buf, admin_chat.n );
            break;
        default:
            servergui(SGUI_CHAT, "       Unknown Command " );
            return false;
    } servergui( SGUI_CCMD, NULL );
    return true;
}/* End cmd_exec Func */


/************************************EOF***************************************/
