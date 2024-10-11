/* $Id: rmhv_stdlib.h,v 1.12 2013/12/07 19:03:18 hildigerr Exp $ */

/******************************************************************************
 *                             rmhv_stdlib.h                                  *
 * Created by:      Roberto Morrel HildigerR Vergaray                         *
 *                                                                            *
 * Modified for use in CSCI 637 @ WWU -- Fall 2013                            *
 *                                                                            *
 *    Includes memory allocation macros from the book                         *
 *      "Advanced C Techniques & Applications"                                *
 *          by Sobelman, Gerald E. & Krekelberg, David E.                     *
 ******************************************************************************/

#pragma once

/* Stystem Headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

//TODO: #define NDEBUG when complete
//#define NDEBUG
#include <assert.h>

#ifndef NDEBUG
    #define DEBUG_BARF
#endif

/* Allow All-Cap Booleans */
#ifndef TRUE
    #define TRUE true
#endif
#ifndef FALSE
    #define FALSE false
#endif

/* Global Variable Define/Init Helper */
#ifdef MAIN
    #define EXTERN
    #define INIT(x) = x
#else
    #define EXTERN extern
    #define INIT(x)
#endif

/* Precompiler Constants */
#define READ_ONLY "r"

/* Macros */
#define ERROR(w,b,s) _ERROR(w,b,s,FALSE)
#define Error(m,i)   _ERROR(NULL,m,i,FALSE)
#define BARF(m,i)    _ERROR("BARF",m,i,TRUE)
/* From Sobelman & Krekelberg: */
#define MALLOC(x)   ((x*)malloc(sizeof(x)))
#define CALLOC(n,x) ((x*)calloc(n,sizeof(x)))

/* Prototypes */
inline int _ERROR( const char* who, const char* barf, int status, bool debug );
inline int fcntl_setf( int fd, int fg );
ssize_t _Write( int fd, const void *buf, size_t count );
#ifdef DEBUG_BARF
    #define Write _Write
#else
    #define Write write
#endif

/************************************EOF***************************************/
