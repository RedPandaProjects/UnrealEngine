/*=============================================================================
	UnParams.cpp: Functions to help parse commands.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	What's happening: When the Visual Basic level editor is being used,
	this code exchanges messages with Visual Basic.  This lets Visual Basic
	affect the world, and it gives us a way of sending world information back
	to Visual Basic.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"

/*-----------------------------------------------------------------------------
	Getters.
	All of these functions return 1 if the appropriate item was
	fetched, or 0 if not.
-----------------------------------------------------------------------------*/

//
// Get a floating-point vector (X=, Y=, Z=), return number of components parsed (0-3).
//
UBOOL ENGINE_API GetFVECTOR( const char* Stream, FVector& Value )
{
	guard(GetFVECTOR);
	int NumVects = 0;

	// Support for old format.
	NumVects += Parse( Stream, "X=", Value.X ); //oldver.
	NumVects += Parse( Stream, "Y=", Value.Y ); //oldver.
	NumVects += Parse( Stream, "Z=", Value.Z ); //oldver.

	// New format.
	if( NumVects==0 )
	{
		Value.X = appAtof(Stream);

		Stream = appStrchr(Stream,',');
		if( !Stream )
			return 0;
		Value.Y = appAtof(++Stream);
		Stream = appStrchr(Stream,',');
		if( !Stream ) return 0;
		Value.Z = appAtof(++Stream);
		NumVects=3;
	}
	return NumVects==3;
	unguard;
}

//
// Get a string enclosed in parenthesis.
//
UBOOL ENGINE_API GetSUBSTRING
(
	const char*	Stream, 
	const char*	Match,
	char*		Value,
	INT			MaxLen
)
{
	guard(GetSUBSTRING);

	const char *Found = appStrfind(Stream,Match);
	const char *Start;

	if( Found == NULL ) return 0; // didn't match.

	Start = Found + appStrlen(Match);
	if( *Start != '(' )
		return 0;

	appStrncpy( Value, Start+1, MaxLen );
	char* Temp=appStrchr( Value, ')' );
	if( Temp )
		*Temp=0;

	return 1;
	unguard;
}

//
// Get a floating-point vector (X=, Y=, Z=), return number of components parsed (0-3).
//
UBOOL ENGINE_API GetFVECTOR
(
	const char*	Stream, 
	const char*	Match, 
	FVector&	Value
)
{
	guard(GetFVECTOR);

	char Temp[80];
	if (!GetSUBSTRING(Stream,Match,Temp,80)) return 0;
	return GetFVECTOR(Temp,Value);

	unguard;
}

//
// Get a floating-point scale value.
//
UBOOL ENGINE_API GetFSCALE
(
	const char*	Stream,
	FScale&		Scale
)
{
	guard(GetFSCALE);

	if
	(	!GetFVECTOR( Stream, Scale.Scale )
	||	!Parse( Stream, "S=", Scale.SheerRate )
	||	!Parse( Stream, "AXIS=", Scale.SheerAxis ) )
		return 0;
	return 1;
	unguard;
}

//
// Get a set of rotations (PITCH=, YAW=, ROLL=), return number of components parsed (0-3).
//
UBOOL ENGINE_API GetFROTATOR
(
	const char*		Stream, 
	FRotator&		Rotation,
	INT				ScaleFactor
)
{
	guard(GetFROTATOR);

	FLOAT	Temp=0.0;
	int 	N = 0;

	// Old format.
	if( Parse(Stream,"PITCH=",Temp)) {Rotation.Pitch = Temp * ScaleFactor; N++;} //oldver.
	if( Parse(Stream,"YAW=",  Temp)) {Rotation.Yaw   = Temp * ScaleFactor; N++;} //oldver.
	if( Parse(Stream,"ROLL=", Temp)) {Rotation.Roll  = Temp * ScaleFactor; N++;} //oldver.

	// New format.
	if( N == 0 )
	{
		Rotation.Pitch = appAtof(Stream) * ScaleFactor;
		Stream = appStrchr(Stream,',');
		if( !Stream )
			return 0;

		Rotation.Yaw = appAtof(++Stream) * ScaleFactor;
		Stream = appStrchr(Stream,',');
		if( !Stream )
			return 0;

		Rotation.Roll = appAtof(++Stream) * ScaleFactor;
		return 1;
	}
	return 0;
	unguard;
}

//
// Get a rotation value, return number of components parsed (0-3).
//
UBOOL ENGINE_API GetFROTATOR
(
	const char*		Stream, 
	const char*		Match, 
	FRotator&		Value,
	INT				ScaleFactor
)
{
	guard(GetFROTATOR);

	char Temp[80];
	if (!GetSUBSTRING(Stream,Match,Temp,80)) return 0;
	return GetFROTATOR(Temp,Value,ScaleFactor);

	unguard;
}

//
// See if Stream starts with the named command.  Returns 1 of match,
// 0 if not.  Does not affect the stream.
//
UBOOL ENGINE_API PeekCMD
(
	const char*	Stream, 
	const char*	Match
)
{
	guard(PeekCMD);
	
	while( (*Stream==' ')||(*Stream==9) )
		Stream++;

	if( appStrnicmp(Stream,Match,appStrlen(Match))==0 )
	{
		Stream += appStrlen(Match);
		if( !appIsAlnum(*Stream) )
			return 1;
		else return 0;
	}
	else return 0;
	unguard;
}

//
// Gets a "BEGIN" string.  Returns 1 if gotten, 0 if not.
// If not gotten, doesn't affect anything.
//
UBOOL ENGINE_API GetBEGIN
(
	const char**	Stream, 
	const char*		Match
)
{
	guard(GetBEGIN);

	const char* Original = *Stream;
	if( ParseCommand( Stream, "BEGIN" ) && ParseCommand( Stream, Match ) )
		return 1;
	*Stream = Original;
	return 0;

	unguard;
}

//
// Gets an "END" string.  Returns 1 if gotten, 0 if not.
// If not gotten, doesn't affect anything.
//
UBOOL ENGINE_API GetEND
(
	const char**	Stream, 
	const char*		Match
)
{
	guard(GetEND);

	const char *Original = *Stream;
	if (ParseCommand (Stream,"END") && ParseCommand (Stream,Match)) return 1; // Gotten.
	*Stream = Original;
	return 0;

	unguard;
}

/*-----------------------------------------------------------------------------
	Setters.
	These don't validate lengths so you need to call them with a big buffer.
-----------------------------------------------------------------------------*/

//
// Output a vector.
//
ENGINE_API char* SetFVECTOR( char* Dest, const FVector* FVector )
{
	guard(SetFVECTOR);
	appSprintf(Dest,"%+013.6f,%+013.6f,%+013.6f",FVector->X,FVector->Y,FVector->Z);
	return Dest;
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
