/*!
 *
 * GRIMREAPER
 *
 * Austin Hudson
 *
 * suspicious.actor
 *
!*/

#include "Common.h"

/*!
 *
 * Purpose:
 *
 * Returns a DJB2 hash representation of the input
 * string up to the specified length. If no length
 * is specified it is presumed it is a ANSI string.
 * and will calculcate the buffer up until the \0
 * terminator.
 *
!*/
D_SEC( B ) UINT32 HashString( _In_ PUINT8 Buffer, _In_ UINT32 Length )
{
	UINT8	Val = 0;
	UINT32	Hsh = 5381;
	PUINT8	Ptr = C_PTR( Buffer );

	/* Loop through until we reach a NULL terminator OR the length specified if not 0 */
	while ( TRUE ) {
		/* Extract the current ANSI character */
		Val = *Ptr;

		/* Was no length specified? */
		if ( ! Length ) {
			/* Have we reached a NULL terminator? */
			if ( ! Val ) {
				/* Abort the loop */
				break;
			};
		} else
		{
			/* Have we exceeded the length of the buffer if a length was specified? */
			if ( ( UINT32 )( Ptr - ( PUINT8 ) Buffer ) >= Length ) {
				/* Abort the loop */
				break;
			};

			/* Has a NULL character? */
			if ( ! Val ) {
				/* Move onto the next character since we skip it */
				++Ptr ; continue;
			};
		};

		/* Is an lowercase character? */
		if ( Val >= 'a' ) {
			/* Force UPPERCASE */
			Val -= 0x20;
		};

		/* Hash the current character and move onto the next char */
		Hsh = ( ( Hsh << 5 ) + Hsh ) + Val; ++Ptr ;
	};

	/* Return the hash */
	return Hsh;
};
