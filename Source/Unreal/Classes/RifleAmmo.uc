//=============================================================================
// RifleAmmo.
//=============================================================================
class RifleAmmo expands Ammo;

#exec AUDIO IMPORT FILE="Sounds\Pickups\AMMOPUP1.WAV" NAME="AmmoSnd"       GROUP="Pickups"

#exec TEXTURE IMPORT NAME=I_RIFLEAmmo FILE=TEXTURES\HUD\i_RIFLE.PCX GROUP="Icons" MIPS=OFF
 
#exec MESH IMPORT MESH=RifleBullets ANIVFILE=MODELS\rifleb_a.3D DATAFILE=MODELS\rifleb_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=RifleBullets X=0 Y=-200 Z=0 YAW=0
#exec MESH SEQUENCE MESH=RifleBullets SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=RifleBul1 FILE=MODELS\rifleb.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=RifleBullets X=0.025 Y=0.025 Z=0.05
#exec MESHMAP SETTEXTURE MESHMAP=RifleBullets NUM=1 TEXTURE=RifleBul1

defaultproperties
{
     AmmoAmount=8
     MaxAmmo=50
     UsedInWeaponSlot(9)=1
     PickupMessage="You got 8 Rifle rounds."
     PickupViewMesh=Mesh'Unreal.RifleBullets'
     MaxDesireability=0.240000
     PickupSound=Sound'Unreal.Pickups.AmmoSnd'
     Icon=Texture'Unreal.Icons.I_RIFLEAmmo'
     Mesh=Mesh'Unreal.RifleBullets'
     bMeshCurvy=False
     CollisionRadius=15.000000
     CollisionHeight=20.000000
     bCollideActors=True
}
