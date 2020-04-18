//=============================================================================
// RifleRound.
//=============================================================================
class RifleRound expands RifleAmmo;

#exec MESH IMPORT MESH=RifleRoundM ANIVFILE=MODELS\rifles_a.3D DATAFILE=MODELS\rifles_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=RifleRoundM X=0 Y=-200 Z=0 YAW=0
#exec MESH SEQUENCE MESH=RifleRoundM SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=RifleR1 FILE=MODELS\rifles.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=RifleRoundM X=0.02 Y=0.02 Z=0.04
#exec MESHMAP SETTEXTURE MESHMAP=RifleRoundM NUM=1 TEXTURE=RifleR1

defaultproperties
{
     AmmoAmount=1
     ParentAmmo=Unreal.RifleAmmo
     PickupMessage="You got a Rifle Round."
     PickupViewMesh=Unreal.RifleRoundM
     Mesh=Unreal.RifleRoundM
     CollisionRadius=+00005.000000
     CollisionHeight=+00015.000000
}
