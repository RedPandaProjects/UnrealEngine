//=============================================================================
// TentacleProjectile.
//=============================================================================
class TentacleProjectile expands Projectile;

#exec MESH IMPORT MESH=TentProjectile ANIVFILE=MODELS\Tproj_a.3D DATAFILE=MODELS\Tproj_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=TentProjectile X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=TentProjectile SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JTentacle1 FILE=MODELS\Tentacle.PCX GROUP="Skins" 
#exec MESHMAP SCALE MESHMAP=TentProjectile X=0.02 Y=0.02 Z=0.04
#exec MESHMAP SETTEXTURE MESHMAP=TentProjectile NUM=1 TEXTURE=JTentacle1

#exec AUDIO IMPORT FILE="Sounds\tentacle\tensht1.WAV" NAME="TentSpawn" GROUP="Tentacle"
#exec AUDIO IMPORT FILE="Sounds\tentacle\tenimp2.WAV" NAME="TentImpact" GROUP="Tentacle"

auto state Flying
{
	function ProcessTouch (Actor Other, Vector HitLocation)
	{
		local vector momentum;
	
		if ((Tentacle(Other) == None) && (TentacleProjectile(Other) == None))
		{
			momentum = 10000.0 * Normal(Velocity);
			Other.TakeDamage(Damage, instigator, HitLocation, momentum, 'stung');
			Destroy();
		}
	}
	
	function Explode(vector HitLocation, vector HitNormal)
	{
		PlaySound(ImpactSound);
		MakeNoise(1.0);
		spawn(class'SmallSpark2',,,HitLocation+HitNormal*5,rotator(HitNormal*2+VRand()));
		destroy();
	}

	function BeginState()
	{
		Velocity = Vector(Rotation) * speed;
		PlaySound(SpawnSound);
	}

Begin:
	Sleep(7.0); //self destruct after 7.0 seconds
	Explode(Location, vect(0,0,0));
	
}

defaultproperties
{
     speed=+00800.000000
     MaxSpeed=+00800.000000
     Damage=+00012.000000
     SpawnSound=Unreal.TentSpawn
     ImpactSound=Unreal.TentImpact
     Mesh=Unreal.TentProjectile
     AmbientGlow=255
     bUnlit=True
     bMeshCurvy=False
     Mass=+00002.000000
     LifeSpan=+00015.000000
     AnimRate=+00001.000000
     RemoteRole=ROLE_SimulatedProxy
     NetPriority=+00006.000000
     Class=Unreal.TentacleProjectile
}
