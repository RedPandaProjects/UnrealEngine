/*=============================================================================
	UnFont.cpp: Unreal font code.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnRender.h"

/*------------------------------------------------------------------------------
	UFont implementation.
------------------------------------------------------------------------------*/

UFont::UFont()
{}

void UFont::Serialize( FArchive& Ar )
{
	guard(UFont::Serialize);
	UTexture::Serialize(Ar);
	Ar << Characters;
	unguardobj;
}
IMPLEMENT_CLASS(UFont);

/*------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------*/
