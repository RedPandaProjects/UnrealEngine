//=============================================================================
// ForceFieldProj.
//=============================================================================
class ForceFieldProj expands Projectile;

#exec AUDIO IMPORT FILE="Sounds\Pickups\FFIELDl2.WAV" NAME="ffieldl2" GROUP="Pickups"

#exec MESH IMPORT MESH=ForceFieldM ANIVFILE=MODELS\forcef_a.3D DATAFILE=MODELS\forcef_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=ForceFieldM X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=ForceFieldM SEQ=All STARTFRAME=0  NUMFRAMES=19
#exec MESH SEQUENCE MESH=ForceFieldM SEQ=Grow STARTFRAME=0  NUMFRAMES=10
#exec MESH SEQUENCE MESH=ForceFieldM SEQ=Spin STARTFRAME=10  NUMFRAMES=9
#exec OBJ LOAD FILE=Textures\fireeffect10.utx PACKAGE=Unreal.Effect10
#exec MESHMAP SCALE MESHMAP=ForceFieldM X=0.08 Y=0.08 Z=0.16
#exec MESHMAP SETTEXTURE MESHMAP=ForceFieldM NUM=1 TEXTURE=Unreal.Effect10.FireEffect10

var int Charge;

	simulated function PostBeginPlay()
	{
		Super.PostBeginPlay();
		PlaySound(SpawnSound);
		SetCollision(True,True,True);
		PlayAnim('Grow',0.2);
		if (Charge<0) Charge = 10;
	}

Auto State Activated
{
	function Touch(Actor Other)
	{
		PlaySound(ImpactSound);	
	}

Begin:
	FinishAnim();
	Sleep(Charge/10);
	TweenAnim('All',0.5);
	FinishAnim();
	Destroy();
}

defaultproperties
{
     bCollideWhenPlacing=True
     Mesh=Unreal.ForceFieldM
     bUnlit=True
     bMeshCurvy=False
     SoundRadius=29
     SoundVolume=67
     AmbientSound=Unreal.ffieldl2
     CollisionRadius=+00030.000000
     CollisionHeight=+00050.000000
     bBlockActors=True
     bBlockPlayers=True
     Physics=PHYS_None
     RemoteRole=ROLE_SimulatedProxy
     NetPriority=+00006.000000
}
