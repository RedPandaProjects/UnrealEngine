//=============================================================================
// WarpZoneMarker.
//=============================================================================
class WarpZoneMarker expands NavigationPoint
	intrinsic;

var WarpZoneInfo markedWarpZone;

// AI related
var Actor TriggerActor;		//used to tell AI how to trigger me
var Actor TriggerActor2;

function PostBeginPlay()
{
	if ( markedWarpZone.numDestinations > 1 )
		FindTriggerActor();
	Super.PostBeginPlay();
}

function FindTriggerActor()
{
	local ZoneTrigger Z;
	ForEach AllActors(class 'ZoneTrigger', Z)
		if ( Z.Event == markedWarpZone.ZoneTag)
		{
			TriggerActor = Z;
			return;
		} 
}

/* SpecialHandling is called by the navigation code when the next path has been found.  
It gives that path an opportunity to modify the result based on any special considerations
*/

/* FIXME - how to figure out if other side actor is OK and use intelligently for all dests? 
*/
function Actor SpecialHandling(Pawn Other)
{
	if (Other.Region.Zone == markedWarpZone)
		markedWarpZone.ActorEntered(Other);
	return self;
}
/*	if ( markedWarpZone.numDestinations <= 1 )
		return self;
	
	if ( markedWarpZone.OtherSideActor is OK )
		return self;
			
	if (TriggerActor == None)
	{
		FindTriggerActor();
		if (TriggerActor == None)
			return None;
	}
	
	return TriggerActor;			
}	 
*/

defaultproperties
{
     bCollideWhenPlacing=False
	 bHiddenEd=true
     CollisionRadius=+00020.000000
     CollisionHeight=+00040.000000
}
