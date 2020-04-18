//=============================================================================
// MercCarcass.
//=============================================================================
class MercCarcass expands CreatureCarcass;

#exec MESH IMPORT MESH=MercArm ANIVFILE=MODELS\g_merca_a.3D DATAFILE=MODELS\g_merca_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=MercArm X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=MercArm SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=MercArm SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jmerce1  FILE=MODELS\g_merc1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=MercArm X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=MercArm NUM=1 TEXTURE=Jmerce1

#exec MESH IMPORT MESH=MercFoot ANIVFILE=MODELS\g_mercf_a.3D DATAFILE=MODELS\g_mercf_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=MercFoot X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=MercFoot SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=MercFoot SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jmerce1  FILE=MODELS\g_merc1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=MercFoot X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=MercFoot NUM=1 TEXTURE=Jmerce1

#exec MESH IMPORT MESH=MercGun ANIVFILE=MODELS\g_mercg_a.3D DATAFILE=MODELS\g_mercg_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=MercGun X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=MercGun SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=MercGun SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jmerce1  FILE=MODELS\g_merc1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=MercGun X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=MercGun NUM=1 TEXTURE=Jmerce1

#exec MESH IMPORT MESH=MercHead ANIVFILE=MODELS\g_merch_a.3D DATAFILE=MODELS\g_merch_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=MercHead X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=MercHead SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=MercHead SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jmerce2  FILE=MODELS\g_merc2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=MercHead X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=MercHead NUM=1 TEXTURE=Jmerce2

#exec MESH IMPORT MESH=MercLeg ANIVFILE=MODELS\g_mercl_a.3D DATAFILE=MODELS\g_mercl_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=MercLeg X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=MercLeg SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=MercLeg SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jmerce2  FILE=MODELS\g_merc2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=MercLeg X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=MercLeg NUM=1 TEXTURE=Jmerce2

#exec MESH IMPORT MESH=MercPart ANIVFILE=MODELS\g_mercp_a.3D DATAFILE=MODELS\g_mercp_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=MercPart X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=MercPart SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=MercPart SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jmerce2  FILE=MODELS\g_merc2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=MercPart X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=MercPart NUM=1 TEXTURE=Jmerce2

#exec AUDIO IMPORT FILE="Sounds\Mercenary\thump1a.WAV" NAME="thumpmr" GROUP="Mercenary"

function ForceMeshToExist()
{
	//never called
	Spawn(class 'Mercenary');
}

function InitFor(actor Other)
{
	Super.InitFor(Other);
	if ( AnimSequence == 'Dead5' )
		bodyparts[5]=None;
}

defaultproperties
{
     bodyparts(0)=Mesh'Unreal.MercLeg'
     bodyparts(1)=Mesh'Unreal.MercPart'
     bodyparts(2)=Mesh'Unreal.MercGun'
     bodyparts(3)=Mesh'Unreal.MercPart'
     bodyparts(4)=Mesh'Unreal.MercLeg'
     bodyparts(5)=Mesh'Unreal.MercHead'
     ZOffset(1)=0.000000
     ZOffset(4)=-0.500000
     ZOffset(5)=-0.500000
     bGreenBlood=True
     LandedSound=Sound'Unreal.Mercenary.thumpmr'
     AnimSequence=Death
     Mesh=Mesh'Unreal.Merc'
     CollisionRadius=35.000000
     CollisionHeight=48.000000
     Mass=150.000000
     Buoyancy=140.000000
}
