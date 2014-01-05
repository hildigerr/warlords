/* $Id: game.c,v 1.2 2013/12/02 10:24:36 moonsdad Exp $ */

/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      game.c  --  Base functions used by both client and server             *
 ******************************************************************************/

#include "game.h"

/******************************************************************************/
/* Function:    buf_copy2                                                     */
/* Parameters:  BUFFER*     dest        Destination for copy (OUTPUT)         */
/*              BUFFER*     source      Coppy from Location  (INPUT)          */
/*              unsigned    pos         Current Position in Source buffer     */
/*              char        term        Terminating chacter for a Message     */
/*              bool        ignore      True if ignoring ASCII < ' '          */
/* Returns:     int             End position of first full message in source  */
/*                               or -1 if empty or partial copy without term. */
/* Copys characters from source into dest, beginning at BUFFER's n.           */
/* WARNING: Expects dest.n to be current location writing to dest.buf         */
/*              and source.n to be beginning location of current message      */
/*          Does not verify BUFFER internals for overflow.                    */
/******************************************************************************/
int buf_copy2( BUFFER* dest, BUFFER* source, unsigned pos, char term, bool ignore )
{
    assert( dest != NULL ); assert( source != NULL );

    while( pos < source->n ) {
        if(( ignore )&&( source->buf[pos] < ' ' )) { ++pos; continue; }
        dest->buf[dest->n] = source->buf[pos];
        if( dest->buf[dest->n++] == term ) return pos;
        else ++pos;
    }/* End pos<n While */
    return -1;
}/* End bufcp Func */
