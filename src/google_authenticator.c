/*
* Generates short passcodes for challenge-response protocols, or as
* timeout passcodes.
*
* Credit for this code goes to James Cuff for the stadalone java port of
* the android google-authenticator client.
*
* http://blog.jcuff.net/2011/02/cli-java-based-google-authenticator.html
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <rsh/debug.h>
#include <rsh/base32string.h>
#include <rsh/google_authenticator.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


/*---------------------------------------------------------------------------*\
| Internal functions
\*---------------------------------------------------------------------------*/

static char* padOutput( int _value, ga_passcodeGenerator* _pg )
{
	debugl( LEVEL_IFUNC, "Entering function with value %s = %d\n", stringify( _value ), _value );
	int plen = _pg->codelength; // keep lookups small
	char* _rv = malloc( sizeof(char)*plen+1 );
	char valuestr[12];
	int len = 0;
	len = sprintf( valuestr, "%d", _value );
	debugl( LEVEL_INSANE, "Turned %d into string \"%s\"\n", _value, valuestr );

	if( len >= plen ) return strcpy( _rv, valuestr );

	for( int i = len; i < plen; i++ )
	{
		_rv[i-len] = '0';
		if( i == plen-1 ) // clean up and append the rest of the string
		{
			_rv[i-len+1] = '\0';
			strcat( _rv, valuestr );
		}
	}

	debugl( LEVEL_IFUNC, "Exiting function with value \"%s\"\n", _rv );
	return _rv;
}

static ga_byte* getByteArr( void* _ptr, size_t _size )
{
	debugl( LEVEL_IFUNC, "Entering function with address %p and size %zu\n", _ptr, _size );
	ga_byte* _rv = malloc( sizeof(ga_byte)*_size );
	for( int i = 0; i < _size; i++ )
		_rv[i] = ((ga_byte*)_ptr)[i];
	debugl( LEVEL_IFUNC, "Exiting function\n" );
	return _rv;
}

static int hashToInt( ga_byte* _bytes, size_t _size, int _start )
{
	debugl( LEVEL_IFUNC, "Entering function with address %p, size %zu, and starting position %d\n", _bytes, _size, _start );
	int _rv = 0;
	if( _start+4 >= _size ) return _rv;
	for( int i = _start; i < _start+4; i++ )
	{
		_rv |= _bytes[i] & 0xFF;
		if( i != _start+3 ) _rv <<= 8;
	}

	debugl( LEVEL_IDEV, "Generated an int with value of %d\n", _rv );
	return _rv;
}

static long getCurrentInterval( ga_passcodeGenerator* _pg )
{
	debugl( LEVEL_IFUNC, "Entering function\n" );
	long currentTimeSeconds = (long)(time(NULL));
	debugl( LEVEL_IFUNC, "Exiting function with time value %ld\n", currentTimeSeconds );
	return currentTimeSeconds/_pg->intervalperiod;
}

/*---------------------------------------------------------------------------*\
| Functions
\*---------------------------------------------------------------------------*/


void ga_setKey( ga_passcodeGenerator* _pg, ga_byte* _key, size_t _keysize  )
{
	debugl( LEVEL_FUNC, "Entering function\n" );
	_pg->key = _key;
	_pg->keysize = _keysize;
	debugl( LEVEL_FUNC, "Exiting function\n" );
}

char* ga_generateTimeoutCode( ga_passcodeGenerator* _pg )
{
	debugl( LEVEL_FUNC, "Entering function and calling %s\n", stringify( ga_generateResponseCodeFromLong ) );
	return ga_generateResponseCodeFromLong( _pg, getCurrentInterval(_pg) );
}

char* ga_generateResponseCodeFromLong( ga_passcodeGenerator* _pg, long _challenge )
{
	debugl( LEVEL_FUNC, "Entering function with challenge %ld\n", _challenge );
	ga_byte* value = getByteArr( &_challenge, sizeof(long) );
	return ga_generateResponseCodeFromByte( _pg, value, sizeof(long) );
}

char* ga_generateResponseCodeFromByte( ga_passcodeGenerator* _pg, ga_byte* _challenge, size_t _size )
{
	// TODO: fix magic numbers
	debugl( LEVEL_FUNC, "Entering function\n" );
	size_t size = _size; 
	ga_byte* _rchallenge /* reverse challenge */ = malloc(sizeof(ga_byte)*_size);
	for( int i = 0; i < _size; i++ )
		_rchallenge[i] = _challenge[_size-(i+1)];
	ga_byte* hash = _pg->signer( _pg->key, _pg->keysize, _rchallenge, size );

	int offset = hash[19] & 0xF;
	int truncatedHash = hashToInt( hash, (size_t)20, offset ) & 0x7FFFFFFF;
	int pinValue = truncatedHash % GA_PIN_MODULO;
	debugl( LEVEL_INSANE, "Offset: %d, truncatedHash: %d, pinValue: %d\n", offset, truncatedHash, pinValue );
	return padOutput( pinValue, _pg );
}

bool ga_verifyResponseCode( ga_passcodeGenerator* _pg, long _challenge, char* _response )
{
	debugl( LEVEL_FUNC, "Entering function with challege %ld and response %s", _challenge, _response );
	char* expectedResponse = ga_generateResponseCodeFromLong( _pg, _challenge );
	debugl( LEVEL_FUNC, "Exiting function with expected response \"%s\"\n", expectedResponse );
	return (strcmp(expectedResponse, _response))?false:true;
}

bool ga_verifyTimeoutCode( ga_passcodeGenerator* _pg, char* _timeoutcode )
{
	debugl( LEVEL_FUNC, "Entering function with timeoutcode \"%s\"\n", _timeoutcode );
	return ga_verifyTimeoutCodeWithIntervals( _pg, _timeoutcode, GA_ADJACENT_INTERVALS, GA_ADJACENT_INTERVALS );
}

bool ga_verifyTimeoutCodeWithIntervals( ga_passcodeGenerator* _pg, char* _timeoutcode, int _pastintervals, int _futureintervals )
{
	debugl( LEVEL_FUNC, "Entering function with timeoutcode \"%s\", pastinterval %d, and futureinterval %d\n", _timeoutcode, _pastintervals, _futureintervals );
	long currentInterval = getCurrentInterval( _pg );
	char* expectedResponse = ga_generateResponseCodeFromLong( _pg, currentInterval );
	if( !strcmp( expectedResponse, _timeoutcode ) ) return true;

	for( int i = 1; i <= _pastintervals; i++ )
	{
		char* pastResponse = ga_generateResponseCodeFromLong( _pg, currentInterval-i );
		if( !strcmp( pastResponse, _timeoutcode ) ) return true;
	}

	for( int i = 1; i <= _futureintervals; i++ )
	{
		char* futureResponse = ga_generateResponseCodeFromLong( _pg, currentInterval+i );
		if( !strcmp( futureResponse, _timeoutcode ) ) return true;
	}

	return false;
}

#ifdef __cplusplus
}
#endif
