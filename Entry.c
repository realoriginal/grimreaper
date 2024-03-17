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
 * Starts a named pipe server and awaits on the connection or until
 * a timeout is reached. When a connection is not made and a timeout
 * is not reached, the shellcode will remain obfuscated.
 *
 * If a connection is made, the shellcode will also attempt to obfuscate 
 * itself during read/write operations. Once a connection is lost or a 
 * timeout is reached during this R/W loop the shellcode will free itself 
 * from memory.
 *
!*/
D_SEC( B ) VOID WINAPI Entry( VOID )
{
	/* Do stuff here */
};
