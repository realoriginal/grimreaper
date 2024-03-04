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
 * Parses a portable executables export directory if it 
 * exists and return the address of the exported function
 * if it is present else NULL.
 *
!*/
D_SEC( B ) PVOID PeGetFuncEat( _In_ PVOID ImageBase, _In_ UINT32 ExportHash );	
