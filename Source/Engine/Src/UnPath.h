/*=============================================================================
	UnPath.h: Path node creation and ReachSpec creations and management specific classes
	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Steven Polge 3/97
=============================================================================*/

#define DEBUGGINGPATHS  1 //1 to put path info in log
#define MAXMARKERS 3000 //bound number of turn markers
#define MAXREACHSPECS 3000 //bound number of reachspecs 
#define MAXCOMMONRADIUS 70 //max radius to consider in building paths
#define MAXCOMMONHEIGHT 70
#define MINCOMMONHEIGHT 48 //min typical height for non-human intelligent creatures
#define MINCOMMONRADIUS 24 //min typical radius for non-human intelligent creatures
#define COMMONRADIUS    52 //max typical radius of intelligent creatures

#define COS30 0.8660254 

//Reachability flags - using bits to save space

enum EReachSpecFlags
{
	R_WALK = 1,	//walking required
	R_FLY = 2,   //flying required 
	R_SWIM = 4,  //swimming required
	R_JUMP = 8,   // jumping required
	R_DOOR = 16,
	R_SPECIAL = 32,
	R_PLAYERONLY = 64
}; 

// path node placement parameters
#define MAXWAYPOINTDIST 2.0  // max distance to a usable waypoint to avoid placing a new one after left turn
							// (ratio to collisionradius)
 
#define MAXSORTED 32
class FSortedPathList
{
public:
	AActor *Path[MAXSORTED];
	INT Dist[MAXSORTED];
	int numPoints;
	int checkPos;

	inline void addPath(AActor * node, INT dist);
	inline void removePath(int p);
	int findEndPoint(APawn *Searcher, INT &startanchor); 
	int checkAnchorPath(APawn *Searcher, FVector Dest); 
	void expandAnchor(APawn *Searcher); 
	void findAltEndPoint(APawn *Searcher, AActor *&bestPath); 
	void FindVisiblePaths(APawn *Searcher, FVector Dest, FSortedPathList *DestPoints, INT bClearPaths, 
							INT &startanchor, INT &endanchor);

};

class FPathMarker
{
public:
	FVector Location;
	FVector Direction;
	DWORD visible:1;
	DWORD marked:1;
	DWORD bigvisible:1;
	DWORD beacon:1;
	DWORD leftTurn:1;
	DWORD permanent:1;
	DWORD stair:1;
	DWORD routable:1;
	FLOAT radius;
	FLOAT budget;

	inline void initialize(const FVector &spot, const FVector &dir, int mrk, const int beac, int left)
	{
		Location = spot;
		Direction = dir;
		marked = mrk;
		bigvisible = 0;
		beacon = beac;
		permanent = 0;
		stair = 0;
		leftTurn = left;
	}

	inline int removable()
	{
		int result =  !marked && !beacon && !leftTurn && !permanent;
		return result;
	}

private:

};

class ENGINE_API FPathBuilder
{
public:
	
	int buildPaths (ULevel *ownerLevel, int optimization);
	int removePaths (ULevel *ownerLevel);
	int showPaths (ULevel *ownerLevel);
	int hidePaths (ULevel *ownerLevel);
	void definePaths (ULevel *ownerLevel);
	void undefinePaths (ULevel *ownerLevel);

private:
	FPathMarker *pathMarkers;
	ULevel * Level;
	APawn * Scout;
	INT	numMarkers;
	FLOAT humanRadius;
	int optlevel;

	int Prune(AActor *Node);
	void CheckDoor(AActor *Node);
	INT specFor(AActor* Start, AActor* End);
	int createPaths (int optimization);
	void	newPath(FVector spot);
	void getScout();
	INT addMarker();
	int findScoutStart(FVector start);
	void createPathsFrom(FVector start);
	void checkObstructionFrom(FPathMarker *marker);
	int checkmergeSpot(const FVector &spot, FPathMarker *path1, FPathMarker *path2);
	void premergePath(INT iMarker);
	void mergePath(INT iMarker);
	void adjustPath(INT iMarker);
	void exploreWall(const FVector &moveDirection);
	inline int walkToward(const FVector &Destination, FLOAT Movesize);
	void followWall(FVector currentDirection);
	int checkLeft(FVector &leftDirection, FVector &currentDirection);
	int checkLeftPassage(FVector &currentDirection);
	int outReachable(FVector start, FVector destination);
	int fullyReachable(FVector start, FVector destination);
	int needPath(const FVector &start);
	int sawNewLeft(const FVector &start);
	int oneWaypointTo(const FVector &upstreamSpot);
	void markLeftReachable(const FVector &start);
	void markReachable(const FVector &start);
	int markReachableFromTwo(FPathMarker *path1, FPathMarker *path2);
	int tryPathThrough(FPathMarker *Waypoint, const FVector &Destination, FLOAT budget);
	int findPathTo(const FVector &Destination);
	int angleNearThirty(FVector dir);
	void nearestThirtyAngle (FVector &currentDirection);
	void addReachSpecs(AActor * start);
	int insertReachSpec(INT *SpecArray, FReachSpec &Spec);
};
