/* $Id: rmhv_stdlib.c,v 1.5 2013/12/02 10:24:36 hildigerr Exp $ */

/******************************************************************************
 *                             rmhv_stdlib.c                                  *
 * Created by:      Roberto Morrel HildigerR Vergaray                          *
 *                                                                            *
 * Modified for use in CSCI 637 @ WWU -- Fall 2013                            *
 ******************************************************************************/

#include "rmhv_stdlib.h"


/******************************************************************************
 * Function:    _ERROR                                                        *
 * Arguments:   const char* who       String descriptor of failed function    *
 *              const char* barf      String message to dump to stderr        *
 *              int         status    Exit status return code                 *
 *              bool        debug     if true display as non-error            *
 * RETURNS:     int                   The value of status input               *
 * Sends a string to stderr in the form:                                      *
 *                  "\nERROR: who: barf [status]\n\n"                         *
 * If status is 0 then [status] will be ommited from the message.             *
 * If who is NULL then it will be ommited from the message as well.           *
 * If debug is true then the form will become: "\nwho: barf [status]\n\n"     *
 * These options can be combined as needed.                                   *
 * Some common combinations are provided as macros.                           *
 * Suggested Usage:                                                           *
 *      exit( ERROR( who, "Your error message", status ) );                   *
 ******************************************************************************/
inline int _ERROR( const char* who, const char* barf, int status, bool debug )
{
    fprintf( stderr, "\n%s",  (debug)? "":"ERROR: ");
    if( who ) fprintf( stderr, "%s: ", who );
    if( barf ) fprintf( stderr, "%s ", barf );
    (status)? fprintf( stderr, "[%d]\n\n", status ): fprintf( stderr, "\n\n" );
    return status;
}/* end ERROR func */


/******************************************************************************
 * Function:    fcntl_setf                                                    *
 * Arguments:   int         fd          The open file descriptor to modify    *
 *              int         fg          The file descriptor flag(s) to set    *
 * RETURNS:     int                     -fd on errors else rval of fcntl (0)  *
 * Appropreately use fcntl for F_SETFL cmd, to avoid clearing other flags.    *
 ******************************************************************************/
inline int fcntl_setf( int fd, int fg )
{
    int flags;

    if( (flags = fcntl( fd, F_GETFL, 0 )) < 0 )
        return ERROR( "fcntl (F_GETFL)", strerror(errno), -fd );
    flags |= fg;

    flags = fcntl( fd, F_SETFL, flags );/* Reusing flags for Return Value */
    if( flags < 0 ) return ERROR( "fcntl (F_SETFL)", strerror(errno), -fd );

    return flags;
}/* End fcntl_setf Func */

/******************************************************************************/
/* Function:    _Write          -- wrapper for write                          */
/*  Modified from Stephens example: Does not re-write, only asserts proper    */
/*  write count values for debugging. XXX UNUSED XXX                          */
/******************************************************************************/
ssize_t _Write( int fd, const void *buf, size_t count )
{
//     size_t rval = count;
//     size_t n;
    const char* msg = buf;

    /* DEBUGGING: */
    int len = strlen( buf );
    fprintf( stderr, "%s : %d == %d ", msg, len, (int)count );
    assert( len == count );

//     while( count > 0 ) {
//         if( (n = write( fd, msg, count ) ) <= 0 ) {
//             if( errno == EINTR ) n = 0;
//             else return n;
//         }/* End write If */
//         count -= n;
//         msg += n;
//     }/* End count While */
//     return rval;
    return write( fd, buf, count );
}/* End Write Func */

/************************************EOF***************************************/
