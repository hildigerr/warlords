/* $Id: client.h,v 1.36 2013/12/05 04:01:58 hildigerr Exp $ */

/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      client.h    --  Header File for the Client                            *
 ******************************************************************************/

#pragma once

#include "game.h"
#include <gtk/gtk.h>
#include <arpa/inet.h>

/* Precompiler Constants */
#define DEFAULT_WINDOW_SIZE 869,581
//#define WINDOW_SIZE_HAND 500,500
#define DISPLAY_MAXLINE 4

/* WIDGET ARRAY ACCESS CONSTANTS */
#define ADD_PLAYERS         0
#define TABLE_TOP           1
#define HAND                2
#define PLAY                3
#define SWAP                4
#define MAX_XTERN_WIDGETS   5
//TODO: Minimize When Ready
#define MAIN_WINDOW 0
#define MAIN_FRAME  1
#define COM_FRAME   2
#define TABLE_FRAME 3
#define HAND_WINDOW 4
#define STAT_VIEW   5
#define STAT_FRAME  6
#define TABLE_TREE  7
#define LOBBY_TREE  8
#define MESG_BOX    9
#define MESG_FIELD  10
#define CHAT_FIELD  11
#define CHAT_BOX    12
#define SAY_BTN     13
#define PLAY_WINDOW 14
#define PLAY_FRAME  15
#define BTN_FRAME   16
#define PLAY_BUTTON 17
#define PASS_BUTTON 18
#define CLR_BUTTON  19
#define MAX_WIDGETS 20

/* Enums, Structures, and Typedefs */
enum { STATUS_COL, NAME_COL, CARD_QT_COL, LIST_COL_QT };
enum { LOBBYL, TABBLL, PLAYER_LIST_QT };


typedef struct {
    BUFFER input;
    BUFFER output; /* To Client Display */
    /* History */
    BUFFER slobb;
    BUFFER stabl;
    BUFFER shand;
    char* cards[MAX_PHAND_LEN];
} IODAT;

/* Global Variables */
EXTERN bool auto_mode                   INIT(TRUE);
EXTERN BUFFER transmit; /* To Server */
EXTERN GtkTextBuffer* display_buf;
EXTERN char my_name[MAX_PNAME_LEN+1]    INIT("Vergaray");
EXTERN GtkListStore* playerlist[PLAYER_LIST_QT];
EXTERN GtkWidget* Widget[MAX_XTERN_WIDGETS];
EXTERN bool iam_playing                 INIT(FALSE);
EXTERN bool iam_joined                  INIT(FALSE);
EXTERN int gsd                          INIT(1);
EXTERN char swap_cd[2][3];
EXTERN bool iam_warlord                 INIT(FALSE);
EXTERN bool is_myturn                   INIT(FALSE);
EXTERN bool used_hand                   INIT(TRUE);

/* PROTOTYPES */
#define SAY(x,y) gtk_text_buffer_insert_at_cursor( display_buf, x, y )
gboolean click_hcard( GtkWidget *event_box, GdkEventButton *event, gpointer data );
gboolean click_cswap( GtkWidget *event_box, GdkEventButton *event, gpointer data );
gboolean click_pbtn( GtkWidget *button, gpointer data );
gboolean chat_key_press( GtkWidget* widget, GdkEventKey* key, gpointer data );
/* clichan.c */
gboolean getmsg( GIOChannel* source, GIOCondition condition, gpointer data );
inline void adjust_display_buf_view( void );
void Have_Complete_Mesg( IODAT* data );
void send_mesg( gpointer b, GIOChannel* Sd );
void swap_message( unsigned qt );
/* clibuf.c */
void parse_chat( gpointer b, GtkTextView* txt );
void update_hand( IODAT* data );
bool update_table( IODAT* data );
void update_lobby( IODAT* data );
/* cligui.c */
GdkPixbuf* load_pixbuf( const gchar * filename );
GtkWidget* create_image( const gchar * filename, GCallback func, gpointer data );
void widget_destroy( GtkWidget* widget, gpointer data );

/************************************EOF***************************************/
