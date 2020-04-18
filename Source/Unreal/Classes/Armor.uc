//=============================================================================
// Armor powerup.
//=============================================================================
class Armor expands Pickup;

#exec AUDIO IMPORT FILE="Sounds\Pickups\ARMOUR2.WAV" NAME="ArmorSnd" GROUP="Pickups"

#exec TEXTURE IMPORT NAME=I_Armor FILE=TEXTURES\HUD\i_armor.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=ArmorM ANIVFILE=MODELS\aniv36.3D DATAFILE=MODELS\data36.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=ArmorM X=0 Y=0 Z=0 YAW=0
#exec MESH SEQUENCE MESH=ArmorM SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jarmor1 FILE=MODELS\armor.PCX GROUP="Skins" FLAGS=2
#exec MESHMAP SCALE MESHMAP=ArmorM X=0.035 Y=0.035 Z=0.07
#exec MESHMAP SETTEXTURE MESHMAP=ArmorM NUM=7 TEXTURE=Jarmor1

defaultproperties
{
     bDisplayableInv=True
     PickupMessage="You got the Assault Vest"
     RespawnTime=30.000000
     PickupViewMesh=Mesh'Unreal.ArmorM'
     Charge=100
     ArmorAbsorption=90
     bIsAnArmor=True
     AbsorptionPriority=7
     MaxDesireability=1.400000
     PickupSound=Sound'Unreal.Pickups.ArmorSnd'
     Icon=Texture'Unreal.Icons.I_Armor'
     Mesh=Mesh'Unreal.ArmorM'
     AmbientGlow=64
     CollisionHeight=11.000000
}
