/*!
 *
 * GRIMREAPER
 *
 * Austin Hudson
 *
 * suspicious.actor
 *
!*/

#pragma once

/*!
 *
 * Purpose:
 *
 * Mimic's realloc and returns the allocated block of heap memory.
 *
!*/
D_SEC( B ) PVOID MemoryReAlloc( _In_ PVOID Memory, _In_ SIZE_T Length );

/*!
 *
 * Purpose:
 *
 * Mimic's malloc and returns a allocated block of heap memory.
 *
!*/
D_SEC( B ) PVOID MemoryAlloc( _In_ SIZE_T Length );

/*!
 *
 * Purpose:
 *
 * Frees the block of memory.
 *
!*/
D_SEC( B ) VOID MemoryFree( _In_ PVOID Buffer );
