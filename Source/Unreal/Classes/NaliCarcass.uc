//=============================================================================
// NaliCarcass.
//=============================================================================
class NaliCarcass expands CreatureCarcass;

#exec MESH IMPORT MESH=NaliFoot ANIVFILE=MODELS\g_nalf_a.3D DATAFILE=MODELS\g_nalf_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=NaliFoot X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=NaliFoot SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=NaliFoot SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgnali2  FILE=MODELS\g_nali2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=NaliFoot X=0.08 Y=0.08 Z=0.16
#exec MESHMAP SETTEXTURE MESHMAP=NaliFoot NUM=1 TEXTURE=Jgnali2

#exec MESH IMPORT MESH=NaliHand1 ANIVFILE=MODELS\g_nalh_a.3D DATAFILE=MODELS\g_nalh_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=NaliHand1 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=NaliHand1 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=NaliHand1 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgnali1  FILE=MODELS\g_nali1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=NaliHand1 X=0.08 Y=0.08 Z=0.16
#exec MESHMAP SETTEXTURE MESHMAP=NaliHand1 NUM=1 TEXTURE=Jgnali1

#exec MESH IMPORT MESH=NaliHand2 ANIVFILE=MODELS\g_nal2_a.3D DATAFILE=MODELS\g_nal2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=NaliHand2 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=NaliHand2 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=NaliHand2 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgnali1  FILE=MODELS\g_nali1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=NaliHand2 X=0.08 Y=0.08 Z=0.16
#exec MESHMAP SETTEXTURE MESHMAP=NaliHand2 NUM=1 TEXTURE=Jgnali1

#exec MESH IMPORT MESH=NaliHead ANIVFILE=MODELS\g_nalz_a.3D DATAFILE=MODELS\g_nalz_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=NaliHead X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=NaliHead SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=NaliHead SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgnali2  FILE=MODELS\g_nali2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=NaliHead X=0.08 Y=0.08 Z=0.16
#exec MESHMAP SETTEXTURE MESHMAP=NaliHead NUM=1 TEXTURE=Jgnali2

#exec MESH IMPORT MESH=NaliLeg ANIVFILE=MODELS\g_nall_a.3D DATAFILE=MODELS\g_nall_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=NaliLeg X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=NaliLeg SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=NaliLeg SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgnali1  FILE=MODELS\g_nali1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=NaliLeg X=0.08 Y=0.08 Z=0.16
#exec MESHMAP SETTEXTURE MESHMAP=NaliLeg NUM=1 TEXTURE=Jgnali1

#exec MESH IMPORT MESH=NaliPart ANIVFILE=MODELS\g_nalb_a.3D DATAFILE=MODELS\g_nalb_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=NaliPart X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=NaliPart SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=NaliPart SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgnali2  FILE=MODELS\g_nali2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=NaliPart X=0.08 Y=0.08 Z=0.16
#exec MESHMAP SETTEXTURE MESHMAP=NaliPart NUM=1 TEXTURE=Jgnali2

#exec AUDIO IMPORT FILE="Sounds\Nali\thump1.WAV" NAME="thumpn" GROUP="Nali"

function ForceMeshToExist()
{
	//never called
	Spawn(class 'Nali');
}

function InitFor(actor Other)
{
	Super.InitFor(Other);
	if ( AnimSequence == 'Dead3' )
		bodyparts[6]=None;
}

defaultproperties
{
      bodyparts(0)=NaliPart
      bodyparts(1)=NaliLeg
      bodyparts(2)=NaliPart
      bodyparts(3)=NaliFoot
      bodyparts(4)=NaliHand1
      bodyparts(5)=NaliHand2
      bodyparts(6)=NaliHead
	  ZOffset(0)=+00000.000000
	  ZOffset(1)=-00000.500000
	  ZOffset(3)=-00000.500000
	  ZOffset(6)=+00000.500000
	  LandedSound=thumpn
      Mesh=Nali1
      CollisionRadius=+00024.000000
      CollisionHeight=+00048.000000
      Mass=+00100.000000
	  Buoyancy=+00096.000000
}
