//=============================================================================
// Fragment1.
//=============================================================================
class Fragment1 expands Fragment;

#exec MESH IMPORT MESH=vfrag1 ANIVFILE=MODELS\frag1_a.3D DATAFILE=MODELS\frag1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=vfrag1 X=0 Y=0 Z=0 YAW=64 ROLL=64
#exec MESH SEQUENCE MESH=vfrag1 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=vfrag1 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=vfrag1 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=vfrag1 NUM=0 TEXTURE=Jvase1

#exec MESH IMPORT MESH=vfrag2 ANIVFILE=MODELS\frag2_a.3D DATAFILE=MODELS\frag2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=vfrag2 X=0 Y=0 Z=0 YAW=64 ROLL=64
#exec MESH SEQUENCE MESH=vfrag2 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=vfrag2 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=vfrag2 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=vfrag2 NUM=0 TEXTURE=Jvase1

#exec MESH IMPORT MESH=vfrag3 ANIVFILE=MODELS\frag3_a.3D DATAFILE=MODELS\frag3_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=vfrag3 X=0 Y=0 Z=0 YAW=64 ROLL=64
#exec MESH SEQUENCE MESH=vfrag3 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=vfrag3 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=vfrag3 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=vfrag3 NUM=0 TEXTURE=Jvase1

#exec MESH IMPORT MESH=vfrag4 ANIVFILE=MODELS\frag4_a.3D DATAFILE=MODELS\frag4_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=vfrag4 X=0 Y=0 Z=0 YAW=64 ROLL=64
#exec MESH SEQUENCE MESH=vfrag4 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=vfrag4 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=vfrag4 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=vfrag4 NUM=0 TEXTURE=Jvase1

#exec MESH IMPORT MESH=vfrag5 ANIVFILE=MODELS\frag5_a.3D DATAFILE=MODELS\frag5_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=vfrag5 X=0 Y=0 Z=0 YAW=64 ROLL=64
#exec MESH SEQUENCE MESH=vfrag5 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=vfrag5 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=vfrag5 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=vfrag5 NUM=0 TEXTURE=Jvase1

#exec MESH IMPORT MESH=vfrag6 ANIVFILE=MODELS\frag6_a.3D DATAFILE=MODELS\frag6_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=vfrag6 X=0 Y=0 Z=0 YAW=64 ROLL=64
#exec MESH SEQUENCE MESH=vfrag6 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=vfrag6 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=vfrag6 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=vfrag6 NUM=0 TEXTURE=Jvase1

#exec MESH IMPORT MESH=vfrag7 ANIVFILE=MODELS\frag7_a.3D DATAFILE=MODELS\frag7_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=vfrag7 X=0 Y=0 Z=0 YAW=64 ROLL=64
#exec MESH SEQUENCE MESH=vfrag7 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=vfrag7 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=vfrag7 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=vfrag7 NUM=0 TEXTURE=Jvase1

#exec AUDIO IMPORT FILE="Sounds\General\Tink1.WAV" NAME="GlassTink1" GROUP="General"
#exec AUDIO IMPORT FILE="Sounds\General\Tink2.WAV" NAME="GlassTink2" GROUP="General"


simulated function CalcVelocity(vector Momentum, float ExplosionSize)
{
	ExplosionSize = VSize(Momentum);
	Velocity = VRand()*(ExplosionSize+FRand()*100.0+100.0); 
	Velocity.z += ExplosionSize/2;
}

defaultproperties
{
     Fragments(0)=Unreal.vfrag1
     Fragments(1)=Unreal.vfrag2
     Fragments(2)=Unreal.vfrag3
     Fragments(3)=Unreal.vfrag4
     Fragments(4)=Unreal.vfrag5
     Fragments(5)=Unreal.vfrag6
     Fragments(6)=Unreal.vfrag7
     numFragmentTypes=7
     ImpactSound=Unreal.GlassTink1
     MiscSound=Unreal.GlassTink2
     Mesh=Unreal.vfrag1
     LifeSpan=+00009.000000
}
