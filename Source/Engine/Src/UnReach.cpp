/*=============================================================================
	UnReach.cpp: Reachspec creation and management

	These methods are members of the FReachSpec class, 

	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Steven Polge 3/97
=============================================================================*/
#include "EnginePrivate.h"

/*
supports() -
 returns true if it supports the requirements of aPawn.  Distance is not considered.
*/
inline int FReachSpec::supports (int iRadius, int iHeight, int moveFlags)
{
	return ( (CollisionRadius >= iRadius) 
		&& (CollisionHeight >= iHeight)
		&& ((reachFlags & moveFlags) == reachFlags) );
}

/* Monster path 
 returns true if supports typical (Skaarj) movement requirements
 */
int FReachSpec::MonsterPath()
{
	guard(FReachSpec::MonsterPath);

	return ( (CollisionRadius >= COMMONRADIUS) && (CollisionHeight >= MINCOMMONHEIGHT)
			&& !(reachFlags & R_FLY) ); 
	unguard;
}

int FReachSpec::BotOnlyPath()
{
	guard(FReachSpec::BotOnlyPath);

	return ( CollisionRadius < MINCOMMONRADIUS );
	
	unguard;
}
/* 
+ adds two reachspecs - returning the combined reachability requirements and total distance 
Note that Start and End are not set
*/
FReachSpec FReachSpec::operator+ (const FReachSpec &Spec) const
{
	guard(FReachSpec::operator+);
	FReachSpec Combined;
	
	Combined.CollisionRadius = Min(CollisionRadius, Spec.CollisionRadius);
	Combined.CollisionHeight = Min(CollisionHeight, Spec.CollisionHeight);
	Combined.reachFlags = (reachFlags | Spec.reachFlags);
	Combined.distance = distance + Spec.distance;
	
	return Combined; 
	unguard;
}
/* operator <=
Used for comparing reachspecs reach requirements
less than means that this has easier reach requirements (equal or easier in all categories,
does not compare distance, start, and end
*/
int FReachSpec::operator<= (const FReachSpec &Spec)
{
	guard(FReachSpec::operator<=);
	int result =  
		(CollisionRadius >= Spec.CollisionRadius) &&
		(CollisionHeight >= Spec.CollisionHeight) &&
		((reachFlags | Spec.reachFlags) == Spec.reachFlags);
	return result; 
	unguard;
}

/* operator ==
Used for comparing reachspecs for choosing the best one
does not compare start and end
*/
int FReachSpec::operator== (const FReachSpec &Spec)
{
	guard(FReachSpec::operator ==);
	int result = (distance == Spec.distance) && 
		(CollisionRadius == Spec.CollisionRadius) &&
		(CollisionHeight == Spec.CollisionHeight) &&
		(reachFlags == Spec.reachFlags);
	
	return result; 
	unguard;
}

/* defineFor()
initialize the reachspec for a  traversal from start actor to end actor.
Note - this must be a direct traversal (no routing).
Returns 1 if the definition was successful (there is such a reachspec), and zero
if no definition was possible
*/

int FReachSpec::defineFor(AActor * begin, AActor * dest, APawn * Scout)
{
	guard(FReachSpec::defineFor);
	Start = begin;
	End = dest;
	Scout->Physics = PHYS_Walking;
	Scout->JumpZ = 320.0; //FIXME- test with range of JumpZ values - or let reachable code set max needed
	Scout->bCanWalk = 1;
	Scout->bCanJump = 1;
	Scout->bCanSwim = 1;
	Scout->bCanFly = 0;
	Scout->GroundSpeed = 320.0; 
	Scout->MaxStepHeight = 25; //FIXME - get this stuff from human class

	return findBestReachable(Start->Location, End->Location,Scout);
	unguard;
}

int FReachSpec::findBestReachable(FVector &begin, FVector &Destination, APawn * Scout)
{
	guard(FReachSpec::findBestReachable);

	Scout->SetCollisionSize( 18.0, 39.0 );

	int result = 0;
	FLOAT stepsize = MAXCOMMONRADIUS - Scout->CollisionRadius;
	int success;
	int stilltrying = 1;
	FLOAT bestRadius = 0;
	FLOAT bestHeight = 0;
	//debugf("Find reachspec from %f %f %f to %f %f %f", begin.X, begin.Y, begin.Z,
	//	Destination.X, Destination.Y, Destination.Z);
	while (stilltrying) //find out max radius
	{
		success = Scout->GetLevel()->FarMoveActor( Scout, begin);

		if (success)
			success = Scout->pointReachable(Destination);

		if (success)
		{
			reachFlags = success;
			result = 1;
			bestRadius = Scout->CollisionRadius;
			bestHeight = Scout->CollisionHeight;
			Scout->SetCollisionSize( Scout->CollisionRadius + stepsize, MINCOMMONHEIGHT);
			stepsize *= 0.5;
			if ( (stepsize < 2) || (Scout->CollisionRadius > MAXCOMMONRADIUS) )
				stilltrying = 0;
		}
		else
		{
			Scout->SetCollisionSize(Scout->CollisionRadius - stepsize, Scout->CollisionHeight);
			stepsize *= 0.5;
			if ( (stepsize < 2) || (Scout->CollisionRadius < 18) )
				stilltrying = 0;
		}
	}
	
	if (result)
	{
		Scout->SetCollisionSize(bestRadius, Scout->CollisionHeight + 4);
		stilltrying = 1;
		stepsize = MAXCOMMONHEIGHT - Scout->CollisionHeight; 
	}

	while (stilltrying) //find out max height
	{
		success = Scout->GetLevel()->FarMoveActor( Scout, begin);
		if (success)
			success = Scout->pointReachable(Destination);
		if (success)
		{
			reachFlags = success;
			bestHeight = Scout->CollisionHeight;
			Scout->SetCollisionSize(Scout->CollisionRadius, Scout->CollisionHeight + stepsize);
			stepsize *= 0.5;
			if ( (stepsize < 1.0) || (Scout->CollisionHeight > MAXCOMMONHEIGHT) ) 
				stilltrying = 0;
		}
		else
		{
			Scout->SetCollisionSize(Scout->CollisionRadius, Scout->CollisionHeight - stepsize);
			stepsize *= 0.5;
			if ( (stepsize < 1.0) || (Scout->CollisionHeight < 40.0) )
				stilltrying = 0;
		}
	}

	if (result)
	{
		CollisionRadius = Scout->CollisionRadius;
		CollisionHeight = bestHeight;
		FVector path = End->Location - Start->Location;
		distance = (int)path.Size(); //fixme - reachable code should calculate
		if ( reachFlags & R_SWIM )
			distance *= 2;
	}

	return result; 
	unguard;
}



