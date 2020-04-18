//=============================================================================
// Sconce.
//=============================================================================
class Sconce expands Decoration;

#exec MESH IMPORT MESH=sconceM ANIVFILE=MODELS\sconce_a.3D DATAFILE=MODELS\sconce_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=sconceM X=0 Y=00 Z=0 YAW=64
#exec MESH SEQUENCE MESH=sconceM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=sconceM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jsconce1 FILE=MODELS\sconce.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=sconceM X=0.03 Y=0.03 Z=0.06
#exec MESHMAP SETTEXTURE MESHMAP=sconceM NUM=0 TEXTURE=Jsconce1

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Unreal.SconceM
     bMeshCurvy=False
     bCollideActors=True
     bCollideWorld=True
     bProjTarget=True
     Class=Unreal.Sconce
}
