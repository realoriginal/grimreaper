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
 * A 'obfuscated' version of NtWaitForSingleObject. Waits
 * on a an object and returns the status of the operation
 * on failure or sfuccess.
 *
!*/
D_SEC( B ) NTSTATUS NTAPI ObfNtWaitForSingleObject( _In_ HANDLE Handle, _In_ BOOLEAN Alertable, _In_ PLARGE_INTEGER Timeout );
