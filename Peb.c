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
 * Searches the loaded modules for the base address. If it
 * is not actively loaded it will return NULL.
 *
!*/
D_SEC( B ) PVOID PebGetModule( _In_ UINT32 ModuleHash )
{
	PLIST_ENTRY		Hdr = NULL;
	PLIST_ENTRY		Ent = NULL;
	PLDR_DATA_TABLE_ENTRY	Ldt = NULL;

	/* Get a pointer to the header and first entry of the list */
	Hdr = C_PTR( & NtCurrentPeb()->Ldr->InLoadOrderModuleList );
	Ent = C_PTR( Hdr->Flink );

	/* Loop through the list until it reaches the end */
	for ( ; Ent != Hdr ; Ent = Ent->Flink ) {
		/* Parse the entry */
		Ldt = C_PTR( CONTAINING_RECORD( Ent, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks ) );

		/* Hash the name and compare to our requested hash */
		if ( HashString( C_PTR( Ldt->BaseDllName.Buffer ), Ldt->BaseDllName.Length ) == ModuleHash ) {
			/* Return its image base */
			return C_PTR( Ldt->DllBase );
		};
	};
	return NULL;
};
