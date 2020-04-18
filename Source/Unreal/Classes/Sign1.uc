//=============================================================================
// Sign1.
//=============================================================================
class Sign1 expands Decoration;

#exec MESH IMPORT MESH=Sign1M ANIVFILE=MODELS\sign1_a.3D DATAFILE=MODELS\sign1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Sign1M X=0 Y=100 Z=-120 YAW=64

#exec MESH SEQUENCE MESH=sign1M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=sign1M SEQ=Still  STARTFRAME=0   NUMFRAMES=1

#exec TEXTURE IMPORT NAME=JSign11 FILE=MODELS\sign1.PCX GROUP=Skins FLAGS=2

#exec MESHMAP SCALE MESHMAP=sign1M X=0.1 Y=0.1 Z=0.2

#exec MESHMAP SETTEXTURE MESHMAP=sign1M NUM=1 TEXTURE=Jsign11
defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Sign1M
     bProjTarget=True
}
