//=============================================================================
// DevilfishCarcass.
//=============================================================================
class DevilfishCarcass expands CreatureCarcass;

#exec MESH IMPORT MESH=FishHead ANIVFILE=MODELS\g_shah_a.3D DATAFILE=MODELS\g_shah_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=FishHead X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=FishHead SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=FishHead SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgfish1  FILE=MODELS\g_shark.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=FishHead X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=FishHead NUM=1 TEXTURE=Jgfish1

#exec MESH IMPORT MESH=FishPart ANIVFILE=MODELS\g_shap_a.3D DATAFILE=MODELS\g_shap_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=FishPart X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=FishPart SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=FishPart SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgfish1  FILE=MODELS\g_shark.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=FishPart X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=FishPart NUM=1 TEXTURE=Jgfish1

#exec MESH IMPORT MESH=FishTail ANIVFILE=MODELS\g_shat_a.3D DATAFILE=MODELS\g_shat_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=FishTail X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=FishTail SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=FishTail SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgfish1  FILE=MODELS\g_shark.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=FishTail X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=FishTail NUM=1 TEXTURE=Jgfish1

function ForceMeshToExist()
{
	//never called
	Spawn(class 'DevilFish');
}

defaultproperties
{
     bodyparts(0)=FishHead
     bodyparts(1)=FishPart
     bodyparts(2)=FishPart
     bodyparts(3)=FishTail
     bodyparts(4)=None
     Mesh=fish
     CollisionRadius=+00022.000000
     CollisionHeight=+00010.000000
	 Buoyancy=+0.00000
}
