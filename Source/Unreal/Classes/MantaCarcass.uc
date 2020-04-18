//=============================================================================
// MantaCarcass.
//=============================================================================
class MantaCarcass expands CreatureCarcass;

#exec MESH IMPORT MESH=MantaHead ANIVFILE=MODELS\g_mtah_a.3D DATAFILE=MODELS\g_mtah_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=MantaHead X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=MantaHead SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=MantaHead SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jmta1  FILE=MODELS\g_manta1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=MantaHead X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=MantaHead NUM=1 TEXTURE=Jmta1

#exec MESH IMPORT MESH=MantaPart ANIVFILE=MODELS\g_mtap_a.3D DATAFILE=MODELS\g_mtap_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=MantaPart X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=MantaPart SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=MantaPart SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jmta1  FILE=MODELS\g_manta1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=MantaPart X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=MantaPart NUM=1 TEXTURE=Jmta1

#exec MESH IMPORT MESH=MantaTail ANIVFILE=MODELS\g_mtat_a.3D DATAFILE=MODELS\g_mtat_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=MantaTail X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=MantaTail SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=MantaTail SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jmta2  FILE=MODELS\g_manta2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=MantaTail X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=MantaTail NUM=1 TEXTURE=Jmta2

#exec MESH IMPORT MESH=MantaWing1 ANIVFILE=MODELS\g_mtaw_a.3D DATAFILE=MODELS\g_mtaw_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=MantaWing1 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=MantaWing1 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=MantaWing1 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jmta2  FILE=MODELS\g_manta2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=MantaWing1 X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=MantaWing1 NUM=1 TEXTURE=Jmta2

#exec MESH IMPORT MESH=MantaWing2 ANIVFILE=MODELS\g_mta2_a.3D DATAFILE=MODELS\g_mta2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=MantaWing2 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=MantaWing2 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=MantaWing2 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jmta2  FILE=MODELS\g_manta2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=MantaWing2 X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=MantaWing2 NUM=1 TEXTURE=Jmta2

#exec AUDIO IMPORT FILE="Sounds\Manta\dslap2.WAV" NAME="thumpmt" GROUP="Manta"

function ForceMeshToExist()
{
	//never called
	Spawn(class 'Manta');
}

defaultproperties
{
     bodyparts(0)=MantaPart
     bodyparts(1)=MantaPart
     bodyparts(2)=MantaHead
     bodyparts(3)=MantaTail
     bodyparts(4)=MantaWing1
     bodyparts(5)=MantaWing2
	 ZOffset(0)=+00000.000000
	 ZOffset(1)=+00000.000000
	 LandedSound=thumpmt
     Mesh=Manta1
	 AnimSequence=Death
	 AnimFrame=+00000.960000
     CollisionRadius=+00027.000000
     CollisionHeight=+00012.000000
     Mass=+00080.000000
}
