//=============================================================================
// Shellbox.
//=============================================================================
class Shellbox expands Ammo;

#exec AUDIO IMPORT FILE="Sounds\Pickups\AMMOPUP1.WAV" NAME="AmmoSnd"       GROUP="Pickups"

#exec TEXTURE IMPORT NAME=I_ShellAmmo FILE=TEXTURES\HUD\i_shell.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=ShellBoxMesh ANIVFILE=MODELS\shelbx_a.3D DATAFILE=MODELS\shelbx_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=ShellBoxMesh X=-50 Y=-40 Z=0 YAW=0
#exec MESH SEQUENCE MESH=ShellBoxMesh SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JPickup21 FILE=MODELS\pickup2.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=ShellBoxMesh X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=ShellBoxMesh NUM=1 TEXTURE=JPickup21

defaultproperties
{
     AmmoAmount=50
     MaxAmmo=200
     UsedInWeaponSlot(0)=1
     UsedInWeaponSlot(2)=1
     PickupMessage="You picked up 50 bullets"
     PickupViewMesh=Mesh'Unreal.ShellBoxMesh'
     PickupSound=Sound'Unreal.Pickups.AmmoSnd'
     Icon=Texture'Unreal.Icons.I_ShellAmmo'
     Mesh=Mesh'Unreal.ShellBoxMesh'
     bMeshCurvy=False
     CollisionRadius=22.000000
     CollisionHeight=11.000000
     bCollideActors=True
}
