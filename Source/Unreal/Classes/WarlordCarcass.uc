//=============================================================================
// WarlordCarcass.
//=============================================================================
class WarlordCarcass expands CreatureCarcass;

#exec MESH IMPORT MESH=WarlordArm ANIVFILE=MODELS\g_wara_a.3D DATAFILE=MODELS\g_wara_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=WarlordArm X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=WarlordArm SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=WarlordArm SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgwar1  FILE=MODELS\g_war1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=WarlordArm X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=WarlordArm NUM=1 TEXTURE=Jgwar1

#exec MESH IMPORT MESH=WarlordFoot ANIVFILE=MODELS\g_warf_a.3D DATAFILE=MODELS\g_warf_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=WarlordFoot X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=WarlordFoot SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=WarlordFoot SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgwar2  FILE=MODELS\g_war2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=WarlordFoot X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=WarlordFoot NUM=1 TEXTURE=Jgwar2

#exec MESH IMPORT MESH=WarlordGun ANIVFILE=MODELS\g_warg_a.3D DATAFILE=MODELS\g_warg_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=WarlordGun X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=WarlordGun SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=WarlordGun SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgwar1  FILE=MODELS\g_war1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=WarlordGun X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=WarlordGun NUM=1 TEXTURE=Jgwar1

#exec MESH IMPORT MESH=WarlordHand ANIVFILE=MODELS\g_warh_a.3D DATAFILE=MODELS\g_warh_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=WarlordHand X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=WarlordHand SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=WarlordHand SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgwar2  FILE=MODELS\g_war2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=WarlordHand X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=WarlordHand NUM=1 TEXTURE=Jgwar2

#exec MESH IMPORT MESH=WarlordHead ANIVFILE=MODELS\g_warz_a.3D DATAFILE=MODELS\g_warz_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=WarlordHead X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=WarlordHead SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=WarlordHead SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgwar2  FILE=MODELS\g_war2.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=WarlordHead X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=WarlordHead NUM=1 TEXTURE=Jgwar2

#exec MESH IMPORT MESH=WarlordLeg ANIVFILE=MODELS\g_warl_a.3D DATAFILE=MODELS\g_warl_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=WarlordLeg X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=WarlordLeg SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=WarlordLeg SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgwar1  FILE=MODELS\g_war1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=WarlordLeg X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=WarlordLeg NUM=1 TEXTURE=Jgwar1

#exec MESH IMPORT MESH=WarlordWing ANIVFILE=MODELS\g_warw_a.3D DATAFILE=MODELS\g_warw_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=WarlordWing X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=WarlordWing SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=WarlordWing SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jgwar1  FILE=MODELS\g_war1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=WarlordWing X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=WarlordWing NUM=1 TEXTURE=Jgwar1

function AnimEnd()
{
	if ( AnimSequence == 'Dead2A' )
	{
		if ( Physics == PHYS_None )
		{
			LieStill();
			PlayAnim('Dead2B', 0.7, 0.07);
		}
		else
			LoopAnim('Fall');
	} 
	else if ( Physics == PHYS_None )
		LieStill();
}

function Landed(vector HitNormal)
{
	if ( AnimSequence == 'Fall' )
	{
		LieStill();
		PlayAnim('Dead2B', 0.7, 0.07);
	}
	SetPhysics(PHYS_None);
}

state Dead 
{
	function BeginState()
	{
	}
}

defaultproperties
{
      LifeSpan=+00000.000000
      bodyparts(0)=WarlordWing
      bodyparts(1)=WarlordHead
      bodyparts(2)=WarlordLeg
      bodyparts(3)=WarlordArm
      bodyparts(4)=WarlordLeg
      bodyparts(5)=WarlordGun
      bodyparts(6)=WarlordFoot
      bodyparts(7)=WarlordHand
      ZOffset(1)=+00000.500000
      ZOffset(2)=-00000.500000
      ZOffset(4)=-00000.500000
      ZOffset(6)=-00000.700000
}
