//=============================================================================
// SearchLight.
//=============================================================================
class SearchLight expands Flashlight;

#exec TEXTURE IMPORT NAME=I_BigFlash FILE=TEXTURES\HUD\i_bigf.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=BigFlash ANIVFILE=MODELS\BigFl_a.3D DATAFILE=MODELS\BigFl_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=BigFlash X=0 Y=0 Z=-90 YAW=64
#exec MESH SEQUENCE MESH=BigFlash SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=BigFlash SEQ=Still  STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JBigFlash1 FILE=MODELS\BigFlash.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=BigFlash X=0.035 Y=0.035 Z=0.07
#exec MESHMAP SETTEXTURE MESHMAP=BigFlash NUM=1 TEXTURE=JBigFlash1

defaultproperties
{
     PickupMessage="You picked up the Searchlight."
     RespawnTime=300.000000
     PickupViewMesh=Mesh'Unreal.BigFlash'
     Charge=20000
     Icon=Texture'Unreal.Icons.I_BigFlash'
     Mesh=Mesh'Unreal.BigFlash'
     CollisionHeight=12.000000
     LightHue=167
     LightRadius=13
}
