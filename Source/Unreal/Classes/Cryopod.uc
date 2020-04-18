//=============================================================================
// Cryopod.
//=============================================================================
class Cryopod expands Decoration;

#exec MESH IMPORT MESH=CryopodM ANIVFILE=MODELS\cryo_a.3D DATAFILE=MODELS\cryo_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=CryopodM X=0 Y=0 Z=-220 YAW=64
#exec MESH SEQUENCE MESH=CryopodM SEQ=All  STARTFRAME=0  NUMFRAMES=22
#exec MESH SEQUENCE MESH=CryopodM SEQ=Close STARTFRAME=0 NUMFRAMES=11
#exec MESH SEQUENCE MESH=CryopodM SEQ=Open STARTFRAME=11 NUMFRAMES=11
#exec TEXTURE IMPORT NAME=JCryopod1 FILE=MODELS\cryo.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=CryopodM X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=CryopodM NUM=1 TEXTURE=JCryopod1

var() Sound CryoOpen;
var() Sound CryoClose;

Auto State CryoPod
{

function Trigger( actor Other, pawn EventInstigator )
{
	if (AnimSequence=='Close')
		GotoState( 'CryoPod','Open');
	else
		GotoState( 'CryoPod','Close');
}

Open: 
	Disable('Trigger');
	PlayAnim('Open',0.4);
	PlaySound(CryoOpen,SLOT_Misc,1.0);
	FinishAnim();
	Enable('Trigger');	
	Stop;

Close:
	Disable('Trigger');
	PlayAnim('Close',0.4);
	PlaySound(CryoClose,SLOT_Misc,1.0);
	FinishAnim();
	Sleep(1.0);
	Enable('Trigger');
	Stop;
	
Begin:
	PlayAnim('Close',0.4);
}

defaultproperties
{
     bStatic=False
     bDirectional=True
     DrawType=DT_Mesh
     Mesh=CryopodM
     bMeshCurvy=False
     CollisionRadius=+00040.000000
     bCollideActors=True
}
