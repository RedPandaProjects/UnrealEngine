//=============================================================================
// WoodFragments.
//=============================================================================
class WoodFragments expands Fragment;

#exec MESH IMPORT MESH=wfrag1 ANIVFILE=MODELS\wfrag1_a.3D DATAFILE=MODELS\wfrag1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=wfrag1 X=0 Y=0 Z=0 YAW=64 ROLL=0
#exec MESH SEQUENCE MESH=wfrag1 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=wfrag1 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JWoodenBox1 FILE=MODELS\Box.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=wfrag1 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=wfrag1 NUM=0 TEXTURE=JWoodenBox1

#exec MESH IMPORT MESH=wfrag2 ANIVFILE=MODELS\wfrag2_a.3D DATAFILE=MODELS\wfrag2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=wfrag2 X=0 Y=0 Z=0 YAW=64 ROLL=0
#exec MESH SEQUENCE MESH=wfrag2 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=wfrag2 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=wfrag2 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=wfrag2 NUM=0 TEXTURE=JWoodenBox1

#exec MESH IMPORT MESH=wfrag3 ANIVFILE=MODELS\wfrag3_a.3D DATAFILE=MODELS\wfrag3_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=wfrag3 X=0 Y=0 Z=0 YAW=64 ROLL=0
#exec MESH SEQUENCE MESH=wfrag3 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=wfrag3 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=wfrag3 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=wfrag3 NUM=0 TEXTURE=JWoodenBox1

#exec MESH IMPORT MESH=wfrag4 ANIVFILE=MODELS\wfrag4_a.3D DATAFILE=MODELS\wfrag4_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=wfrag4 X=0 Y=0 Z=0 YAW=64 ROLL=0
#exec MESH SEQUENCE MESH=wfrag4 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=wfrag4 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=wfrag4 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=wfrag4 NUM=0 TEXTURE=JWoodenBox1

#exec MESH IMPORT MESH=wfrag5 ANIVFILE=MODELS\wfrag5_a.3D DATAFILE=MODELS\wfrag5_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=wfrag5 X=0 Y=0 Z=0 YAW=64 ROLL=0
#exec MESH SEQUENCE MESH=wfrag5 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=wfrag5 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=wfrag5 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=wfrag5 NUM=0 TEXTURE=JWoodenBox1

#exec MESH IMPORT MESH=wfrag6 ANIVFILE=MODELS\wfrag6_a.3D DATAFILE=MODELS\wfrag6_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=wfrag6 X=0 Y=0 Z=0 YAW=64 ROLL=0
#exec MESH SEQUENCE MESH=wfrag6 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=wfrag6 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=wfrag6 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=wfrag6 NUM=0 TEXTURE=JWoodenBox1

#exec MESH IMPORT MESH=wfrag7 ANIVFILE=MODELS\wfrag7_a.3D DATAFILE=MODELS\wfrag7_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=wfrag7 X=0 Y=0 Z=0 YAW=64 ROLL=0
#exec MESH SEQUENCE MESH=wfrag7 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=wfrag7 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=wfrag7 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=wfrag7 NUM=0 TEXTURE=JWoodenBox1

#exec MESH IMPORT MESH=wfrag8 ANIVFILE=MODELS\wfrag8_a.3D DATAFILE=MODELS\wfrag8_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=wfrag8 X=0 Y=0 Z=0 YAW=64 ROLL=0
#exec MESH SEQUENCE MESH=wfrag8 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=wfrag8 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=wfrag8 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=wfrag8 NUM=0 TEXTURE=JWoodenBox1

#exec MESH IMPORT MESH=wfrag9 ANIVFILE=MODELS\wfrag9_a.3D DATAFILE=MODELS\wfrag9_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=wfrag9 X=0 Y=0 Z=0 YAW=64 ROLL=0
#exec MESH SEQUENCE MESH=wfrag9 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=wfrag9 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=wfrag9 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=wfrag9 NUM=0 TEXTURE=JWoodenBox1

#exec AUDIO IMPORT FILE="Sounds\General\WoodHit1.WAV" NAME="WoodHit1" GROUP="General"
#exec AUDIO IMPORT FILE="Sounds\General\WoodHit2.WAV" NAME="WoodHit2" GROUP="General"

simulated function CalcVelocity(vector Momentum, float ExplosionSize)
{
	Super.CalcVelocity(Momentum, ExplosionSize);
	Velocity.z += ExplosionSize/2;
}

defaultproperties
{
     Fragments(0)=Unreal.wfrag1
     Fragments(1)=Unreal.wfrag2
     Fragments(2)=Unreal.wfrag3
     Fragments(3)=Unreal.wfrag4
     Fragments(4)=Unreal.wfrag5
     Fragments(5)=Unreal.wfrag6
     Fragments(6)=Unreal.wfrag7
     Fragments(7)=Unreal.wfrag8
     Fragments(8)=Unreal.wfrag9
     numFragmentTypes=9
     ImpactSound=Unreal.WoodHit1
     MiscSound=Unreal.WoodHit2
     Mesh=Unreal.wfrag2
     CollisionRadius=+00012.000000
     CollisionHeight=+00002.000000
     Mass=+00005.000000
     Buoyancy=+00006.000000
}
