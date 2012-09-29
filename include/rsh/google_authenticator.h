/*
* Generates short passcodes for challenge-response protocols, or as
* timeout passcodes.
*
* Credit for this code goes to James Cuff for the stadalone java port of
* the android google-authenticator client.
*
* http://blog.jcuff.net/2011/02/cli-java-based-google-authenticator.html
*/

#ifndef GOOGLE_AUTHENTICATOR_H
#define GOOGLE_AUTHENTICATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rsh/debug.h>
#include <rsh/base32string.h>
#include <string.h>
#include <stdbool.h>
#include <math.h> // pow

/*---------------------------------------------------------------------------*\
| Defines, typedefs, structs, etc
\*---------------------------------------------------------------------------*/

// Default passcode length
#define GA_PASSCODE_LENGTH 6 
#define GA_INTERVAL 30
#define GA_ADJACENT_INTERVALS 1
// probably a better way of doing this, but if the passcode length changes
// then You'll have to modify the numbers here.
#define GA_PIN_MODULO ((int)pow((double)10, (double)GA_PASSCODE_LENGTH))

typedef ga_byte* (*ga_sign)( ga_byte* /* key */, size_t /* keylength */, ga_byte* /* in */, size_t /* inlength */ );

typedef struct _ga_passcodeGenerator
{
	ga_sign signer;
	int codelength;
	int intervalperiod;
	ga_byte* key;
	size_t keysize;
} ga_passcodeGenerator;

/*---------------------------------------------------------------------------*\
| Functions
\*---------------------------------------------------------------------------*/


/** Sets the passcode key
* @param[out] _pg The passcode generator to set the key on
* @param[in] _key The key to set.
* @param[in] _keysize The size of the key
*/
void ga_setKey( ga_passcodeGenerator* /* _pg */, ga_byte* /* _key */, size_t /* _keysize */ );

/** Gives a decimal timeout code.
* @param[in] _pg The passcodeGenerator
* @return A timeout code.
*/
char* ga_generateTimeoutCode( ga_passcodeGenerator* /* _pg */ );

/** 
* @param[in] _challenge A long-valued challenge
* @return A decimal response code
*/
char* ga_generateResponseCodeFromLong( ga_passcodeGenerator* /* _pg */, long /* _challenge */ );

/**
* @param[in] _challenge A byte array challenge.
* @param[in] _size The size of the byte array.
* @return a decimal response code
*/
char* ga_generateResponseCodeFromByte( ga_passcodeGenerator* /* _pg */, ga_byte* /* _challenge */, size_t /* _size */ );

/**
* @param[in] _challenge A challenge to check response against.
* @param[in] _response A response to verify
* @return True if the response is valid
*/
bool ga_verifyResponseCode( ga_passcodeGenerator* _pg, long /* _challenge */, char* /* _response */ );

/**
* @param[in] _timeoutcode The timeout code.
* @return True if the timeout code is valid.
*/
bool ga_verifyTimeoutCode( ga_passcodeGenerator*, char* /* _timeoutcode */ );

bool ga_verifyTimeoutCodeWithIntervals( ga_passcodeGenerator*, char* /* _timeoutcode */, int /* _pastintervals */, int /* _futureintervals */ );

#ifdef __cplusplus
}
#endif

#endif /* GOOGLE_AUTHENTICATOR_H */
