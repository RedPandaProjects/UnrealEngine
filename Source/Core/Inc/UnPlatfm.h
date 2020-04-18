/*=============================================================================
	UnPlatfm.h: All generic hooks for platform-specific routines

	This structure is shared between the generic Unreal code base and
	the platform-specific routines.

	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*------------------------------------------------------------------------------
	Interfaces.
------------------------------------------------------------------------------*/

//
// An output device.  Player consoles, the debug log, and the script
// compiler result text accumulator are output devices.
//
class CORE_API FOutputDevice
{
public:
	virtual void WriteBinary( const void* Data, INT Length, EName MsgType=NAME_None )=0;
	void Log( EName MsgType, const char* Text );
	void Log( const char* Text );
	void VARARGS Logf( EName MsgType, const char* Fmt, ... );
	void VARARGS Logf( const char* Fmt, ... );
};

//
// Any object that is capable of taking commands.
//
class CORE_API FExec
{
public:
	virtual UBOOL Exec( const char* Cmd, FOutputDevice* Out )=0;
};

//
// Notification hook.
//
class CORE_API FNotifyHook
{
public:
	virtual void NotifyDestroy( void* Src ) {}
	virtual void NotifyPreChange( void* Src ) {}
	virtual void NotifyPostChange( void* Src ) {}
	virtual void NotifyExec( void* Src, const char* Cmd ) {}
};

//
// A context for displaying modal warning messages.
//
class CORE_API FFeedbackContext
{
public:
	virtual void Warnf( const char* Fmt, ... )=0;
	virtual UBOOL YesNof( const char* Fmt, ... )=0;
	virtual void BeginSlowTask( const char* Task, UBOOL StatusWindow, UBOOL Cancelable )=0;
	virtual void EndSlowTask()=0;
	virtual UBOOL VARARGS StatusUpdatef( INT Numerator, INT Denominator, const char* Fmt, ... )=0;
};

/*----------------------------------------------------------------------------
	FGlobalPlatform.
----------------------------------------------------------------------------*/

//
// Platform-specific code's communication structure.
//
class CORE_API FGlobalPlatform : public FOutputDevice, public FFeedbackContext, public FExec
{
public:
	// FOutputDevice interface.
	void WriteBinary( const void* Data, int Length, EName Event );

	// FFeedbackContext interface.
	void Warnf( const char* Fmt, ... );
	UBOOL YesNof( const char* Fmt, ... );
	void BeginSlowTask( const char* Task, UBOOL StatusWindow, UBOOL Cancelable );
	void EndSlowTask();
	UBOOL VARARGS StatusUpdatef( INT Numerator, INT Denominator, const char* Fmt, ... );

	// FExec interface.
	virtual UBOOL Exec( const char* Cmd, FOutputDevice* Out=GSystem );
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
