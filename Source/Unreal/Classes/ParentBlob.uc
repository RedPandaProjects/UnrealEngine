//=============================================================================
// ParentBlob.
//=============================================================================
class ParentBlob expands FlockMasterPawn;

var bool bEnemyVisible;
var int numBlobs;
var	bloblet blobs[16]; 
var localized string[128] BlobKillMessage;

function setMovementPhysics()
{
	SetPhysics(PHYS_Spider);
}

function string[64] KillMessage(name damageType, pawn Other)
{
	return(BlobKillMessage);
}

function Shrink(bloblet b)
{
	local int i,j;
	
	for (i=0; i<numBlobs; i++ )
		if ( blobs[i] == b )
			break;
	numBlobs--;
	for (j=i;j<numBlobs; j++ )
		blobs[j] = blobs[j+1];
	if (numBlobs == 0)
		Destroy();
	else
		SetRadius();
}

function SetRadius()
{
	local int i;
	local float size;
	
	size = 24 + 1.5 * numBlobs;
	for (i=0; i<numBlobs; i++)
		blobs[i].Orientation = size * vector(rot(0,65536,0) * i/numBlobs);
}
	
function PreSetMovement()
{
	bCanWalk = true;
	bCanSwim = true;
	bCanFly = false;
	MinHitWall = -0.6;
}


function BaseChange()
{
}

function Killed(pawn Killer, pawn Other, name damageType)
{
	local int i;

	if (Other == Enemy)
	{
		for (i=0; i<numBlobs; i++ )
			blobs[i].GotoState('Sleep');
		GotoState('stasis');
	}
}

auto state stasis
{
ignores EncroachedBy, EnemyNotVisible;
	
	function SeePlayer(Actor SeenPlayer)
	{
		local bloblet b;
		local pawn aPawn;
		local int i;

		if ( numBlobs == 0)
		{
			aPawn = Level.PawnList;
			while ( aPawn != None )
			{
				b = bloblet(aPawn);
				if ( (b != None) && (b.tag == tag) )
				{
					blobs[numBlobs] = b;
					numBlobs++;
					b.parentBlob = self;
					b.GotoState('Active');
				}
				if (numBlobs < 15)
					aPawn = aPawn.nextPawn;
				else
					aPawn = None;
			}
			SetRadius();
		}
		enemy = Pawn(SeenPlayer);
		bEnemyVisible = true;
		Gotostate('Attacking');
	}

Begin:
	SetPhysics(PHYS_None);
}

state Attacking
{
	function Timer()
	{
		local int i;

		Enemy = None;
		for (i=0; i<numBlobs; i++ )
			blobs[i].GotoState('asleep');
		GotoState('Stasis');
	}

	function Tick(float DeltaTime)
	{
		local int i;
		
		for (i=0; i<numBlobs; i++ )
			if ( blobs[i].MoveTarget == None )
				blobs[i].Destination = Location + blobs[i].Orientation;
	}
	
	function SeePlayer(Actor SeenPlayer)
	{
		Disable('SeePlayer');
		Enable('EnemyNotVisible');
		bEnemyVisible = true;
		SetTimer(0, false);
	}
	
	function EnemyNotVisible()
	{
		Disable('EnemyNotVisible');
		Enable('SeePlayer');
		bEnemyVisible = false;
		SetTimer(35, false);
	}
		
Begin:
	SetPhysics(PHYS_Spider);
	
Chase:
	if (bEnemyVisible)
		MoveToward(Enemy);
	else
		MoveTo(LastSeenPos);

	Sleep(0.1);
	Goto('Chase');
}



defaultproperties
{
	  BlobKillMessage="was corroded by a Blob"
      bIgnoreFriends=True
      SightRadius=+01000.000000
      PeripheralVision=-00005.000000
      HearingThreshold=+00050.000000
      Aggressiveness=+00001.000000
      Intelligence=BRAINS_NONE
      GroundSpeed=+00150.000000
      WaterSpeed=+00150.000000
      AccelRate=+00800.000000
      JumpZ=-00001.000000
      MaxStepHeight=+00050.000000
      Tag=blob1
      bHidden=True
      bCollideActors=False
      bBlockActors=False
      bBlockPlayers=False
      bProjTarget=False
}
