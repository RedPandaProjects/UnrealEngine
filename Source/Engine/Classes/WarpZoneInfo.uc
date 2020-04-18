//=============================================================================
// WarpZoneInfo. For making disjoint spaces appear as if they were connected;
// supports both in-level warp zones and cross-level warp zones.
//=============================================================================
class WarpZoneInfo expands ZoneInfo
	intrinsic
	localized;

//-----------------------------------------------------------------------------
// Information set by the level designer.

var() localized string[80] OtherSideURL; // URL of otherside WarpZoneInfo.
var() name       ThisTag;      // Tag of this warp zone.

//-----------------------------------------------------------------------------
// Internal.

var const int              iWarpZone;
var const coords           WarpCoords;
var transient WarpZoneInfo OtherSideActor;
var transient object       OtherSideLevel;
var() string[80]		   Destinations[8];
var int					   numDestinations;

//-----------------------------------------------------------------------------
// Network replication.

replication
{
	reliable if( Role==ROLE_Authority )
		OtherSideURL, ThisTag, OtherSideActor;
}

//-----------------------------------------------------------------------------
// Functions.

// Warp coordinate system transformations.
intrinsic(314) final function Warp  ( out vector Loc, out vector Vel, out rotator R );
intrinsic(315) final function UnWarp( out vector Loc, out vector Vel, out rotator R );

function PreBeginPlay()
{
	Super.PreBeginPlay();

	// Generate the local connection.
	Generate();

	// Setup destination list.
	numDestinations = 0;
	While( numDestinations < 8 )
		if (Destinations[numDestinations] != "")
			numDestinations++;
		else
			numDestinations = 8;

	// Generate URL if necessary.
	if( numDestinations>0 && (OtherSideURL == "") )
		OtherSideURL = Destinations[0];
}

function Trigger( actor Other, pawn EventInstigator )
{
	local int nextPick;
	if (numDestinations == 0)
		return;
	
	nextPick = 0;
	While( (nextPick < 8) && (Destinations[nextPick] != OtherSideURL )  )
		nextPick++;

	nextPick++;
	if ( (nextPick > 7) || (Destinations[nextPick] == "") )
		nextPick = 0;
	
	OtherSideURL = Destinations[nextPick];
	ForceGenerate();
}

// Set up this warp zone's destination.
simulated event Generate()
{
	if( OtherSideLevel != None )
		return;
	ForceGenerate();
}

// Set up this warp zone's destination.
simulated event ForceGenerate()
{
	if( InStr(OtherSideURL,"/") >= 0 )
	{
		// Remote level.
		//log( "Warpzone " $ Self $ " remote" );
		OtherSideLevel = None;
		OtherSideActor = None;
	}
	else
	{
		// Local level.
		OtherSideLevel = XLevel;
		foreach AllActors( class 'WarpZoneInfo', OtherSideActor )
			if( string(OtherSideActor.ThisTag)~=OtherSideURL && OtherSideActor!=Self )
				break;
		//log( "Warpzone " $ Self $ " local, connected to " $ OtherSideActor );
	}
}

// When an actor enters this warp zone.
function ActorEntered( actor Other )
{
	local vector L;
	local rotator R;
	local Pawn P;

	Super.ActorEntered( Other );
	if( !Other.bJustTeleported )
	{
		Generate();

		if( OtherSideActor != None )
		{
			// This needs to also perform a coordinate system transformation,
			// in case the portals aren't directionally aligned. This is easy to
			// do but UnrealScript doesn't provide coordinate system operators yet.
			Other.Disable('Touch');
			Other.Disable('UnTouch');

			L = Other.Location;
			R = Other.Rotation;
			UnWarp( L, Other.Velocity, R );
			OtherSideActor.Warp( L, Other.Velocity, R );
			Other.SetLocation(L);
			if( Pawn(Other)!=None )
			{
				//tell enemies about teleport
				P = Level.PawnList;
				While ( P != None )
				{
					if (P.Enemy == Other)
						P.LastSeenPos = Other.Location; 
					P = P.nextPawn;
				}
				//if ( Other.IsA('PlayerPawn') )
				//	PlayerPawn(Other).SetFOVAngle(150);
				R.Roll = 0;
				Pawn(Other).ClientSetRotation( R );
				Pawn(Other).MoveTimer = -1.0;
			}
			else
				Other.SetRotation( R );

			Other.Enable('Touch');
			Other.Enable('UnTouch');
			// Change rotation according to portal's rotational change.
		}
	}
}

defaultproperties
{
     MaxCarcasses=0
}
