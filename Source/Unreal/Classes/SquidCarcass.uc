//=============================================================================
// SquidCarcass.
//=============================================================================
class SquidCarcass expands CreatureCarcass;

#exec MESH IMPORT MESH=SquidBody ANIVFILE=MODELS\g_sqdh_a.3D DATAFILE=MODELS\g_sqdh_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SquidBody X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SquidBody SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SquidBody SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgsquid  FILE=MODELS\g_squid.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=SquidBody X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SquidBody NUM=1 TEXTURE=Jgsquid

#exec MESH IMPORT MESH=SquidPart ANIVFILE=MODELS\g_sqdb_a.3D DATAFILE=MODELS\g_sqdb_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SquidPart X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SquidPart SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SquidPart SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgsquid  FILE=MODELS\g_squid.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=SquidPart X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SquidPart NUM=1 TEXTURE=Jgsquid

#exec MESH IMPORT MESH=SquidTail ANIVFILE=MODELS\g_sqdt_a.3D DATAFILE=MODELS\g_sqdt_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SquidTail X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SquidTail SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SquidTail SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgsquid  FILE=MODELS\g_squid.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=SquidTail X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SquidTail NUM=1 TEXTURE=Jgsquid

#exec MESH IMPORT MESH=SquidTail2 ANIVFILE=MODELS\g_sqd2_a.3D DATAFILE=MODELS\g_sqd2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SquidTail2 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SquidTail2 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SquidTail2 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgsquid  FILE=MODELS\g_squid.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=SquidTail2 X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SquidTail2 NUM=1 TEXTURE=Jgsquid

#exec MESH IMPORT MESH=SquidTail3 ANIVFILE=MODELS\g_sqd3_a.3D DATAFILE=MODELS\g_sqd3_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SquidTail3 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SquidTail3 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SquidTail3 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgsquid  FILE=MODELS\g_squid.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=SquidTail3 X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SquidTail3 NUM=1 TEXTURE=Jgsquid

#exec MESH IMPORT MESH=SquidTail4 ANIVFILE=MODELS\g_sqd4_a.3D DATAFILE=MODELS\g_sqd4_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SquidTail4 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SquidTail4 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SquidTail4 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgsquid  FILE=MODELS\g_squid.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=SquidTail4 X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SquidTail4 NUM=1 TEXTURE=Jgsquid

#exec MESH IMPORT MESH=SquidTail5 ANIVFILE=MODELS\g_sqd5_a.3D DATAFILE=MODELS\g_sqd5_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SquidTail5 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SquidTail5 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SquidTail5 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgsquid  FILE=MODELS\g_squid.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=SquidTail5 X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SquidTail5 NUM=1 TEXTURE=Jgsquid

defaultproperties
{
     bodyparts(0)=SquidBody
     bodyparts(1)=SquidPart
     bodyparts(2)=SquidPart
     bodyparts(3)=SquidTail
     bodyparts(4)=SquidTail2
     bodyparts(5)=SquidTail3
     bodyparts(6)=SquidTail4
     bodyparts(7)=SquidTail5
     CollisionRadius=+00036.000000
     CollisionHeight=+00040.000000
     Mass=+00200.000000
     Mesh=Squid1
	 AnimSequence=Dead1
}
