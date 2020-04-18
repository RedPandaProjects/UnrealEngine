//=============================================================================
// PeaceRocket.
//=============================================================================
class PeaceRocket expands Projectile;

#exec MESH IMPORT MESH=perock ANIVFILE=MODELS\perock_a.3D DATAFILE=MODELS\perock_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=perock X=0 Y=0 Z=0 YAW=0 ROLL=0 PITCH=-64
#exec MESH SEQUENCE MESH=perock SEQ=All       STARTFRAME=0   NUMFRAMES=16
#exec MESH SEQUENCE MESH=perock SEQ=Ignite    STARTFRAME=0   NUMFRAMES=3
#exec MESH SEQUENCE MESH=perock SEQ=Flying    STARTFRAME=3   NUMFRAMES=13
#exec TEXTURE IMPORT NAME=Jpeace1 FILE=MODELS\peaceg.PCX
#exec MESHMAP SCALE MESHMAP=perock  X=0.07 Y=0.07 Z=0.14
#exec MESHMAP SETTEXTURE MESHMAP=perock NUM=1 TEXTURE=Jpeace1

var vector X,Y,Z;
var pawn ClosestPawn;
var float ClosestDistance;
var int Count;

function PostBeginPlay()
{
	Super.PostBeginPlay();
	PlaySound(SpawnSound);	
	Velocity = Vector(Rotation) * 120.0;
	Acceleration = Velocity * 1.67;	
	Count = 0;
	Timer();
	SetTimer(1.0,True);
}

function Timer()
{
	local pawn Victims;
	local float thisDist;

	ClosestDistance = 100000;
	ClosestPawn = None;	
	Count++;
	if (Count==8)
	{
		Explode(Location, vect(0,0,0));
		return;
	}

	//FIXME - use the pawnlist for this
	foreach VisibleCollidingActors( class'Pawn', Victims, 500.0 )
	{
		thisDist = VSize(Victims.Location - Location); 
		if ( thisDist < ClosestDistance) 
		{
			ClosestPawn = Victims;
			ClosestDistance = thisDist;
		}		
	}			
}
function Tick( float DeltaTime )
{
	local float MagnitudeVel;
	if (ClosestPawn != None)
	{
		MagnitudeVel = VSize(Velocity);
		Velocity =  MagnitudeVel * Normal( Normal(ClosestPawn.Location - Location) 
			* MagnitudeVel * DeltaTime * 6 + Velocity);		
		SetRotation(rotator(Velocity));
	}
}
function ProcessTouch (Actor Other, Vector HitLocation)
{
	local int hitdamage;
	if ((PeaceRocket(Other) == none)) Explode(HitLocation, HitLocation);
}

function Explode(vector HitLocation, vector HitNormal)
{
	HurtRadius(damage, 200.0, 'exploded', MomentumTransfer, HitLocation);
	Spawn(class'BallExplosion');	
	Destroy();
}

defaultproperties
{
     MaxSpeed=+00600.000000
     Damage=+00100.000000
     MomentumTransfer=70000
     Mesh=Unreal.perock
     AmbientGlow=96
     bMeshCurvy=False
     LightType=LT_Steady
     LightEffect=LE_NonIncidence
     LightBrightness=79
     LightHue=28
     LightSaturation=157
     LightRadius=6
     bBounce=True
     LifeSpan=+00015.000000
}
