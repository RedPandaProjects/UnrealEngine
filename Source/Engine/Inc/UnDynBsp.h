/*=============================================================================
	UnDynBsp.h: Unreal dynamic Bsp object support
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*---------------------------------------------------------------------------------------
	FMovingBrushTrackerBase virtual base class.
---------------------------------------------------------------------------------------*/

//
// Moving brush tracker.
//
class FMovingBrushTrackerBase
{
public:
	// Constructors/destructors.
	virtual ~FMovingBrushTrackerBase() {};
	
	// Public operations:
	virtual void Update( AActor *Actor ) = 0;
	virtual void Flush( AActor *Actor ) = 0;
	virtual int SurfIsDynamic( INT iSurf ) = 0;
	virtual void Exit() = 0;
};
ENGINE_API FMovingBrushTrackerBase* GNewBrushTracker( ULevel* Level );

/*---------------------------------------------------------------------------------------
	The End.
---------------------------------------------------------------------------------------*/
