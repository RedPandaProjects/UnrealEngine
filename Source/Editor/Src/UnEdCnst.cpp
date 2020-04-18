/*=============================================================================
	UnEdCnst.cpp: Editor movement contraints.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

#include "EditorPrivate.h"

/*------------------------------------------------------------------------------
	Functions.
------------------------------------------------------------------------------*/

FEditorConstraints::FEditorConstraints()
{
	guard(FEditorConstraints::FEditorConstraints);
	GridEnabled		= 0;
	RotGridEnabled	= 0;
	SnapVertices	= 1;
	GridSize		= FVector(0,0,0);
	RotGridSize		= FRotator(0,0,0);
	SnapDistance	= 0.0;
	unguard;
}
void FEditorConstraints::Snap( FVector& Point, FVector GridBase )
{
	guard(FEditorConstraints::Snap);
	if( GridEnabled )
		Point = (Point - GridBase).GridSnap( GridSize ) + GridBase;
	unguard;
}
void FEditorConstraints::Snap( FRotator& Rotation )
{
	guard(FEditorConstraints::Snap);
	if( RotGridEnabled )
		Rotation = Rotation.GridSnap( RotGridSize );
	unguard;
}
UBOOL FEditorConstraints::Snap( ULevel* Level, FVector& Location, FVector GridBase, FRotator& Rotation )
{
	guard(FEditorConstraints::Snap);

	UBOOL Snapped = 0;
	Snap( Rotation );
	if( Level && SnapVertices )
	{
		FVector	DestPoint;
		INT Temp;
		if( Level->Model->FindNearestVertex( Location, DestPoint, SnapDistance, Temp ) >= 0.0)
		{
			Location = DestPoint;
			Snapped = 1;
		}
	}
	if( !Snapped )
		Snap( Location, GridBase );
	return Snapped;
	unguard;
}

/*------------------------------------------------------------------------------
	The end.
------------------------------------------------------------------------------*/
