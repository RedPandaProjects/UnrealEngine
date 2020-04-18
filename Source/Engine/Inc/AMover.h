/*=============================================================================
	AMover.h: Class functions residing in the AMover class.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

	// Constructors.
	AMover();

	// UObject interface.
	void PostLoad();
	void PostEditChange();

	// AActor interface.
	void Spawned();
	void PostEditMove();
	void PreRaytrace();
	void PostRaytrace();

	// ABrush interface.
	virtual void SetWorldRaytraceKey();
	virtual void SetBrushRaytraceKey();

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
