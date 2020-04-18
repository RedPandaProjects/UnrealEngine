//=============================================================================
// SlithProjectile.
//=============================================================================
class SlithProjectile expands Projectile;

#exec MESH IMPORT MESH=ngel ANIVFILE=MODELS\ngel_a.3D DATAFILE=MODELS\ngel_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=ngel X=-20 Y=0 Z=0 YAW=0 PITCH=-64 ROLL=0

#exec MESH SEQUENCE MESH=ngel SEQ=All     STARTFRAME=0   NUMFRAMES=56
#exec MESH SEQUENCE MESH=ngel SEQ=Flying  STARTFRAME=0   NUMFRAMES=13
#exec MESH SEQUENCE MESH=ngel SEQ=Still   STARTFRAME=13  NUMFRAMES=1
#exec MESH SEQUENCE MESH=ngel SEQ=Hit     STARTFRAME=14  NUMFRAMES=10
#exec MESH SEQUENCE MESH=ngel SEQ=Drip    STARTFRAME=24  NUMFRAMES=13
#exec MESH SEQUENCE MESH=ngel SEQ=Slide   STARTFRAME=37  NUMFRAMES=7
#exec MESH SEQUENCE MESH=ngel SEQ=Shrivel STARTFRAME=44  NUMFRAMES=12

#exec TEXTURE IMPORT NAME=Jflare FILE=MODELS\flare.PCX
#exec MESHMAP SCALE MESHMAP=ngel X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=ngel NUM=0 TEXTURE=Jflare

#exec AUDIO IMPORT FILE="Sounds\slith\Slispt1.WAV" NAME="SliSpawn" GROUP="Slith"
#exec AUDIO IMPORT FILE="Sounds\slith\SliImp2.WAV" NAME="SliImpact" GROUP="Slith"

var rotator RandRot;
var vector SurfaceNormal;	
var bool bInAir;
var float DotProduct;
var int i;

function Timer()
{
	local GreenSmokePuff gsp;

	gsp = Spawn(class'GreenSmokePuff',,,Location+SurfaceNormal*9);
	if (i!=-1) 
	{
		if (LightBrightness > 10) LightBrightness -= 10;
		DrawScale = 0.9*DrawScale;
		gsp.DrawScale = DrawScale*5;
		i++;
		if (i>12) Explode(Location, vect(0,0,0));
	}
}

function Explode(vector HitLocation, vector HitNormal)
{
	local FlameBall f;

  	HurtRadius(damage * DrawScale, DrawScale * 200, 'corroded', MomentumTransfer, HitLocation);
	Destroy();	
}
	
auto state Flying
{
	function ProcessTouch (Actor Other, vector HitLocation) 
	{ 
		if ( Slith(Other) == None )
			Explode(HitLocation, vect(0,0,0)); 
	}

	singular function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
							vector momentum, name damageType )
	{
		Explode(Location, vect(0,0,0));
	}

	function HitWall( vector HitNormal, actor Wall )
	{
		MakeNoise(0.6);	
		bInAir = False;
		PlaySound(ImpactSound);	
		SurfaceNormal = HitNormal;
		RandRot = rotator(HitNormal);
		RandRot.Roll += 32768;
		SetRotation(RandRot);	
		PlaySound(ImpactSound);		
		SetPhysics(PHYS_None);
		GoToState('OnSurface');
	}

	function BeginState()
	{	
		i=-1;
		if (FRand() < 0.5)
			DrawScale = 0.3 + 0.7 * FRand();
		Velocity = Vector(Rotation) * speed;	
		SetTimer(0.2,True);
		RotationRate.Yaw = Int(200000 * FRand()) - 100000;
		RotationRate.Pitch = Int(FRand() * (200000 - Abs(RotationRate.Yaw))) 
								- (100000 - Abs(RotationRate.Yaw)/2);
		LoopAnim('Flying',0.4);
		SurfaceNormal = Vect(0,0,0);
		bInAir=True;
		PlaySound(SpawnSound);
		if( Region.zone.bWaterZone )
			Velocity=Velocity*0.7;
	}
}

state OnSurface
{
	function ProcessTouch (Actor Other, vector HitLocation)
	{
		Explode(HitLocation, vect(0,0,0));
	}
Begin:
	i=0;
	PlayAnim('Hit');
	FinishAnim();
	DotProduct = SurfaceNormal dot vect(0,0,-1);
	If( DotProduct > 0.7 )
	{
		PlayAnim('Drip',0.1);
		FinishAnim();
	}
	else if (DotProduct > -0.5)
	{
		PlayAnim('Slide',0.2);
		Finishanim();
	}
}

defaultproperties
{
     speed=+00750.000000
     Damage=+00040.000000
     MomentumTransfer=20000
     SpawnSound=Unreal.SliSpawn
     ImpactSound=Unreal.SliImpact
     Mesh=Unreal.ngel
     DrawScale=+00000.500000
     bUnlit=True
     CollisionRadius=+00006.000000
     CollisionHeight=+00004.000000
     LightType=LT_Steady
     LightEffect=LE_NonIncidence
     LightBrightness=101
     LightHue=88
     LightSaturation=9
     LightRadius=4
     bBounce=True
     bFixedRotationDir=True
     Class=Unreal.SlithProjectile
}
