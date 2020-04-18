//=============================================================================
// ASMDAmmo.
//=============================================================================
class ASMDAmmo expands Ammo;

#exec AUDIO IMPORT FILE="Sounds\Pickups\AMMOPUP1.WAV" NAME="AmmoSnd" GROUP="Pickups"
#exec AUDIO IMPORT FILE="Sounds\general\steam4.WAV" NAME="Steam" GROUP="Pickups"

#exec TEXTURE IMPORT NAME=I_ASMD FILE=TEXTURES\HUD\i_asmd.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=AsmdAmmoM ANIVFILE=MODELS\asmdam_a.3D DATAFILE=MODELS\asmdam_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=AsmdAmmoM X=0 Y=0 Z=0 YAW=0
#exec MESH SEQUENCE MESH=AsmdAmmoM SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JAsmdAmmo1 FILE=MODELS\asmdammo.PCX GROUP="Skins"
#exec OBJ LOAD FILE=Textures\fireeffectASMD.utx  PACKAGE=Unreal.EffectASMD
#exec MESHMAP SCALE MESHMAP=AsmdAmmoM X=0.045 Y=0.045 Z=0.09
#exec MESHMAP SETTEXTURE MESHMAP=AsmdAmmoM NUM=1 TEXTURE=JAsmdammo1
#exec MESHMAP SETTEXTURE MESHMAP=AsmdAmmoM NUM=0 TEXTURE=Unreal.EffectASMD.FireEffectASMD
#exec MESHMAP SETTEXTURE MESHMAP=AsmdAmmoM NUM=2 TEXTURE=Unreal.EffectASMD.ASMDSMoke

defaultproperties
{
     AmmoAmount=10
     MaxAmmo=50
     UsedInWeaponSlot(4)=1
     PickupMessage="You picked up an ASMD core."
     PickupViewMesh=Mesh'Unreal.AsmdAmmoM'
     PickupSound=Sound'Unreal.Pickups.AmmoSnd'
     Icon=Texture'Unreal.Icons.I_ASMD'
     Mesh=Mesh'Unreal.AsmdAmmoM'
     bMeshCurvy=False
     SoundRadius=26
     SoundVolume=37
     SoundPitch=73
     CollisionRadius=10.000000
     CollisionHeight=20.000000
     bCollideActors=True
}
