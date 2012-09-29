/*
* Encodes arbitrary byte arrays as case-sensitive base-32 strings
*
* Credit for this code goes to James Cuff for the stadalone java port of
* the android google-authenticator client.
*
* http://blog.jcuff.net/2011/02/cli-java-based-google-authenticator.html
*/

#ifndef BASE32STRING_H
#define BASE32STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h> // for uint8_t
#include <string.h> // size_t

/*---------------------------------------------------------------------------*\
| Defines, typedefs, structs, etc
\*---------------------------------------------------------------------------*/
	
#define GA_ALPHABET   "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"
#define GA_MASK       31
#define GA_SHIFT      5

extern const char GA_DIGITS[GA_MASK+1];

typedef int8_t ga_byte;

/*---------------------------------------------------------------------------*\
| Functions
\*---------------------------------------------------------------------------*/

/** Base32 encode a string
* @param[in]    _encoded  The 'encoded' string
* @param[out]   _length   The length of the returned byte array
* @return                 A base32-encoded byte array
*/
ga_byte* ga_decode( char* /*_encoded*/, size_t* /*_length*/ );

/** Decodes a Base32 string.
* @param[in]   _data       The encoded byte arary.
* @param[in,out]  _length  The length of the input data, and the length of the 
*                          returned string.
* @return                  A regular string
*/
char* ga_encode( ga_byte* /*_data*/, size_t* /*_length*/ );

#ifdef __cplusplus
}
#endif

#endif /* BASE32STRING_H */
