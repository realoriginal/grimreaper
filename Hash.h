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
 * Returns a DJB2 hash representation of the input
 * string up to the specified length. If no length
 * is specified it is presumed it is a ANSI string.
 * and will calculcate the buffer up until the \0
 * terminator.
 *
!*/
D_SEC( B ) UINT32 HashString( _In_ PUINT8 Buffer, _In_ UINT32 Length );
