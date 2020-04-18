//=============================================================================
// rocket.
//=============================================================================
class rocket expands Projectile;

#exec MESH IMPORT MESH=RocketM ANIVFILE=MODELS\rocket_a.3D DATAFILE=MODELS\rocket_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=RocketM X=0 Y=-240 Z=0 YAW=-64

#exec MESH SEQUENCE MESH=RocketM SEQ=All       STARTFRAME=0   NUMFRAMES=6
#exec MESH SEQUENCE MESH=RocketM SEQ=Still     STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=RocketM SEQ=WingIn    STARTFRAME=1   NUMFRAMES=1
#exec MESH SEQUENCE MESH=RocketM SEQ=Armed     STARTFRAME=1   NUMFRAMES=3
#exec MESH SEQUENCE MESH=RocketM SEQ=SpinCCW   STARTFRAME=4   NUMFRAMES=1
#exec MESH SEQUENCE MESH=RocketM SEQ=SpinCW    STARTFRAME=5   NUMFRAMES=1

#exec TEXTURE IMPORT NAME=JRocket1 FILE=MODELS\rocket.PCX
#exec OBJ LOAD FILE=Textures\fireeffect16.utx PACKAGE=Unreal.Effect16
#exec MESHMAP SCALE MESHMAP=rocketM X=0.5 Y=0.5 Z=1.0 YAW=128
#exec MESHMAP SETTEXTURE MESHMAP=rocketM NUM=1 TEXTURE=Jrocket1

#exec AUDIO IMPORT FILE="Sounds\EightBal\Ignite.WAV" NAME="Ignite" GROUP="Eightball"
#exec AUDIO IMPORT FILE="Sounds\EightBal\grenflor.wav" NAME="GrenadeFloor" GROUP="Eightball"
#exec AUDIO IMPORT FILE="Sounds\General\brufly1.WAV" NAME="Brufly1" GROUP="General"

var Actor Seeking;
var float MagnitudeVel,Count,SmokeRate;
var vector InitialDir;
var bool bRing,bHitWater,bWaterStart;
var int NumExtraRockets;

simulated function PostBeginPlay()
{
	Count = -0.1;
	if (Level.bHighDetailMode) SmokeRate = 0.035;
	else SmokeRate = 0.15;
}

simulated function Tick(float DeltaTime)
{
	local SpriteSmokePuff b;

	if (bHitWater)
	{
		Disable('Tick');
		Return;
	}
	Count += DeltaTime;
	if ( (Count>(SmokeRate+FRand()*(SmokeRate+NumExtraRockets*0.035))) && (Level.NetMode!=NM_DedicatedServer) ) 
	{
		b = Spawn(class'SpriteSmokePuff');
		b.RemoteRole = ROLE_None;		
		Count=0.0;
	}
}

auto state Flying
{

	simulated function ZoneChange( Zoneinfo NewZone )
	{
		local waterring w;
		
		if (!NewZone.bWaterZone || bHitWater) Return;

		bHitWater = True;
		Disable('Tick');
		w = Spawn(class'WaterRing',,,,rot(16384,0,0));
		w.DrawScale = 0.2;
		w.RemoteRole = ROLE_None;
		Velocity=0.6*Velocity;
		PlayAnim( 'Still', 3.0 );		
	}

	function ProcessTouch (Actor Other, Vector HitLocation)
	{
		if ((Other != instigator) && (Rocket(Other) == none)) 
			Explode(HitLocation,Normal(HitLocation-Other.Location));
	}

	function Explode(vector HitLocation, vector HitNormal)
	{
		HurtRadius(Damage,200.0, 'exploded', MomentumTransfer, HitLocation );
 		spawn(class'SpriteBallExplosion',,,HitLocation + HitNormal*16);	
 		if (bRing) Spawn(class'RingExplosion3',,,HitLocation + HitNormal*16,rotator(HitNormal));
 		Destroy();
	}

	function BeginState()
	{
		initialDir = vector(Rotation);	
		Velocity = speed*initialDir;
		Acceleration = initialDir*50;
		PlaySound(SpawnSound, SLOT_None, 2.3);	
		PlayAnim( 'Armed', 0.2 );
		if (Region.Zone.bWaterZone)
		{
			bHitWater = True;
			Velocity=0.6*Velocity;
		}
	}
}

defaultproperties
{
     speed=900.000000
     MaxSpeed=1600.000000
     Damage=85.000000
     MomentumTransfer=80000
     SpawnSound=Sound'Unreal.Eightball.Ignite'
     ImpactSound=Sound'Unreal.Eightball.GrenadeFloor'
     RemoteRole=ROLE_SimulatedProxy
     LifeSpan=6.000000
     AnimSequence=Armed
     Skin=FireTexture'Unreal.Effect16.fireeffect16'
     Mesh=Mesh'Unreal.rocketM'
     DrawScale=0.050000
     AmbientGlow=96
     bUnlit=True
     bMeshCurvy=False
     SoundRadius=9
     SoundVolume=255
     AmbientSound=Sound'Unreal.General.Brufly1'
     LightType=LT_Steady
     LightEffect=LE_NonIncidence
     LightBrightness=126
     LightHue=28
     LightSaturation=64
     LightRadius=6
     bCorona=True
     bBounce=True
     NetPriority=6.000000
}
