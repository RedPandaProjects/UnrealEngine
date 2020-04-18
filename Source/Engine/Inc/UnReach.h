/*=============================================================================
	UnReach.h: AI reach specs.
	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Steven Polge 3/97
=============================================================================*/

class ENGINE_API FReachSpec
{

public:
	INT distance; 
	AActor *Start;
	AActor *End; //actor at endpoint of this path (next waypoint or goal)
	INT CollisionRadius; 
    INT CollisionHeight; 
	INT reachFlags; //see defined bits above
	BYTE  bPruned;

	inline int supports (int iRadius, int iHeight, int moveFlags);
	FReachSpec operator+ (const FReachSpec &Spec) const;
	int defineFor (AActor * begin, AActor * dest, APawn * Scout);
	int operator<= (const FReachSpec &Spec);
	int operator== (const FReachSpec &Spec);
	int MonsterPath();
	int BotOnlyPath();

	void Init()
	{
		guard(FReachSpec::Init);
		// Init everything here.
		Start = End = NULL;
		distance = CollisionRadius = CollisionHeight = 0.0;
		reachFlags = 0;
		bPruned = 0;
		unguard;
	};

	friend FArchive& operator<< (FArchive &Ar, FReachSpec &ReachSpec )
	{
		guard(FReachSpec<<);
		// Serialize everything here.
		if( Ar.Ver() < 54 )
		{
				//old
			FLOAT fCollisionRadius, fCollisionHeight;
			Ar << ReachSpec.distance << ReachSpec.Start << ReachSpec.End;
			Ar << fCollisionRadius << fCollisionHeight;
			Ar << ReachSpec.reachFlags;

			ReachSpec.CollisionRadius = (int)fCollisionRadius;
			ReachSpec.CollisionHeight = (int)fCollisionHeight;
		}
		else
		{
				//new
			Ar << ReachSpec.distance << ReachSpec.Start << ReachSpec.End;
			Ar << ReachSpec.CollisionRadius << ReachSpec.CollisionHeight;
			Ar << ReachSpec.reachFlags << ReachSpec.bPruned;
		}
		return Ar;
		unguard;
	};

	int findBestReachable(FVector &Start, FVector &Destination, APawn * Scout);
};

