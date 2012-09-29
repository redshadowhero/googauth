#ifndef RSH_DEBUG_H
#define RSH_DEBUG_H

#define LEVEL_CRITICAL 0
#define LEVEL_ERROR    1
#define LEVEL_WARNING  2
#define LEVEL_INFO     3
#define LEVEL_DEV      4
#define LEVEL_FUNC     5
#define LEVEL_IDEV     6 // internal dev
#define LEVEL_IFUNC    7 // internal functions
#define LEVEL_INSANE   12

#if defined( RSHDEBUG )

#include <stdio.h>

#if !defined( LEVEL )
#define LEVEL 0
#endif

/*
* General purpose debug messages
*/
#define debugf( ... ) \
	do { \
		fprintf( stderr, "[%s/%s():%d] ", __FILE__, __func__, __LINE__ ); \
		fprintf( stderr, __VA_ARGS__ ); \
	} while( 0 )

/*
* Allows for more fine control of when messages are printed out
*/
#define debugl( level, ... ) \
	do {\
		if( (level) < LEVEL ) \
			debugf( __VA_ARGS__ ); \
	} while( 0 )

/*
* Stringify variable names
*/
#define stringify( x ) #x

#else

#define debugf( ... ) 
#define debugl( ... )
#define stringify( x )

#endif /* if !defined( LEVEL ) */

#endif /* RSH_DEBUG_H */
