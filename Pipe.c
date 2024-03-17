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

typedef struct
{
	D_API( NtCreateNamedPipeFile );
	D_API( NtDeviceIoControlFile );
	D_API( NtClose );
} API ;

/*!
 *
 * Purpose:
 *
 * Opens a named pipe as a server and awaits for a connection.
 *
!*/
D_SEC( B ) NTSTATUS PipeOpen( _In_ LPWSTR PipeName )
{

};
