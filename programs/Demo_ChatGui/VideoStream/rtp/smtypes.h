/*****************************************************************************
 * smtypes.h: a Basic types definitions
*****************************************************************************
* $Id: smtypes.h 10101 2005-03-02 16:47:31Z robux4 $
*
*****************************************************************************

*****************************************************************************/

#ifndef _SMTYPES_H_
#define _SMTYPES_H_

/*****************************************************************************
* Basic types definitions
*****************************************************************************/
#if defined( HAVE_STDINT_H )
#   include <stdint.h>
#elif defined( HAVE_INTTYPES_H )
#   include <inttypes.h>
#elif defined( SYS_CYGWIN )
#   include <sys/types.h>
/* Cygwin only defines half of these... */
typedef u_int8_t            uint8_t;
typedef u_int16_t           uint16_t;
typedef u_int32_t           uint32_t;
typedef u_int64_t           uint64_t;
#else
/* Fallback types (very x86-centric, sorry) */
typedef unsigned char       uint8_t;
typedef signed char         int8_t;
typedef unsigned short      uint16_t;
typedef signed short        int16_t;
typedef unsigned int        uint32_t;
typedef signed int          int32_t;
# if defined( _MSC_VER ) || defined( UNDER_CE ) || ( defined( WIN32 ) && !defined( __MINGW32__ ) )
typedef unsigned __int64    uint64_t;
typedef signed __int64      int64_t;
#   else
typedef unsigned long long  uint64_t;
typedef signed long long    int64_t;
#   endif
typedef uint32_t            uintptr_t;
typedef int32_t             intptr_t;
#endif

/* Systems that don't have stdint.h may not define INT64_MIN and
INT64_MAX */
#ifndef INT64_MIN
#define INT64_MIN (-9223372036854775807LL-1)
#endif
#ifndef INT64_MAX
#define INT64_MAX (9223372036854775807LL)
#endif

#ifndef ERR_OK
#define ERR_OK 0
#endif
#ifndef ERR_STOP
#define ERR_STOP -1
#endif

// #ifndef OK
// #define OK 1
// #endif
// 
// #ifndef STOP
// #define STOP 0
// #endif


#define RNOK(x)     {if((x)!=ERR_OK) return ERR_STOP;}
#define ROTV(x,y)    {if(x) return (y);}
#define RONULL(x,y) {if((x)==(NULL)) {return (y);}

#if defined (_DEBUG)
#define AOF( exp )     \
{                      \
    if( !( exp ) )       \
{                    \
    assert( 0 );       \
}                    \
}
#else
#define AOF( exp )
#endif

#if !defined (INLINEHEADER)
#ifdef WIN32
#define INLINEHEADER __inline 
#else
#define INLINEHEADER static inline 
#endif
#else
#define INLINEHEADER
#endif

#define LMCASTD_ERR_BASE        0
#define LMCASTD_ERR_GENERIC     (LMCASTD_ERR_BASE - 1)
#define LMCASTD_ERR_NULL_PTR    (LMCASTD_ERR_BASE - 10)


#endif //#ifndef _SMTYPES_H_
