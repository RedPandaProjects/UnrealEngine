//=============================================================================
// SlithCarcass.
//=============================================================================
class SlithCarcass expands CreatureCarcass;

#exec MESH IMPORT MESH=SlithArm ANIVFILE=MODELS\g_slia_a.3D DATAFILE=MODELS\g_slia_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SlithArm X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SlithArm SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SlithArm SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgslith1  FILE=MODELS\g_slith1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=SlithArm X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SlithArm NUM=1 TEXTURE=Jgslith1

#exec MESH IMPORT MESH=SlithHand ANIVFILE=MODELS\g_slih_a.3D DATAFILE=MODELS\g_slih_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SlithHand X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SlithHand SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SlithHand SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgslith1  FILE=MODELS\g_slith1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=SlithHand X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SlithHand NUM=1 TEXTURE=Jgslith1

#exec MESH IMPORT MESH=SlithHead ANIVFILE=MODELS\g_sliz_a.3D DATAFILE=MODELS\g_sliz_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SlithHead X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SlithHead SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SlithHead SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgslith2  FILE=MODELS\g_slith2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=SlithHead X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SlithHead NUM=1 TEXTURE=Jgslith2

#exec MESH IMPORT MESH=SlithPart ANIVFILE=MODELS\g_slib_a.3D DATAFILE=MODELS\g_slib_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SlithPart X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SlithPart SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SlithPart SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgslith2  FILE=MODELS\g_slith2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=SlithPart X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SlithPart NUM=1 TEXTURE=Jgslith2

#exec MESH IMPORT MESH=SlithTail ANIVFILE=MODELS\g_slit_a.3D DATAFILE=MODELS\g_slit_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SlithTail X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SlithTail SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SlithTail SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgslith1  FILE=MODELS\g_slith1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=SlithTail X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SlithTail NUM=1 TEXTURE=Jgslith1

function ForceMeshToExist()
{
	//never called
	Spawn(class 'Slith');
}
defaultproperties
{
     bodyparts(0)=SlithPart
     bodyparts(1)=SlithPart
     bodyparts(2)=SlithHand
     bodyparts(3)=SlithHead
     bodyparts(4)=SlithArm
     bodyparts(5)=SlithArm
     bodyparts(6)=CowBody1
     bodyparts(7)=SlithTail
     ZOffset(0)=+00000.000000
     ZOffset(1)=+00000.000000
     Mesh=Slith1
	 AnimSequence=Dead1
     CollisionRadius=+00048.000000
     CollisionHeight=+00044.000000
     Mass=+00200.000000
     Buoyancy=+00190.000000
}
