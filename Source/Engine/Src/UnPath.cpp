/*=============================================================================
	UnPath.cpp: Unreal pathnode placement

  These methods are members of the FPathBuilder class, which adds pathnodes to a level.  
  Paths are placed when the level is built.  FPathBuilder is not used during game play
 
   General comments:
   Path building
   The FPathBuilder does a tour of the level (or a part of it) using the "Scout" actor, starting from
   every actor in the level.  
   This guarantees that correct reachable paths are generated to all actors.  It starts by going to the
   wall to the right of the actor, and following that wall, keeping it to the left.  NOTE: my definition
   of left and right is based on normal Cartesian coordinates, and not screen coordinates (e.g. Y increases
   upward, not downward).  This wall following is accomplished by moving along the wall, constantly looking
   for a leftward passage.  If the FPathBuilder is stopped, he rotates clockwise until he can move forward 
   again.  While performing this tour, it keeps track of the set of previously placed pathnodes which are
   visible and reachable.  If a pathnode is removed from this set, then the FPathBuilder searches for an 
   acceptable waypoint.  If none is found, a new pathnode is added to provide a waypoint back to the no longer
   reachable pathnode.  The tour ends when a full circumlocution has been made, or if the FPathBuilder 
   encounters a previously placed path node going in the same direction.  Pathnodes that were not placed as
   the result of a left turn are marked, since they are due to some possibly unmapped obstruction.  In the
   final phase, these obstructions are inspected.  Then, the pathnode due to the obstruction is removed (since
   the obstruction is now mapped, and any needed waypoints have been built).

	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

  FIXME - ledge and landing marking
  FIXME - mark door centers with left turns (reduces total paths)
  FIXME - paths on top of all platforms
	Revision history:
		* Created by Steven Polge 3/97
=============================================================================*/

#include "EnginePrivate.h"

#if DEBUGGINGPATHS
    static inline void CDECL DebugPrint(const char * Message, ...)
    {
        static int Count = 0;
        if( Count <= 300 ) // To reduce total messages.
        {
            char Text[4096];
			GET_VARARGS( Text, Message );
            debugf( NAME_Log, Text );
        }
    }
    static inline void CDECL DebugVector(const char *Text, const FVector & Vector )
    {
        char VectorText[100];
        char Message[100];
        appSprintf( VectorText, "[%4.4f,%4.4f,%4.4f]", Vector.X, Vector.Y, Vector.Z );
        appSprintf( Message, Text, VectorText );
        DebugPrint(Message);
		DebugPrint(VectorText);
    }
	static inline void CDECL DebugFloat(const char *Text, const FLOAT & val )
    {
        char VectorText[100];
        char Message[100];
        appSprintf( VectorText, "[%4.4f]", val);
        appSprintf( Message, Text, VectorText );
        DebugPrint(Message);
		DebugPrint(VectorText);
    }
	static inline void CDECL DebugInt(const char *Text, const int & val )
    {
        char VectorText[100];
        char Message[100];
        appSprintf( VectorText, "[%6d]", val);
        appSprintf( Message, Text, VectorText );
        DebugPrint(Message);
		DebugPrint(VectorText);
    }
#else
    static inline void CDECL DebugPrint(const char * Message, ...)
    {
    }
     static inline void CDECL DebugVector(const char *Text, const FVector & Vector )
    {
    }
	static inline void CDECL DebugFloat(const char *Text, const FLOAT & val )
    {
    }
	static inline void CDECL DebugInt(const char *Text, const int & val )
    {
    }
   
#endif

//FIXME: add hiding places, camping and sniping spots (not pathnodes, but other keypoints)
int ENGINE_API FPathBuilder::buildPaths (ULevel *ownerLevel, int optimization)
{
	guard(FPathBuilder::buildPaths);

	int numpaths = 0;
	numMarkers = 0;
	Level = ownerLevel;
	pathMarkers = new FPathMarker[MAXMARKERS]; //FIXME - use DArray?
	getScout();

	humanRadius = 52; 
	FLOAT humanHeight = 50; 
	Scout->SetCollision(1, 1, 1);
	Scout->bCollideWorld = 1;
	Scout->SetCollisionSize(humanRadius, humanHeight);

	Scout->JumpZ = -1.0; //NO jumping
	Scout->GroundSpeed = 320; //FIXME?
	Scout->MaxStepHeight = 24; //FIXME
	//DebugFloat("collision radius", Scout->CollisionRadius);
	//DebugFloat("collision height", Scout->CollisionHeight);
	optlevel = optimization;
	numpaths = numpaths + createPaths(optimization);
	Level->DestroyActor(Scout);
	return numpaths;
	unguard;
}

int ENGINE_API FPathBuilder::removePaths (ULevel *ownerLevel)
{
	guard(FPathBuilder::removePaths);
	Level = ownerLevel;
	int removed = 0;

	for (INT i=0; i<Level->Num(); i++)
	{
		AActor *Actor = Level->Actors(i); 
		if (Actor && Actor->IsA(APathNode::StaticClass))
		{
			removed++;
			Level->DestroyActor( Actor ); 
		}
	}
	return removed;
	unguard;
}

int ENGINE_API FPathBuilder::showPaths (ULevel *ownerLevel)
{
	guard(FPathBuilder::showPaths);
	Level = ownerLevel;
	int shown = 0;

	for (INT i=0; i<Level->Num(); i++)
	{
		AActor *Actor = Level->Actors(i); 
		if (Actor && Actor->IsA(APathNode::StaticClass))
		{
			shown++;
			Actor->DrawType = DT_Sprite; 
		}
	}
	return shown;
	unguard;
}

int ENGINE_API FPathBuilder::hidePaths (ULevel *ownerLevel)
{
	guard(FPathBuilder::hidePaths);
	Level = ownerLevel;
	int shown = 0;

	for (INT i=0; i<Level->Num(); i++)
	{
		AActor *Actor = Level->Actors(i); 
		if (Actor && Actor->IsA(APathNode::StaticClass))
		{
			shown++;
			Actor->DrawType = DT_None; 
		}
	}
	return shown;
	unguard;
}

void ENGINE_API FPathBuilder::undefinePaths (ULevel *ownerLevel)
{
	guard(FPathBuilder::undefinePaths);
	Level = ownerLevel;

	//remove all reachspecs
	debugf(NAME_DevPath,"Remove %d old reachspecs", Level->ReachSpecs.Num());
	Level->ReachSpecs.Empty();

	// clear navigationpointlist
	Level->GetLevelInfo()->NavigationPointList = NULL;

	//clear pathnodes
	for (INT i=0; i<Level->Num(); i++)
	{
		AActor *Actor = Level->Actors(i); 
		if (Actor && Actor->IsA(ANavigationPoint::StaticClass))
		{
			if ( Actor->IsA(AWarpZoneMarker::StaticClass) || Actor->IsA(ATriggerMarker::StaticClass) 
					|| Actor->IsA(AInventorySpot::StaticClass) || Actor->IsA(AButtonMarker::StaticClass) )
				Level->DestroyActor(Actor);
			else
			{
				((ANavigationPoint *)Actor)->nextNavigationPoint = NULL;
				for (INT i=0; i<16; i++)
				{
					((ANavigationPoint *)Actor)->Paths[i] = -1;
					((ANavigationPoint *)Actor)->upstreamPaths[i] = -1;
					((ANavigationPoint *)Actor)->PrunedPaths[i] = -1;
				}
			}
		}
	}
	unguard;
}

void ENGINE_API FPathBuilder::definePaths (ULevel *ownerLevel)
{
	guard(FPathBuilder::definePaths);
	Level = ownerLevel;
	getScout();
	Level->GetLevelInfo()->NavigationPointList = NULL;

	// Add WarpZoneMarkers and InventorySpots
	debugf( NAME_DevPath, "Add WarpZone and Inventory markers" );
	for (INT i=0; i<Level->Num(); i++)
	{
		AActor *Actor = Level->Actors(i); 
		if ( Actor )
		{
			if ( Actor->IsA(AWarpZoneInfo::StaticClass) )
			{
				if ( !findScoutStart(Actor->Location) || (Scout->Region.Zone != Actor->Region.Zone) )
				{
					Scout->SetCollisionSize(20, Scout->CollisionHeight);
					if ( !findScoutStart(Actor->Location) || (Scout->Region.Zone != Actor->Region.Zone) )
						Level->FarMoveActor(Scout, Actor->Location, 1, 1);
					Scout->SetCollisionSize(humanRadius, Scout->CollisionHeight);
				}
				UClass *pathClass = FindObjectChecked<UClass>( ANY_PACKAGE, "WarpZoneMarker" );
				AWarpZoneMarker *newMarker = (AWarpZoneMarker *)Level->SpawnActor(pathClass, NAME_None, NULL, NULL, Scout->Location);
				newMarker->markedWarpZone = (AWarpZoneInfo *)Actor;
			}
			else if ( Actor->IsA(AInventory::StaticClass) )
			{
				if ( !findScoutStart(Actor->Location) || (Abs(Scout->Location.Z - Actor->Location.Z) > Scout->CollisionHeight) )
					Level->FarMoveActor(Scout, Actor->Location + FVector(0,0,20), 1, 1);
				UClass *pathClass = FindObjectChecked<UClass>( ANY_PACKAGE, "InventorySpot" );
				AInventorySpot *newMarker = (AInventorySpot *)Level->SpawnActor(pathClass, NAME_None, NULL, NULL, Scout->Location);
				newMarker->markedItem = (AInventory *)Actor;
				((AInventory *)Actor)->myMarker = newMarker;
			}
		}
	}

	//calculate and add reachspecs to pathnodes
	debugf(NAME_DevPath,"Add reachspecs");
	for (i=0; i<Level->Num(); i++)
	{
		AActor *Actor = Level->Actors(i); 
		if (Actor && Actor->IsA(ANavigationPoint::StaticClass))
		{
			((ANavigationPoint *)Actor)->nextNavigationPoint = Level->GetLevelInfo()->NavigationPointList;
			Level->GetLevelInfo()->NavigationPointList = (ANavigationPoint *)Actor;
			addReachSpecs(Actor);
			debugf( NAME_DevPath, "Added reachspecs to %s",Actor->GetName() );
		}
	}

	debugf(NAME_DevPath,"Added %d reachspecs", Level->ReachSpecs.Num()); 
	//remove extra reachspecs from teleporters

	//prune excess reachspecs
	debugf(NAME_DevPath,"Prune reachspecs");
	ANavigationPoint *Nav = Level->GetLevelInfo()->NavigationPointList;
	int numPruned = 0;
	while (Nav)
	{
		numPruned += Prune(Nav);
		Nav = Nav->nextNavigationPoint;
	}
	debugf(NAME_DevPath,"Pruned %d reachspecs", numPruned);

	// check for doors, and update reachspecs
	/*
	ANavigationPoint *Nav = Level->GetLevelInfo()->NavigationPointList;
	Level->SetActorCollision(1);
	while (Nav)
	{
		CheckDoor(Nav);
		Nav = Nav->nextNavigationPoint;
	}
	Level->SetActorCollision(0);
	debugf("Check for doors");
	*/
	Level->DestroyActor(Scout);
	debugf(NAME_DevPath,"All done");
	unguard;
}

//------------------------------------------------------------------------------------------------
//Private methods

/*	Check if there is a door along this path

void FPathBuilder::CheckDoor(AActor *Node)
{
	guard(FPathBuilder::CheckDoor);
	
	ANavigationPoint* NavNode = (ANavigationPoint *)Node;
	FCheckResult Hit;
	FReachSpec aPath;
	int i=0;
	while ( (i<16) && (NavNode->upstreamPaths[i] != -1) )
	{
		aPath = Level->ReachSpecs(NavNode->upstreamPaths[i]);
		Level->SingleLineCheck(Hit, this, aPath->End, aPath->Start, TRACE_VisBlocking, GetCylinderExtent());
		if ( (Hit.Actor != None) &&  )

		i++;
	}
	unguard;
}
*/
/*	if Node is an acceptable waypoint between an upstream path and a downstreampath
	who are also connected, remove their connection
*/
int FPathBuilder::Prune(AActor *Node)
{
	guard(FPathBuilder::Prune);
	
	int pruned = 0;
	ANavigationPoint* NavNode = (ANavigationPoint *)Node;

	int n,j;
	FReachSpec alternatePath, straightPath, part1, part2;
	int i=0;
	while ( (i<16) && (NavNode->upstreamPaths[i] != -1) )
	{
		part1 = Level->ReachSpecs(NavNode->upstreamPaths[i]);
		n=0;
		while ( (n<16) && (NavNode->Paths[n] != -1) )
		{
			part2 = Level->ReachSpecs(NavNode->Paths[n]);
			INT straightPathIndex = specFor(part1.Start, part2.End);
			if (straightPathIndex != -1)
			{
				straightPath = Level->ReachSpecs(straightPathIndex);
				alternatePath = part1 + part2;
				if ( ((float)straightPath.distance * 1.2 >= (float)alternatePath.distance) 
					&& ((alternatePath <= straightPath) || straightPath.BotOnlyPath() 
						|| alternatePath.MonsterPath()) )
				{
					//prune straightpath
					pruned++;
					j=0;
					ANavigationPoint* StartNode = (ANavigationPoint *)(straightPath.Start);
					ANavigationPoint* EndNode = (ANavigationPoint *)(straightPath.End);
					//debugf("Prune reachspec %d from %s to %s because of %s", straightPathIndex,
					//	StartNode->GetName(), EndNode->GetName(), NavNode->GetName()); 
					while ( (j<15) && (StartNode->Paths[j] != straightPathIndex) )
						j++;
					if ( StartNode->Paths[j] == straightPathIndex )
					{
						while ( (j<15) && (StartNode->Paths[j] != -1) )
						{
							StartNode->Paths[j] = StartNode->Paths[j+1];
							j++;
						}
						StartNode->Paths[15] = -1;
					}

					j=0;
					while ( (j<15) && (StartNode->PrunedPaths[j] != -1) )
						j++;
					StartNode->PrunedPaths[j] = straightPathIndex;
					Level->ReachSpecs(straightPathIndex).bPruned = 1;

					j=0;
					while ( (j<15) && (EndNode->upstreamPaths[j] != straightPathIndex) )
						j++;
					if ( EndNode->upstreamPaths[j] == straightPathIndex )
					{
						while ( (j<15) && (EndNode->upstreamPaths[j] != -1) )
						{
							EndNode->upstreamPaths[j] = EndNode->upstreamPaths[j+1];
							j++;
						}
						EndNode->upstreamPaths[15] = -1;
					}
					// Note that all specs remain in reachspec list and still referenced in PrunedPaths[]
					// but removed from visible Navigation graph:
				}
			}
			n++;
		}
		i++;
	}

	return pruned;
	unguard;
}

int FPathBuilder::specFor(AActor* Start, AActor* End)
{
	guard(FPathBuilder::specFor);

	FReachSpec pathSpec;
	ANavigationPoint* StartNode = (ANavigationPoint *)Start;
	int i=0;
	while ( (i<16) && (StartNode->Paths[i] != -1) )
	{
		pathSpec = Level->ReachSpecs(StartNode->Paths[i]);
		if (pathSpec.End == End)
			return StartNode->Paths[i];
		i++;
	}
	return -1;
	unguard;
}

/* insert a reachspec into the array, ordered by longest to shortest distance.
However, if there is no room left in the array, remove longest first
*/
int FPathBuilder::insertReachSpec(INT *SpecArray, FReachSpec &Spec)
{
	guard(FPathBuilder::insertReachSpec);

	int n = 0;
	while ( (n < 16) && (SpecArray[n] != -1) && (Level->ReachSpecs(SpecArray[n]).distance > Spec.distance) )
		n++;
	int pos = n;

	if (SpecArray[15] == -1) //then not full
	{
		if (SpecArray[n] != -1)
		{
			int current = SpecArray[n];
			while (n < 15)
			{
				int next = SpecArray[n+1];
				SpecArray[n+1] = current;
				current = next;
				if (next == -1)
					n = 15;
				n++;
			}
		}
	}
	else if (n == 0) // current is bigger than biggest
	{
		//debugf("Node out of Reachspecs - don't add!");
		return -1;
	}
	else //full, so remove from front
	{
		//debugf("Node out of Reachspecs -add at %d !", pos-1);

		n--;
		pos = n;
		int current = SpecArray[n];
		while (n > 0)
		{
			int next = SpecArray[n-1];
			SpecArray[n-1] = current;
			current = next;
			n--;
		}
	}
	return pos;

	/* Shortest to longest - slower in inner loop of breadpath search
	INT n = 0;
	while ( (n < 16) && (SpecArray[n] != -1) && (Level->ReachSpecs(SpecArray[n]).distance < Spec.distance) )
		n++;
	
	if ( n == 16 )
		return -1;

	for ( INT i=15; i>n; i-- )
		SpecArray[i] = SpecArray[i-1];

	return n;
	*/
	unguard;
}

/* add reachspecs to path for every path reachable from it. Also add the reachspec to that
paths upstreamPath list
*/
void FPathBuilder::addReachSpecs(AActor *start)
{
	guard(FPathBuilder::addReachspecs);

	FReachSpec newSpec;
	ANavigationPoint *node = (ANavigationPoint *)start;
	//debugf("Add Reachspecs for node at (%f, %f, %f)", node->Location.X,node->Location.Y,node->Location.Z);

	if ( node->IsA(ALiftCenter::StaticClass) )
	{
		FName myLiftTag = ((ALiftCenter *)node)->LiftTag;
		for (INT i=0; i<Level->Num(); i++)
		{
			AActor *Actor = Level->Actors(i); 
			if ( Actor && Actor->IsA(ALiftExit::StaticClass) && (((ALiftExit *)Actor)->LiftTag == myLiftTag) ) 
			{
				newSpec.Init();
				newSpec.CollisionRadius = 60;
				newSpec.CollisionHeight = 60;
				newSpec.reachFlags = R_SPECIAL;
				newSpec.Start = node;
				newSpec.End = Actor;
				newSpec.distance = 150;
				int pos = insertReachSpec(node->Paths, newSpec);
				if (pos != -1)
				{
					int iSpec = Level->ReachSpecs.AddItem(newSpec);
					node->Paths[pos] = iSpec;
					pos = insertReachSpec(((ANavigationPoint *)Actor)->upstreamPaths, newSpec);
					if (pos != -1)
						((ANavigationPoint *)Actor)->upstreamPaths[pos] = iSpec;
				}
				newSpec.Init();
				newSpec.CollisionRadius = 60;
				newSpec.CollisionHeight = 60;
				newSpec.reachFlags = R_SPECIAL;
				newSpec.Start = Actor;
				newSpec.End = node;
				newSpec.distance = 150;
				pos = insertReachSpec(((ANavigationPoint *)Actor)->Paths, newSpec);
				if (pos != -1)
				{
					int iSpec = Level->ReachSpecs.AddItem(newSpec);
					((ANavigationPoint *)Actor)->Paths[pos] = iSpec;
					pos = insertReachSpec(node->upstreamPaths, newSpec);
					if (pos != -1)
						node->upstreamPaths[pos] = iSpec;
				}
			}
		}
		return;
	}

	if ( node->IsA(ATeleporter::StaticClass) || node->IsA(AWarpZoneMarker::StaticClass) )
	{
		for (INT i=0; i<Level->Num(); i++)
		{
			int bFoundMatch = 0;
			AActor *Actor = Level->Actors(i); 
			if ( node->IsA(ATeleporter::StaticClass) )
			{
				if ( Actor && Actor->IsA(ATeleporter::StaticClass) && (Actor != node) ) 
					bFoundMatch = ( appStricmp(((ATeleporter *)node)->URL, *Actor->Tag) == 0 );
			}
			else if ( Actor && Actor->IsA(AWarpZoneMarker::StaticClass) && (Actor != node) )
				bFoundMatch = ( appStricmp(((AWarpZoneMarker *)node)->markedWarpZone->OtherSideURL, *((AWarpZoneMarker *)Actor)->markedWarpZone->ThisTag) == 0 );

			if ( bFoundMatch )
			{
				newSpec.Init();
				newSpec.CollisionRadius = 150;
				newSpec.CollisionHeight = 150;
				newSpec.reachFlags = R_SPECIAL;
				newSpec.Start = node;
				newSpec.End = Actor;
				newSpec.distance = 100;
				int pos = insertReachSpec(node->Paths, newSpec);
				if (pos != -1)
				{
					int iSpec = Level->ReachSpecs.AddItem(newSpec);
					//debugf("     Add teleport reachspec %d to node at (%f, %f, %f)", iSpec, Actor->Location.X,Actor->Location.Y,Actor->Location.Z);
					node->Paths[pos] = iSpec;
					pos = insertReachSpec(((ANavigationPoint *)Actor)->upstreamPaths, newSpec);
					if (pos != -1)
						((ANavigationPoint *)Actor)->upstreamPaths[pos] = iSpec;
				}
				break;
			}
		}
	}

	for (INT i=0; i<Level->Num(); i++)
	{
		AActor *Actor = Level->Actors(i); 
		if (Actor && Actor->IsA(ANavigationPoint::StaticClass) && !Actor->IsA(ALiftCenter::StaticClass) 
				&& (Actor != node) && ((node->Location - Actor->Location).SizeSquared() < 1000000) )
		{
			newSpec.Init();
			if (newSpec.defineFor(node, Actor, Scout))
			{
				int pos = insertReachSpec(node->Paths, newSpec);
				if (pos != -1)
				{
					int iSpec = Level->ReachSpecs.AddItem(newSpec);
					//debugf("     Add reachspec %d to node at (%f, %f, %f)", iSpec, Actor->Location.X,Actor->Location.Y,Actor->Location.Z);
					node->Paths[pos] = iSpec;
					pos = insertReachSpec(((ANavigationPoint *)Actor)->upstreamPaths, newSpec);
					if (pos != -1)
						((ANavigationPoint *)Actor)->upstreamPaths[pos] = iSpec;
				} 
			}
		}
	}
	unguard;
}

/* createPaths()
build paths for a given pawn type (which Scout is emulating)
*/
int FPathBuilder::createPaths (int optimization)
{
	guard(FPathBuilder::createPaths);
/*	int numpaths = 0;
	numMarkers = 0;

	//Add a permanent+leftturn+beacon+marked path for every pathnode actor in the level
	int newMarker;
	for (INT i=0; i<Level->Num(); i++) 
	{
		AActor *Actor = Level->Actors(i);
		if (Actor)
		{
			if (Actor->IsA(APathNode::StaticClass))
			{
				DebugPrint("Found a Pathnode");
				newMarker = addMarker();
				pathMarkers[newMarker].initialize(Actor->Location,FVector(0,0,0),1,1,1);
				pathMarkers[newMarker].permanent = 1;
			}
			else if (Actor->IsA(APawn::StaticClass) && !Actor->IsA(AScout::StaticClass))
				Actor->SetCollision(0, 0, 0); //temporarily turn off Pawn collision while placing paths
		}
	}

	// build paths from every actor position in level
	for (i=0; i<Level->Num(); i++) 
	{
		AActor *Actor = Level->Actors(i);
		if ( Actor && (!Actor->Location.IsZero()) && (!Actor->IsA(APathNode::StaticClass)) && (!Actor->IsA(AScout::StaticClass)))
		{
			DebugInt("----------------------Starting From", i);
			DebugVector("Location ", Actor->Location); 
			createPathsFrom(Actor->Location); 
		}
	}
	
	//iteratively fill out paths by checking for obstructions and by checking for new connections
	// until no more markers are added - unless opt0
	
		DebugInt("Markers before obstruction check =", numMarkers);
		DebugPrint("Check obstructions--------------------------------------------------");

	if (optimization < 2)
	{
		int notDone = optimization + 1;
		while (notDone)
		{
			int oldmarkers = numMarkers;
			for (i=0; i<oldmarkers; i++)  
			{
				if (pathMarkers[i].marked)
				{
					DebugInt("Check obstruction at",i);
					DebugInt("Out of", numMarkers);
					if (!pathMarkers[i].leftTurn) //only check from left turns at opt2
						checkObstructionFrom(&pathMarkers[i]);	
					pathMarkers[i].marked = 0; //once the obstruction is mapped, remove the path
				}
			}
			notDone--;
		}
	}
	else //optimization == 2
	{
		for (i=0; i<numMarkers; i++)  
		{
			if (pathMarkers[i].marked)
			{
				DebugInt("Check obstruction at",i);
				DebugInt("Out of", numMarkers);
			 	checkObstructionFrom(&pathMarkers[i]); //only check from left turns at opt2	
				pathMarkers[i].marked = 0; //once the obstruction is mapped, remove the path
			}
		}
	}

	// pre-merge "identical paths"
	for (i=0; i<numMarkers; i++)  
	{
		if (pathMarkers[i].leftTurn)
			premergePath(i);
	}

	//try to move left turn markers out from walls
	for (i=0; i<numMarkers; i++)  
	{
		if (pathMarkers[i].leftTurn)
			adjustPath(i);
	}

	// merge and prune left turn markers (based on reachability to beacons)
	for (i=0; i<numMarkers; i++)  
	{
		if (pathMarkers[i].leftTurn)
			mergePath(i);
	}

	DebugPrint("Build Paths");
	//Now create an actor for all remaining left turn markers (except permanent paths which already have a node
	for (i=0; i<numMarkers; i++)  
	{
		if (pathMarkers[i].leftTurn && !pathMarkers[i].permanent)
		{
			newPath(pathMarkers[i].Location);
			numpaths++;
		}
	}
	
	for (i=0; i<Level->Num(); i++) 
	{
		AActor *Actor = Level->Actors(i);
		if (Actor && (Actor->IsA(APawn::StaticClass)))
			Actor->SetCollision(1, 1, 1); //turn Pawn collision back on
	}

	DebugInt("Optimization Level = ", optimization);
	DebugInt("Number of Markers =",numMarkers);
	return numpaths; */ return 0;
	unguard;
}

//newPath() 
//- add new pathnode to level at position spot
void FPathBuilder::newPath(FVector spot)
{
	guard(FPathBuilder::newPath);
	
	if (Scout->CollisionHeight < 48) // fixme - base on Skaarj final height
		spot.Z = spot.Z + 48 - Scout->CollisionHeight;
	UClass *pathClass = FindObjectChecked<UClass>( ANY_PACKAGE, "PathNode" );
	APathNode *addedPath = (APathNode *)Level->SpawnActor( pathClass, NAME_None, NULL, NULL, spot );
	//clear pathnode reachspec lists
	for (INT i=0; i<16; i++)
	{
		addedPath->Paths[i] = -1;
		addedPath->upstreamPaths[i] = -1;
	}

	unguard;
	};


/*getScout()
Find the scout actor in the level. If none exists, add one.
*/ 

void FPathBuilder::getScout()
{
	guard(FPathBuilder::findScout);
	Scout = NULL;
	for( INT i=0; i<Level->Num(); i++ )
	{
		AActor *Actor = Level->Actors(i); 
		if (Actor && Actor->IsA(AScout::StaticClass))
			Scout = (APawn *)Actor;
	}
	if( !Scout )
	{
		UClass *scoutClass = FindObjectChecked<UClass>( ANY_PACKAGE, "Scout" );
		Scout = (APawn *)Level->SpawnActor( scoutClass );
	}
	Scout->SetCollision(1,1,1);
	Scout->bCollideWorld = 1;
	Level->SetActorZone( Scout,1,1 );
	return;
	unguard;
}


int FPathBuilder::findScoutStart(FVector start)
{
	guard(FPathBuilder::findScoutStart);
	
	if (Level->FarMoveActor(Scout, start)) //move Scout to starting point
	{
		//slide down to floor
		FCheckResult Hit(1.0);
		FVector Down = FVector(0,0, -50);
		Hit.Normal.Z = 0.0;
		INT iters = 0;
		while (Hit.Normal.Z < 0.7)
		{
			Level->MoveActor(Scout, Down, Scout->Rotation, Hit, 1,1);
			if ((Hit.Time < 1.0) && (Hit.Normal.Z < 0.7)) 
			{
				//adjust and try again
				FVector OldHitNormal = Hit.Normal;
				FVector Delta = (Down - Hit.Normal * (Down | Hit.Normal)) * (1.0 - Hit.Time);
				if( (Delta | Down) >= 0 )
				{
					Level->MoveActor(Scout, Delta, Scout->Rotation, Hit, 1,1);
					if ((Hit.Time < 1.0) && (Hit.Normal.Z < 0.7))
					{
						FVector downDir = Down.SafeNormal();
						Scout->TwoWallAdjust(downDir, Delta, Hit.Normal, OldHitNormal, Hit.Time);
						Level->MoveActor(Scout, Delta, Scout->Rotation, Hit, 1,1);
					}
				}
			}
			iters++;
			if (iters >= 50)
			{
				debugf(NAME_DevPath,"No valid start found");
				return 0;
			}
		}
		debugf(NAME_DevPath,"scout placed on valid floor");
		return 1;
 	}

	debugf(NAME_DevPath,"Scout didn't fit");
	return 0;
	unguard;
}
/*
//createPathsFrom() -
//create paths beginning from wall to the right of start location
void FPathBuilder::createPathsFrom(FVector start)
{
	guard(FPathBuilder::createPathsFrom);
	
	if (findScoutStart(start)) //move Scout to starting point
	{
		FVector moveDirection = FVector(1,0,0);
		exploreWall(moveDirection);
	}
	return;
	unguard;
}

/* adjustPath()
adjust the left turn marker out from its corner
FIXME - use farmoves instead of walk?

void FPathBuilder::adjustPath(INT iMarker)
{
	guard(FPathBuilder::adjustPath);
	
	FPathMarker *marker = &pathMarkers[iMarker];
	
	FVector OutDir;
	FVector NonWallDir;
	NonWallDir.X = marker->Direction.Y;
	NonWallDir.Y = -1.0 * marker->Direction.X;
	NonWallDir.Z = 0.0;

	if (optlevel == 0)
		optlevel = 1; //always check when adjusting

	if (marker->stair)
		return; //don't adjust at all - FIXME adjust if doesn't screw up smaller creatures (check bottom)
	else
	{
		OutDir.X = 0.707106781 * (marker->Direction.X + marker->Direction.Y);
		OutDir.Y = 0.707106781 * (marker->Direction.Y - marker->Direction.X);
		OutDir.Z = 0.0;
	}

	Scout->SetCollisionSize(humanRadius, Scout->CollisionHeight);
	markReachable(marker->Location);  //mark all markers reachable from this marker
	marker->leftTurn = 0; //don't use as waypoint
	marker->radius = humanRadius;
	FLOAT DesiredRadius = MAXCOMMONRADIUS;
	FVector GoodSpot = marker->Location;
	FLOAT AdjustSize = 0.5 * (DesiredRadius - humanRadius);

	while ((AdjustSize > 1.0) && (DesiredRadius <= MAXCOMMONRADIUS)) 
	{
		Scout->SetCollisionSize(DesiredRadius, Scout->CollisionHeight);
		FVector DesiredAdjust = OutDir * 1.415 * (DesiredRadius - humanRadius);
		FVector testSpot = marker->Location + DesiredAdjust;
		int success = 0;
		if (Level->FarMoveActor(Scout, testSpot))
		{
			//test if can reach old marker, at human radius
			Scout->SetCollisionSize(humanRadius, Scout->CollisionHeight);
			Level->DropToFloor(Scout); 
			if (fullyReachable(Scout->Location, marker->Location))
			{
				if (!needPath(Scout->Location))
				{
					success = 1;
					debugf(NAME_DevPath,"works with radius %f", DesiredRadius);
					GoodSpot = Scout->Location;
					marker->radius = DesiredRadius;
					DesiredRadius += AdjustSize;
					AdjustSize *= 0.5;
				}
			}
		}

		if (!success)
		{
			DesiredRadius -= AdjustSize;
			AdjustSize *= 0.5;
		}
	}
	
	FVector Adjustment = GoodSpot - marker->Location;
	DebugInt("Adjusted path ", iMarker);
	DebugFloat("By ", Adjustment.Size()); 
	marker->Location = GoodSpot;
	marker->leftTurn = 1;
	unguard;
}

/* checkmergeSpot()
worker function for mergePath()

int FPathBuilder::checkmergeSpot(const FVector &spot, FPathMarker *path1, FPathMarker *path2)
{
	guard(FPathBuilder::checkmergeSpot);
	int acceptable = 1;
	FLOAT oldRadius = Scout->CollisionRadius;
	
	for (INT i=0; i<numMarkers; i++) //check if reachable path list changed
	{
		if (acceptable && pathMarkers[i].visible && pathMarkers[i].beacon)
		{
			Scout->SetCollisionSize(humanRadius, Scout->CollisionHeight);
			if (!fullyReachable(spot,pathMarkers[i].Location))
			{
				path1->leftTurn = 0;
				path2->leftTurn = 0; //don't use either of these as a waypoint
				acceptable = findPathTo(pathMarkers[i].Location);
				path1->leftTurn = 1;
				path2->leftTurn = 1;
			}
			//if (!acceptable) DebugVector("failed because of",pathMarkers[i].Location);
		}
		if (acceptable && pathMarkers[i].bigvisible && pathMarkers[i].leftTurn)
		{
			Scout->SetCollisionSize(Max(path1->radius, path2->radius), Scout->CollisionHeight);
			if (!fullyReachable(spot,pathMarkers[i].Location))
			{
				path1->leftTurn = 0;
				path2->leftTurn = 0; //don't use either of these as a waypoint
				acceptable = findPathTo(pathMarkers[i].Location);
				path1->leftTurn = 1;
				path2->leftTurn = 1;
			}
			//if (!acceptable) DebugVector("failed because of",pathMarkers[i].Location);
		}
	}
	Scout->SetCollisionSize(oldRadius, Scout->CollisionHeight);
	return acceptable;
	unguard;
}

int FPathBuilder::markReachableFromTwo(FPathMarker *path1, FPathMarker *path2)
{
	guard(FPathBuilder::markReachableFromTwo);

	FLOAT oldRadius = Scout->CollisionRadius;
	//mark human reachable as visible
	Scout->CollisionRadius = humanRadius;
	markReachable(path1->Location);  //mark all markers reachable from marker 1
	int addedmarkers = 0;
	for (INT j=0; j<numMarkers; j++) // add those reachable from marker 2
	{
		if (!pathMarkers[j].visible && pathMarkers[j].beacon)
		{
			pathMarkers[j].visible = fullyReachable(path2->Location, pathMarkers[j].Location);
			if (pathMarkers[j].visible)
				addedmarkers = 1;
		}
	}

	// mark big radius reachable leftturns as bigvisible
	Scout->SetCollisionSize(Max(path1->radius, path2->radius), Scout->CollisionHeight);
	if (Scout->CollisionRadius > humanRadius)
	{
		for (INT i=0; i<numMarkers; i++) 
		{
			if (pathMarkers[i].leftTurn) 
				pathMarkers[i].bigvisible = fullyReachable(path1->Location,pathMarkers[i].Location);
		}
		for (j=0; j<numMarkers; j++) // add those reachable from marker 2
		{
			if (!pathMarkers[j].bigvisible && pathMarkers[j].leftTurn)
			{
				pathMarkers[j].bigvisible = fullyReachable(path2->Location, pathMarkers[j].Location);
				if (pathMarkers[j].bigvisible)
					addedmarkers = 1;
			}
		}
	}
	Scout->SetCollisionSize(oldRadius, Scout->CollisionHeight);
	return addedmarkers;
	unguard;
}

/* premergePath()
look for other nearby nodes that are identical in terms of reachability, and merge these

void FPathBuilder::premergePath(INT iMarker)
{
	guard(FPathBuilder::premergePath);
	
	FPathMarker *marker = &pathMarkers[iMarker];
	marker->radius = humanRadius;
	FLOAT maxmergesqr = MAXCOMMONRADIUS * MAXCOMMONRADIUS; //fixme reduce to just MAXRADIUS squared?
	for (INT i=0; i<numMarkers; i++) 
	{
		FPathMarker *candidate = &pathMarkers[i];
		if (candidate->leftTurn && !candidate->permanent && (i != iMarker))
		{
			FVector direction = marker->Location - candidate->Location;
			Scout->SetCollisionSize(humanRadius, Scout->CollisionHeight);
			candidate->radius = humanRadius;
			if (direction.SizeSquared() < maxmergesqr )  
				if (fullyReachable(marker->Location, candidate->Location))
				{
					DebugVector("Try to pre-merge path at", marker->Location);
					DebugVector("And path at", candidate->Location);
					int addedmarkers = markReachableFromTwo(marker, candidate);  //mark all markers reachable from marker
					int acceptable = 0;
					if (marker->permanent) //then can't move it
					{
						acceptable = !addedmarkers;
						if (acceptable)
						{
							Level->FarMoveActor(Scout, marker->Location);
							Level->DropToFloor(Scout); //FIXME - why do I need this?
						}
					}
					else //try to find suitable in-between point
					{
						FVector span = candidate->Location - marker->Location;
						FVector start = 0.5 * (marker->Location + candidate->Location);
						//First try between the two
						FVector dir;
						dir.X = -1.0/span.Y;
						dir.Y = span.X;
						dir.Z = 0.0;
						dir.Normalize();

						FVector end = start + dir * 0.5 * span.Size();
						Level->FarMoveActor(Scout, start);
						Level->DropToFloor(Scout); //FIXME - why do I need this?
						int stillmoving = 1;
						while (stillmoving) 
						{
							acceptable = checkmergeSpot(Scout->Location, marker, candidate);
							if (!acceptable) 
								stillmoving = walkToward(end,6.5); //SEPJUN
							else
								stillmoving = 0;
						}

						if (!acceptable)
						{
							end = start - dir * 0.5 * span.Size();
							Level->FarMoveActor(Scout, start);
							Level->DropToFloor(Scout); //FIXME - why do I need this?
							int stillmoving = 1;
							while (stillmoving) 
							{
								acceptable = checkmergeSpot(Scout->Location, marker, candidate);
								if (!acceptable) 
									stillmoving = walkToward(end,6.5); //SEPJUN
								else
									stillmoving = 0;
							}
						}
					}
				
					if (acceptable) //found an acceptable merge point
						{
							DebugVector("Successful merge at", Scout->Location);
							marker->Location = Scout->Location; //move to merge point
							candidate->leftTurn = 0; //mark other node for removal
							candidate->beacon = 0; //other node no longer beacon
						}
				}
		}
	}
	
	return;
	unguard;
}
/* mergePath()
examine reachable path nodes. Try to merge this pathnode with another pathnode.  
Between it and every nearby reachable permanent pathnode, look for a point at which
all beacon pathnodes reachable from either can be reached from this point 

To be merged, the candidate point must support full reachability both at a human radius, and at the
maximum of the two path node radii.

void FPathBuilder::mergePath(INT iMarker)
{
	guard(FPathBuilder::mergePath);
	
	FPathMarker *marker = &pathMarkers[iMarker];
	FLOAT maxmergesqr = 4 * MAXCOMMONRADIUS * MAXCOMMONRADIUS; //fixme reduce to just MAXRADIUS squared?
	for (INT i=0; i<numMarkers; i++) 
	{
		FPathMarker *candidate = &pathMarkers[i];
		if (candidate->leftTurn && !candidate->permanent && (i != iMarker))
		{
			FVector direction = marker->Location - candidate->Location;
			Scout->SetCollisionSize(Max(marker->radius, candidate->radius), Scout->CollisionHeight);
			if (direction.SizeSquared() < maxmergesqr )  
				if (fullyReachable(marker->Location, candidate->Location))
				{
					//DebugVector("Try to merge path at", marker->Location);
					//DebugVector("And path at", candidate->Location);
					int addedmarkers = markReachableFromTwo(marker, candidate);  //mark all markers reachable from marker
					int acceptable = 0;
					if (marker->permanent) //then can't move it
					{
						acceptable = !addedmarkers;
						if (acceptable)
						{
							Level->FarMoveActor(Scout, marker->Location);
							Level->DropToFloor(Scout); //FIXME - why do I need this?
						}
					}
					else //try to find suitable in-between point
					{
						FVector span = candidate->Location - marker->Location;
						FVector start = 0.5 * (marker->Location + candidate->Location);
						//First try between the two
						FVector dir;
						dir.X = -1.0/span.Y;
						dir.Y = span.X;
						dir.Z = 0.0;
						dir.Normalize();

						FVector end = start + dir * 0.5 * span.Size();
						Level->FarMoveActor(Scout, start);
						Level->DropToFloor(Scout); //FIXME - why do I need this?
						int stillmoving = 1;
						while (stillmoving) 
						{
							acceptable = checkmergeSpot(Scout->Location, marker, candidate);
							if (!acceptable) 
								stillmoving = walkToward(end,6.5); //SEPJUN
							else
								stillmoving = 0;
						}

						if (!acceptable)
						{
							end = start - dir * 0.5 * span.Size();
							Level->FarMoveActor(Scout, start);
							Level->DropToFloor(Scout); //FIXME - why do I need this?
							int stillmoving = 1;
							while (stillmoving) 
							{
								acceptable = checkmergeSpot(Scout->Location, marker, candidate);
								if (!acceptable) 
									stillmoving = walkToward(end,6.5); //SEPJUN
								else
									stillmoving = 0;
							}
						}

						if (!acceptable)
						{
							end = marker->Location;
							Level->FarMoveActor(Scout, start);
							Level->DropToFloor(Scout); //FIXME - why do I need this?
							int stillmoving = 1;
							while (stillmoving) 
							{
								acceptable = checkmergeSpot(Scout->Location, marker, candidate);
								if (!acceptable) 
									stillmoving = walkToward(end,6.5); //SEPJUN
								else
									stillmoving = 0;
							}
						}

						if (!acceptable)
						{
							end = candidate->Location;
							Level->FarMoveActor(Scout, start);
							Level->DropToFloor(Scout); //FIXME - why do I need this?
							int stillmoving = 1;
							while (stillmoving) 
							{
								acceptable = checkmergeSpot(Scout->Location, marker, candidate);
								if (!acceptable) 
									stillmoving = walkToward(end,6.5); //SEPJUN
								else
									stillmoving = 0;
							}
						}
					}
				
					if (acceptable) //found an acceptable merge point
						{
							//DebugVector("Successful merge at", Scout->Location);
							marker->Location = Scout->Location; //move to merge point
							candidate->leftTurn = 0; //mark other node for removal
							candidate->beacon = 0; //other node no longer beacon
						}
				}
		}
	}
	
	return;
	unguard;
}

//checkObstructionsFrom() -
//pathnode was marked as not being created as result of left turn - which means some other (possibly unmapped)
//obstruction created it.  Explore this obstruction
//FIXME - why does it ever fail to find a path to the obstructed marker if the obstruction has been mapped?
//(e.g. cases where findPathTo fails, but no leftturn markers added to obstruction)
void FPathBuilder::checkObstructionFrom(FPathMarker *marker)
{
	guard(FPathBuilder::checkObstructionFrom);

	if (!Level->FarMoveActor(Scout, marker->Location, 0 ,1))
		debugf(NAME_DevPath,"obstruction far move failed");
	Level->DropToFloor(Scout); //FIXME - why do I need this?
	if (marker->leftTurn) //if this is a left turn marker, then walk at current direction (look for outside wall)
	{
		DebugPrint("exploring out from left turn");
		exploreWall(marker->Direction);
	}
	else
	{
		markLeftReachable(marker->Location);
		Scout->walkMove(marker->Direction * 16.0); //move to spot where obstruction was observed	
	
		for (INT i=0; i<numMarkers; i++) //check if visible+reachable path list changed
		{
			FPathMarker *checkMarker = &pathMarkers[i];
			if (checkMarker->visible && checkMarker->leftTurn)
				if (!fullyReachable(Scout->Location,checkMarker->Location)) //vis+reach changed - examine obstruction
					if (!findPathTo(checkMarker->Location))
					{
						DebugPrint("found the obstruction");
						FVector moveDirection = checkMarker->Location - Scout->Location;
						moveDirection.Z = 0;
						moveDirection.Normalize();
						exploreWall(moveDirection);
					}				 
		}
	}
	return;
	unguard;
}

void FPathBuilder::exploreWall(const FVector &moveDirection)
{
	guard(FPathBuilder::exploreWall);
	int stillmoving = 1;
	DebugVector("Move dir",moveDirection);
	while (stillmoving == 1)
		stillmoving = Scout->walkMove(moveDirection * 16.0); //SEPJUN
		
	DebugVector("Start location", Scout->Location);

	//  follow wall
	FVector newDirection = FVector(moveDirection.Y, -moveDirection.X, 0);
	//newDirection.X = moveDirection.Y; 
	//newDirection.Y = -1.0 * moveDirection.X; //turn clockwise 90 degrees
	nearestThirtyAngle(newDirection); //FIXME don't do this if I use normal to wall
	int oldMarkers = numMarkers;
	followWall(newDirection); 
	DebugInt("New paths created", numMarkers - oldMarkers);

	return;
	unguard;
}

/*
needPath()
checks if any paths or markers marked reachable are no longer reachable from start
if so, returns true


int FPathBuilder::needPath(const FVector &start)
{
	guard(FPathBuilder::needPath);

	if (optlevel == 0)
		return 0;

	int need = 0;
	for (INT i=0; i<numMarkers; i++) //check if visible+reachable path list changed
	{
		if (!need && pathMarkers[i].visible && pathMarkers[i].beacon)
			if (!fullyReachable(start,pathMarkers[i].Location))
				need = !findPathTo(pathMarkers[i].Location); //vis+reach changed - look for an acceptable waypoint
	}
	return need;
	unguard;
}

int FPathBuilder::sawNewLeft(const FVector &start)
{
	guard(FPathBuilder::sawNewLeft);

	if (optlevel == 0)
		return 0;

	int seen = 0;
	for (INT i=0; i<numMarkers; i++) //check if visible+reachable path list changed
	{
		if (!seen && !pathMarkers[i].visible && !pathMarkers[i].routable && pathMarkers[i].leftTurn)
			if (fullyReachable(start,pathMarkers[i].Location))
				seen = 1; //vis+reach changed - look for an acceptable waypoint
	}
	return seen;
	unguard;
}
/*
markReachable()
marks all beacon paths and markers as reachable or not from start.


void FPathBuilder::markReachable(const FVector &start)
{
	guard(FPathBuilder::markReachable);

	for (INT i=0; i<numMarkers; i++) 
	{
		if (pathMarkers[i].beacon) 
			pathMarkers[i].visible = fullyReachable(start,pathMarkers[i].Location);
	}

	return;
	unguard;
}

/*
markLeftReachable()
marks all left turn markers as reachable or not from start.


void FPathBuilder::markLeftReachable(const FVector &start)
{
	guard(FPathBuilder::markLeftReachable);

	if (optlevel == 0)
		return;

	FCheckResult Hit(1.0);
	for (INT i=0; i<numMarkers; i++) 
	{
		if (pathMarkers[i].leftTurn) 
		{
			pathMarkers[i].visible = 0;
			pathMarkers[i].routable = 0;
			if (fullyReachable(start,pathMarkers[i].Location))
			{
				pathMarkers[i].visible = 1;
				//debugf("Marked %d as reachable", i);
			}
			else 
			{
				Level->SingleLineCheck(Hit, Scout, pathMarkers[i].Location, start, TRACE_Level); //VisBlocking); 
				if (Hit.Time == 1.0)
					pathMarkers[i].routable = findPathTo(pathMarkers[i].Location);
				else
					pathMarkers[i].routable = 1;
			}
		}
		else
			pathMarkers[i].visible = 0;
	}

	return;
	unguard;
}
/* oneWaypointTo()
looks for a NEARBY permanent waypoint which will reach to upstream spot.  Since scout has just made a left turn,
we are looking for a closeby waypoint which was also dropped as a result of the left turn (perhaps with
a different collision radius).  

int FPathBuilder::oneWaypointTo(const FVector &upstreamSpot)
{
	guard(FPathBuilder::oneWaypointTo);
	int success = 0;
	FLOAT maxdistSquared = MAXWAYPOINTDIST * MAXWAYPOINTDIST * Scout->CollisionRadius * Scout->CollisionRadius;
	for (INT i=0; i<numMarkers; i++) 
	{
		if (!success && pathMarkers[i].leftTurn)
		{
			FVector distance = pathMarkers[i].Location - Scout->Location;
			if (distance.SizeSquared() < maxdistSquared)
				success = (fullyReachable(pathMarkers[i].Location, upstreamSpot) && fullyReachable(Scout->Location,pathMarkers[i].Location));
		}			
	}
	if (success)
		DebugPrint("Found an acceptable alternate left turn marker");
	return success;
	unguard;
}

/* addMarker()
returns index to a new marker

INT FPathBuilder::addMarker()
{
	guard(FPathBuilder::addMarker);
	if (numMarkers < MAXMARKERS - 1)
		numMarkers++;
	else  //try to remove an old obstruction marker
	{
		int compressed = 0;
		INT i = 0;
		while (!compressed)
		{
			if (pathMarkers[i].removable())
			{
				pathMarkers[i] = pathMarkers[numMarkers - 1];
				compressed = 1;
			}
			i++;
			if (i == MAXMARKERS)
				compressed = 1;
		}
		DebugPrint("RAN OUT OF MARKERS!");
	}
	if (numMarkers > MAXMARKERS - 100) DebugInt("ADDED MARKER #", numMarkers);
	return (numMarkers - 1);
	unguard;
}

//followWall() - 
//follow wall, always keeping wall to my left
//look for reachable paths, and drop paths when my reachability list changes by removal
//stop when either I touch a path which was placed while going in the same direction as my
//current direction, or if I pass the last path I dropped
//Compare current direction to heading for start location.  
//If angle >= 90 degrees then I've passed it , and if NetYaw > 360, I've gone all the way around
//FIXME - to improve speed, remove redundant reachability checks
void FPathBuilder::followWall(FVector currentDirection)
{
	guard(FPathBuilder::followWall);
	int stillmoving = 1;  //This isn't the case, but I need to find out what it is really
	FVector newDirection;
	FLOAT NetYaw = 0.0;
	INT LastDropped = 0;
	INT LastRightTurn = 0;
	int turnedLeft = 0;
	FVector tempV;
	int turning = 0;
	FVector upstreamSpot = Scout->Location;
	FVector startLocation = Scout->Location;
	int keepMapping = 1;
	DebugVector("Following wall", currentDirection);
	int stepcount = 0;
	FCheckResult Hit(1.0);
	FVector Up(0,0,2);
	FVector Down(0,0,-2);
	FVector oldPosition = Scout->Location + FVector(2,2,2);
	int rampStuck = 0;
	int newTurn = 0;
	FVector realPosition = Scout->Location;

	while (keepMapping)
	{
		//Level->MoveActor(Scout, Up, Hit, 1, 1); //check good floor
		//Level->MoveActor(Scout, Down, Hit, 1, 1); 
		//Level->MoveActor(Scout, Hit.Normal, Hit, 1, 1);

		//if (stepcount > 5)
		//{
		//debugf("walked to %f %f %f", Scout->Location.X, Scout->Location.Y, Scout->Location.Z);
		//	stepcount = 0;
		//}
		if (!newTurn && (oldPosition - Scout->Location).Size2D() < 2.0)
			rampStuck++; //FIXME - change how I handle ramps in physics (slide up them)
		else
			rampStuck = 0;
		newTurn = 0;
		oldPosition = Scout->Location;
		FVector oldDirection = currentDirection;
		if (checkLeftPassage(currentDirection)) //made a left turn
		{
			NetYaw = NetYaw - 90.0;
			if (!fullyReachable(Scout->Location, upstreamSpot))  //can I still reach my anchor? 
				if (!oneWaypointTo(upstreamSpot)) //check if there is a nearby legal waypoint 
				{
					upstreamSpot = oldPosition;
					newTurn = 1;
					LastDropped = addMarker();
					turnedLeft = 1;
					pathMarkers[LastDropped].initialize(oldPosition,oldDirection,1,1,1);
					NetYaw = 0.0;
					startLocation = oldPosition;
					debugf(NAME_DevPath,"made left turn marker %d",LastDropped);
					stepcount = 0;
				}
		}
		else
		{
			if (rampStuck)
			{
				//Level->FarMoveActor(Scout, realPosition, 1, 1);
				//Level->MoveActor(Scout, Up, Hit, 1, 1); //check good floor
				//Level->MoveActor(Scout, Down, Hit, 1, 1); 
				//if (Hit.Normal.Z < 1.0)
				//	Level->MoveActor(Scout, Hit.Normal, Hit, 1, 1);

				debugf(NAME_DevPath,"Ramp Stuck!");
				if (rampStuck > 10)
					return;
			}
			stillmoving = Scout->walkMove(currentDirection * 16.0);
			realPosition = Scout->Location;

			if (stillmoving == 1) //check for interior obstructions
			{	
				markLeftReachable(oldPosition); //mark all visible/reachable turn and permanent paths from oldPosition
				if (needPath(Scout->Location)) //check if I need an obstruction marker at oldPosition
				{
 					if (!fullyReachable(Scout->Location, upstreamSpot) && !oneWaypointTo(upstreamSpot)
						&& (Abs(upstreamSpot.Z - Scout->Location.Z) > 1 + Scout->MaxStepHeight))
					{
							LastDropped = addMarker(); //its a stairway/ramp
							pathMarkers[LastDropped].initialize(oldPosition,oldDirection,0,1,1);
							pathMarkers->stair = 1;
							upstreamSpot = oldPosition;
							NetYaw = 0.0;
							startLocation = oldPosition;
							DebugVector("marked stairway at", oldPosition);
							stepcount = 0;
					}
					else
					{
						newTurn = 1;
						LastDropped = addMarker();
						pathMarkers[LastDropped].initialize(oldPosition,oldDirection,1,0,0);
						NetYaw = 0.0;
						startLocation = oldPosition;
						DebugVector("marked obstruction at", oldPosition);
						stepcount = 0;
					}
				}
				else if (sawNewLeft(Scout->Location))
				{
					LastDropped = addMarker();
					pathMarkers[LastDropped].initialize(Scout->Location,-1 * currentDirection,1,0,0);
					NetYaw = 0.0;
					startLocation = Scout->Location;
					DebugVector("marked out new obstruction at", Scout->Location);
					stepcount = 0;
				}
			}
		}

		if (stillmoving == 1) //check if tour is complete
		{
			turning = 0; //not in a turn
			//Stop if I touch a marker with the same direction as my current direction
			FLOAT touchRangeSquared = Scout->CollisionRadius * Scout->CollisionRadius * 0.25 * 0.25;
			INT i = 0;
			while (i<numMarkers) 
			{
				if (i != LastDropped) //Don't touch last dropped
				{
					tempV = pathMarkers[i].Location - Scout->Location;
					if (tempV.SizeSquared() < touchRangeSquared) //touching path
					{
						DebugVector("Near path at", pathMarkers[i].Location);
						tempV = currentDirection - pathMarkers[i].Direction;
						tempV.Normalize();
						DebugVector("Relative direction is", tempV);
						keepMapping = !tempV.IsNearlyZero(); //check if direction same as when path was laid
						if (!keepMapping) 
						{
							i = numMarkers;
							DebugVector("Touched a compatible marker at", pathMarkers[i].Location);
							stepcount = 0;
						}
					}
				}
				i++;
			}
		}	
		else //adjust direction clockwise (I couldn't go forward or left)
		{
			//FIXME: implement using normal of forward barrier 
			// make direction 90 degrees clockwise of xy component of normal
			//temporarily - just rotate around slowly (30 degrees at a time)
			//if current direction isn't multiple of 30, first correct to nearest 30 (so compatible path checks will work)
			upstreamSpot = Scout->Location; //set anchor at turn point
			newTurn = 1;
			DebugVector("turn right at", Scout->Location);
			stepcount = 0;
			if ((!turning) && (turnedLeft)) //check if its a stairstep right turn (odd angled wall)
					if (fullyReachable(Scout->Location, pathMarkers[LastRightTurn].Location))
					{
						turnedLeft = 0;
						turning = 1; //don't mark this turn
						DebugPrint("Not marking stair step right"); //FIXME - is this not working?
					}
			if (!turning) //mark turn
			{
				turning = 1;
				turnedLeft = 0;
				LastDropped = addMarker();
				LastRightTurn = LastDropped;
				pathMarkers[LastDropped].initialize(oldPosition,oldDirection,0,1,0);
				startLocation = oldPosition;
				NetYaw = 0.0;
				DebugPrint("made right turn marker");
			}
			if (angleNearThirty(currentDirection))
			{
				newDirection.Y = COS30 * currentDirection.Y - 0.5 * currentDirection.X;
				newDirection.X = COS30 * currentDirection.X + 0.5 * currentDirection.Y;
				newDirection.Z = 0.0;
				currentDirection = newDirection.SafeNormal();
				stillmoving = 1;
				NetYaw += 30.0;
			}
			else //correct clockwise to nearest multiple of 30 
			{
				newDirection = currentDirection;
				nearestThirtyAngle(newDirection);
				newDirection.Z = 0.0;
				FVector OldDirection = currentDirection;
				currentDirection = newDirection.SafeNormal();
				stillmoving = 1;
				FLOAT CosTurn = OldDirection | currentDirection;
				if (CosTurn < 0.9)
					NetYaw += 25.0;
				else if (CosTurn < 0.95)
					NetYaw += 18.0;
				else if (CosTurn < 0.98)
					NetYaw += 11.0;
				else if (CosTurn < 0.995)
					NetYaw += 6.0;
				else
					NetYaw += 3.0;
			}

			newDirection.Z = 0.0;
			currentDirection = newDirection.SafeNormal();
			stillmoving = 1;
			DebugVector("new direction", currentDirection);
		}
				
		//Alternate test for stopping (in case pathnode touch fails because it was the lastDropped)
		if (keepMapping && (Abs(NetYaw) >= 360.0))  //have rotated all the way around from start
		{ 
			tempV = startLocation - Scout->Location;
			keepMapping = ((tempV | currentDirection) > 0.0); //keep mapping if angle < 90 degrees
			if (!keepMapping) DebugVector("All the way around at",Scout->Location);
		}
	} //while keepMapping

	return;
	unguard;
}

//nearestThirtyAngle()
// returns the nearest direction whose xy angle is a multiple of 30, in the clockwise direction
// assumes the FVector passed is normalized 
//FIXME move the code above to here
void FPathBuilder::nearestThirtyAngle(FVector &currentDirection)
{	
	FVector newDirection;
	newDirection.Z = 0.0;
	int signX = 2 * (currentDirection.X >= 0) - 1; //+1 if positive, -1 if negative
	int signY = 2 * (currentDirection.Y >= 0) - 1;
	FLOAT magX = Abs(currentDirection.X);
	FLOAT magY = Abs(currentDirection.Y);
	DebugVector ("correct current direction of", currentDirection);

	if (signX == signY) //quadrants I and III - increase X
	{
		if (magX > COS30)
		{
			newDirection.X = 1.0 * signX;
			newDirection.Y = 0.0;
		}
		else if (magX > 0.5)
		{
			newDirection.X = COS30 * signX;
			newDirection.Y = 0.5 * signY;
		}
		else
		{
			newDirection.X = 0.5 * signX;
			newDirection.Y = COS30 * signY;
		}
	}
	else //quadrants II and IV - decrease X
	{
		if (magX > COS30)
		{
			newDirection.X = COS30 * signX;
			newDirection.Y = 0.5 * signY;
		}
		else if (magX > 0.5)
		{
			newDirection.X = 0.5 * signX;
			newDirection.Y = COS30 * signY;
		}
		else 
		{
			newDirection.X = 0.0;
			newDirection.Y = 1.0 * signY;
		}
	}
	DebugVector("Corrected direction = ",newDirection);
	currentDirection = newDirection;
	return;
};

//angleNearThirty()
//FIXME - make this a member of FVector?
// returns true if the xy angle of the vector is near a multiple of 30 degrees
// and z direction is zero
// use FVector::IsNearlyZero(), since that's my standard measure

int FPathBuilder::angleNearThirty(FVector dir)
{
	guard(FPathBuilder::angleNearThirty);
	int result = 0;
	dir.Normalize(); 
	if (dir.Z == 0.0)
	{
		dir.X = Abs(dir.X);
		FVector test = dir;
		test.Y = 0;  //since normalized, and Z=0, know Y from X
		if (test.IsNearlyZero())
			result = 1;
		else
		{
			test.X = test.X - 1.0;
			if (test.IsNearlyZero())
				result = 1;
			else
			{
				test.X = test.X + 0.5;
				if (test.IsNearlyZero())
					result = 1;
				else
				{
					test.X = test.X + 0.5 - COS30;
					if (test.IsNearlyZero())
						result = 1;
				}
			}
		}
	}

	return result;
	unguard;
}

/* tryPathThrough()
recursively try to find a path to destination from Waypoint that fits into the distance budget
check budget first, since its cheaper than the reachability test

int FPathBuilder::tryPathThrough(FPathMarker *Waypoint, const FVector &Destination, FLOAT budget)
{
	guard(FPathBuilder::tryPathThrough);

	int result = 0;
	if (fullyReachable(Waypoint->Location,Destination))
		result = 1; //I know it fits into my budget, since I wouldn't have tried this waypoint if it didn't
	else
	{
		Waypoint->budget = budget;
		FVector direction;
		FLOAT distance;
		FLOAT minTotal;
		FPathMarker *NextNode;

		for (INT iNext=0; iNext<numMarkers; iNext++)  //check all reachable pathnodes
		{
			NextNode = &pathMarkers[iNext];
			if (!result && NextNode->leftTurn)	
			{
				direction = Waypoint->Location - NextNode->Location;
				distance = direction.Size();
				direction = NextNode->Location - Destination;
				minTotal = distance + direction.Size();
				if ((NextNode->budget < (budget - distance)) && (minTotal < budget)) //check not visited with better budget, and current min distance fits budget
					if (fullyReachable(Waypoint->Location,NextNode->Location)) //if fits in budget, try this path
						result = tryPathThrough(NextNode,Destination, budget - distance);
			}
		}
	}
	return result;
	unguard;
}

/* findPathTo() -
// iDest is no longer reachable from Scout's position, but was from oldPosition
(because of some obstruction)  If the obstruction is marked, there is an almost straight path
to iDest - look for that

int FPathBuilder::findPathTo(const FVector &Destination)
{
	guard(FPathBuilder::findPathTo);
	FVector direction = Destination - Scout->Location;
	FLOAT budget = direction.Size() + Scout->CollisionRadius * (1 + 2 * MAXWAYPOINTDIST); 
		
	//clear budgets (temp used for storing remaining budget through that pathnode)
	for (INT i=0; i<numMarkers; i++)  
	{
			pathMarkers[i].budget = 0.0;	
	}
	
	FPathMarker ScoutMarker;
	ScoutMarker.Location = Scout->Location;
	int acceptable = tryPathThrough(&ScoutMarker,Destination,budget);
	return acceptable;
	unguard;
}

/* checkLeft()
Worker function for checkLeftPassage()

int FPathBuilder::checkLeft(FVector &leftDirection, FVector &currentDirection)
{
	guard(FPathBuilder::checkLeft);
	int leftTurn = 0;
	FVector oldLocation = Scout->Location;

	int walkresult = Scout->walkMove(leftDirection * 16.0); 
	if (walkresult == 1) //check if it was a full move
	{
		FVector move = Scout->Location - oldLocation;
		walkresult = (move.Size() > 10.0);  //FIXME - problem with steep slopes?
	}
	if (walkresult == 1) //explore this path
	{
		DebugVector("Follow left passage", leftDirection);
		DebugVector("Turned left at", oldLocation);
//		Scout->walkMove(currentDirection * 4.0); //stay out from wall a little
		currentDirection.X = leftDirection.X;
		currentDirection.Y = leftDirection.Y;
		leftDirection.X = -1.0 * currentDirection.Y; //turn left 90 degrees
		leftDirection.Y = currentDirection.X;
		Scout->walkMove(leftDirection * 16.0); //get all the way over to wall
		leftTurn = 1;
		DebugVector("New location",Scout->Location);
	}
	// else if (walkresult == -1) //FIXME: mark ledge?
	else
		Level->FarMoveActor(Scout, oldLocation, 0, 1); //no left passage, go back to exploration

	return leftTurn;
	unguard;
}

/* checkLeftPassage()
Looks for a left turn.  Returns 1 if left turn was made, zero otherwise.
If possible, changes currentDirection, and moves Scout 
FIXME - should it be based on normal of barrier to the left?

int FPathBuilder::checkLeftPassage(FVector &currentDirection)
{
	guard(FPathBuilder::checkLeftPassage);
	FVector oldLocation = Scout->Location;
	FVector leftDirection;
	leftDirection.X = -1.0 * currentDirection.Y; //turn left 90 degrees
	leftDirection.Y = currentDirection.X;
	leftDirection.Z = 0;
	int leftTurn = 0;
	int stillmoving = 1;

	leftTurn = checkLeft(leftDirection, currentDirection);

	if (!leftTurn)
	{
		stillmoving = Scout->walkMove(currentDirection * 6.0); //SEPJUN
		leftTurn = checkLeft(leftDirection, currentDirection);
	}

	if (!leftTurn && stillmoving)
	{
		stillmoving = Scout->walkMove(currentDirection * 6.0); //SEPJUN
		leftTurn = checkLeft(leftDirection, currentDirection);
	}

	if (!leftTurn)
		Level->FarMoveActor(Scout, oldLocation, 0, 1);

	return leftTurn;
	unguard;
}

//check if there is a line of sight from start to destination, and if Scout can walk between the two
//only used for path building
//then check that here
int FPathBuilder::fullyReachable(FVector start,FVector destination)
{
	guard(FPathBuilder::fullyReachable);

	FVector oldPosition = Scout->Location;
	Scout->SetCollisionSize(Scout->CollisionRadius - 6.0, Scout->CollisionHeight);
	int result = Level->FarMoveActor(Scout,start);
	if (Scout->Physics != PHYS_Walking)
		debugf(NAME_DevPath,"Scout Physics is %d", Scout->Physics);

	Scout->Physics = PHYS_Walking;
	if (result)
		result = Scout->pointReachable(destination); 

	if (result) //symmetric check
	{
		Level->FarMoveActor(Scout,destination);
		Level->DropToFloor(Scout); //FIXME - why do I need this?
		result = result	&& (Scout->walkReachable(start, 15.0, 0, NULL));
		//if (!result) DebugPrint("Movement not symmetric!");
	}
	Level->FarMoveActor(Scout,oldPosition, 0, 1);

	Scout->SetCollisionSize(Scout->CollisionRadius + 6.0, Scout->CollisionHeight);

	return result; 
	unguard;
}

int FPathBuilder::outReachable(FVector start,FVector destination)
{
	guard(FPathBuilder::outReachable);

	FVector oldPosition = Scout->Location;
	Scout->SetCollisionSize(Scout->CollisionRadius - 6.0, Scout->CollisionHeight);
	int result = Level->FarMoveActor(Scout,start);
	if (Scout->Physics != PHYS_Walking)
		debugf(NAME_DevPath,"Scout Physics is %d", Scout->Physics);

	Scout->Physics = PHYS_Walking;
	if (result)
		result = Scout->pointReachable(destination); 

	Scout->SetCollisionSize(Scout->CollisionRadius + 6.0, Scout->CollisionHeight);
	Level->FarMoveActor(Scout,oldPosition, 0, 1);

	return result; 
	unguard;
}
/* walkToward()
walk Scout toward a point.  Returns 1 if Scout successfully moved

inline int FPathBuilder::walkToward(const FVector &Destination, FLOAT Movesize)
{
	guard(FPathBuilder::walkToward);
	FVector Direction = Destination - Scout->Location;
	Direction.Z = 0; //this is a 2D move
	FLOAT DistanceSquared = Direction.SizeSquared();
	int success = 0;

	if (DistanceSquared > 1.0) //move not too small to do //FIXME - match with walkmove threshold (4.1?)
	{
		if (DistanceSquared < Movesize * Movesize)
			success = (Scout->walkMove(Direction) == 1);
		else
		{
			Direction.Normalize();
			success = (Scout->walkMove(Direction * Movesize) == 1);
		}
	}
	return success;
	unguard;
}

*/

