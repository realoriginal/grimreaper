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
 * Parses a portable executables export directory if it 
 * exists and return the address of the exported function
 * if it is present else NULL.
 *
!*/
D_SEC( B ) PVOID PeGetFuncEat( _In_ PVOID ImageBase, _In_ UINT32 ExportHash )
{
	PUINT16			Aoo = NULL;
	PUINT32			Aof = NULL;
	PUINT32			Aon = NULL;
	PIMAGE_DOS_HEADER	Dos = NULL;
	PIMAGE_NT_HEADERS	Nth = NULL;
	PIMAGE_DATA_DIRECTORY	Dir = NULL;
	PIMAGE_EXPORT_DIRECTORY	Exp = NULL;

	/* Get the export directory entry */
	Dos = C_PTR( ImageBase );
	Nth = C_PTR( U_PTR( Dos ) + Dos->e_lfanew );
	Dir = & Nth->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ];

	/* Is this a valid export directory? */
	if ( Dir->VirtualAddress ) {
		/* Set the export directory address */
		Exp = C_PTR( U_PTR( Dos ) + Dir->VirtualAddress );
		Aon = C_PTR( U_PTR( Dos ) + Exp->AddressOfNames );
		Aof = C_PTR( U_PTR( Dos ) + Exp->AddressOfFunctions );
		Aoo = C_PTR( U_PTR( Dos ) + Exp->AddressOfNameOrdinals );

		/* Enumerate all the available exports */
		for ( INT Idx = 0 ; Idx < Exp->NumberOfNames ; ++Idx ) {
			/* Hash the name and compare to our export hash */
			if ( HashString( C_PTR( U_PTR( Dos ) + Aon[ Idx ] ), 0 ) == ExportHash ) {
				/* Return the pointer if we found a match */
				return C_PTR( U_PTR( Dos ) + Aof[ Aoo[ Idx ] ] );
			};
		};
	};

	/* Return NULL */
	return NULL;
};	
