//=============================================================================
// ParticleBurst.
//=============================================================================
class ParticleBurst expands Effects;

#exec MESH IMPORT MESH=PartBurst ANIVFILE=MODELS\pexpl_a.3D DATAFILE=MODELS\pexpl_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=PartBurst X=0 Y=0 Z=0 YAW=-64 
#exec MESH SEQUENCE MESH=PartBurst SEQ=All       STARTFRAME=0   NUMFRAMES=2
#exec MESH SEQUENCE MESH=PartBurst SEQ=Explo     STARTFRAME=0   NUMFRAMES=2
#exec MESHMAP SCALE MESHMAP=PartBurst X=0.15 Y=0.15 Z=0.3
#exec TEXTURE IMPORT NAME=T_PBurst FILE=MODELS\rflare.pcx GROUP=Effects
#exec MESHMAP SETTEXTURE MESHMAP=PartBurst NUM=1  TEXTURE=T_PBurst


auto state Explode
{
	simulated function Tick( float DeltaTime )
	{
		ScaleGlow = (Lifespan/Default.Lifespan);	
	}

	simulated function BeginState()
	{
		PlayAnim('Explo',0.05);
	}
	
	simulated function AnimEnd()
	{
		Destroy();
	}		
}

defaultproperties
{
     Physics=PHYS_Rotating
     LifeSpan=0.600000
     DrawType=DT_Mesh
     Style=STY_Translucent
     Texture=Texture'Unreal.Effects.T_PBurst'
     Mesh=Mesh'Unreal.PartBurst'
     DrawScale=0.700000
     bParticles=True
     bFixedRotationDir=True
     RotationRate=(Pitch=100,Yaw=100,Roll=-200)
}
