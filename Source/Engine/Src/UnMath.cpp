/*=============================================================================
	UnMath.cpp: Unreal math routines, implementation of FGlobalMath class
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"

/*-----------------------------------------------------------------------------
	FGlobalMath constructor.
-----------------------------------------------------------------------------*/

// Constructor.
FGlobalMath::FGlobalMath()
:	WorldMin			(-32700.0,-32700.0,-32700.0),
	WorldMax			(32700.0,32700.0,32700.0),
	UnitCoords			(FVector(0,0,0),FVector(1,0,0),FVector(0,1,0),FVector(0,0,1)),
	ViewCoords			(FVector(0,0,0),FVector(0,1,0),FVector(0,0,-1),FVector(1,0,0)),
	UnitScale			(FVector(1,1,1),0.0,SHEER_ZX)
{
	// Init base angle table.
	for( int i=0; i<NUM_ANGLES; i++ )
		TrigFLOAT[i] = appSin((FLOAT)i * 2.0 * PI / (FLOAT)NUM_ANGLES);

	// Init square root table.
	for( i=0; i<NUM_SQRTS; i++ )
	{
		FLOAT S				= appSqrt((FLOAT)(i+1) * (1.0/(FLOAT)NUM_SQRTS));
		FLOAT Temp			= (1.0-S);// Was (2*S*S*S-3*S*S+1);
		SqrtFLOAT[i]		= appSqrt((FLOAT)i / 16384.0);
	}
}

/*-----------------------------------------------------------------------------
	Conversion functions.
-----------------------------------------------------------------------------*/

// Return the FRotator corresponding to the direction that the vector
// is pointing in.  Sets Yaw and Pitch to the proper numbers, and sets
// roll to zero because the roll can't be determined from a vector.
FRotator FVector::Rotation()
{
	FRotator R;

	// Find yaw.
	R.Yaw = appAtan2(Y,X) * (FLOAT)MAXWORD / (2.0*PI);

	// Find pitch.
	R.Pitch = appAtan2(Z,appSqrt(X*X+Y*Y)) * (FLOAT)MAXWORD / (2.0*PI);

	// Find roll.
	R.Roll = 0;

	return R;
}

//
// Find good arbitrary axis vectors to represent U and V axes of a plane
// given just the normal.
//
void FVector::FindBestAxisVectors( FVector& Axis1, FVector& Axis2 )
{
	guard(FindBestAxisVectors);

	FLOAT NX = Abs(X);
	FLOAT NY = Abs(Y);
	FLOAT NZ = Abs(Z);

	// Find best basis vectors.
	if( NZ>NX && NZ>NY )	Axis1 = FVector(1,0,0);
	else					Axis1 = FVector(0,0,1);

	Axis1 = (Axis1 - *this * (Axis1 | *this)).SafeNormal();
	Axis2 = Axis1 ^ *this;

#if CHECK_ALL
	// Check results.
	if (
		(Abs(Axis1 | *this)>0.0001) ||
		(Abs(Axis2 | *this)>0.0001) ||
		(Abs(Axis1 | Axis2 )>0.0001)
		) appError ("FindBestAxisVectors failed");
#endif
	unguard;
}

/*-----------------------------------------------------------------------------
	Matrix inversion.
-----------------------------------------------------------------------------*/

//
// Coordinate system inverse.
//
FCoords FCoords::Inverse() const
{
	FLOAT RDet = 1.0 / FTriple( XAxis, YAxis, ZAxis );
	return FCoords
	(	-Origin.TransformVectorBy(*this)
	,	RDet * FVector
		(	(YAxis.Y * ZAxis.Z - YAxis.Z * ZAxis.Y)
		,	(ZAxis.Y * XAxis.Z - ZAxis.Z * XAxis.Y)
		,	(XAxis.Y * YAxis.Z - XAxis.Z * YAxis.Y) )
	,	RDet * FVector
		(	(YAxis.Z * ZAxis.X - ZAxis.Z * YAxis.X)
		,	(ZAxis.Z * XAxis.X - XAxis.Z * ZAxis.X)
		,	(XAxis.Z * YAxis.X - XAxis.X * YAxis.Z))
	,	RDet * FVector
		(	(YAxis.X * ZAxis.Y - YAxis.Y * ZAxis.X)
		,	(ZAxis.X * XAxis.Y - ZAxis.Y * XAxis.X)
		,	(XAxis.X * YAxis.Y - XAxis.Y * YAxis.X) )
	);
}

//
// Convert this orthogonal coordinate system to a rotation.
//
FRotator FCoords::OrthoRotation() const
{
	FRotator R
	(
		appAtan2( XAxis.Z, appSqrt(Square(XAxis.X)+Square(XAxis.Y)) ) * 32768.0 / PI,
		appAtan2( XAxis.Y, XAxis.X                                  ) * 32768.0 / PI,
		0
	);
	FCoords S = GMath.UnitCoords / R;
	R.Roll = appAtan2( ZAxis | S.YAxis, YAxis | S.YAxis ) * 32768.0 / PI;
	return R;
}

/*-----------------------------------------------------------------------------
	FSphere implementation.
-----------------------------------------------------------------------------*/

//
// Compute a bounding sphere from an array of points.
//
FSphere::FSphere( const TArray<FVector>& Pts )
: FVector(0,0,0), W(0)
{
	guard(FSphere::FSphere);
	if( Pts.Num() )
	{
		FBox Box( Pts );
		*this = FSphere( (Box.Min+Box.Max)/2, 0 );
		for( INT i=0; i<Pts.Num(); i++ )
		{
			FLOAT Dist = (Pts(i) - *this).SizeSquared();
			if( Dist > W )
				W = Dist;
		}
		W = appSqrt(W) * 1.001;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	FBox implementation.
-----------------------------------------------------------------------------*/

FBox::FBox( const TArray<FVector>& Points )
: Min(0,0,0), Max(0,0,0), IsValid(0)
{
	guard(FBox::FBox);
	for( INT i=0; i<Points.Num(); i++ )
		*this += Points(i);
	unguard;
}


// SafeNormal
FVector FVector::SafeNormal() const
{
	FLOAT SquareSum = X*X + Y*Y + Z*Z;
	if( SquareSum < SMALL_NUMBER )
		return FVector( 0.0, 0.0, 0.0 );

	FLOAT Size = appSqrt(SquareSum); 
	FLOAT Scale = 1.0/Size;
	return FVector( X*Scale, Y*Scale, Z*Scale );
}
/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
