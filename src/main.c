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

#define   PINLENGTH        6    // The number of digits a pin is supposed to be
#define   INTERVALLENGTH   30   // The number of seconds in any one given interval
#define   HEADER ":----------------------------:--------:\n" \
                 ":       Code Wait Time       :  Code  :\n" \
                 ":----------------------------:--------:\n"

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

int main( int argc, const char* argv[] )
{
	// TODO: cli options
	FILE* fd = fopen( argv[1], "r" );
	char* key = malloc( sizeof(char)*18 );
	time_t nsec = time(NULL)+1;     // next second
	unsigned long nextInterval = getCurrentInterval(); // next interval to update at
	int count = 0; // count for the interface
	int pin = 0;
	char* pinstr = NULL; // current pin to print out

	// get the secret from the specified file (~/.google-authenticator by default)
	if( fd )
		fscanf( fd, "%s", key );
	else
	{
		fprintf( stderr, "Can't open file `%s`\n", argv[1] );
		free( key );
		return 2;
	}

	printf( HEADER );

	while( 1 )
	{
		if( time(NULL) >= nsec )
		{
			if( getCurrentInterval() < nextInterval ) 
			{
				printf( "." );
				count++;
				fflush( stdout );
			}
			else
			// we've moved on to the next interval, and have to update the pin
			{
				nextInterval = getCurrentInterval()+1;
				for( int i = count+1; i < 30; i++ )
					printf( "+" );
				count = 0;

				if( pinstr ) free( pinstr );
				pin = generateCode( key, getCurrentInterval() );
				pinstr = padOutput( pin );
				printf( ": %s :\n", pinstr );
			}
			nsec = time(NULL)+1;
		}
		usleep( 900000 );
	}
	return 0;
}
