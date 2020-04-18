//=============================================================================
// KraalBolt.
//=============================================================================
class KraalBolt expands Projectile;

#exec AUDIO IMPORT FILE="Sounds\Krall\krasht2.wav" NAME="Krasht2" GROUP="Krall"

#exec MESH IMPORT MESH=Krallbm ANIVFILE=MODELS\cros_t_a.3D DATAFILE=MODELS\cros_t_d.3D X=0 Y=0 Z=0 
#exec MESH ORIGIN MESH=Krallbm X=0 Y=0 Z=0 YAW=-64
#exec MESH SEQUENCE MESH=Krallbm SEQ=All STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=Krallbm SEQ=Still  STARTFRAME=0 NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=Krallbm X=0.04 Y=0.04 Z=0.08
#exec OBJ LOAD FILE=Textures\fireeffect1.utx PACKAGE=Unreal.Effect1
#exec MESHMAP SETTEXTURE MESHMAP=Krallbm NUM=0 TEXTURE=Unreal.Effect1.FireEffect1a
#exec MESHMAP SETTEXTURE MESHMAP=Krallbm NUM=1 TEXTURE=Unreal.Effect1.FireEffect1
  
function PostBeginPlay()
{
	if ( ScriptedPawn(Instigator) != None )
		Speed = ScriptedPawn(Instigator).ProjectileSpeed;
	Velocity = Vector(Rotation) * speed;
	PlaySound(SpawnSound,SLOT_None,4.0);
	Super.PostBeginPlay();
} 

function Explode(vector HitLocation, vector HitNormal)
{
	PlaySound(ImpactSound, SLOT_Interact);
	MakeNoise(1.0);
 	spawn(class'SmokeColumn',,,Location+Vect(0,0,38));
	destroy();
}
	

auto state Flying
{
	function ProcessTouch (Actor Other, Vector HitLocation)
	{
		local vector momentum;
	
		if ( (Krall(Other) == None) && (KraalBolt(Other) == None))
		{
			momentum = MomentumTransfer * Normal(Velocity);
			Other.TakeDamage( Damage, instigator, HitLocation, momentum, 'zapped');
			Destroy();
		}
	}

Begin:
	Sleep(7.0); //self destruct after 7.0 seconds
	Explode(Location, vect(0,0,0));
}

defaultproperties
{
     speed=800.000000
     MaxSpeed=800.000000
     Damage=15.000000
     MomentumTransfer=10000
     SpawnSound=Sound'Unreal.Krall.Krasht2'
     RemoteRole=ROLE_SimulatedProxy
     Mesh=Mesh'Unreal.Krallbm'
     AmbientGlow=255
     bUnlit=True
     bMeshCurvy=False
     LightType=LT_Steady
     LightEffect=LE_NonIncidence
     LightBrightness=200
     LightHue=102
     LightRadius=4
     NetPriority=6.000000
}
