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
 * Searches the loaded modules for the base address. If it
 * is not actively loaded it will return NULL.
 *
!*/
D_SEC( B ) PVOID PebGetModule( _In_ UINT32 ModuleHash );
