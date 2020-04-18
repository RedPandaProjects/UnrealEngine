//=============================================================================
// Spark3.
//=============================================================================
class Spark3 expands SmallSpark;

#exec MESH IMPORT MESH=Spark3M ANIVFILE=MODELS\Spark3_a.3D DATAFILE=MODELS\Spark3_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Spark3M X=0 Y=0 Z=0 PITCH=-64
#exec MESH SEQUENCE MESH=Spark3M SEQ=All       STARTFRAME=0   NUMFRAMES=2
#exec MESH SEQUENCE MESH=Spark3M SEQ=Explosion STARTFRAME=0   NUMFRAMES=2
#exec  OBJ LOAD FILE=Textures\fireeffect1.utx PACKAGE=Unreal.Effect1
#exec TEXTURE IMPORT NAME=JSmlSpark1 FILE=MODELS\Spark.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=Spark3M X=0.06 Y=0.06 Z=0.12
#exec MESHMAP SETTEXTURE MESHMAP=Spark3M NUM=1 TEXTURE=JSmlSpark1

simulated function Tick(float DeltaTime)
{
	if ( Level.NetMode != NM_DedicatedServer )
	{
		ScaleGlow = LifeSpan / Default.LifeSpan;
		AmbientGlow = ScaleGlow * 255;
	}
}

simulated function PostBeginPlay()
{
	local rotator NewRotation;

	NewRotation = Rotation;
	NewRotation.Pitch = FRand()*65535;
	PlayAnim  ( 'Explosion', 0.09 );
	PlaySound (EffectSound1);
	SetRotation(NewRotation);
}

defaultproperties
{
     LifeSpan=0.250000
     Style=STY_Translucent
     Texture=FireTexture'Unreal.Effect1.FireEffect1u'
     Mesh=Mesh'Unreal.Spark3M'
     DrawScale=0.100000
     bUnlit=False
     bParticles=True
}
