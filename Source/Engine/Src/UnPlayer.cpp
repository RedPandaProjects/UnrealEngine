/*=============================================================================
	UnActor.cpp: Actor list functions.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"

/*-----------------------------------------------------------------------------
	UPlayer object implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UPlayer);

/*-----------------------------------------------------------------------------
	UPlayer object implementation.
-----------------------------------------------------------------------------*/

UPlayer::UPlayer( ULevel* InLevel )
:	Actor		(NULL)
{}
void UPlayer::Serialize(FArchive &Ar)
{
	guard(UPlayer::Serialize);
	UObject::Serialize(Ar);
	unguard;
}
void UPlayer::Destroy()
{
	guard(UPlayer::Destroy);
	if( GSystem && GIsRunning && Actor )
	{
		ULevel* Level = Actor->XLevel;
		Actor->Player = NULL;
		Level->DestroyActor( Actor, 1 );
		Actor = NULL;
	}
	UObject::Destroy();
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
