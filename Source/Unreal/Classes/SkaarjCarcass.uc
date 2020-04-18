//=============================================================================
// SkaarjCarcass.
//=============================================================================
class SkaarjCarcass expands CreatureCarcass;

#exec MESH IMPORT MESH=SkaarjBody ANIVFILE=MODELS\g_Skrb_a.3D DATAFILE=MODELS\g_skrb_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SkaarjBody X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SkaarjBody SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SkaarjBody SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JGSkaarj1  FILE=MODELS\skr1.PCX FAMILY=Skins 
#exec MESHMAP SCALE MESHMAP=SkaarjBody X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SkaarjBody NUM=1 TEXTURE=JGSkaarj1

#exec MESH IMPORT MESH=SkaarjHand ANIVFILE=MODELS\g_Skrh_a.3D DATAFILE=MODELS\g_skrh_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SkaarjHand X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SkaarjHand SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SkaarjHand SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JGSkaarj2  FILE=MODELS\skr2.PCX FAMILY=Skins 
#exec MESHMAP SCALE MESHMAP=SkaarjHand X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SkaarjHand NUM=1 TEXTURE=JGSkaarj2

#exec MESH IMPORT MESH=SkaarjHead ANIVFILE=MODELS\g_Skrz_a.3D DATAFILE=MODELS\g_skrz_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SkaarjHead X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SkaarjHead SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SkaarjHead SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JGSkaarj1  FILE=MODELS\skr1.PCX FAMILY=Skins 
#exec MESHMAP SCALE MESHMAP=SkaarjHead X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SkaarjHead NUM=1 TEXTURE=JGSkaarj1

#exec MESH IMPORT MESH=SkaarjLeg ANIVFILE=MODELS\g_Skrl_a.3D DATAFILE=MODELS\g_skrl_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SkaarjLeg X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SkaarjLeg SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SkaarjLeg SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JGSkaarj2  FILE=MODELS\skr2.PCX FAMILY=Skins 
#exec MESHMAP SCALE MESHMAP=SkaarjLeg X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=SkaarjLeg NUM=1 TEXTURE=JGSkaarj2

#exec MESH IMPORT MESH=SkaarjTail ANIVFILE=MODELS\g_Skrt_a.3D DATAFILE=MODELS\g_skrt_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SkaarjTail X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SkaarjTail SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SkaarjTail SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JGSkaarj2  FILE=MODELS\skr2.PCX FAMILY=Skins 
#exec MESHMAP SCALE MESHMAP=SkaarjTail X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=SkaarjTail NUM=1 TEXTURE=JGSkaarj2

function ForceMeshToExist()
{
	//never called
	Spawn(class 'Skaarjwarrior');
}

function InitFor(actor Other)
{
	Super.InitFor(Other);
	if ( AnimSequence == 'Death5' )
		bodyparts[7]=None;
}

defaultproperties
{
     bodyparts(0)=SkaarjTail
     bodyparts(1)=SkaarjBody
     bodyparts(2)=SkaarjHand
     bodyparts(3)=SkaarjBody
     bodyparts(4)=SkaarjLeg
     bodyparts(5)=SkaarjLeg
     bodyparts(6)=CowBody1
     bodyparts(7)=SkaarjHead
     ZOffset(0)=+00000.500000
     ZOffset(1)=+00000.000000
     ZOffset(3)=+00000.300000
     ZOffset(4)=-00000.500000
     ZOffset(5)=-00000.500000
     CollisionRadius=+00035.000000
     CollisionHeight=+00046.000000
     Mass=+00150.000000
     Buoyancy=+00140.000000
     Mesh=Skaarjw
     AnimSequence=Death
}
