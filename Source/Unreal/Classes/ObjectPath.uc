//=============================================================================
// ObjectPath.
//=============================================================================
class ObjectPath expands Keypoint;

// Allows an object to follow a defined path, by specifying
// PathPoint nodes. M

// Note: At least 4 PathPoints must exist.  The first and last
//       do not need speed/deltaU settings.

// Note: If there are N points to interpolate through, there must
//       be in total N+2 points specified.  Point 0 and Point N+1
//       are dummy control points, where Point 1-Point 0 is the 
//       initial direction of motion, and Point N+1 - Point N is
//       the final direction of motion.  The object will start at
//       point 1 and end up at point N.

// Uses the Bernstein basis functions for Bezier interpolation:
//   B0(u) = (1-u)^3
//   B1(u) = 3u(1-u)^2
//   B2(u) = 3u^2(1-u)
//   B3(u) = u^3

var() name     PathActorTag; 	// The Tag of the actor which should be moved
var() bool     bAlterPitch;	// should the pitch of the actor be modified during movement
var() bool     bAlterYaw;		// should the yaw ...
var() bool     bAlterRoll;		// should the roll ...
var() rotator  RAdjust;		// Adjust the rotation of the object

//var() bool  bLoopMotion;	// The last PathPoint should lead to the first

var Actor   	PathActor;		// what should be moved
var PathPoint Path[35];		// maximum 35 nodes in the path, hence 33 real positions 
var int 		numPathNodes;		// how many elements in the path array
var int 		curNode;			// Which node are we at?
var float 	uValue;			// Offset in the segment
var bool 	bTriggeredOnce;	// Don't repeat the path if it's already played through
var bool		bPlayedOnce;		// Really don't play it again since it's already finished last time
var vector 	lastPosition;		// Where the actor was in the most recent frame
var rotator lastRotation;		// The orientation of the actor in the most recent frame


function BeginPlay()
{
	local int i, l;
	local PathPoint tempPP;
	
	// Find all relevant PathPoint elements and load them 
	// into the temporary array (in sorted order).  Also set up
	// velocities, etc..
	// Wait until something triggers the motion
	Disable('Tick');
	bTriggeredOnce = false;
	bPlayedOnce = true;

	// Find the object which must be moved
	PathActor = None;
	foreach AllActors (class 'Actor', PathActor)
	{
		if( PathActor.Tag == PathActorTag )
			// found the matching Actor
			break;	
	}
	if( PathActor == None ) 
	{
		log("ObjectPath: No object to be moved.  Aborting.");
		Destroy();
		return;
	}

	// Find all the Path Nodes.
	numPathNodes=0;
	foreach AllActors (class 'PathPoint', tempPP) 
	{
		// This PathPoint must have the same tag as this ObjectPath actor
		if (tempPP.Tag == Tag) {
		
			Path[numPathNodes] = tempPP;
			numPathNodes++;	
			// Make sure that the user didn't specify too many points
			if (numPathNodes > 35) {
				log ("ObjectPath: Maximum number of path elements exceeded.  Aborting.");
				log ("            Tag = " $ tempPP.Tag);
				Destroy();
				return;
			}		
		}
	}
	
	// Make sure that there are at least four PathPoint nodes.
	if (numPathNodes < 4)
	{
		log("ObjectPath: Not enough PathPoints specified.  Needed 4.  Aborting.");
		Destroy();
		return;
	}

	// Now sort the elements (using a crappy bubble sort.. hey it's < 30 elements)
	for (i=0; i<numPathNodes-1; i++) {
		for (l=i+1; l<numPathNodes; l++) {
			if (Path[i].sequence_Number > Path[l].sequence_Number) {
			
				// switch them
				tempPP  = Path[i];
				Path[i] = Path[l];
				Path[l] = tempPP;
			}		
		}
	}

	// Precalculate the direction vectors at each point, based on 
	// relative position and curveSpeed.
	// The first (1) and last (N-1) segments' velocities are
	// related to the (0) and (N)'th points respectively.
	// first segment
	Path[1].pVelocity = Normal(Path[1].Location - 
					  		  Path[0].Location) * Path[1].curvespeed;
	// last segment
	Path[numPathNodes-2].pVelocity = Normal(Path[numPathNodes-1].Location - 
								    	    Path[numPathNodes-2].Location) * Path[numPathNodes-2].curvespeed;

	// The tangent of the middle nodes is parallel to the vector
	// between the previous and next nodes.
	for (i=2; i<=numPathNodes-3; i++) {
		Path[i].pVelocity = Normal(Path[i+1].Location - 
							      Path[i-1].Location) * Path[i].curvespeed;
	}

	Enable('Trigger');
}

function PostBeginPlay()
{
	Super.PostBeginPlay();

	// Set the object at the initial position and orientation, so that it doesn't
	// suddenly jerk when triggered to move.
	PathActor.SetLocation( Path[1].Location );
}

function Trigger( actor Other, pawn EventInstigator )
{
	// Play the motion if it hasn't occured yet
	if( !bTriggeredOnce )
	{
		// Set update parameters
		bTriggeredOnce = true;
		bPlayedOnce = false;
		curNode = 1;
		uValue = 0;
		
		// Put the actor at the initial position
		PathActor.SetLocation( Path[1].Location );
		//PathActor.SetRotation( RAdjust );
		//PathActor.SetPhysics( PHYS_None );

		// So that the orientation of the object can be set immediately
		lastPosition = Path[0].Location;
		lastRotation = PathActor.Rotation;
		
		// Start the motion
		Enable('Tick');
	}
}



function Tick( float DeltaTime )
{
	local float 		curSpeedU;
	local vector		actorPosition;
	local rotator 	    actorRotation;

	if( bPlayedOnce )
	{
		Disable('Tick');
		return;
	}

	// Update the position of the object, based on DeltaTime and it's
	// index in the sequence of positions.
		
	// Interpolate the deltaU value from this to the next position,
	// based on the current U value in the segment.
	// FIXME: This is not exact by far, and would cause discontinuities
	//        in velocity if the arclengths of the bezier's are different.
	curSpeedU = (Path[curNode+1].speedU - Path[curNode].speedU)*uValue +
				Path[curNode].speedU;

	uValue += curSpeedU * DeltaTime;
	
	// Check if the difference sends the point to the next segment (or the one after that, etc..)
	while( uValue >= 1 && curNode < numPathNodes-2 )
	{
		curNode++;
		uValue -= 1;
	}

	// Check if the point is beyond, or at the end of the path
	if( curNode >= numPathNodes-2 )
	{
		// Set the position of the object to the position of the last node.
		PathActor.SetLocation( Path[numPathNodes-2].Location );
		PathActor.GotoState('');

		// Don't ever play it again!!!
		bPlayedOnce = true;
		
		// Finished the path, discontinue the updates
		Disable( 'Tick' );
		Disable( 'Trigger' );
		return;
	}
					
	// Calculate the position of the object based on the current node index
	// and U offset in the curve segment.
	//   B0(u) = (1-u)^3				Bernstein basis polys
	//   B1(u) = 3u(1-u)^2
	//   B2(u) = 3u^2(1-u)
	//   B3(u) = u^3
	actorPosition  = Path[curNode].Location * ((1 - uValue)**3) +
					(Path[curNode].Location + Path[curNode].pVelocity) *
					 (3*uValue* ((1-uValue)**2)) +
					(Path[curNode+1].Location - Path[curNode+1].pVelocity) *
					 (3*(uValue**2)* (1-uValue)) +
					Path[curNode+1].Location * (uValue**3);
	
	PathActor.Move( actorPosition - PathActor.Location );
		
	// Calculate new Pitch, Yaw, Roll values if applicable.
	actorRotation = rotator( actorPosition - lastPosition );
	actorRotation += RAdjust;

	if( !bAlterPitch ) actorRotation.Pitch = lastRotation.Pitch;
	if( !bAlterYaw   ) actorRotation.Yaw   = lastRotation.Yaw;
	// The roll is proportional to the change in yaw (airplane turning effect)
	if( !bAlterRoll  ) actorRotation.Roll  = lastRotation.Roll;
	              else actorRotation.Roll  = 0.4*(actorRotation.Yaw - lastRotation.Yaw);

	PathActor.SetRotation( actorRotation );
	
	// Save this frame's position/yaw for next frame calculations
	lastRotation = actorRotation;
	lastPosition = actorPosition;
}

defaultproperties
{
}
