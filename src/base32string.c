/*
* Encodes arbitrary byte arrays as case-sensitive base-32 strings
*
* Credit for this code goes to James Cuff for the stadalone java port of 
* the android google-authenticator client.
*
* http://blog.jcuff.net/2011/02/cli-java-based-google-authenticator.html
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <rsh/base32string.h>
#include <rsh/debug.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*\
| Globals
\*---------------------------------------------------------------------------*/

const char GA_DIGITS[GA_MASK+1] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5',
	'6', '7'
};

/*---------------------------------------------------------------------------*\
| Internal functions
\*---------------------------------------------------------------------------*/

// TODO: rewrite so that it does not modify the string. Copy to a new string,
// and modify that.
static char* ga_trim( char* _str ) // warning, this modifies the string!
{
	debugl( LEVEL_IFUNC, "Entering function with argument %s = \"%s\"\n", stringify(_str), _str );
	char* end;
	char* p1;
	char* p2;

	// Leading spaces
	debugl( LEVEL_IDEV, "Trimming leading whitespaces\n" );
	while( isspace(*_str) )
		_str++;
	
	debugl( LEVEL_IDEV, "Leading whitespace trimmed! Current string is now \"%s\"\n", _str );

	if( *_str == 0 )
	{
		debugl( LEVEL_IDEV, "The entire string was whitespace! Returning empty string.\n" );
		return ""; // it was all whitespaces
	}

	// Trailing spaces
	debugl( LEVEL_IDEV, "Trimming trailing spaces\n" );
	end = _str + (strlen(_str) - 1);
	while( end > _str && isspace( *end ) ) 
		end--;
	
	// set the new ending
	*(end+1) = 0; // is this really needed, since we set another null terminator later?
	debugl( LEVEL_IDEV, "Finished looking through trailing whitespace! String is now \"%s\"\n", _str );

	// remove hyphens and spaces, if any
	debugl( LEVEL_IDEV, "Removing characters from string and converting to uppercase\n" );
	p1 = _str;
	p2 = _str;
	while( *p1 != 0 )
		if( (*p1) == '-' || (*p1) == ' ' ) ++p1;
		else *p2++ = toupper( *p1++ );
	*p2 = 0;

	debugl( LEVEL_IFUNC, "Exiting function with return value \"%s\"\n", _str );
	
	return _str;
}

#if defined( RSHDEBUG )
static char* getBitString( ga_byte a )
{
	debugl( LEVEL_IFUNC, "Entering function\n" );
	char* resp = malloc( sizeof(char)*8 );

	ga_byte positions[8] =
	{
		0x1,  // 00000001
		0x2,  // 00000010
		0x4,  // 00000100
		0x8,  // 00001000
		0x10, // 00010000
		0x20, // 00100000
		0x30, // 01000000
		0x40  // 10000000
	};

	for( int i = 7; i >= 0; i-- )
		resp[7-i] = (a&positions[i])?'1':'0';

	debugl( LEVEL_IFUNC, "Exiting function with value %s\n", resp );
	return resp;
}
#endif

/*---------------------------------------------------------------------------*\
| Public functions
\*---------------------------------------------------------------------------*/

char* ga_encode( ga_byte* _data, size_t* _length )
{
	debugl( LEVEL_FUNC, "Entering function with data length %zu\n", *_length );

	if( _data == NULL || _length == NULL || *_length >= (1 << 28) )
	{
		debugl( LEVEL_WARNING, "[WARN] Illegal arguments!\n" );
		return NULL;
	}

	size_t outlen = ( *_length * 8 + GA_SHIFT - 1) / GA_SHIFT;
	char*  result = malloc( sizeof(char)*outlen );
	int resultpos = 0;
	int buffer = _data[0];
	int next = 1;
	int bitsLeft = 8;

	while( bitsLeft > 0 || next < *_length )
	{
		debugl( LEVEL_INSANE, "(%d out of %zu) %d bits left to process.\n", next, *_length, bitsLeft );
		if( bitsLeft < GA_SHIFT )
		{
			if( next < *_length )
			{
				// there might be a problem with this section of code
				buffer <<= 8;
				buffer |= (_data[next++] & 0xff);
				debugl( LEVEL_INSANE, "Now looking at buffer (%u '%s')\n", _data[next], getBitString( _data[next]&0xff ) );
				bitsLeft += 8;
			}
			else
			{
				int pad = GA_SHIFT - bitsLeft;
				buffer <<= pad;
				bitsLeft += pad;
			}
		}
		int index = GA_MASK & (buffer >> (bitsLeft - GA_SHIFT));
		bitsLeft -= GA_SHIFT;
		debugl( LEVEL_INSANE, "Setting position %d to %c\n", resultpos, GA_DIGITS[index] );
		result[resultpos++] = GA_DIGITS[index];
	}
	result[outlen+1] = 0;

	debugl( LEVEL_FUNC, "Exiting function with value %s\n", result );
	return result;
}

ga_byte* ga_decode( char* _encoded, size_t* _length )
{
	debugl( LEVEL_FUNC, "Entering function with encoded string %s\n", _encoded );
	/* Local variables */
	ga_byte* result; // Final result to return
	char* encoded_trimmed; // Stored encoded string
	size_t inlen; // stored input length
	size_t outlen; // stored output length
	int buffer = 0;
	int next = 0;
	int bitsLeft = 0;

	encoded_trimmed = ga_trim( _encoded );
	inlen = strlen( encoded_trimmed );
	debugl( LEVEL_DEV, "Encoded string is \"%s\" (len %zu)\n", encoded_trimmed, inlen );

	if( !inlen ) 
	{
		debugl( LEVEL_WARNING, "[WARN] String is empty after encoding!\n" );
		return NULL;
	}

	outlen = inlen * GA_SHIFT / 8;
	*_length = outlen;
	debugl( LEVEL_DEV, "Setting output length to %zu\n", *_length );
	result = malloc( sizeof(ga_byte)*outlen );
	
	for( int i = 0; i < inlen; i++ )
	{
		for( int j = 0; j <= GA_MASK; j++ )
		{
			if( encoded_trimmed[i] == GA_DIGITS[j] )
			{
				debugl( LEVEL_DEV, "Letter found at index %d (%c)\n", j, GA_DIGITS[j] );
				buffer <<= GA_SHIFT;
				buffer |= j & GA_MASK;
				bitsLeft += GA_SHIFT;
				if( bitsLeft >= 8 )
				{
					result[next++] = (ga_byte)(buffer >> (bitsLeft-8));
					bitsLeft -= 8;
					debugl( LEVEL_DEV, "Wrote new byte value at position %d: %u\n", next-1, result[next-1] );
				}
				break;
			}
			if( j == GA_MASK ) // we've gone through everything, so there's an error
			{
				debugl( LEVEL_WARNING, "[WARN] Illegal characters found!\n" );
				debugl( LEVEL_FUNC, "Unsuccessfully exiting function\n" );
				free( result );
				return NULL;
			}
		}
	}

	debugl( LEVEL_FUNC, "Successfully exiting function\n" );
	return result;
}
#ifdef __cplusplus
}
#endif
