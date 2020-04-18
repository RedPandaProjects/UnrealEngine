//=============================================================================
// TentacleCarcass.
//=============================================================================
class TentacleCarcass expands CreatureCarcass;

#exec MESH IMPORT MESH=TentArm ANIVFILE=MODELS\g_tentt_a.3D DATAFILE=MODELS\g_tentt_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=TentArm X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=TentArm SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=TentArm SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgtent  FILE=MODELS\g_tntcl1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=TentArm X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=TentArm NUM=1 TEXTURE=Jgtent

#exec MESH IMPORT MESH=TentBody ANIVFILE=MODELS\g_tentb_a.3D DATAFILE=MODELS\g_tentb_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=TentBody X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=TentBody SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=TentBody SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgtent  FILE=MODELS\g_tntcl1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=TentBody X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=TentBody NUM=1 TEXTURE=Jgtent

#exec MESH IMPORT MESH=TentHead ANIVFILE=MODELS\g_tenth_a.3D DATAFILE=MODELS\g_tenth_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=TentHead X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=TentHead SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=TentHead SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgtent  FILE=MODELS\g_tntcl1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=TentHead X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=TentHead NUM=1 TEXTURE=Jgtent

#exec MESH IMPORT MESH=TentPart ANIVFILE=MODELS\g_tentp_a.3D DATAFILE=MODELS\g_tentp_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=TentPart X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=TentPart SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=TentPart SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgtent  FILE=MODELS\g_tntcl1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=TentPart X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=TentPart NUM=1 TEXTURE=Jgtent

function Drop()
{
	//implemented in TentacleCarcass
	Velocity.X = 0;
	Velocity.Y = 0;
	SetPhysics(PHYS_Falling);
}


function Landed(vector HitNormal)
{
	if ( AnimSequence == 'Dead1')
		PlayAnim('Dead1Land', 1.5);
	SetPhysics(PHYS_None);
	LieStill();
}

defaultproperties
{
     bodyparts(0)=TentBody
     bodyparts(1)=TentPart
     bodyparts(2)=TentPart
     bodyparts(3)=TentArm
     bodyparts(4)=TentArm
     bodyparts(5)=TentHead
     bodyparts(6)=TentArm
     Mesh=Tentacle1
     CollisionRadius=+00028.000000
     CollisionHeight=+00036.000000
	 AnimSequence=Dead1Land
     Mass=+00200.000000
	 Buoyancy=+00190.000000
}
