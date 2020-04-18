//=============================================================================
// EscapePod.
//=============================================================================
class EscapePod expands Decoration;


#exec MESH IMPORT MESH=Escapep ANIVFILE=MODELS\Escape_a.3D DATAFILE=MODELS\Escape_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Escapep X=0 Y=100 Z=0 YAW=64
#exec MESH SEQUENCE MESH=Escapep SEQ=All    STARTFRAME=0   NUMFRAMES=5
#exec MESH SEQUENCE MESH=Escapep SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Escapep SEQ=Flame  STARTFRAME=1   NUMFRAMES=4
#exec TEXTURE IMPORT NAME=JEscapep1 FILE=MODELS\escape.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=Escapep X=0.5 Y=0.5 Z=1.0
#exec MESHMAP SETTEXTURE MESHMAP=Escapep NUM=1 TEXTURE=JEscapep1

function Trigger( actor Other, pawn EventInstigator )
{
	SetPHysics(PHYS_Projectile);
	Velocity = Vect(0,0,50.0);
	SetTimer(15.0,False);
}


function Timer()
{
	Destroy();
}

defaultproperties
{
     bStatic=False
     bDirectional=True
     DrawType=DT_Mesh
     Mesh=Mesh'Unreal.Escapep'
}
