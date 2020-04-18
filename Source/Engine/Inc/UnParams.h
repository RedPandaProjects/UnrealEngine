/*=============================================================================
	UnParams.h: Parameter-parsing routines
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Parameter parsing functions.
-----------------------------------------------------------------------------*/

ENGINE_API UBOOL GetFVECTOR( const char* Stream, const char* Match, FVector& Value );
ENGINE_API UBOOL GetFVECTOR( const char* Stream, FVector& Value );
ENGINE_API UBOOL GetFSCALE( const char* Stream, FScale& Scale );
ENGINE_API UBOOL GetFROTATOR( const char* Stream, const char* Match, FRotator& Rotation, int ScaleFactor );
ENGINE_API UBOOL GetFROTATOR( const char* Stream, FRotator& Rotation, int ScaleFactor );
ENGINE_API UBOOL GetBEGIN( const char** Stream, const char* Match );
ENGINE_API UBOOL GetEND( const char** Stream, const char* Match );
ENGINE_API UBOOL PeekCMD( const char* Stream, const char* Match );
ENGINE_API char* SetFVECTOR( char* Dest, const FVector* Value );

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
