//=============================================================================
// PupaeCarcass.
//=============================================================================
class PupaeCarcass expands CreatureCarcass;

#exec MESH IMPORT MESH=PupaeBody ANIVFILE=MODELS\g_pupb_a.3D DATAFILE=MODELS\g_pupb_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=PupaeBody X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=PupaeBody SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=PupaeBody SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgpupae1  FILE=MODELS\g_pupae.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=PupaeBody X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=PupaeBody NUM=1 TEXTURE=Jgpupae1

#exec MESH IMPORT MESH=PupaeHead ANIVFILE=MODELS\g_puph_a.3D DATAFILE=MODELS\g_puph_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=PupaeHead X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=PupaeHead SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=PupaeHead SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgpupae1  FILE=MODELS\g_pupae.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=PupaeHead X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=PupaeHead NUM=1 TEXTURE=Jgpupae1

#exec MESH IMPORT MESH=PupaeLeg1 ANIVFILE=MODELS\g_pupl_a.3D DATAFILE=MODELS\g_pupl_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=PupaeLeg1 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=PupaeLeg1 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=PupaeLeg1 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgpupae1  FILE=MODELS\g_pupae.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=PupaeLeg1 X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=PupaeLeg1 NUM=1 TEXTURE=Jgpupae1

#exec MESH IMPORT MESH=PupaeLeg2 ANIVFILE=MODELS\g_pup2_a.3D DATAFILE=MODELS\g_pup2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=PupaeLeg2 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=PupaeLeg2 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=PupaeLeg2 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgpupae1  FILE=MODELS\g_pupae.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=PupaeLeg2 X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=PupaeLeg2 NUM=1 TEXTURE=Jgpupae1

#exec MESH IMPORT MESH=PupaeLeg3 ANIVFILE=MODELS\g_pup3_a.3D DATAFILE=MODELS\g_pup3_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=PupaeLeg3 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=PupaeLeg3 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=PupaeLeg3 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgpupae1  FILE=MODELS\g_pupae.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=PupaeLeg3 X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=PupaeLeg3 NUM=1 TEXTURE=Jgpupae1

#exec AUDIO IMPORT FILE="Sounds\Pupae\thump1.WAV" NAME="thumppp" GROUP="Pupae"

function InitFor(actor Other)
{
	Super.InitFor(Other);
	if ( AnimSequence == 'Dead2' )
		bodyparts[4]=None;
	else if ( AnimSequence == 'Dead3' )
	{
		bodyparts[4]=None;
		bodyparts[3]=mesh'PupaeHead';
	}
}
defaultproperties
{
     bodyparts(0)=PupaeBody
     bodyparts(1)=PupaeLeg3
     bodyparts(2)=PupaeLeg1
     bodyparts(3)=PupaeLeg2
     bodyparts(4)=PupaeHead
     bodyparts(5)=None
	 ZOffset(0)=+00000.000000
	 ZOffset(1)=+00000.000000
	 LandedSound=thumppp
	 Mesh=pupae1
     CollisionRadius=+00028.000000
     CollisionHeight=+00009.000000
     Mass=+00080.000000
}
