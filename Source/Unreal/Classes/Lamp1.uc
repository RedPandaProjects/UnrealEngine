//=============================================================================
// Lamp1.
//=============================================================================
class Lamp1 expands Decoration;

#exec MESH IMPORT MESH=Lamp1M ANIVFILE=MODELS\lamp1_a.3D DATAFILE=MODELS\lamp1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Lamp1M X=0 Y=0 Z=0 YAW=64

#exec MESH SEQUENCE MESH=lamp1M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=lamp1M SEQ=Still  STARTFRAME=0   NUMFRAMES=1

#exec TEXTURE IMPORT NAME=JLamp11 FILE=MODELS\lamp1.PCX GROUP=Skins FLAGS=2

#exec MESHMAP SCALE MESHMAP=lamp1M X=0.1 Y=0.1 Z=0.2

#exec MESHMAP SETTEXTURE MESHMAP=lamp1M NUM=1 TEXTURE=Jlamp11
defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Lamp1M
     CollisionRadius=+00016.000000
     CollisionHeight=+00076.000000
     bCollideActors=True
     bCollideWorld=True
     Physics=PHYS_None
}
