/*=============================================================================
	Hit.cpp: Software rendering code hit testing.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "SoftDrvPrivate.h"

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

#define HIT_SIZE 8
INT Pixels[HIT_SIZE][HIT_SIZE];
#define IGNORE 0xfe0d

/*-----------------------------------------------------------------------------
	Hit testing.
-----------------------------------------------------------------------------*/

//
// Push hit data.
//
void USoftwareRenderDevice::PushHit( const BYTE* Data, INT Count )
{
	guard(USoftwareRenderDevice::PushHit);
	check(Viewport->HitYL<=HIT_SIZE);
	check(Viewport->HitXL<=HIT_SIZE);

	// Save the passed info on the working stack.
	INT Index = HitStack.Add(Count);
	appMemcpy( &HitStack(Index), Data, Count );

	// Cleanup under cursor.
	if( Viewport->ColorBytes==2 )
	{
		_WORD* W = (_WORD*)Viewport->_Screen(Viewport->HitX,Viewport->HitY);
		for( INT Y=0; Y<Viewport->HitYL; Y++,W+=Viewport->Stride )
		{
			for( INT X=0; X<Viewport->HitXL; X++ )
			{
				Pixels[X][Y] = W[X];
				W[X] = IGNORE;
			}
		}
	}
	else if( Viewport->ColorBytes==4 )
	{
		DWORD* W = (DWORD*)Viewport->_Screen(Viewport->HitX,Viewport->HitY);
		for( INT Y=0; Y<Viewport->HitYL; Y++,W+=Viewport->Stride )
		{
			for( INT X=0; X<Viewport->HitXL; X++ )
			{
				Pixels[X][Y] = W[X];
				W[X] = IGNORE;
			}
		}
	}
	else appErrorf( "Invalid color bytes" );

	unguard;
}

//
// Pop hit data.
//
void USoftwareRenderDevice::PopHit( INT Count, UBOOL bForce )
{
	guard(USoftwareRenderDevice::PopHit);
	check(Count<=HitStack.Num());
	UBOOL Hit=0;

	// Check under cursor.
	if( Viewport->ColorBytes==2 )
	{
		_WORD* W = (_WORD*)Viewport->_Screen(Viewport->HitX,Viewport->HitY);
		for( INT Y=0; Y<Viewport->HitYL; Y++,W+=Viewport->Stride )
		{
			for( INT X=0; X<Viewport->HitXL; X++ )
			{
				if( W[X] != IGNORE )
					Hit=1;
				W[X] = Pixels[X][Y];
			}
		}
	}
	else if( Viewport->ColorBytes==4 )
	{
		DWORD* W = (DWORD*)Viewport->_Screen(Viewport->HitX,Viewport->HitY);
		for( INT Y=0; Y<Viewport->HitYL; Y++,W+=Viewport->Stride )
		{
			for( INT X=0; X<Viewport->HitXL; X++ )
			{
				if( W[X] != IGNORE )
					Hit=1;
				W[X] = Pixels[X][Y];
			}
		}
	}
	else appErrorf( "Invalid color bytes" );

	// Handle hit.
	if( Hit || bForce )
	{
		if( HitStack.Num() <= *HitSize )
		{
			HitCount = HitStack.Num();
			appMemcpy( HitData, &HitStack(0), HitCount );
		}
		else HitCount = 0;
	}

	// Remove the passed info from the working stack.
	HitStack.Remove( HitStack.Num()-Count, Count );

	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
