//=============================================================================
// Bubble.
//=============================================================================
class Bubble expands Effects;


#exec MESH IMPORT MESH=SBubbles ANIVFILE=MODELS\SRocket_a.3D DATAFILE=MODELS\SRocket_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SBubbles X=0 Y=0 Z=0 YAW=0 ROLL=0 PITCH=0
#exec MESH SEQUENCE MESH=SBubbles SEQ=All       STARTFRAME=0   NUMFRAMES=16
#exec MESH SEQUENCE MESH=SBubbles SEQ=Ignite    STARTFRAME=0   NUMFRAMES=3
#exec MESH SEQUENCE MESH=SBubbles SEQ=Flying    STARTFRAME=3   NUMFRAMES=13
#exec TEXTURE IMPORT NAME=S_Bubble1 FILE=MODELS\Bubble1.PCX
#exec TEXTURE IMPORT NAME=S_Bubble2 FILE=MODELS\Bubble2.PCX
#exec TEXTURE IMPORT NAME=S_Bubble3 FILE=MODELS\Bubble3.PCX
#exec MESHMAP SCALE MESHMAP=SBubbles  X=0.3 Y=0.3 Z=0.4

simulated function ZoneChange( ZoneInfo NewZone )
{
	if ( !NewZone.bWaterZone && (Role == ROLE_Authority) )
	{ 
		Destroy();
		PlaySound (EffectSound1);
	}	
}

simulated function BeginPlay()
{
	Super.BeginPlay();
	PlaySound(EffectSound2); //Spawned Sound
	LifeSpan = 3 + 4 * FRand();
	if (FRand()<0.3) Texture = texture'S_Bubble2';
	else if (FRand()<0.3) Texture = texture'S_Bubble3';
	LoopAnim('Flying',0.6);
}

defaultproperties
{
     DrawType=DT_Mesh
     Style=STY_Translucent
     Texture=Texture'Unreal.S_bubble1'
     Mesh=Mesh'Unreal.SBubbles'
     DrawScale=0.200000
     bUnlit=True
     bParticles=True
     Mass=3.000000
     Buoyancy=5.000000
	 Physics=PHYS_Falling
     LifeSpan=2.000000
     RemoteRole=ROLE_SimulatedProxy
	 bNetOptional=true
}
