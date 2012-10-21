// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <google/google-authenticator.h>

#define _BSD_SOURCE // for usleep
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>

#define   VERSIONMAJOR     1
#define   VERSIONMINOR     1
#define   BUGFIX           1
#define   PINLENGTH        6    // The number of digits a pin is supposed to be
#define   INTERVALLENGTH   30   // The number of seconds in any one given interval
#define   HEADER ":----------------------------:--------:\n" \
                 ":       Code Wait Time       :  Code  :\n" \
                 ":----------------------------:--------:\n"

// getopt args
#define OPTS "k:f:nl"
#define USAGE \
	"Usage: %s [OPTIONS...] [keyfile | key]\n" \
	"\t-k, --key=KEY\t\tThe key to use\n" \
	"\t-f, --file=FILE\t\tThe location of the keyfile\n" \
	"\t-n, --no-interface\tTurn the interface off, printing only the latest pin on a new interval\n" \
	"\t-l, --no-loop\t\tDon't loop and print the previous key, current key, and next key before exiting\n" \
	"\tOptionally, you may specifiy the keyfile or the key with no flags.\n" \
	"Version %d.%d.%d\n"

// global
static int   nointerface = 0;
static int   noloop      = 0;
static char* exename     = NULL;
static char* argstr      = NULL;
static int   sigintGiven = 0;

struct option long_options[] =
{
	{ "key",            required_argument,   0,        'k' },
	{ "file",           required_argument,   0,        'f' },
	{ "no-interface",   no_argument,         0,        'n' },
	{ "no-loop",        no_argument,         0,        'l' },
	{ 0,                0,                   0,         0  }
};


void printUsage()
{
	printf( USAGE, exename, VERSIONMAJOR, VERSIONMINOR, BUGFIX );
}


// end getopts variables

#if (defined ( _WIN32 ) || defined ( _WIN64 )) && !defined (LINUX_HOST)

#include <windows.h>

void usleep( int waitTime )
{
    __int64 time1 = 0, time2 = 0, freq = 0;

    QueryPerformanceCounter((LARGE_INTEGER *) &time1);
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    do {
        QueryPerformanceCounter((LARGE_INTEGER *) &time2);
    } while((time2-time1) < waitTime);
}

#endif

void sigHandler( int sig )
{
	signal( sig, SIG_IGN );

	sigintGiven = 1;

	signal( SIGINT, sigHandler );
}

char* padOutput( int pin )
{
	char   pinstr[12]; 
	char*  rv      = malloc( sizeof(char)*7 ); rv[6] = 0;
	size_t pinlen  = sprintf( pinstr, "%d", pin );

	// We need to make sure the length isn't greater than the accepted pin
	// length. The behavior is undefined for situations in which the generated
	// pin is greater in length than 6, so I just return the number.
	if( pinlen == PINLENGTH || pinlen > PINLENGTH ) return strcpy( rv, pinstr );

	// Pad the return string with the appropriate number of zeros
	for( int i = 0; i < PINLENGTH-pinlen; i++ ) rv[i] = '0';
	rv[PINLENGTH-pinlen] = 0;

	// Concatenate the two strings and return.
	strcat( rv, pinstr );
	return rv;
}

unsigned long getCurrentInterval()
{
	return (unsigned long)(time(NULL)/INTERVALLENGTH);
}

void pinLoop( char* key )
{
	// TODO: sigint support so we can clean up the input pin
	time_t nsec = time(NULL)+1;     // next second
	unsigned long nextInterval = getCurrentInterval(); // next interval to update at
	int count = 0; // count for the interface
	int pin = 0;
	char* pinstr = NULL; // current pin to print out
	
	if( !nointerface ) printf( HEADER );

	while( 1 )
	{
		if( time(NULL) >= nsec )
		{
			if( getCurrentInterval() < nextInterval ) 
			{
				if( !nointerface )
				{
					printf( "." );
					count++;
					fflush( stdout );
				}

			}
			else
			// we've moved on to the next interval, and have to update the pin
			{
				nextInterval = getCurrentInterval()+1;
				if( !nointerface )
				{
					for( int i = count+1; i < 30; i++ )
						printf( "+" );
				}
				count = 0;

				if( pinstr ) free( pinstr );
				pin = generateCode( key, getCurrentInterval() );
				pinstr = padOutput( pin );
				if( !nointerface ) printf( ": %s :\n", pinstr );
				else printf( "%s\n", pinstr );
			}
			nsec = time(NULL)+1;
		}
		
		if( sigintGiven == 1 )
		{
			if( pinstr ) free( pinstr );
			printf( "\n" ); // just so we start on a fresh line.
			fflush( stdout );
			return;
		}
		usleep( 900000 );
	}

}

void parseOpts( int argc, char** argv )
{
	int c;
	int option_index = 0;
	FILE* fd;
	char* key = malloc( sizeof(char)*20 );
	char* pin = NULL;
	exename = argv[0];

	while( 1 )
	{
		c = getopt_long( argc, argv, OPTS, long_options, &option_index );
		if( c == -1 ) break;

		switch( c )
		{
			case 'k':
				if( argstr )
				{
					printUsage();
					exit( 1 );
				}
				argstr = optarg;
			break;
			case 'f':
				if( argstr )
				{
					printUsage();
					exit( 1 );
				}
				argstr = optarg;
			break;
			case 'n':
				nointerface = 1;
			break;
			case 'l':
				noloop = 1;
			break;
		}
	}
	
	while( optind < argc ) // just cycle through and get the last argument
	{
		argstr = argv[optind];
		optind++;
	}


	if( argstr == NULL )
	{
		free( key );
		exit( 4 );
	}

	// check if argstr is a file
	if( (fd = fopen( argstr, "r" )) )
		fscanf( fd, "%s", key );
	else // try as the key
		key = argstr;

	if( !noloop ) pinLoop( key );
	else
	{
		unsigned long interval = getCurrentInterval();

		pin = padOutput( generateCode( key, interval-1 ) ); printf( "%s, ", pin );
		free( pin );
		pin = padOutput( generateCode( key, interval ) ); printf( "%s, ", pin );
		free( pin );
		pin = padOutput( generateCode( key, interval+1 ) ); printf( "%s\n", pin );
		free( pin );
	}
	if( key != argstr ) free( key ); // argstr comes from argv; best not to free it
}

int main( int argc, char* argv[] )
{
	signal( SIGINT, sigHandler );
	exename = argv[0];
	if( argc < 2 )
	{
		printUsage();
		return 4;
	}
	parseOpts( argc, argv );

	return 0;
}
