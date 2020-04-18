//=============================================================================
// StingerAmmo.
//=============================================================================
class StingerAmmo expands Ammo;

#exec AUDIO IMPORT FILE="Sounds\Pickups\AMMOPUP1.WAV" NAME="AmmoSnd"       GROUP="Pickups"

#exec TEXTURE IMPORT NAME=I_StingerAmmo FILE=TEXTURES\HUD\i_sting.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=TarydiumPickup ANIVFILE=MODELS\aniv54.3D DATAFILE=MODELS\data54.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=TarydiumPickup X=0 Y=0 Z=80 YAW=0
#exec MESH SEQUENCE MESH=TarydiumPickup SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JTaryPick1 FILE=MODELS\Shells.PCX GROUP="Skins" FLAGS=2
#exec MESHMAP SCALE MESHMAP=TarydiumPickup X=0.02 Y=0.02 Z=0.04
#exec MESHMAP SETTEXTURE MESHMAP=TarydiumPickup NUM=4 TEXTURE=JTaryPick1

defaultproperties
{
     AmmoAmount=40
     MaxAmmo=200
     UsedInWeaponSlot(3)=1
     PickupMessage="You picked up 40 Tarydium Shards"
     PickupViewMesh=Mesh'Unreal.TarydiumPickup'
     PickupSound=Sound'Unreal.Pickups.AmmoSnd'
     Icon=Texture'Unreal.Icons.I_StingerAmmo'
     Mesh=Mesh'Unreal.TarydiumPickup'
     bMeshCurvy=False
     CollisionRadius=22.000000
     CollisionHeight=6.000000
     bCollideActors=True
}
