//=============================================================================
// BloodTrail.
//=============================================================================
class BloodTrail expands Blood2;

#exec MESH IMPORT MESH=BloodTrl ANIVFILE=MODELS\Blood2_a.3D DATAFILE=MODELS\Blood2_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=BloodTrl X=0 Y=0 Z=0 YAW=128
#exec MESH SEQUENCE MESH=BloodTrl SEQ=All       STARTFRAME=0   NUMFRAMES=45
#exec MESH SEQUENCE MESH=BloodTrl SEQ=Spray     STARTFRAME=0   NUMFRAMES=6
#exec MESH SEQUENCE MESH=BloodTrl SEQ=Still     STARTFRAME=6   NUMFRAMES=1
#exec MESH SEQUENCE MESH=BloodTrl SEQ=GravSpray STARTFRAME=7   NUMFRAMES=5
#exec MESH SEQUENCE MESH=BloodTrl SEQ=Stream    STARTFRAME=12  NUMFRAMES=11
#exec MESH SEQUENCE MESH=BloodTrl SEQ=Trail     STARTFRAME=23  NUMFRAMES=11
#exec MESH SEQUENCE MESH=BloodTrl SEQ=Burst     STARTFRAME=34  NUMFRAMES=2
#exec MESH SEQUENCE MESH=BloodTrl SEQ=GravSpray2 STARTFRAME=36 NUMFRAMES=7

#exec TEXTURE IMPORT NAME=BloodSpot FILE=MODELS\bloods2.PCX GROUP=Skins FLAGS=2
#exec TEXTURE IMPORT NAME=BloodSGrn FILE=MODELS\bloodg2.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=BloodTrl X=0.11 Y=0.055 Z=0.11 YAW=128
#exec MESHMAP SETTEXTURE MESHMAP=BloodTrl NUM=0  TEXTURE=BloodSpot

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();	
	LoopAnim('Trail');
}

auto state Trail
{
}

defaultproperties
{
     Physics=PHYS_Trailer
     LifeSpan=5.000000
     AnimSequence=trail
     Mesh=Mesh'Unreal.BloodTrl'
     DrawScale=0.200000
     AmbientGlow=0
}
