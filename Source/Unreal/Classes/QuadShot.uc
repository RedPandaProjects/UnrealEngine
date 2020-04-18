//=============================================================================
// QuadShot.
//=============================================================================
class QuadShot expands Weapon;

#exec MESH IMPORT MESH=QuadShotPickup ANIVFILE=MODELS\aniv25.3D DATAFILE=MODELS\data25.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=QuadShotPickup X=0 Y=0 Z=0 YAW=0
#exec MESH SEQUENCE MESH=QuadShotPickup SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=GunPick1 FILE=MODELS\quadhand.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=QuadShotPickup X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=QuadShotPickup NUM=4 TEXTURE=GunPick1 

#exec MESH IMPORT MESH=QuadShotHeld ANIVFILE=MODELS\aniv20.3D DATAFILE=MODELS\data20.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=QuadShotHeld X=0 Y=0 Z=0 YAW=128
#exec MESH SEQUENCE MESH=QuadShotHeld SEQ=All  STARTFRAME=0  NUMFRAMES=30
#exec MESH SEQUENCE MESH=QuadShotHeld SEQ=Fire STARTFRAME=0  NUMFRAMES=9
#exec MESH SEQUENCE MESH=QuadShotHeld SEQ=Reload STARTFRAME=8  NUMFRAMES=21
#exec TEXTURE IMPORT NAME=QuadHand1 FILE=MODELS\QUADHAND.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=QuadShotHeld X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=QuadShotHeld NUM=4 TEXTURE=QuadHand1

defaultproperties
{
     PickupAmmoCount=20
     AutoSwitchPriority=2
     InventoryGroup=3
     PickupMessage="Grabbed the QuadShot"
     PlayerViewOffset=(X=5.000000,Y=-1.000000,Z=-3.000000)
     PlayerViewMesh=Unreal.QuadShotHeld
     PlayerViewScale=+00001.590000
     PickupViewMesh=Unreal.QuadShotPickup
     Mesh=Unreal.QuadShotPickup
     bNoSmooth=False
     bMeshCurvy=False
     Mass=+00010.000000
}
