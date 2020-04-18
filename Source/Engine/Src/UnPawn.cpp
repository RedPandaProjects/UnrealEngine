/*=============================================================================
	UnPawn.cpp: APawn AI implementation

  This contains both C++ methods (movement and reachability), as well as some 
  AI related intrinsics

	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Steven Polge 3/97
=============================================================================*/

#include "EnginePrivate.h"
/*-----------------------------------------------------------------------------
	APawn object implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(APawn);
IMPLEMENT_CLASS(APlayerPawn);

/*-----------------------------------------------------------------------------
	ANavigationPoint functions.
-----------------------------------------------------------------------------*/

enum EAIFunctions
{
	AI_MoveTo = 500,
	AI_PollMoveTo = 501,
	AI_MoveToward = 502,
	AI_PollMoveToward = 503,
	AI_StrafeTo = 504,
	AI_PollStrafeTo = 505,
	AI_StrafeFacing = 506,
	AI_PollStrafeFacing = 507,
	AI_TurnTo = 508,
	AI_PollTurnTo = 509,
	AI_TurnToward = 510,
	AI_PollTurnToward = 511,
	AI_MakeNoise = 512,
	AI_LineOfSightTo = 514,
	AI_FindPathToward = 517,
	AI_FindPathTo = 518,
	AI_DescribeSpec = 519,
	AI_ActorReachable = 520,
	AI_PointReachable = 521,
	AI_ClearPaths = 522,
	AI_EAdjustJump = 523,
	AI_FindStairRotation = 524, 
	AI_FindRandomDest = 525,
	AI_PickWallAdjust = 526,
	AI_WaitForLanding = 527,
	AI_PollWaitForLanding = 528,
	AI_AddPawn = 529,
	AI_RemovePawn = 530,
	AI_PickTarget = 531,
	AI_PlayerCanSeeMe = 532, 
	AI_CanSee = 533,
	AI_PickAnyTarget = 534,
	AI_StopWaiting = 535,
	AI_SaveConfig = 536, // in core
	AI_ConsoleCommand = 537,
	AI_GetFolderName = 538,
	AI_GetMapName = 539,
	AI_FindBestInventoryPath = 540,
	AI_DynamicLoadObject = 541,
	AI_ConsoleCommandResult = 542,
	AI_ResetConfig = 543, // in core
	AI_ResetKeyboard = 544,
	AI_GetNextSkin = 545,
	AI_UpdateURL = 546
};

void APlayerPawn::execUpdateURL( FFrame& Stack, BYTE*& Result )
{
	guard(APlayerPawn::execUpdateURL);

	P_GET_STRING(NewOption);
	P_FINISH;

	((UGameEngine*)GetLevel()->Engine)->LastURL.AddOption(NewOption);
	unguard;
}
AUTOREGISTER_INTRINSIC( APlayerPawn, AI_UpdateURL, execUpdateURL);

void AActor::execGetNextSkin( FFrame& Stack, BYTE*& Result )
{
	guard(AActor::execGetNextSkin);

	P_GET_STRING(Prefix);
	P_GET_STRING(CurrentSkin);
	P_GET_INT(Dir);
	P_FINISH;

	*(char*)Result = 0;

	TArray<FRegistryObjectInfo> List;
	GObj.GetRegistryObjects( List, UTexture::StaticClass, NULL, 0 );
	char PrevSkin[64];
	PrevSkin[0] = 0;
	char NextSkin[64];
	char FirstSkin[64];
	FirstSkin[0] = 0;
	int bLeaveOnNext = 0;
	int bLeaveOnLast = 0;
	int N = appStrlen(Prefix);

	for( INT i=0; i<List.Num(); i++ )
	{
		if ( appStricmp(List(i).Object, CurrentSkin) == 0 ) 
		{
			if ( Dir == -1 )
			{
				if ( PrevSkin[0] )
				{
					appStrcpy( (char*)Result, PrevSkin);
					return;
				}
				else
					bLeaveOnLast = 1;
			}
			else
				bLeaveOnNext = 1;
		}
		else 
		{
			appStrncpy( NextSkin, List(i).Object, N+1 );
			if ( appStricmp(NextSkin,Prefix) == 0 )
			{
				if ( !FirstSkin[0] )
					appStrcpy( FirstSkin, List(i).Object);
				if ( bLeaveOnNext )
				{
					appStrcpy( (char*)Result, List(i).Object);
					return;
				}
				else
					appStrcpy( PrevSkin, List(i).Object);
			}
		}
	}

	if ( bLeaveOnLast )
		appStrcpy( (char*)Result, PrevSkin);
	else
		appStrcpy( (char*)Result, FirstSkin);

	unguard;
}
AUTOREGISTER_INTRINSIC( AActor, AI_GetNextSkin, execGetNextSkin);

void APlayerPawn::execResetKeyboard( FFrame& Stack, BYTE*& Result )
{
	guard(APlayerPawn::execResetKeyboard);

	P_FINISH;

	UViewport* Viewport = Cast<UViewport>(Player);
	if( Viewport && Viewport->Input )
		GObj.ResetConfig(Viewport->Input->GetClass());
	unguard;
}
AUTOREGISTER_INTRINSIC( APlayerPawn, AI_ResetKeyboard, execResetKeyboard);

void APawn::execFindBestInventoryPath( FFrame& Stack, BYTE*& Result )
{
	guard(APawn::execFindBestInventoryPath);

	P_GET_FLOAT_REF(Weight);
	P_GET_UBOOL(bPredictRespawns);
	P_FINISH;

	clock(XLevel->FindPathCycles);
	AActor * bestPath = NULL;
	AActor * newPath;
	FLOAT BestWeight = findPathTowardBestInventory(newPath, 1, *Weight, bPredictRespawns);
	//debugf(NAME_DevPath,"BestWeight is %f compared to weight %f", BestWeight, *Weight);
	if ( BestWeight > *Weight )
	{
		bestPath = newPath;
		*Weight = BestWeight;
		//debugf(NAME_DevPath,"Recommend move to %s", bestPath->GetName());

		SpecialPause = 0.0;
		bShootSpecial = 0;

		if ( bestPath && bestPath->IsProbing(NAME_SpecialHandling) )
		{
			//debugf(NAME_DevPath,"Handle Special");
			HandleSpecial(bestPath);
			//debugf(NAME_DevPath,"Done Handle Special");
		}

		if ( bestPath == SpecialGoal )
			SpecialGoal = NULL;
	}
	unclock(XLevel->FindPathCycles);
	//debugf("Find path to time was %f", XLevel->FindPathCycles * GApp->MSecPerCycle);

	*(AActor**)Result = bestPath; 
	unguard;
}
AUTOREGISTER_INTRINSIC( APawn, AI_FindBestInventoryPath, execFindBestInventoryPath);

void AActor::execGetMapName( FFrame& Stack, BYTE*& Result )
{
	guard(AActor::execGetMapName);

	P_GET_STRING(Prefix);
	P_GET_STRING(MapName);
	P_GET_INT(Dir);
	P_FINISH;

	*(char*)Result = 0;
	char Wildcard[256];
	TArray<FString> MapNames;
	appSprintf( Wildcard, "*.%s", FURL::DefaultMapExt );
	for( INT i=0; i<ARRAY_COUNT(GSys->Paths); i++ )
	{
		if( appStrstr( GSys->Paths[i], Wildcard ) )
		{
			char Tmp[256];
			appStrcpy( Tmp, GSys->Paths[i] );
			*appStrstr( Tmp, Wildcard )=0;
			appStrcat( Tmp, "\\" );
			appStrcat( Tmp, Prefix );
			appStrcat( Tmp, Wildcard );
			TArray<FString>	TheseNames = appFindFiles(Tmp);
			for( INT i=0; i<TheseNames.Num(); i++ )
			{
				for( INT j=0; j<MapNames.Num(); j++ )
					if( appStricmp(*MapNames(j),*TheseNames(i))==0 )
						break;
				if( j==MapNames.Num() )
					new(MapNames)FString(TheseNames(i));
			}
		}
	}
	for( i=0; i<MapNames.Num(); i++ )
	{
		if( appStrcmp(*MapNames(i), MapName)==0 )
		{
			INT Offset = i+Dir;
			if( Offset < 0 )
				Offset = MapNames.Num() - 1;
			else if( Offset >= MapNames.Num() )
				Offset = 0;
			appStrcpy( (char*)Result, *MapNames(Offset) );
			return;
		}
	}
	if( MapNames.Num() > 0 )
		appStrcpy( (char*)Result, *MapNames(0));

	unguard;
}
AUTOREGISTER_INTRINSIC( AActor, AI_GetMapName, execGetMapName);

void APlayerPawn::execConsoleCommandResult( FFrame& Stack, BYTE*& Result )
{
	guard(APlayerPawn::execConsoleCommandResult);

	P_GET_STRING(Command);
	P_FINISH;

	UViewport* Viewport = Cast<UViewport>(Player);
	FStringOut StrOut;
	if( Viewport )
		Viewport->Exec(Command, &StrOut);
	appStrcpy( (char *)Result, *StrOut );

	unguard;
}
AUTOREGISTER_INTRINSIC( APlayerPawn, AI_ConsoleCommandResult, execConsoleCommandResult);
 
void APlayerPawn::execConsoleCommand( FFrame& Stack, BYTE*& Result )
{
	guard(APlayerPawn::execConsoleCommand);

	P_GET_STRING(Command);
	P_FINISH;

	UViewport* Viewport = Cast<UViewport>(Player);
	if( Viewport )
		Viewport->Exec(Command, Viewport);

	unguard;
}
AUTOREGISTER_INTRINSIC( APlayerPawn, AI_ConsoleCommand, execConsoleCommand);

void APawn::execStopWaiting( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execStopWaiting);

	P_FINISH;

	if( GetMainFrame()->LatentAction == EPOLL_Sleep )
		LatentFloat = -1.0;

	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, INDEX_NONE, execStopWaiting);

/* CanSee()
returns true if LineOfSightto object and it is within creature's 
peripheral vision
*/

void APawn::execCanSee( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execCanSee);

	P_GET_ACTOR(Other);
	P_FINISH;

	*(DWORD*)Result = LineOfSightTo(Other, 1);
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_CanSee, execCanSee);

/* PlayerCanSeeMe()
	returns true if actor is visible to some player
*/
void AActor::execPlayerCanSeeMe( FFrame& Stack, BYTE*& Result )
{
	guardSlow(AActor::PlayerCanSeeMe);
	P_FINISH;
	APawn *next = GetLevel()->GetLevelInfo()->PawnList;
	int seen = 0;
	while ( next )
	{
		if ( next->IsA(APlayerPawn::StaticClass) )
		{
			float distSq = (Location - next->Location).SizeSquared();
			if ( (distSq < 110000.f * CollisionRadius) 
				&& (next->bBehindView 
					|| (Square(next->ViewRotation.Vector() | (Location - next->Location)) >= 0.25 * distSq))
				&& next->LineOfSightTo(this) )
			{
				seen = 1;
				break;
			}
		}
	next = next->nextPawn;
	}

	*(DWORD*)Result = seen;
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( AActor, AI_PlayerCanSeeMe, execPlayerCanSeeMe );

/* execDescribeSpec - temporary debug
*/
void ANavigationPoint::execdescribeSpec( FFrame& Stack, BYTE*& Result )
{
	guardSlow(ANavigationPoint::execdescribeSpec);

	P_GET_INT(iSpec);
	P_FINISH;

	FReachSpec spec = GetLevel()->ReachSpecs(iSpec);
	debugf(NAME_DevPath,"Reachspec from %s to %s", spec.Start->GetName(), spec.End->GetName());
	debugf(NAME_DevPath,"Distance %d, Height %f , Radius %f, reachflags %d", spec.distance, spec.CollisionHeight, spec.CollisionRadius, spec.reachFlags);

	unguardSlow;
}
AUTOREGISTER_INTRINSIC( ANavigationPoint, AI_DescribeSpec, execdescribeSpec );

/*-----------------------------------------------------------------------------
	Pawn related functions.
-----------------------------------------------------------------------------*/

void APawn::execPickTarget( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execPickTarget);

	P_GET_FLOAT_REF(bestAim);
	P_GET_FLOAT_REF(bestDist);
	P_GET_VECTOR(FireDir);
	P_GET_VECTOR(projStart);
	P_FINISH;
	APawn *pick = NULL;
	APawn *next = GetLevel()->GetLevelInfo()->PawnList;

	while ( next )
	{
		if ( (next != this) && (next->Health > 0) && next->bProjTarget )
		{
			FLOAT newAim = FireDir | (next->Location - projStart);
			if ( newAim > 0 )
			{
				FLOAT FireDist = (next->Location - projStart).SizeSquared();
				if ( FireDist < 4000000.f )
				{
					FireDist = appSqrt(FireDist);
					newAim = newAim/FireDist;
					if ( (newAim > *bestAim) && LineOfSightTo(next) )
					{
						pick = next;
						*bestAim = newAim;
						*bestDist = FireDist;
					}
				}
			}
		}
		next = next->nextPawn;
	}

	*(APawn**)Result = pick; 
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_PickTarget, execPickTarget);

void APawn::execPickAnyTarget( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execPickAnyTarget);

	P_GET_FLOAT_REF(bestAim);
	P_GET_FLOAT_REF(bestDist);
	P_GET_VECTOR(FireDir);
	P_GET_VECTOR(projStart);
	P_FINISH;
	AActor *pick = NULL;

	for( INT iActor=0; iActor<XLevel->Num(); iActor++ )
		if( XLevel->Actors(iActor) )
		{
			AActor* next = XLevel->Actors(iActor);
			if ( next->bProjTarget && !next->IsA(APawn::StaticClass) )
			{
				FLOAT newAim = FireDir | (next->Location - projStart);
				if ( newAim > 0 )
				{
					FLOAT FireDist = (next->Location - projStart).SizeSquared();
					if ( FireDist < 4000000.f )
					{
						FireDist = appSqrt(FireDist);
						newAim = newAim/FireDist;
						if ( (newAim > *bestAim) && LineOfSightTo(next) )
						{
							pick = next;
							*bestAim = newAim;
							*bestDist = FireDist;
						}
					}
				}
			}
		}

	*(AActor**)Result = pick; 
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_PickAnyTarget, execPickAnyTarget);

void APawn::execAddPawn( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execAddPawn);

	P_FINISH;

	nextPawn = GetLevel()->GetLevelInfo()->PawnList;
	GetLevel()->GetLevelInfo()->PawnList = this;
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_AddPawn, execAddPawn);

void APawn::execRemovePawn( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execRemovePawn);

	P_FINISH;

	APawn *next = GetLevel()->GetLevelInfo()->PawnList;
	if ( next == this )
		GetLevel()->GetLevelInfo()->PawnList = next->nextPawn;
	else
	{
		while ( next )
		{
			if ( next->nextPawn == this )
			{
				next->nextPawn = nextPawn;
				break;
			}
			next = next->nextPawn;
		}
	}

	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_RemovePawn, execRemovePawn);

/* execWaitForLanding()
wait until physics is not PHYS_Falling
*/
void APawn::execWaitForLanding( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execWaitForLanding);

	P_FINISH;

	LatentFloat = 2.5;
	if (Physics == PHYS_Falling)
		GetMainFrame()->LatentAction = AI_PollWaitForLanding;
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_WaitForLanding, execWaitForLanding);

void APawn::execPollWaitForLanding( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execPollWaitForLanding);

	debug(Stack.Object->IsA(APawn::StaticClass));
	APawn *Pawn = (APawn*)Stack.Object;

	if (Pawn->Physics != PHYS_Falling)
		Pawn->GetMainFrame()->LatentAction = 0;
	else
	{
		FLOAT DeltaSeconds = *(FLOAT*)Result;
		LatentFloat -= DeltaSeconds;
		if ( LatentFloat < 0 )
			Pawn->eventLongFall();
	}
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_PollWaitForLanding, execPollWaitForLanding);

void APawn::execPickWallAdjust( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execPickWallAdjust);

	P_FINISH;

	clock(XLevel->FindPathCycles);
	*(DWORD*)Result = PickWallAdjust();
	unclock(XLevel->FindPathCycles);
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_PickWallAdjust, execPickWallAdjust);

void APawn::execFindStairRotation( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execFindStairRotation);

	P_GET_FLOAT(deltaTime);
	P_FINISH;

	if (deltaTime > 0.33)
	{
		*(DWORD*)Result = ViewRotation.Pitch;
		return;
	}
	if (ViewRotation.Pitch > 32768)
		ViewRotation.Pitch = (ViewRotation.Pitch & 65535) - 65536;
	
	FCheckResult Hit(1.0);
	FRotator LookRot = ViewRotation;
	LookRot.Pitch = 0;
	FVector Dir = LookRot.Vector();
	FVector EyeSpot = Location + FVector(0,0,BaseEyeHeight);
	FLOAT height = CollisionHeight + BaseEyeHeight; 
	FVector CollisionSlice(CollisionRadius,CollisionRadius,1);

	GetLevel()->SingleLineCheck(Hit, this, EyeSpot + 2 * height * Dir, EyeSpot, TRACE_VisBlocking, CollisionSlice);
	FLOAT Dist = 2 * height * Hit.Time;
	int stairRot = 0;
	if (Dist > 0.8 * height)
	{
		FVector Spot = EyeSpot + 0.5 * Dist * Dir;
		FLOAT Down = 3 * height;
		GetLevel()->SingleLineCheck(Hit, this, Spot - FVector(0,0,Down), Spot, TRACE_VisBlocking, CollisionSlice);
		if (Hit.Time < 1.0)
		{
			FLOAT firstDown = Down * Hit.Time;
			if (firstDown < 0.7 * height - 6.0) // then up or level
			{
				Spot = EyeSpot + Dist * Dir;
				GetLevel()->SingleLineCheck(Hit, this, Spot - FVector(0,0,Down), Spot, TRACE_VisBlocking, CollisionSlice);
				stairRot = ::Max(0, ViewRotation.Pitch);
				if ( Down * Hit.Time < firstDown - 10 ) 
					stairRot = 5400;
			}
			else if  (firstDown > 0.7 * height + 6.0) // then down or level
			{
				GetLevel()->SingleLineCheck(Hit, this, Location + 0.9*Dist*Dir, Location, TRACE_VisBlocking);
				if (Hit.Time == 1.0)
				{
					Spot = EyeSpot + Dist * Dir;
					GetLevel()->SingleLineCheck(Hit, this, Spot - FVector(0,0,Down), Spot, TRACE_VisBlocking, CollisionSlice);
					stairRot = Min(0, ViewRotation.Pitch);
					if (Down * Hit.Time > firstDown + 10)
						stairRot = -5000;
				}
			}
		}
	}
	INT Diff = Abs(ViewRotation.Pitch - stairRot);
	if ( Diff > 0 )
	{
		FLOAT RotRate = 8;
		if ( Diff < 1000 )
			RotRate = 8000/Diff; 

		RotRate = ::Min(1.f, RotRate * deltaTime);
		stairRot = int(FLOAT(ViewRotation.Pitch) * ( 1 - RotRate) + FLOAT(stairRot) * RotRate);
	}
	*(DWORD*)Result = stairRot; 
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_FindStairRotation, execFindStairRotation);

void APawn::execEAdjustJump( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execEAdjustJump);

	FVector Landing;
	FVector vel = Velocity;
	SuggestJumpVelocity(Destination, vel);

	P_FINISH;

	*(FVector*)Result = vel;
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_EAdjustJump, execEAdjustJump);

void APawn::execactorReachable( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execActorReachable);

	P_GET_ACTOR(actor);
	P_FINISH;
	
	if ( !actor )
	{
		//debugf(NAME_DevPath,"Warning: No goal for ActorReachable by %s in %s",GetName(), GetMainFrame()->Describe() );
		*(DWORD*)Result = 0; 
		return;
	}
	clock(XLevel->FindPathCycles);
	if ( actor->IsA(AInventory::StaticClass) && ((AInventory *)actor)->myMarker )
		actor = ((AInventory *)actor)->myMarker;
	if ( actor->IsA(ANavigationPoint::StaticClass) && GetLevel()->ReachSpecs.Num() && (CollisionRadius <= MAXCOMMONRADIUS) )
	{
		FLOAT MaxDistSq = ::Max(48.f, CollisionRadius);
		FVector Dir;
		INT bOnPath = 0;
		MaxDistSq = MaxDistSq * MaxDistSq;
		if ( MoveTarget && MoveTarget->IsA(ANavigationPoint::StaticClass)
			&& (Abs(MoveTarget->Location.Z - Location.Z) < CollisionHeight) ) 
		{
			Dir = MoveTarget->Location - Location;
			Dir.Z = 0;
			if ( (Dir | Dir) < MaxDistSq )
			{
				bOnPath = 1;
				if ( (MoveTarget == actor) || CanMoveTo(MoveTarget, actor) )
				{
					*(DWORD*)Result = 1;
					unclock(XLevel->FindPathCycles);
					return;
				}
			}
		}
		ANavigationPoint *Nav = GetLevel()->GetLevelInfo()->NavigationPointList;
		while (Nav)
		{
			if ( Abs(Nav->Location.Z - Location.Z) < CollisionHeight )
			{
				Dir = Nav->Location - Location;
				Dir.Z = 0;
				if ( (Dir | Dir) < MaxDistSq ) 
				{
					bOnPath = 1;
					if ( (Nav == actor) || CanMoveTo(Nav, actor) )
					{
						*(DWORD*)Result = 1;
						unclock(XLevel->FindPathCycles);
						return;
					}
				}
			}
			Nav = Nav->nextNavigationPoint;
		}
		if ( bOnPath && (Physics != PHYS_Flying) )
		{
			*(DWORD*)Result = 0;
			unclock(XLevel->FindPathCycles);
			return;
		}
	}	
	*(DWORD*)Result = actorReachable(actor);  
	unclock(XLevel->FindPathCycles);
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_ActorReachable, execactorReachable);

void APawn::execpointReachable( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execPointReachable);

	P_GET_VECTOR(point);
	P_FINISH;

	clock(XLevel->FindPathCycles);
	*(DWORD*)Result = pointReachable(point);  
	unclock(XLevel->FindPathCycles);
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_PointReachable, execpointReachable);

/* FindPathTo()
returns the best pathnode toward a point - even if point is directly reachable
If there is no path, returns None
By default clears paths.  If script wants to preset some path weighting, etc., then
it can explicitly clear paths using execClearPaths before presetting the values and 
calling FindPathTo with clearpath = 0

  FIXME add optional bBlockDoors (no paths through doors), bBlockTeleporters, bBlockSwitches,
  maxNodes (max number of nodes), etc.
*/

void APawn::execFindPathTo( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execFindPathTo);

	P_GET_VECTOR(point);
	P_GET_INT_OPT(bSinglePath, 0); 
	P_GET_UBOOL_OPT(bClearPaths, 1);
	P_FINISH;

	clock(XLevel->FindPathCycles);
	AActor * bestPath = NULL;
	AActor * newPath;
	if (findPathTo(point, bSinglePath, newPath, bClearPaths))
		bestPath = newPath;
	SpecialPause = 0.0;
	bShootSpecial = 0;

	if ( bestPath && bestPath->IsProbing(NAME_SpecialHandling) )
	{
		//debugf(NAME_DevPath,"Handle Special");
		HandleSpecial(bestPath);
		//debugf(NAME_DevPath,"Done Handle Special");
	}

	if ( bestPath == SpecialGoal )
		SpecialGoal = NULL;
	unclock(XLevel->FindPathCycles);
	//debugf("Find path to time was %f", XLevel->FindPathCycles * GApp->MSecPerCycle);

	*(AActor**)Result = bestPath; 
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_FindPathTo, execFindPathTo);

void APawn::execFindPathToward( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execFindPathToward);

	P_GET_ACTOR(goal);
	P_GET_INT_OPT(bSinglePath, 0);
	P_GET_UBOOL_OPT(bClearPaths, 1);
	P_FINISH;

	if ( !goal )
	{
		//debugf(NAME_DevPath,"Warning: No goal for FindPathToward by %s in %s",GetName(), GetMainFrame()->Describe() );
		*(AActor**)Result = NULL; 
		return;
	}
	clock(XLevel->FindPathCycles);
	AActor * bestPath = NULL;
	AActor * newPath;
	if (findPathToward(goal, bSinglePath, newPath, bClearPaths))
		bestPath = newPath;
	SpecialPause = 0.0;
	bShootSpecial = 0;

	if ( bestPath && bestPath->IsProbing(NAME_SpecialHandling) )
	{
		//debugf(NAME_DevPath,"Handle Special");
		HandleSpecial(bestPath);
		//debugf(NAME_DevPath,"Done Handle Special");
	}

	if ( bestPath == SpecialGoal )
		SpecialGoal = NULL;
	unclock(XLevel->FindPathCycles);
	//debugf("Find path toward time was %f", XLevel->FindPathCycles * GApp->MSecPerCycle);

	*(AActor**)Result = bestPath; 
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_FindPathToward, execFindPathToward);

/* FindRandomDest()
returns a random pathnode which is reachable from the creature's location
*/
void APawn::execFindRandomDest( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execFindPathTo);

	P_GET_UBOOL_OPT(bClearPaths, 1);
	P_FINISH;

	clock(XLevel->FindPathCycles);
	if (bClearPaths)
		clearPaths();
	ANavigationPoint * bestPath = NULL;
	AActor * newPath;
	if ( findRandomDest(newPath) )
		bestPath = (ANavigationPoint *)newPath;

	unclock(XLevel->FindPathCycles);

	*(ANavigationPoint**)Result = bestPath; 
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_FindRandomDest, execFindRandomDest);

void APawn::execClearPaths( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execClearPaths);

	P_FINISH;

	clock(XLevel->FindPathCycles);
	clearPaths(); 
	unclock(XLevel->FindPathCycles);
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_ClearPaths, execClearPaths);

/*MakeNoise
- check to see if other creatures can hear this noise
*/
void AActor::execMakeNoise( FFrame& Stack, BYTE*& Result )
{
	guardSlow(AActor::execMakeNoise);

	P_GET_FLOAT(Loudness);
	P_FINISH;
	
	//debugf(" %s Make Noise with instigator", GetFullName(),Instigator->GetClassName());
	if ( GetLevel()->GetLevelInfo()->NetMode != NM_Client )
		CheckNoiseHearing(Loudness);
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( AActor, AI_MakeNoise, execMakeNoise);

void APawn::execLineOfSightTo( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execLineOfSightTo);

	P_GET_ACTOR(Other);
	P_FINISH;

	*(DWORD*)Result = LineOfSightTo(Other);
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_LineOfSightTo, execLineOfSightTo);

/* execMoveTo()
start moving to a point -does not use routing
Destination is set to a point
//FIXME - don't use ground speed for flyers (or set theirs = flyspeed)
*/
void APawn::execMoveTo( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execMoveTo);

	P_GET_VECTOR(dest);
	P_GET_FLOAT_OPT(speed, 1.0);
	P_FINISH;

	FVector Move = dest - Location;
	MoveTarget = NULL;
	bReducedSpeed = 0;
	DesiredSpeed = ::Max(0.f, Min(MaxDesiredSpeed, speed));
	FLOAT MoveSize = Move.Size();
	setMoveTimer(MoveSize); 
	GetMainFrame()->LatentAction = AI_PollMoveTo;
	Destination = dest;
	Focus = dest;
	rotateToward(Focus);
	moveToward(Destination);
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_MoveTo, execMoveTo);

void APawn::execPollMoveTo( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execPollMoveTo);
	debug(Stack.Object->IsA(APawn::StaticClass));
	APawn *Pawn = (APawn*)Stack.Object;

	Pawn->rotateToward(Pawn->Focus);
	if (Pawn->moveToward(Pawn->Destination))
		Pawn->GetMainFrame()->LatentAction = 0;

	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_PollMoveTo, execPollMoveTo);

/* execMoveToward()
start moving toward a goal actor -does not use routing
MoveTarget is set to goal
*/
void APawn::execMoveToward( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execMoveToward);

	P_GET_ACTOR(goal);
	P_GET_FLOAT_OPT(speed, 1.0);
	P_FINISH;

	if (!goal)
	{
		//Stack.ScriptWarn(0,"MoveToward with no goal");
		return;
	}

	FVector Move = goal->Location - Location;	
	bReducedSpeed = 0;
	DesiredSpeed = ::Max(0.f, Min(MaxDesiredSpeed, speed));
	if (goal->IsA(APawn::StaticClass))
		MoveTimer = 1.2; //max before re-assess movetoward
	else
	{
		FLOAT MoveSize = Move.Size();
		setMoveTimer(MoveSize);
	}
	MoveTarget = goal;
	Destination = MoveTarget->Location; 
	Focus = Destination;
	GetMainFrame()->LatentAction = AI_PollMoveToward;
	rotateToward(Focus);
	moveToward(Destination);
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_MoveToward, execMoveToward);

void APawn::execPollMoveToward( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execPollMoveToward);
	debug(Stack.Object->IsA(APawn::StaticClass));
	APawn *Pawn = (APawn*)Stack.Object;

	if ( !Pawn->MoveTarget )
	{
		//Stack.ScriptWarn(0,"MoveTarget cleared during movetoward");
		Pawn->GetMainFrame()->LatentAction = 0;
		return;
	}

	Pawn->Destination = Pawn->MoveTarget->Location;
	if ( (Pawn->Physics == PHYS_Flying) && (Pawn->MoveTarget->IsA(APawn::StaticClass)) )
		Pawn->Destination.Z += 0.7 * Pawn->MoveTarget->CollisionHeight;
	else if (Pawn->Physics == PHYS_Spider)
		Pawn->Destination = Pawn->Destination - Pawn->MoveTarget->CollisionRadius * Pawn->Floor;

	Pawn->Focus = Pawn->Destination;
	Pawn->rotateToward(Pawn->Focus);
	FLOAT oldDesiredSpeed = Pawn->DesiredSpeed;
	if (Pawn->moveToward(Pawn->Destination))
		Pawn->GetMainFrame()->LatentAction = 0;
	if (Pawn->MoveTarget->IsA(APawn::StaticClass))
	{
		Pawn->DesiredSpeed = oldDesiredSpeed; //don't slow down when moving toward a pawn
		if (!Pawn->bCanSwim && Pawn->MoveTarget->Region.Zone->bWaterZone)
			Pawn->MoveTimer = -1.0; //give up
	}

	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_PollMoveToward, execPollMoveToward);

/* execStrafeTo()
Strafe to Destination, pointing at Focus
*/
void APawn::execStrafeTo( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execStrafeTo);

	P_GET_VECTOR(Dest);
	P_GET_VECTOR(FocalPoint);
	P_FINISH;

	FVector Move = Dest - Location;
	MoveTarget = NULL;
	bReducedSpeed = 0;
	if (bIsPlayer)
		DesiredSpeed = MaxDesiredSpeed;
	else
		DesiredSpeed = 0.8 * MaxDesiredSpeed;
	FLOAT MoveSize = Move.Size();
	setMoveTimer(MoveSize); 
	GetMainFrame()->LatentAction = AI_PollStrafeTo;
	Destination = Dest;
	Focus = FocalPoint;
	rotateToward(Focus);
	moveToward(Destination);
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_StrafeTo, execStrafeTo);

void APawn::execPollStrafeTo( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execPollStrafeTo);
	debug(Stack.Object->IsA(APawn::StaticClass));
	APawn *Pawn = (APawn*)Stack.Object;

	Pawn->rotateToward(Pawn->Focus);
	if (Pawn->moveToward(Pawn->Destination))
		Pawn->GetMainFrame()->LatentAction = 0;

	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_PollStrafeTo, execPollStrafeTo);

/* execStrafeFacing()
strafe to Destination, pointing at MoveTarget
*/
void APawn::execStrafeFacing( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execStrafeFacing);

	P_GET_VECTOR(Dest)
	P_GET_ACTOR(goal);
	P_FINISH;

	if (!goal)
	{
		//Stack.ScriptWarn(0,"StrafeFacing without goal");
		return;
	}
	FVector Move = Dest - Location;	
	bReducedSpeed = 0;
	if (bIsPlayer)
		DesiredSpeed = MaxDesiredSpeed;
	else
		DesiredSpeed = 0.8 * MaxDesiredSpeed;
	FLOAT MoveSize = Move.Size();
	setMoveTimer(MoveSize); 
	Destination = Dest;
	MoveTarget = goal;
	Focus = MoveTarget->Location;
	GetMainFrame()->LatentAction = AI_PollStrafeFacing;
	rotateToward(Focus);
	moveToward(Destination);
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_StrafeFacing, execStrafeFacing);

void APawn::execPollStrafeFacing( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execPollStrafeFacing);
	debug(Stack.Object->IsA(APawn::StaticClass));
	APawn *Pawn = (APawn*)Stack.Object;

	if (!Pawn->MoveTarget)
	{
		//Stack.ScriptWarn(0,"MoveTarget cleared during strafefacing");
		Pawn->GetMainFrame()->LatentAction = 0;
		return;
	}

	Pawn->Focus = Pawn->MoveTarget->Location;
	Pawn->rotateToward(Pawn->Focus);
	if (Pawn->moveToward(Pawn->Destination))
		Pawn->GetMainFrame()->LatentAction = 0;

	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_PollStrafeFacing, execPollStrafeFacing);

/* execTurnToward()
turn toward MoveTarget
*/
void APawn::execTurnToward( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execTurnToward);

	P_GET_ACTOR(goal);
	P_FINISH;
	
	if (!goal)
		return;

	MoveTarget = goal;
	GetMainFrame()->LatentAction = AI_PollTurnToward;
	if ( !bCanStrafe && ((Physics == PHYS_Flying) || (Physics == PHYS_Swimming)) )
		Acceleration = Rotation.Vector() * AccelRate;

	Focus = MoveTarget->Location;
	rotateToward(Focus);
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_TurnToward, execTurnToward);

void APawn::execPollTurnToward( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execPollTurnToward);
	debug(Stack.Object->IsA(APawn::StaticClass));
	APawn *Pawn = (APawn*)Stack.Object;

	if (!Pawn->MoveTarget)
	{
		//Stack.ScriptWarn(0,"MoveTarget cleared during turntoward");
		Pawn->GetMainFrame()->LatentAction = 0;
		return;
	}

	if ( !Pawn->bCanStrafe && ((Pawn->Physics == PHYS_Flying) || (Pawn->Physics == PHYS_Swimming)) )
		Pawn->Acceleration = Pawn->Rotation.Vector() * Pawn->AccelRate;

	Pawn->Focus = Pawn->MoveTarget->Location;
	if (Pawn->rotateToward(Pawn->Focus))
		Pawn->GetMainFrame()->LatentAction = 0;  

	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_PollTurnToward, execPollTurnToward);

/* execTurnTo()
Turn to focus
*/
void APawn::execTurnTo( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execTurnTo);

	P_GET_VECTOR(FocalPoint);
	P_FINISH;

	MoveTarget = NULL;
	GetMainFrame()->LatentAction = AI_PollTurnTo;
	Focus = FocalPoint;
	if ( !bCanStrafe && ((Physics == PHYS_Flying) || (Physics == PHYS_Swimming)) )
		Acceleration = Rotation.Vector() * AccelRate;

	rotateToward(Focus);
	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_TurnTo, execTurnTo);

void APawn::execPollTurnTo( FFrame& Stack, BYTE*& Result )
{
	guardSlow(APawn::execPollTurnTo);
	debug(Stack.Object->IsA(APawn::StaticClass));
	APawn *Pawn = (APawn*)Stack.Object;

	if ( !Pawn->bCanStrafe && ((Pawn->Physics == PHYS_Flying) || (Pawn->Physics == PHYS_Swimming)) )
		Pawn->Acceleration = Pawn->Rotation.Vector() * Pawn->AccelRate;

	if (Pawn->rotateToward(Pawn->Focus))
		Pawn->GetMainFrame()->LatentAction = 0;  

	unguardSlow;
}
AUTOREGISTER_INTRINSIC( APawn, AI_PollTurnTo, execPollTurnTo);

//=================================================================================
void APawn::setMoveTimer(FLOAT MoveSize)
{
	guard(APawn::setMoveTimer);

	FLOAT MaxSpeed = 200.0; //safety in case called with Physics = PHYS_None (shouldn't be)

	if (Physics == PHYS_Walking)
		MaxSpeed = GroundSpeed;
	else if (Physics == PHYS_Falling)
		MaxSpeed = GroundSpeed;
	else if (Physics == PHYS_Flying)
		MaxSpeed = AirSpeed;
	else if (Physics == PHYS_Swimming)
		MaxSpeed = WaterSpeed;
	else if (Physics == PHYS_Spider)
		MaxSpeed = GroundSpeed;

	MoveTimer = 1.0 + 1.3 * MoveSize/(DesiredSpeed * MaxSpeed); 
	unguard;
}

/* moveToward()
move Actor toward a point.  Returns 1 if Actor reached point
(Set Acceleration, let physics do actual move)
*/
int APawn::moveToward(const FVector &Dest)
{
	guard(APawn::moveToward);
	FVector Direction = Dest - Location;
	if (Physics == PHYS_Walking) 
		Direction.Z = 0.0;
	else if (Physics == PHYS_Falling)
		return 0;

	FLOAT Distance = Direction.Size();
	INT bGlider = ( !bCanStrafe && ((Physics == PHYS_Flying) || (Physics == PHYS_Swimming)) ); 

	if ( (Direction.X * Direction.X + Direction.Y * Direction.Y < 256) 
			&& (Abs(Direction.Z) < ::Max(48.f, CollisionHeight)) ) 
	{
		if ( !bGlider )
			Acceleration = FVector(0,0,0);
		return 1;
	}
	else if ( bGlider )
		Direction = Rotation.Vector();
	else
		Direction = Direction/Distance;

	Acceleration = Direction * AccelRate;

	if (MoveTimer < 0.0)
		return 1; //give up
	if (MoveTarget && MoveTarget->IsA(APawn::StaticClass))
	{
		if (Distance < CollisionRadius + MoveTarget->CollisionRadius + 0.8 * MeleeRange)
			return 1;
		return 0;
	}

	FLOAT speed = Velocity.Size(); 

	if ( !bGlider && (speed > 100) )
	{
		FVector VelDir = Velocity/speed;
		Acceleration -= 0.2 * (1 - (Direction | VelDir)) * speed * (VelDir - Direction); 
	}
	if (Distance < 1.4 * AvgPhysicsTime * speed )
	{
		if (!bReducedSpeed) //haven't reduced speed yet
		{
			DesiredSpeed = 0.5 * DesiredSpeed;
			bReducedSpeed = 1;
		}
		if (speed > 0)
			DesiredSpeed = Min(DesiredSpeed, 200.f/speed);
		if ( bGlider ) 
			return 1;
	}
	return 0;
	unguard;
}

/* rotateToward()
rotate Actor toward a point.  Returns 1 if target rotation achieved.
(Set DesiredRotation, let physics do actual move)
*/
int APawn::rotateToward(const FVector &FocalPoint)
{
	guard(APawn::rotateToward);
	if (Physics == PHYS_Spider)
		return 1;
	FVector Direction = FocalPoint - Location;

	// Rotate toward destination
	DesiredRotation = Direction.Rotation();
	DesiredRotation.Yaw = DesiredRotation.Yaw & 65535;
	if ( (Physics == PHYS_Walking) && (!MoveTarget || !MoveTarget->IsA(APawn::StaticClass)) )
		DesiredRotation.Pitch = 0;

	//only base success on Yaw 
	int success = (Abs(DesiredRotation.Yaw - (Rotation.Yaw & 65535)) < 2000);
	if (!success) //check if on opposite sides of zero
		success = (Abs(DesiredRotation.Yaw - (Rotation.Yaw & 65535)) > 63535);	

	return success;
	unguard;
}

DWORD APawn::LineOfSightTo(AActor *Other, int bShowSelf)
{
	guard(APawn::LineOfSightTo);
	if (!Other)
		return 0;

	//FIXME - when PVS, only do this test if in same PVS
	if (Other == Enemy)
		bShowSelf = 0;
	if ( bShowSelf ) //only players do showself
		bLOSflag = !bLOSflag;

	FLOAT distSq = (Other->Location - Location).SizeSquared();
	FLOAT maxdistSq; 

	if (bShowSelf)
	{
		if ( Other->IsA(APawn::StaticClass) )
		{
			maxdistSq = SightRadius * Min(1.f, (FLOAT)((APawn *)Other)->Visibility * 0.0078125f);
			maxdistSq = Min(9000000.f, maxdistSq * maxdistSq);
		}
		else
			maxdistSq = SightRadius * SightRadius;
		if (distSq > maxdistSq)
			return 0;
		//check field of view
		FVector SightDir = (Other->Location - Location).SafeNormal();
		Stimulus = (SightDir | Rotation.Vector()) - PeripheralVision;
		if ( Stimulus > 0 )
			Stimulus = 0.8 * Stimulus + 0.2;
		else
			Stimulus = 0.17 * Stimulus + 0.2;
		if ( Stimulus < 0 )
			return 0;
		FLOAT heightMod = Abs(Other->Location.Z - Location.Z)/(::Max(1.f, Skill + 1));
		distSq += heightMod * heightMod;
		distSq = distSq/(Stimulus * Stimulus);
		if (distSq > maxdistSq)
			return 0;
		Stimulus = 1;
	}
	else
	{
		if ( Other->IsA(APawn::StaticClass) )
		{
			maxdistSq = 3000 * Min(1.f, (FLOAT)(((APawn *)Other)->Visibility + 16) * 0.015f);
			maxdistSq = Min(9000000.f, maxdistSq * maxdistSq);
		}
		else
			maxdistSq = 9000000.f;
		if (distSq > maxdistSq)
			return 0;
	}

	FCheckResult Hit(1.0);
	FVector ViewPoint = Location;
	ViewPoint.Z += BaseEyeHeight; //look from eyes

	if (Other == Enemy)
	{
		GetLevel()->SingleLineCheck(Hit, this, Other->Location, ViewPoint, TRACE_VisBlocking);  
		if ( Hit.Time == 1.0)
		{
			LastSeeingPos = Location;
			LastSeenPos = Enemy->Location;
			return 1;
		}
		GetLevel()->SingleLineCheck(Hit, this, Other->Location, Location, TRACE_VisBlocking);  
		if ( Hit.Time == 1.0)
		{
			LastSeeingPos = Location;
			LastSeenPos = Enemy->Location;
			return 1;
		}
		if ( distSq > 1000000.f)
			return 0;
	}
	else if ( distSq > 1000000.f ) 
	{
		if ( !bLOSflag && (distSq > 0.5 * maxdistSq) )
			return 0;
		if ( !bIsPlayer && (appFrand() < 0.5) )
			return 0;
		GetLevel()->SingleLineCheck(Hit, this, Other->Location, ViewPoint, TRACE_VisBlocking);  
		return ( Hit.Time == 1.0);
	}		
	
	//try viewpoint to head
	FVector OtherBody = Other->Location;
	if ( !bShowSelf || !bLOSflag )
	{
		OtherBody.Z += Other->CollisionHeight * 0.8;
		GetLevel()->SingleLineCheck(Hit, this, OtherBody, ViewPoint, TRACE_VisBlocking);  
		if ( Hit.Time == 1.0)
			return 1;
	}

	if (distSq > 250000)
		return 0;

	//try checking sides - look at dist to four side points, and cull furthest and closest
	FVector Points[4];
	Points[0] = Other->Location - FVector(CollisionRadius, -1 * CollisionRadius, 0);
	Points[1] = Other->Location + FVector(CollisionRadius, CollisionRadius, 0);
	Points[2] = Other->Location - FVector(CollisionRadius, CollisionRadius, 0);
	Points[3] = Other->Location + FVector(CollisionRadius, -1 * CollisionRadius, 0);
	int imin = 0;
	int imax = 0;
	FLOAT currentmin = Points[0].SizeSquared(); 
	FLOAT currentmax = currentmin; 
	for (INT i=1;i<4;i++)
	{
		FLOAT nextsize = Points[i].SizeSquared(); 
		if (nextsize > currentmax)
		{
			currentmax = nextsize;
			imax = i;
		}
		else if (nextsize < currentmin)
		{
			currentmin = nextsize;
			imin = i;
		}
	}

	int bSkip = bLOSflag;
	for (i=0;i<4;i++)
		if ( (i != imin) && (i != imax) )
		{
			if ( bSkip && bShowSelf )
				bSkip = 0;
			else
			{
				bSkip = 1;
				GetLevel()->SingleLineCheck(Hit, this, Points[i], ViewPoint, TRACE_VisBlocking); 
				if (Hit.Time == 1.0)
					return 1;
			}
		}

	return 0;
	unguard;
}

int APawn::CanHear(FVector NoiseLoc, FLOAT Loudness)
{
	guard(APawn::CanHear);

	FLOAT DistSq = (Location - NoiseLoc).SizeSquared();
	if ( DistSq > 4000000.f * Loudness * Loudness)
		return 0;

	//FIXME - provide some hearing out of line of sight? (maybe PVS)
	// Note - a loudness of 1.0 is a typical loud noise

	FLOAT perceived = Min(1200000.f/DistSq, 2.f);
		
	Stimulus = Loudness * perceived + Alertness * Min(0.5f,perceived); 	
	if (Stimulus < HearingThreshold)
		return 0;
		
	FCheckResult Hit(1.0);

	//FIXME - check PVS
	GetLevel()->SingleLineCheck(Hit, this, NoiseLoc, Location, TRACE_VisBlocking); 
	return (Hit.Time == 1.0);
	unguard;
}

/* Send a HearNoise() message to all Pawns which could possibly hear this noise
NOTE that there is a hard-coded limit to the radius of hearing, which depends on the loudness of the noise
*/
void AActor::CheckNoiseHearing(FLOAT Loudness)
{
	guard(AActor::CheckNoiseHearing);

	APawn *NoiseOwner = this->Instigator;
	if ( !NoiseOwner )
	{
		//debugf(NAME_DevPath,"Noise by %s with no instigator", GetFullName());
		return;
	}

	APawn *OtherPawn = NoiseOwner;
	if ( !NoiseOwner->bIsPlayer )
	{
		NoiseOwner = NoiseOwner->Enemy;
		if ( !NoiseOwner || !NoiseOwner->bIsPlayer )
		{
			// non-player noise. Inform others of same class
			APawn *next = GetLevel()->GetLevelInfo()->PawnList;

			while ( next )
			{
				if ( (next != this) && (next->IsA(GetClass()) || this->IsA(next->GetClass())) 
					&& next->CanHear(Location, Loudness) )
					next->eventHearNoise(Loudness, OtherPawn);
				next = next->nextPawn;
			}
			return;
		}
		if ( NoiseOwner->IsProbing(NAME_HearNoise ) // for bots
			&& (NoiseOwner != OtherPawn)
			&& NoiseOwner->CanHear(Location, Loudness) ) 
				NoiseOwner->eventHearNoise(Loudness, OtherPawn);		
	}

	if ( GetLevel()->TimeSeconds - NoiseOwner->noise1time >= 0.3 )
	{
		NoiseOwner->noise1time = GetLevel()->TimeSeconds;
		NoiseOwner->noise1spot = Location;
		NoiseOwner->noise1other = OtherPawn;
		NoiseOwner->noise1loudness = Loudness;
		return;
	}

	if ( GetLevel()->TimeSeconds - NoiseOwner->noise2time >= 0.3 )
	{
		NoiseOwner->noise2time = GetLevel()->TimeSeconds;
		NoiseOwner->noise2spot = Location;
		NoiseOwner->noise2other = OtherPawn;
		NoiseOwner->noise2loudness = Loudness;
		return;
	}

	if ( (NoiseOwner->noise1spot - Location).SizeSquared() < (NoiseOwner->noise2spot - Location).SizeSquared() )
	{
		NoiseOwner->noise1time = GetLevel()->TimeSeconds;
		NoiseOwner->noise1spot = Location;
		NoiseOwner->noise1other = OtherPawn;
		NoiseOwner->noise1loudness = Loudness;
		return;
	}

	NoiseOwner->noise2time = GetLevel()->TimeSeconds;
	NoiseOwner->noise2spot = Location;
	NoiseOwner->noise2other = OtherPawn;
	NoiseOwner->noise2loudness = Loudness;
	return;
	unguard;
}

void APawn::CheckEnemyVisible()
{
	guard(APawn::CheckEnemyVisible);

	clock(XLevel->SeePlayer);
	if ( Enemy )
	{
		check(Enemy->IsValid());
		if ( !LineOfSightTo(Enemy) )
			eventEnemyNotVisible();
		if ( Enemy && IsProbing(NAME_HearNoise) && Enemy->bIsPlayer )
		{
			if ( (Enemy->noise1other != this) && (GetLevel()->TimeSeconds - Enemy->noise1time <= 0.3)  
					&& CanHear(Enemy->noise1spot, Enemy->noise1loudness) )
				eventHearNoise(Enemy->noise1loudness, Enemy->noise1other);
			if ( Enemy && (Enemy->noise2other != this) && (GetLevel()->TimeSeconds - Enemy->noise2time <= 0.3)  
					&& CanHear(Enemy->noise2spot, Enemy->noise1loudness) )
				eventHearNoise(Enemy->noise2loudness, Enemy->noise2other);
		}
	}
	unclock(XLevel->SeePlayer);

	unguard;
}


/* Player shows self to pawns that are ready
*/
void APawn::ShowSelf()
{
	guard(APawn::ShowSelf);

	clock(XLevel->SeePlayer);
	APawn *Pawn = GetLevel()->GetLevelInfo()->PawnList;

	while ( Pawn )
	{
		if ( (Pawn != this) && (Pawn->SightCounter < 0.0) )
		{
			//check visibility
			int bSeePlayer = ( Pawn->IsProbing(NAME_SeePlayer) && Pawn->LineOfSightTo(this, 1) );
			if ( bSeePlayer )
				Pawn->eventSeePlayer(this);
			//check hearing
			if ( Pawn->IsProbing(NAME_HearNoise) )
			{
				if ( noise1other && (noise1other != Pawn) && (GetLevel()->TimeSeconds - noise1time <= 0.3) && (!bSeePlayer || (noise1other != this)) 
						&& Pawn->CanHear(noise1spot, noise1loudness) )
					Pawn->eventHearNoise(noise1loudness, noise1other);
				if ( noise2other && (noise2other != Pawn) && (GetLevel()->TimeSeconds - noise2time <= 0.3) && (!bSeePlayer || (noise2other != this)) 
						&& Pawn->CanHear(noise2spot, noise2loudness) )
					Pawn->eventHearNoise(noise2loudness, noise2other);
			}
		}
		Pawn = Pawn->nextPawn;
	}

	unclock(XLevel->SeePlayer);
	unguard;
}

int APawn::actorReachable(AActor *Other, int bKnowVisible)
{
	guard(AActor::actorReachable);

	if (!Other)
		return 0;

	FVector Dir = Other->Location - Location;
	FLOAT distsq = Dir.SizeSquared();
	debug(Other->Region.Zone!=NULL);

	if ( !Other->IsA(APawn::StaticClass) )
	{
		if ( !GIsEditor ) //only look past 800 for pawns
		{
			if (distsq > 640000.0) //non-pawns must be within 800.0
				return 0;
			if (Other->Region.Zone->bPainZone) //FIXME - unless bNoPain
				return 0;
		}
	}
	else if (((APawn *)Other)->FootRegion.Zone->bPainZone) //FIXME - unless bNoPain
		return 0;
	
	if ( Other->Region.Zone->bWaterZone && !bCanSwim)
		return 0;

	//check other visible
	if ( !bKnowVisible )
	{
		FCheckResult Hit(1.0);
		FVector	ViewPoint = Location;
		ViewPoint.Z += BaseEyeHeight; //look from eyes
		GetLevel()->SingleLineCheck(Hit, this, Other->Location, ViewPoint, TRACE_VisBlocking);
		if ( (Hit.Time != 1.0) && (Hit.Actor != Other) )
			return 0;
	}

	if (Other->IsA(APawn::StaticClass))
	{
		FLOAT Threshold = CollisionRadius + ::Min(1.5f * CollisionRadius, MeleeRange) + Other->CollisionRadius;
		FLOAT Thresholdsq = Threshold * Threshold;
		if (distsq <= Thresholdsq)
			return 1;
		else if (distsq > 640000.0)
		{
			FLOAT dist = Dir.Size();
			Threshold = ::Max(dist - 800.f, Threshold);
		}
		FVector realLoc = Location;
		FVector aPoint = Other->Location; //adjust destination
		if ( GetLevel()->FarMoveActor(this, Other->Location, 1) )
		{
			aPoint = Location;
			GetLevel()->FarMoveActor(this, realLoc,1,1);
		}
		return Reachable(aPoint, Threshold, Other);
	}
	else
	{
		FVector realLoc = Location;
		FVector aPoint = Other->Location;
		if ( GetLevel()->FarMoveActor(this, Other->Location, 1) )
		{
			aPoint = Location; //adjust destination
			GetLevel()->FarMoveActor(this, realLoc,1,1);
		}
		if ( Other->bBlockActors || Other->IsA(AWarpZoneMarker::StaticClass) )
			return Reachable(aPoint, 15.0, Other);
		else
			return Reachable(aPoint, 15.0, NULL);
	}
	return 0;
	unguard;
}

int APawn::pointReachable(FVector aPoint, int bKnowVisible)
{
	guard(AActor::pointReachable);

	if (!GIsEditor)
	{
		FVector Dir2D = aPoint - Location;
		Dir2D.Z = 0.0;
		if (Dir2D.SizeSquared() > 640000.0) //points must be within 800.0
			return 0;
	}

	FPointRegion NewRegion = GetLevel()->Model->PointRegion( Level, aPoint );
	if (!Region.Zone->bWaterZone && !bCanSwim && NewRegion.Zone->bWaterZone)
		return 0;

	if (!FootRegion.Zone->bPainZone && NewRegion.Zone->bPainZone)
		return 0;

	//check aPoint visible
	if ( !bKnowVisible )
	{
		FCheckResult Hit(1.0);
		FVector	ViewPoint = Location;
		ViewPoint.Z += BaseEyeHeight; //look from eyes
		GetLevel()->SingleLineCheck(Hit, this, aPoint, ViewPoint, TRACE_VisBlocking);
		if (Hit.Time != 1.0)
			return 0;
	}

	FVector realLoc = Location;
	if ( GetLevel()->FarMoveActor(this, aPoint, 1) )
	{
		aPoint = Location; //adjust destination
		GetLevel()->FarMoveActor(this, realLoc,1,1);
	}
	return Reachable(aPoint, 15.0, NULL);

	unguard;
}

int APawn::Reachable(FVector aPoint, FLOAT Threshold, AActor* GoalActor)
{
	guard(AActor::Reachable);

	if (Region.Zone->bWaterZone)
		return swimReachable(aPoint, Threshold, 0, GoalActor);
	else if (Physics == PHYS_Walking)
		return walkReachable(aPoint, Threshold, 0, GoalActor);
	else if (Physics == PHYS_Flying)
		return flyReachable(aPoint, Threshold, 0, GoalActor);
	else //if (Physics == PHYS_Falling)
		return 0; // jumpReachable(aPoint, Threshold, 0, GoalActor);

	unguard;
}

int APawn::flyReachable(FVector Dest, FLOAT Threshold, int reachFlags, AActor* GoalActor)
{
	guard(APawn::flyReachable);

	reachFlags = reachFlags | R_FLY;
	int success = 0;
	FVector OriginalPos = Location;
	FVector realVel = Velocity;
	int stillmoving = 1;
	FLOAT closeSquared = Threshold * Threshold; 
	FVector Direction = Dest - Location;
	FLOAT Movesize = ::Max(200.f, CollisionRadius);
	FLOAT MoveSizeSquared = Movesize * Movesize;
	int ticks = 100; 

	while (stillmoving) 
	{
		Direction = Dest - Location;
		FLOAT DistanceSquared = Direction.SizeSquared(); 
		if ( (DistanceSquared > closeSquared) || (Abs(Direction.Z) > CollisionHeight) )  //move not too small to do
		{
			if ( DistanceSquared < MoveSizeSquared ) 
				stillmoving = flyMove(Direction, GoalActor, 8.0, 0);
			else
			{
				Direction = Direction.SafeNormal();
				stillmoving = flyMove(Direction * Movesize, GoalActor, 4.1, 0);
			}
			if ( stillmoving == 5 ) //bumped into goal
			{
				stillmoving = 0;
				success = 1;
			}
			if ( stillmoving && Region.Zone->bWaterZone )
			{
				stillmoving = 0;
				if ( bCanSwim && (!Region.Zone->bPainZone || (Region.Zone->DamageType == ReducedDamageType)) )
				{
					reachFlags = swimReachable(Dest, Threshold, reachFlags, GoalActor);
					success = reachFlags;
				}
			}
		}
		else
		{
			stillmoving = 0;
			success = 1;
		}
		ticks--;
		if (ticks < 0)
			stillmoving = 0;
	}

	if ( !success && GoalActor && GoalActor->IsA(AWarpZoneMarker::StaticClass) )
		success = ( Region.Zone == ((AWarpZoneMarker *)GoalActor)->markedWarpZone );
	GetLevel()->FarMoveActor(this, OriginalPos, 1, 1); //move actor back to starting point
	Velocity = realVel;	
	if (success)
		return reachFlags;
	else
		return 0;
	unguard;
}

int APawn::swimReachable(FVector Dest, FLOAT Threshold, int reachFlags, AActor* GoalActor)
{
	guard(APawn::swimReachable);

	//debugf("swim reach");
	reachFlags = reachFlags | R_SWIM;
	int success = 0;
	FVector OriginalPos = Location;
	FVector realVel = Velocity;
	int stillmoving = 1;
	FLOAT closeSquared = Threshold * Threshold; 
	FVector Direction = Dest - Location;
	FLOAT Movesize = ::Max(200.f, CollisionRadius);
	FLOAT MoveSizeSquared = Movesize * Movesize;
	int ticks = 100; 

	while (stillmoving) 
	{
		Direction = Dest - Location;
		FLOAT DistanceSquared = Direction.SizeSquared(); 
		if ( (DistanceSquared > closeSquared) || (Abs(Direction.Z) > CollisionHeight) )  //move not too small to do
		{
			if ( DistanceSquared < MoveSizeSquared ) 
				stillmoving = swimMove(Direction, GoalActor, 8.0, 0);
			else
			{
				Direction = Direction.SafeNormal();
				stillmoving = swimMove(Direction * Movesize, GoalActor, 4.1, 0);
			}
			if ( stillmoving == 5 ) //bumped into goal
			{
				stillmoving = 0;
				success = 1;
			}
			if ( !Region.Zone->bWaterZone )
			{
				stillmoving = 0;
				if (bCanFly)
				{
					reachFlags = flyReachable(Dest, Threshold, reachFlags, GoalActor);
					success = reachFlags;
				}
				else if ( bCanWalk && (Dest.Z < Location.Z + CollisionHeight + MaxStepHeight) )
				{
					FCheckResult Hit(1.0);
					GetLevel()->MoveActor(this, FVector(0,0,(CollisionHeight + MaxStepHeight)), Rotation, Hit, 1, 1);
					if (Hit.Time == 1.0)
					{
						success = flyReachable(Dest, Threshold, reachFlags, GoalActor);
						reachFlags = R_WALK | (success & !R_FLY);
					}
				}
			}
			else if ( Region.Zone->bPainZone && (Region.Zone->DamageType != ReducedDamageType) )
			{
				stillmoving = 0;
				success = 0;
			}
		}
		else
		{
			stillmoving = 0;
			success = 1;
		}
		ticks--;
		if (ticks < 0)
			stillmoving = 0;
	}

	if ( !success && GoalActor && GoalActor->IsA(AWarpZoneMarker::StaticClass) )
		success = ( Region.Zone == ((AWarpZoneMarker *)GoalActor)->markedWarpZone );

	GetLevel()->FarMoveActor(this, OriginalPos, 1, 1); //move actor back to starting point
	Velocity = realVel;	
	if (success)
		return reachFlags;
	else
		return 0;
	unguard;
}
/*walkReachable() -
//walkReachable returns 0 if Actor cannot reach dest, and 1 if it can reach dest by moving in
// straight line
FIXME - take into account zones (lava, water, etc.). - note that pathbuilder should
initialize Scout to be able to do all these things
// actor must remain on ground at all times
// Note that Actor is not moved (when all is said and done)
// FIXME - allow jumping up and down if bCanJump (false for Scout!)

*/
int APawn::walkReachable(FVector Dest, FLOAT Threshold, int reachFlags, AActor* GoalActor)
{
	guard(APawn::walkReachable);
	reachFlags = reachFlags | R_WALK;
	int success = 0;
	FVector OriginalPos = Location;
	FVector realVel = Velocity;
	int stillmoving = 1;
	FLOAT closeSquared = Threshold * Threshold; //should it be less for path building? its 15 * 15
	FLOAT Movesize = 16.0; 
	FVector Direction;
	if (!GIsEditor)
	{
		if (bCanJump)
			Movesize = ::Max(128.f, CollisionRadius);
		else
			Movesize = CollisionRadius;
	}
	
	int ticks = 100; 
	FLOAT MoveSizeSquared = Movesize * Movesize;
	FLOAT MaxZDiff = CollisionHeight;
	if ( GoalActor )
		MaxZDiff = ::Max(CollisionHeight, GoalActor->CollisionHeight);
	FCheckResult Hit(1.0);

	while (stillmoving == 1) 
	{
		Direction = Dest - Location;
		FLOAT Zdiff = Direction.Z;
		Direction.Z = 0; //this is a 2D move
		FLOAT DistanceSquared = Direction.SizeSquared(); //2D size
		if ( (Zdiff > MaxZDiff) && (DistanceSquared < 0.8 * (Zdiff - MaxZDiff) * (Zdiff - MaxZDiff)) )
			stillmoving = 0; //too steep to get there
		else
		{
			if (DistanceSquared > closeSquared) //move not too small to do
			{
				if (DistanceSquared < MoveSizeSquared) 
					stillmoving = walkMove(Direction, Hit, GoalActor, 8.0, 0);
				else
				{
					Direction = Direction.SafeNormal();
					stillmoving = walkMove(Direction * Movesize, Hit, GoalActor, 4.1, 0);
				} 
				if (stillmoving != 1)
				{
					if ( stillmoving == 5 ) //bumped into goal
					{
						stillmoving = 0;
						success = 1;
					}
					else if ( Region.ZoneNumber == 0 )
					{
						stillmoving = 0;
						success = 0;
					}
					else if (bCanFly)
					{
						stillmoving = 0;
						reachFlags = flyReachable(Dest, Threshold, reachFlags, GoalActor);
						success = reachFlags;
					}
					else if (bCanJump) 
					{
						reachFlags = reachFlags | R_JUMP;
						if (stillmoving == -1) 
						{
							FVector Landing;
							Direction = Direction.SafeNormal();
							stillmoving = FindBestJump(Dest, GroundSpeed * Direction, Landing, 1);
						}
						else if (stillmoving == 0)
						{
							FVector Landing;
							Direction = Direction.SafeNormal();
							stillmoving = FindJumpUp(Dest, GroundSpeed * Direction, Landing, 1);
						}
					}
					else if ( (stillmoving == -1) && (Movesize > MaxStepHeight) ) //try smaller  
					{
						stillmoving = 1;
						Movesize = MaxStepHeight;
					}
				}
				/*else // FIXME - make sure fully on path
				{
					FCheckResult Hit(1.0);
					GetLevel()->SingleLineCheck(Hit, this, Location + FVector(0,0,-1 * (0.5 * CollisionHeight + MaxStepHeight + 4.0)) , Location, TRACE_VisBlocking, 0.5 * GetCylinderExtent());
					if ( Hit.Time == 1.0 )
						reachFlags = reachFlags | R_JUMP;	
				}
				*/
				if ( FootRegion.Zone->bPainZone )
				{
					stillmoving = 0;
					success = 0;
				}
				if ( Region.Zone->bWaterZone ) 
				{
					stillmoving = 0;
					if (bCanSwim && (!Region.Zone->bPainZone || (Region.Zone->DamageType == ReducedDamageType)) )
					{
						reachFlags = swimReachable(Dest, Threshold, reachFlags, GoalActor);
						success = reachFlags;
					}
				}
			}
			else
			{
				stillmoving = 0;
				if ( Abs(Zdiff) < MaxZDiff )
					success = 1;
				else if ( (Hit.Normal.Z < 0.95) && (Hit.Normal.Z > 0.7) )
				{
					// check if above because of slope
					if ( (Zdiff < 0) 
						&& (Zdiff * -1 < CollisionHeight + CollisionRadius * appSqrt(1/(Hit.Normal.Z * Hit.Normal.Z) - 1)) )
						success = 1;
					else 
					{
						// might be below because on slope
						FLOAT adjRad = 46; //Navigation point default
						if ( GoalActor )
							adjRad = GoalActor->CollisionRadius;
						if ( (CollisionRadius < adjRad) 
							&& (Zdiff < MaxZDiff + (adjRad + 15 - CollisionRadius) * appSqrt(1/(Hit.Normal.Z * Hit.Normal.Z) - 1)) ) 
							success = 1;
					}
				}
			}
			ticks--;
			if (ticks < 0)
				stillmoving = 0;
		}
	}

	if ( !success && GoalActor && GoalActor->IsA(AWarpZoneMarker::StaticClass) )
		success = ( Region.Zone == ((AWarpZoneMarker *)GoalActor)->markedWarpZone );

	GetLevel()->FarMoveActor(this, OriginalPos, 1, 1); //move actor back to starting point
	Velocity = realVel;
	if (success)
		return reachFlags;
	else
		return 0;
	unguard;
}

int APawn::jumpReachable(FVector Dest, FLOAT Threshold, int reachFlags, AActor* GoalActor)
{
	guard(APawn::jumpReachable);

	reachFlags = reachFlags | R_JUMP;
	FVector OriginalPos = Location;
	FVector Landing;
	jumpLanding(Velocity, Landing, 1); 
	if ( Landing == OriginalPos )
		return 0;
	int success = walkReachable(Dest, Threshold, reachFlags, GoalActor);
	GetLevel()->FarMoveActor(this, OriginalPos, 1, 1); //move actor back to starting point
	return success;
	unguard;
}

/* jumpLanding()
determine landing position of current fall, given testVel as initial velocity.
Assumes near-zero acceleration by pawn during jump (make sure creatures do this FIXME)
*/
void APawn::jumpLanding(FVector testVel, FVector &Landing, int movePawn)
{
	guard(APawn::jumpLanding);


	FVector OriginalPos = Location;
	int landed = 0;
	int ticks = 0;
	FLOAT tickTime = 0.1;

	while (!landed)
	{
		testVel = testVel * (1 - Region.Zone->ZoneFluidFriction * tickTime) + Region.Zone->ZoneGravity * tickTime; 
		FVector Adjusted = (testVel + Region.Zone->ZoneVelocity) * tickTime;
		FCheckResult Hit(1.0);
		GetLevel()->MoveActor(this, Adjusted, Rotation, Hit, 1, 1);
		if (Hit.Time < 1.0)
		{
			if ( (Hit.Normal.Z > 0.7) || Region.Zone->bWaterZone)
				landed = 1;
			else
			{
				FVector OldHitNormal = Hit.Normal;
				FVector Delta = (Adjusted - Hit.Normal * (Adjusted | Hit.Normal)) * (1.0 - Hit.Time);
				if( (Delta | Adjusted) >= 0 )
				{
					GetLevel()->MoveActor(this, Delta, Rotation, Hit, 1, 1);
					if (Hit.Time < 1.0) //hit second wall
					{
						if (Hit.Normal.Z > 0.7)
							landed = 1;	
						FVector DesiredDir = Adjusted.SafeNormal();
						TwoWallAdjust(DesiredDir, Delta, Hit.Normal, OldHitNormal, Hit.Time);
						GetLevel()->MoveActor(this, Delta, Rotation, Hit, 1, 1);
						if (Hit.Normal.Z > 0.7)
							landed = 1;
					}
				}
			}
		}
		ticks++;
		if ( (Region.ZoneNumber == 0) || (ticks > 35) || (testVel.SizeSquared() > 2500000.f) ) 
		{
			GetLevel()->FarMoveActor(this, OriginalPos, 1, 1); //move actor back to starting point
			landed = 1;
		}
	}

	Landing = Location;
	if (!movePawn)
		GetLevel()->FarMoveActor(this, OriginalPos, 1, 1); //move actor back to starting point

	unguard;
}

int APawn::FindJumpUp(FVector Dest, FVector vel, FVector &Landing, int moveActor)
{
	guard(APawn::FindJumpUp);

	float realStep = MaxStepHeight;
	MaxStepHeight = 48; 
	FVector Direction = vel.SafeNormal();
	FCheckResult Hit(1.0);
	int success = walkMove(Direction * realStep, Hit, NULL, 4.1, 1);
	if ( success == 5 )
		success = 1;
	MaxStepHeight = realStep;
	return success;

	unguard;
}

void APawn::SuggestJumpVelocity(FVector Dest, FVector &vel)
{
	guard(APawn::SuggestJumpVelocity);

	//determine how long I might be in the air 
	//FIXME - make sure creatures set no acceleration (normal of direction) while jumping down
	FLOAT StartVelZ = vel.Z;
	FLOAT floor = Dest.Z - Location.Z;
	FLOAT currentZ = 0.0;
	FLOAT gravZ = Region.Zone->ZoneGravity.Z;
	FLOAT ticks = 0.0;
	while ( (currentZ > floor) || (vel.Z > 0.0) )
	{
		vel.Z = vel.Z + gravZ * 0.05;
		ticks += 0.05; 
		currentZ = currentZ + vel.Z * 0.05;
	}
	if (Abs(vel.Z) > 1.0) 
		ticks = ticks - (currentZ - floor)/vel.Z; //correct overshoot
	vel = Dest - Location;
	vel.Z = 0.0;
	if (ticks > 0.0)
	{
		FLOAT velsize = vel.Size();
		vel = vel/velsize;
		velsize = Min(1.f * GroundSpeed, velsize/ticks); //FIXME - longwinded because of compiler bug
		vel *= velsize;
	}
	else
	{
		vel = vel.SafeNormal();
		vel *= GroundSpeed;
	}

	vel.Z = StartVelZ;

	unguard;
}

/* Find best jump from current position toward destination.  Assumes that there is no immediate 
barrier.  Sets vel to the suggested initial velocity, Landing to the expected Landing, 
and moves actor if moveActor is set */
int APawn::FindBestJump(FVector Dest, FVector vel, FVector &Landing, int movePawn)
{
	guard(APawn::FindBestJump);

	FVector realLocation = Location;

	vel.Z = JumpZ;
	SuggestJumpVelocity(Dest, vel);

	// Now imagine jump
	jumpLanding(vel, Landing, 1);
	FVector olddist = Dest - realLocation;
	FVector dist = Dest - Location;
	int success;
	if (FootRegion.Zone->bPainZone)
		success = 0;
	else if (!bCanSwim && Region.Zone->bWaterZone)
		success = 0;
	else
	{
		FLOAT netchange = olddist.Size();
		netchange -= dist.Size();
		success = (netchange > 8.0);
	}
	// FIXME - if failed, imagine with no jumpZ (step out first)
	if (!movePawn)
		GetLevel()->FarMoveActor(this, realLocation, 1, 1); //move actor back to starting point
	return success;

	unguard;
}


/* walkMove() 
- returns 1 if move happened, zero if it didn't because of barrier, and -1
if it didn't because of ledge
Move direction must not be adjusted.
*/
int APawn::walkMove(FVector Delta, FCheckResult& Hit, AActor* GoalActor, FLOAT threshold, int bAdjust)
{
	guard(APawn::walkMove);

	FVector StartLocation = Location;
	Delta.Z = 0.0;
	//-------------------------------------------------------------------------------------------
	//Perform the move
	FVector GravDir = FVector(0,0,-1);
	if (Region.Zone->ZoneGravity.Z > 0)
		GravDir.Z = 1; 
	FVector Down = GravDir * MaxStepHeight;
	FVector Up = -1 * Down;

	GetLevel()->MoveActor(this, Delta, Rotation, Hit, 1, 1);
	if ( GoalActor && (Hit.Actor == GoalActor) )
		return 5; //bumped into goal
	if (Hit.Time < 1.0) //try to step up
	{
		Delta = Delta * (1.0 - Hit.Time);
		GetLevel()->MoveActor(this, Up, Rotation, Hit, 1, 1); 
		GetLevel()->MoveActor(this, Delta, Rotation, Hit, 1, 1);
		if ( GoalActor && (Hit.Actor == GoalActor) )
			return 5; //bumped into goal
		GetLevel()->MoveActor(this, Down, Rotation, Hit, 1, 1);
		if ( GoalActor && (Hit.Actor == GoalActor) )
			return 5; //bumped into goal
		//Scouts want only good floors, else undo move
		if ((Hit.Time < 1.0) && (Hit.Normal.Z < 0.7))
		{
			GetLevel()->FarMoveActor(this, StartLocation, 1, 1);
			return 0;
		}
	}

	//drop to floor
	FVector Loc = Location;
	Down = GravDir * (MaxStepHeight + 2.0);
	GetLevel()->MoveActor(this, Down, Rotation, Hit, 1, 1);
	if (Hit.Time == 1.0) //then falling
	{
		if (bAdjust) 
			GetLevel()->FarMoveActor(this, StartLocation, 1, 1);
		else
			GetLevel()->FarMoveActor(this, Loc, 1, 1);
		return -1;
	}
	else if (Hit.Normal.Z < 0.7)
	{
		GetLevel()->FarMoveActor(this, StartLocation, 1, 1);
		return -1;
	}

	//check if move successful
	FVector RealMove = Location - StartLocation;
	if (RealMove.SizeSquared() < threshold * threshold) 
	{
		if (bAdjust)
			GetLevel()->FarMoveActor(this, StartLocation, 1, 1);
		return 0;
	}

	return 1;
	unguard;
}

int APawn::flyMove(FVector Delta, AActor* GoalActor, FLOAT threshold, int bAdjust)
{
	guard(APawn::flyMove);
	int result = 1;
	FVector StartLocation = Location;

	//-------------------------------------------------------------------------------------------
	//Perform the move
	FVector Down = FVector(0,0,-1) * MaxStepHeight;
	FVector Up = -1 * Down;
	FCheckResult Hit(1.0);

	GetLevel()->MoveActor(this, Delta, Rotation, Hit, 1, 1);
	if ( GoalActor && (Hit.Actor == GoalActor) )
		return 5; //bumped into goal
	if (Hit.Time < 1.0) //try to step up
	{
		Delta = Delta * (1.0 - Hit.Time);
		GetLevel()->MoveActor(this, Up, Rotation, Hit, 1, 1); 
		GetLevel()->MoveActor(this, Delta, Rotation, Hit, 1, 1);
		if ( GoalActor && (Hit.Actor == GoalActor) )
			return 5; //bumped into goal
		//GetLevel()->MoveActor(this, Down, Rotation, Hit, 1, 1);
	}

	FVector RealMove = Location - StartLocation;
	if (RealMove.SizeSquared() < threshold * threshold) 
	{
		if (bAdjust)
			GetLevel()->FarMoveActor(this, StartLocation, 1, 1);
		result = 0;
	}
	return result;
	unguard;
}

int APawn::swimMove(FVector Delta, AActor* GoalActor, FLOAT threshold, int bAdjust)
{
	guard(APawn::swimMove);
	int result = 1;
	FVector StartLocation = Location;

	//-------------------------------------------------------------------------------------------
	//Perform the move
	FVector Down = FVector(0,0,-1) * MaxStepHeight;
	FVector Up = -1 * Down;
	FCheckResult Hit(1.0);

	GetLevel()->MoveActor(this, Delta, Rotation, Hit, 1, 1);
	if ( GoalActor && (Hit.Actor == GoalActor) )
		return 5; //bumped into goal
	if (!Region.Zone->bWaterZone)
	{
		FVector End = Location;
		findWaterLine(StartLocation, End);
		if (End != Location)
			GetLevel()->MoveActor(this, End - Location, Rotation, Hit, 1, 1);
		return 0;
	}
	else if (Hit.Time < 1.0) //try to step up
	{
		Delta = Delta * (1.0 - Hit.Time);
		GetLevel()->MoveActor(this, Up, Rotation, Hit, 1, 1); 
		GetLevel()->MoveActor(this, Delta, Rotation, Hit, 1, 1);
		if ( GoalActor && (Hit.Actor == GoalActor) )
			return 5; //bumped into goal
		//GetLevel()->MoveActor(this, Down, Rotation, Hit, 1, 1);
	}

	FVector RealMove = Location - StartLocation;
	if (RealMove.SizeSquared() < threshold * threshold) 
	{
		if (bAdjust)
			GetLevel()->FarMoveActor(this, StartLocation, 1, 1);
		result = 0;
	}
	return result;
	unguard;
}

/* PickWallAdjust()
Check if could jump up over obstruction (only if there is a knee height obstruction)
If so, start jump, and return current destination
Else, try to step around - return a destination 90 degrees right or left depending on traces
out and floor checks

*/
int APawn::PickWallAdjust()
{
	guard(APawn::PickWallAdjust);

	if ( Physics == PHYS_Falling )
		return 0;

	// first pick likely dir with traces, then check with testmove
	FCheckResult Hit(1.0);
	FVector ViewPoint = Location;
	ViewPoint.Z += BaseEyeHeight; //look from eyes
	FVector Dir = Destination - Location;
	FLOAT zdiff = Dir.Z;
	FLOAT AdjustDist = CollisionRadius + 16;
	Dir.Z = 0;
	if ( (zdiff < CollisionHeight) && ((Dir | Dir) < CollisionRadius * CollisionRadius) )
		return 0;
	
	Dir = Dir.SafeNormal();
	FVector Out = CollisionRadius * 2 * Dir;
	GetLevel()->SingleLineCheck(Hit, this, Destination, ViewPoint, TRACE_VisBlocking); 
	if (Hit.Time < 1.0) 
	{
		if ( (Physics != PHYS_Swimming) && (Physics != PHYS_Flying) )
			AdjustDist = 2 * CollisionRadius + 16;
		else if (zdiff > 0)
		{
			FVector Up = FVector(0,0, CollisionHeight);
			GetLevel()->SingleLineCheck(Hit, this, Location + 2 * Up, Location, TRACE_VisBlocking, GetCylinderExtent());
			if (Hit.Time == 1.0)
			{
				Destination = Location + Up;
				return 1;
			}
		}
	}

	//try left and right
	FVector Left = FVector(Dir.Y, -1 * Dir.X, 0);
	INT bCheckRight = 0;
	FVector CheckLeft = Left * 1.42 * CollisionRadius;
	GetLevel()->SingleLineCheck(Hit, this, Destination, ViewPoint + CheckLeft, TRACE_VisBlocking); 
	if (Hit.Time < 1.0) //try right
	{
		bCheckRight = 1;
		Left *= -1;
		CheckLeft *= -1;
		GetLevel()->SingleLineCheck(Hit, this, Destination, ViewPoint + CheckLeft, TRACE_VisBlocking); 
	}

	if (Hit.Time < 1.0) //neither side has visibility
		return 0;

	//try step left or right
	Out = 14 * Dir;
	Left *= AdjustDist;
	GetLevel()->SingleLineCheck(Hit, this, Location + Left, Location, TRACE_VisBlocking, GetCylinderExtent());
	if (Hit.Time == 1.0)
	{
		GetLevel()->SingleLineCheck(Hit, this, Location + Left + Out, Location + Left, TRACE_VisBlocking, GetCylinderExtent());
		if (Hit.Time == 1.0)
		{
			Destination = Location + Left;
			return 1;
		}
	}
	
	if ( !bCheckRight ) // if didn't already try right, now try it
	{
		CheckLeft *= -1;
		GetLevel()->SingleLineCheck(Hit, this, Destination, ViewPoint + CheckLeft, TRACE_VisBlocking); 
		if ( Hit.Time < 1.0 )
			return 0;
		Left *= -1;
		GetLevel()->SingleLineCheck(Hit, this, Location + Left, Location, TRACE_VisBlocking, GetCylinderExtent());
		if (Hit.Time == 1.0)
		{
			GetLevel()->SingleLineCheck(Hit, this, Location + Left + Out, Location + Left, TRACE_VisBlocking, GetCylinderExtent());
			if (Hit.Time == 1.0)
			{
				Destination = Location + Left;
				return 1;
			}
		}
	}

	//try jump up if walking, or adjust up or down if swimming
	if ( Physics == PHYS_Walking )
	{
		if (!bCanJump)
			return 0;
		FVector Up;
		Up = FVector(0,0,48); 

		GetLevel()->SingleLineCheck(Hit, this, Location + Up, Location, TRACE_VisBlocking, GetCylinderExtent());
		if (Hit.Time == 1.0)
		{
			GetLevel()->SingleLineCheck(Hit, this, Location + Up + Out, Location + Up, TRACE_VisBlocking, GetCylinderExtent());
			if (Hit.Time == 1.0)
			{
				Velocity = GroundSpeed * Dir;
				Acceleration = AccelRate * Dir;
				Velocity.Z = JumpZ;
				bJumpOffPawn = true; // don't let script adjust this jump again
				setPhysics(PHYS_Falling);
				return 1;
			}
		}
	}
	else 
	{
		FVector Up = FVector(0,0,CollisionHeight);
		GetLevel()->SingleLineCheck(Hit, this, Location + Up, Location, TRACE_VisBlocking, GetCylinderExtent());
		if (Hit.Time == 1.0)
		{
			GetLevel()->SingleLineCheck(Hit, this, Location + Up + Out, Location + Up, TRACE_VisBlocking, GetCylinderExtent());
			if (Hit.Time == 1.0)
			{
				Destination = Location + Up;
				return 1;
			}
		}

		Up *= -1; //try adjusting down
		GetLevel()->SingleLineCheck(Hit, this, Location + Up, Location, TRACE_VisBlocking, GetCylinderExtent());
		if (Hit.Time == 1.0)
		{
			GetLevel()->SingleLineCheck(Hit, this, Location + Up + Out, Location + Up, TRACE_VisBlocking, GetCylinderExtent());
			if (Hit.Time == 1.0)
			{
				Destination = Location + Up;
				return 1;
			}
		}
	}

	return 0;
	unguard;
}

void APawn::HandleSpecial(AActor *&bestPath)
{
	guard(APawn::HandleSpecial);

	AActor * realPath = bestPath;
	AActor * newPath = bestPath->eventSpecialHandling(this);
	bestPath = newPath;
	if ( !newPath)
		return;

	if ( bestPath && (bestPath != realPath) )
	{
		if ( !bCanDoSpecial )
		{
			bestPath = NULL;
			return;
		}
		SpecialGoal = bestPath;
		if ( actorReachable(bestPath) )
		{
			if ( bestPath->IsProbing(NAME_SpecialHandling) )
			{
				AActor * ReturnedActor = bestPath->eventSpecialHandling(this);
				if ( !ReturnedActor )
					bestPath = NULL;
				else if ( bestPath != ReturnedActor )
				{
					if ( (bestPath != realPath) && actorReachable(ReturnedActor) )
						bestPath = ReturnedActor;
					else
						bestPath = NULL;
				}
			}
		}
		else 
		{
			int success = findPathToward(bestPath, 0, newPath, 1);
			if ( !success || (newPath == realPath) ) 
				bestPath = NULL;
			else
			{
				SpecialGoal = bestPath; 
				bestPath = newPath;
			}
		}
	}
	unguard;
}
