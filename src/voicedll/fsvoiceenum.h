#ifndef FSVOICEENUM_IS_INCLUDED
#define FSVOICEENUM_IS_INCLUDED
/* { */

#ifndef YSRESULT_IS_DEFINED
#define YSRESULT_IS_DEFINED
typedef enum
{
	YSERR,
	YSOK
} YSRESULT;

#endif

#ifndef YSBOOL_IS_DEFINED
#define YSBOOL_IS_DEFINED
typedef enum
{
	YSFALSE,
	YSTRUE,
	YSTFUNKNOWN
} YSBOOL;
#endif

typedef enum
{
	FSVOICE_GENERAL,
	FSVOICE_COMMA,  // This should be ignored by the voice dll.
	FSVOICE_SPACE,  // This should be ignored by the voice dll.
	FSVOICE_PERIOD, // This should be ignored by the voice dll.
	FSVOICE_END_OF_SENTENCE,
	FSVOICE_CALLSIGN,
	FSVOICE_HEADING,
	FSVOICE_GENERAL_NUMBER,
	FSVOICE_ALTITUDE_IN_FEET,
	FSVOICE_ALTITUDE_IN_FLIGHT_LEVEL,
	FSVOICE_SPEED_IN_KNOT,
	FSVOICE_RUNWAY,
	FSVOICE_APPROACH_GENERAL,
	FSVOICE_APPROACH_ILS,
	FSVOICE_APPROACH_VOR,
	FSVOICE_APPROACH_NDB,
	FSVOICE_APPROACH_GPS

	// Memo to myself:  Don't change the order.  Always add something new to the end for future DLL compatibility.

} FSVOICE_PHRASE_TYPE;

struct FsVoicePhrase
{
	FSVOICE_PHRASE_TYPE phraseType;
	const char *phrase;
};

/* } */
#endif
