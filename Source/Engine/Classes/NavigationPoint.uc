//=============================================================================
// NavigationPoint.
//=============================================================================
class NavigationPoint expands Actor
	intrinsic;

#exec Texture Import File=Textures\S_Pickup.pcx Name=S_Pickup Mips=Off Flags=2

//------------------------------------------------------------------------------
// NavigationPoint variables
var() name ownerTeam;	//creature clan owning this area (area visible from this point)
var bool taken; //set when a creature is occupying this spot
var int upstreamPaths[16];
var int Paths[16]; //index of reachspecs (used by C++ Navigation code)
var int PrunedPaths[16];
var int visitedWeight;
var actor routeCache;
var const int bestPathWeight;
var const NavigationPoint nextNavigationPoint;
var const NavigationPoint nextOrdered;
var const NavigationPoint prevOrdered;
var const NavigationPoint startPath;
var() int cost; //added cost to visit this pathnode
var() bool bPlayerOnly;	//only players should use this path

var bool bEndPoint; //used by C++ navigation code
var bool bEndPointOnly; //only used as an endpoint in routing network
 
intrinsic(519) final function describeSpec(int iSpec); //FIXME - replace with function that returns spec values

function PostBeginPlay()
{
	taken = false;
	Super.PostBeginPlay();
}

// Accept an actor that has teleported in.
// used for random spawning and initial placement of creatures
event bool Accept( actor Incoming )
{
	// Move the actor here.
	taken = Incoming.SetLocation( Location + vect (0,0,20));
	if (taken)
	{
		Incoming.Velocity = vect(0,0,0);
		Incoming.SetRotation(Rotation);
	}
	// Play teleport-in effect.
	PlayTeleportEffect(Incoming, true);
	return taken;
}

function PlayTeleportEffect(actor Incoming, bool bOut)
{
		Level.Game.PlayTeleportEffect(Incoming, bOut, false);
}

defaultproperties
{
     upstreamPaths(0)=-1
     upstreamPaths(1)=-1
     upstreamPaths(2)=-1
     upstreamPaths(3)=-1
     upstreamPaths(4)=-1
     upstreamPaths(5)=-1
     upstreamPaths(6)=-1
     upstreamPaths(7)=-1
     upstreamPaths(8)=-1
     upstreamPaths(9)=-1
     upstreamPaths(10)=-1
     upstreamPaths(11)=-1
     upstreamPaths(12)=-1
     upstreamPaths(13)=-1
     upstreamPaths(14)=-1
     upstreamPaths(15)=-1
     Paths(0)=-1
     Paths(1)=-1
     Paths(2)=-1
     Paths(3)=-1
     Paths(4)=-1
     Paths(5)=-1
     Paths(6)=-1
     Paths(7)=-1
     Paths(8)=-1
     Paths(9)=-1
     Paths(10)=-1
     Paths(11)=-1
     Paths(12)=-1
     Paths(13)=-1
     Paths(14)=-1
     Paths(15)=-1
     PrunedPaths(0)=-1
     PrunedPaths(1)=-1
     PrunedPaths(2)=-1
     PrunedPaths(3)=-1
     PrunedPaths(4)=-1
     PrunedPaths(5)=-1
     PrunedPaths(6)=-1
     PrunedPaths(7)=-1
     PrunedPaths(8)=-1
     PrunedPaths(9)=-1
     PrunedPaths(10)=-1
     PrunedPaths(11)=-1
     PrunedPaths(12)=-1
     PrunedPaths(13)=-1
     PrunedPaths(14)=-1
     PrunedPaths(15)=-1
     bStatic=True
     bHidden=True
     bCollideWhenPlacing=True
     SoundVolume=0
     CollisionRadius=+00046.000000
     CollisionHeight=+00050.000000
}
