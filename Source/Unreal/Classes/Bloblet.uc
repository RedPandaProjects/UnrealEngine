//=============================================================================
// Bloblet.
// Must tag with same tag as a Blob
//=============================================================================
class Bloblet expands FlockPawn;

#exec MESH IMPORT MESH=MiniBlob ANIVFILE=MODELS\MiniB_a.3D DATAFILE=MODELS\MiniB_d.3D ZEROTEX=1
#exec MESH ORIGIN MESH=MiniBlob X=0 Y=-100 Z=0 YAW=64 ROll=-64

#exec MESH SEQUENCE MESH=MiniBlob SEQ=All    STARTFRAME=0   NUMFRAMES=75
#exec MESH SEQUENCE MESH=MiniBlob SEQ=Glob1  STARTFRAME=0   NUMFRAMES=10
#exec MESH SEQUENCE MESH=MiniBlob SEQ=Glob2  STARTFRAME=10   NUMFRAMES=10
#exec MESH SEQUENCE MESH=MiniBlob SEQ=Glob3  STARTFRAME=20   NUMFRAMES=15
#exec MESH SEQUENCE MESH=MiniBlob SEQ=GlobProj  STARTFRAME=35   NUMFRAMES=20
#exec MESH SEQUENCE MESH=MiniBlob SEQ=Splat  STARTFRAME=55   NUMFRAMES=20
#exec MESH SEQUENCE MESH=MiniBlob SEQ=Flat   STARTFRAME=62   NUMFRAMES=1

#exec TEXTURE IMPORT NAME=JBlob1 FILE=MODELS\bloblet.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=MiniBlob X=0.06 Y=0.06 Z=0.12
#exec MESHMAP SETTEXTURE MESHMAP=MiniBlob NUM=0 TEXTURE=Jblob1

#exec AUDIO IMPORT FILE="Sounds\Blob\death1Bl.WAV" NAME="BlobDeath" GROUP="Blob"
#exec AUDIO IMPORT FILE="Sounds\BRifle\GelHit1.WAV" NAME="GelHit" GROUP="BioRifle"
#exec AUDIO IMPORT FILE="Sounds\Blob\goop2Bl.WAV" NAME="BlobGoop1" GROUP="Blob"
#exec AUDIO IMPORT FILE="Sounds\Blob\goop3Bl.WAV" NAME="BlobGoop2" GROUP="Blob"
#exec AUDIO IMPORT FILE="Sounds\Blob\goop4Bl.WAV" NAME="BlobGoop3" GROUP="Blob"
#exec AUDIO IMPORT FILE="Sounds\Blob\hit1Bl.WAV" NAME="BlobHit" GROUP="Blob"
#exec AUDIO IMPORT FILE="Sounds\Blob\injur1Bl.WAV" NAME="BlobInjur" GROUP="Blob"

var parentBlob parentBlob;
var vector	Orientation;
var float	LastParentTime;

function PostBeginPlay()
{
	Super.PostBeginPlay();
	DrawScale = 0.8 + 0.4 * FRand();
}

function Died(pawn Killer, name damageType, vector HitLocation)
{
	PlaySound(Die);
	SetCollision(false,false, false);
	parentBlob.shrink(self);
	GotoState('DiedState');
}

function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
{
	local rotator newRotation;
	local GreenSmokePuff f;

	if (damageType == 'corroded')
		return;
	PlaySound(HitSound1);
	f = spawn(class'GreenSmokePuff',,,Location - Normal(Momentum)*12); 
	f.DrawScale = FClamp(0.1 * Damage, 1.0, 4.0);
	SetPhysics(PHYS_Falling);
	newRotation = Rotation;
	newRotation.Roll = 0;
	setRotation(newRotation);
	Super.TakeDamage(Damage,instigatedBy,hitLocation,momentum,damageType);	
}

function BaseChange()
{
}

function wakeup()
{
	GotoState('Active');
}

function PreSetMovement()
{
	bCanWalk = true;
	bCanSwim = true;
	bCanFly = false;
	MinHitWall = -0.7;
}


function Timer()
{
	local int i;
	local bool bHasEnemy;

	bHasEnemy = ((parentBlob != None) && (parentBlob.Enemy != None));
	if ( bHasEnemy && (VSize(Location - parentBlob.Enemy.Location) < parentBlob.Enemy.CollisionHeight + CollisionRadius) )
	{
		parentBlob.Enemy.TakeDamage(18 * FRand(), parentBlob, location, vect(0,0,0), 'corroded');
		PlaySound(sound'BlobHit');
	}
	
	if ( Physics == PHYS_Spider )
	{
		if ( FRand() < 0.33 )
			PlaySound(sound'BlobGoop1');
		else if ( FRand() < 0.5 )
			PlaySound(sound'BlobGoop2');
		else
			PlaySound(sound'BlobGoop3');
	}
	if ( bHasEnemy )
		SetTimer(0.5 + 0.5 * FRand(), false);
	else
		SetTimer(1 + FRand(), false);	
}
	 
function PlayGlob(float rate)
{
	if (FRand() < 0.75)
		LoopAnim('Glob1', rate * 0.7 * FRand());
	else
		LoopAnim('Glob3', rate * (0.5 + 0.5 * FRand()));
}

auto state asleep
{
	function Landed(vector HitNormal)
	{
		if ( !FootRegion.Zone.bWaterZone )
			PlaySound(Land);
		PlayAnim('Splat');
		SetPhysics(PHYS_None);
	}	
	
Begin:
	SetTimer(2 * FRand(), false);
	if (Physics != PHYS_Falling)
		SetPhysics(PHYS_None);
	PlayGlob(0.3);
}

state active
{
	function AnimEnd()
	{
		playGlob(1);
	}

	function Landed(vector HitNormal)
	{
		SetRotation(Rot(0,0,0));
		if ( Velocity.Z > 200 )
		{
			PlayAnim('Splat');
			PlaySound(Land);
		}
		SetPhysics(PHYS_Spider);
	}

begin:
	SetTimer(FRand(), false);
	SetPhysics(PHYS_Spider);
	PlayGlob(1);
	LastParentTime = Level.TimeSeconds;

wander:
	if (parentBlob == None)
		GotoState('DiedState');
	if ( VSize(Location - parentBlob.Location) > 120 )
	{
		if ( LastParentTime - Level.TimeSeconds > 20 )
			GotoState('DiedState');
		else	
			MoveToward(parentBlob);
	}
	else 
	{
		LastParentTime = Level.TimeSeconds;
		MoveTo(ParentBlob.Location);
	}
	Sleep(0.1);
	Goto('Wander');
}


state fired
{
	function HitWall(vector HitNormal, actor Wall)
	{
		PlaySound(Land);
		GotoState('Active');
	}

	function Landed(vector HitNormal)
	{
		if ( !FootRegion.Zone.bWaterZone )
			PlaySound(Land);
		GotoState('Active');
	}
}

state DiedState
{
	ignores TakeDamage;

	function Landed(vector HitNormal)
	{
		if ( !FootRegion.Zone.bWaterZone )
			PlaySound(Land);
		SetRotation(Rot(0,0,0));
		PlayAnim('Splat');
		SetPhysics(PHYS_None);
	}	

	function Tick(float DeltaTime)
	{
		DrawScale = DrawScale - 0.06 * DeltaTime;
		if (DrawScale < 0.1)
			Destroy();
	}

Begin:
	SetPhysics(PHYS_Falling);
	PlayAnim('Splat');
	FinishAnim();
	TweenAnim('Flat', 0.2);
}	

	

defaultproperties
{
     Health=120
     ReducedDamageType=exploded
     ReducedDamagePct=+00000.250000
     SightRadius=+04100.000000
     GroundSpeed=+00450.000000
     AccelRate=+01200.000000
     JumpZ=-00001.000000
     Tag=blob1
     DrawType=DT_Mesh
	 Die=BlobDeath
	 HitSound1=BlobInjur
	 Land=GelHit
     Texture=Unreal.JBlob1
     Mesh=Unreal.MiniBlob
     bMeshEnviroMap=True
     CollisionRadius=+00006.000000
     CollisionHeight=+00006.000000
     bBlockActors=False
     bBlockPlayers=False
     Mass=+00040.000000
     RotationRate=(Pitch=0,Yaw=0,Roll=0)
}
