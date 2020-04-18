//=============================================================================
// ToxinSuit.
//=============================================================================
class ToxinSuit expands Suits;

#exec AUDIO IMPORT FILE="Sounds\Pickups\ARMOUR2.WAV"  NAME="ArmorSnd"      GROUP="Pickups"

#exec MESH IMPORT MESH=ToxSuit ANIVFILE=MODELS\Suit_a.3D DATAFILE=MODELS\Suit_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=ToxSuit X=0 Y=100 Z=40 YAW=64 ROLL=64
#exec MESH SEQUENCE MESH=ToxSuit SEQ=All STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=AToxSuit1 FILE=MODELS\bToxSuit.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=ToxSuit X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=ToxSuit NUM=1 TEXTURE=AToxSuit1

defaultproperties
{
     PickupMessage="You picked up the Toxin Suit"
     PickupViewMesh=Unreal.ToxSuit
     ProtectionType1=Corroded
     Charge=50
     ArmorAbsorption=50
     bIsAnArmor=True
     AbsorptionPriority=6
     PickupSound=Unreal.SuitSnd
     DrawType=DT_Mesh
     Mesh=Unreal.ToxSuit
     CollisionRadius=+00030.000000
     CollisionHeight=+00030.000000
}
