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
 * Tests the chain proof of concept
 *
!*/
D_SEC( B ) VOID WINAPI Entry( VOID )
{
	LARGE_INTEGER	Lin;

	/* Zero otu stack structures */
	RtlSecureZeroMemory( &Lin, sizeof( Lin ) );

	/* Start the wait! */
	ObfNtWaitForSingleObject( NtCurrentThread(), FALSE, NULL );

	/* Zero out stack structures */
	RtlSecureZeroMemory( &Lin, sizeof( Lin ) );
};
