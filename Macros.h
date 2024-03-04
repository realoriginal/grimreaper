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

/* Gets the address of an interal function or variable relative to GetIp */
#define G_PTR( x )	( ULONG_PTR )( GetIp( ) - ( ( ULONG_PTR ) & GetIp - ( ULONG_PTR ) x ) )		

/* Puts a function or variable in a specific region of .text */
#define D_SEC( x )	__attribute__(( section( ".text$" #x ) ))

/* Cast as a pointer with the specified typedef and same name */
#define D_API( x )	__typeof__( x ) * x

/* Cast as a pointer-wide integer */
#define U_PTR( x )	( ( ULONG_PTR ) x )

/* Cast as a pointer */
#define C_PTR( x )	( ( PVOID ) x )
