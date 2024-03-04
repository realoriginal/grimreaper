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
	D_API( RtlReAllocateHeap );
	D_API( RtlAllocateHeap );
	D_API( RtlCompactHeap );
	D_API( RtlFreeHeap );
	D_API( RtlZeroHeap );
	D_API( RtlSizeHeap );
} API ;

/*!
 *
 * Purpose:
 * 
 * Mimic's realloc and returns the allocated block of heap memory.
 *
!*/
D_SEC( B ) PVOID MemoryReAlloc( _In_ PVOID Memory, _In_ SIZE_T Length )
{
	API	Api;

	PVOID	Ptr = NULL;

	/* Zero out stack structures */
	RtlSecureZeroMemory( &Api, sizeof( Api ) );

	Api.RtlReAllocateHeap = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "RtlReAllocateHeap" ) );
	Api.RtlCompactHeap    = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "RtlCompactHeap" ) );
	Api.RtlZeroHeap       = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "RtlZeroHeap" ) );

	/* Allocate a block of memory */
	Ptr = Api.RtlReAllocateHeap( NtCurrentPeb()->ProcessHeap, HEAP_ZERO_MEMORY, Memory, Length );

	if ( Ptr != NULL ) {

		/* Zero the unused blocks of memory */
		Api.RtlZeroHeap( NtCurrentPeb()->ProcessHeap, 0 );

		/* Compact the heap */
		Api.RtlCompactHeap( NtCurrentPeb()->ProcessHeap, 0 );
	};

	/* Zero out stack structures */
	RtlSecureZeroMemory( &Api, sizeof( Api ) );

	/* Return the pointer */
	return Ptr;
};

/*!
 *
 * Purpose:
 *
 * Mimic's malloc and returns a allocated block of heap memory.
 *
!*/
D_SEC( B ) PVOID MemoryAlloc( _In_ SIZE_T Length )
{
	API	Api;

	PVOID	Ptr = NULL;

	/* Zero out stack structures */
	RtlSecureZeroMemory( &Api, sizeof( Api ) );

	Api.RtlAllocateHeap = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "RtlAllocateHeap" ) );

	/* Return the pointer to the heap */
	Ptr = Api.RtlAllocateHeap( NtCurrentPeb()->ProcessHeap, HEAP_ZERO_MEMORY, Length );

	/* Zero out stack structures */
	RtlSecureZeroMemory( &Api, sizeof( Api ) );

	/* Return the pointer */
	return Ptr;
}

/*!
 *
 * Purpose:
 *
 * Frees the block of memory.
 *
!*/
D_SEC( B ) VOID MemoryFree( _In_ PVOID Buffer )
{
	API	Api;

	SIZE_T	Len = 0;

	/* Zero out stack structures */
	RtlSecureZeroMemory( &Api, sizeof( Api ) );

	Api.RtlCompactHeap = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "RtlCompactHeap" ) );
	Api.RtlFreeHeap    = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "RtlFreeHeap" ) );
	Api.RtlZeroHeap    = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "RtlZeroHeap" ) );
	Api.RtlSizeHeap    = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "RtlSizeHeap" ) );

	/* Lookup the length of the buffer */
	if ( ( Len = Api.RtlSizeHeap( NtCurrentPeb()->ProcessHeap, 0, Buffer ) ) != -1 ) {

		/* Zero the entire heap buffer */
		__builtin_memset( Buffer, 0, Len );

		/* Free the heap buffer */
		Api.RtlFreeHeap( NtCurrentPeb()->ProcessHeap, 0, Buffer );	

		/* Zero all allocations */
		Api.RtlZeroHeap( NtCurrentPeb()->ProcessHeap, 0 );

		/* Comparess the heap */
		Api.RtlCompactHeap( NtCurrentPeb()->ProcessHeap, 0 );
	};

	/* Zero out stack structures */
	RtlSecureZeroMemory( &Api, sizeof( Api ) );
};
