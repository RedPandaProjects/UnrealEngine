//=============================================================================
// Tapestry1.
//=============================================================================
class Tapestry1 expands Decoration;

#exec MESH IMPORT MESH=Tap ANIVFILE=MODELS\Tap_a.3D DATAFILE=MODELS\Tap_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Tap X=0 Y=00 Z=0 YAW=0
#exec MESH SEQUENCE MESH=Tap SEQ=All    STARTFRAME=0   NUMFRAMES=15
#exec MESH SEQUENCE MESH=Tap SEQ=Waver  STARTFRAME=0   NUMFRAMES=15
#exec TEXTURE IMPORT NAME=JTap1 FILE=MODELS\Tap1.PCX GROUP=Skins FLAGS=2

#exec MESHMAP SCALE MESHMAP=Tap X=0.15 Y=0.15 Z=0.3

#exec MESHMAP SETTEXTURE MESHMAP=Tap NUM=1 TEXTURE=JTap1


Auto State Animate
{
Begin:
	LoopAnim('Waver',0.2);
}
defaultproperties
{
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Tap
     bMeshCurvy=False
     Physics=PHYS_Walking
}
