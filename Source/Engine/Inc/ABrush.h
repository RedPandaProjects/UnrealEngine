/*=============================================================================
	ABrush.h.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

	// Constructors.

	// UObject interface.
	void PostLoad();

	// AActor interface.
	FCoords ToLocal() const
	{
		guardSlow(ABrush::ToLocal);
		return GMath.UnitCoords / -PrePivot / MainScale / Rotation / PostScale / Location;
		unguardSlow;
	}
	FCoords ToWorld() const
	{
		guardSlow(ABrush::ToWorld);
		return GMath.UnitCoords * Location * PostScale * Rotation * MainScale * -PrePivot;
		unguardSlow;
	}
	FLOAT BuildCoords( FModelCoords* Coords, FModelCoords* Uncoords )
	{
		guard(ABrush::BuildCoords);
		if( Coords )
		{
			Coords->PointXform    = (GMath.UnitCoords * PostScale * Rotation * MainScale);
			Coords->VectorXform   = (GMath.UnitCoords / MainScale / Rotation / PostScale).Transpose();
		}
		if( Uncoords )
		{
			Uncoords->PointXform  = (GMath.UnitCoords / MainScale / Rotation / PostScale);
			Uncoords->VectorXform = (GMath.UnitCoords * PostScale * Rotation * MainScale).Transpose();
		}
		return MainScale.Orientation() * PostScale.Orientation();
		unguard;
	}

	// ABrush interface.
	virtual void CopyPosRotScaleFrom( ABrush* Other )
	{
		guard(ABrush::CopyPosRotScaleFrom);
		check(Brush);
		check(Other);
		check(Other->Brush);

		Location    = Other->Location;
		Rotation    = Other->Rotation;
		PrePivot	= Other->PrePivot;
		MainScale	= Other->MainScale;
		PostScale	= Other->PostScale;

		Brush->BuildBound();

		unguard;
	}
	virtual void InitPosRotScale();

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
