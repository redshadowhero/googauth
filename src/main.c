#include <rsh/base32string.h>
#include <rsh/google_authenticator.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <rsh/debug.h>

#include <hmac.h>
#include <sha1.h>

/*****************************************************************************\
* Globals
\*****************************************************************************/

char* secret;
size_t secretlen;

ga_byte* hmacsha1_sign( ga_byte* keybyte, size_t keysize, ga_byte* inbyte, size_t insize )
{
	ga_byte* response = malloc( sizeof(ga_byte)*20 );
	if( !hmac_sha1( (void*)keybyte, keysize, (void*)inbyte, insize, (void*)response ) )
		return response;

	return NULL;
}

char* computepin( char* _secret, size_t _s_len )
{
	char* response = malloc( sizeof(char)*2048 );
	ga_byte* keyBytes;
	size_t keyBytesSize = _s_len;
	ga_passcodeGenerator pg;

	if( secret == NULL || _s_len == 0 ) return strcpy( response, "(\\/) (°,,°) (\\/) Your secret is bad and you should feel bad!" ); 

	keyBytes = ga_decode( _secret, &keyBytesSize );

	ga_setKey( &pg, keyBytes, keyBytesSize );
	pg.codelength = GA_PASSCODE_LENGTH;
	pg.intervalperiod = GA_INTERVAL;
	pg.signer = hmacsha1_sign;

	return ga_generateTimeoutCode( &pg );
}

void run()
{
	static char* previouscode = NULL; 
	static int count = 1; 
	if( previouscode == NULL )
	{
		previouscode = malloc(sizeof(char)*1024);
		previouscode[0] = '\0';
	}

	char* newout = computepin( secret, secretlen );

	if( !strcmp( newout, previouscode ) )
	{
		printf( "." );
		fflush(stdout);
	}
	else
	{
		if( count < 30 )
			for( int i = count+1; i <= 30; i++ )
				printf( "+" );
		printf( ": %s :\n", newout );
		count = 0;
	}

	strcpy( previouscode, newout );	
	
	count++;
	usleep( 900000 );
}

int main( int argc, const char *argv[] )
{
	if( argc < 2 )
	{
		fprintf( stderr, "You've made a terrible mistake. And should thus feel bad.\n" );
		return 42;
	}

	FILE* file;
	if( (file = fopen( argv[1], "r" )) == NULL ) // They gave us something, but it sure as hell ain't a file.
	{
		fprintf( stderr, "Error: can't open file `%s'\n", argv[1] );
		return 42+1; // because it's one worse than 42!
	}

	secret = malloc( sizeof(char)*2048 );
	if( fscanf( file, "%s", secret ) <= 0 ) fprintf( stderr, "A valid secret could not be read from the file\n" );
	secretlen = strlen( secret );

	printf( ":----------------------------:--------:\n" );
	printf( ":       Code Wait Time       :  Code  :\n" );
	printf( ":----------------------------:--------:\n" );
	
	time_t thefuture = time(NULL)+1;

	while(1)
	{
		if( time(NULL) >= thefuture ) 
		{
			run();
			thefuture = time(NULL)+1;
		}	
	}

	return 0;
}
