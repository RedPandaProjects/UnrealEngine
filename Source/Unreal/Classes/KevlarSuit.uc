//=============================================================================
// KevlarSuit.
//=============================================================================
class KevlarSuit expands Suits;

#exec TEXTURE IMPORT NAME=I_kevlar FILE=TEXTURES\HUD\i_kevlar.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=KevSuit ANIVFILE=MODELS\kevlar_a.3D DATAFILE=MODELS\kevlar_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=KevSuit X=0 Y=0 Z=0 YAW=64 ROLL=0
#exec MESH SEQUENCE MESH=KevSuit SEQ=All STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=AkevSuit1 FILE=MODELS\Kevlar.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=KevSuit X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=KevSuit NUM=1 TEXTURE=AkevSuit1

defaultproperties
{
     PickupMessage="You picked up the Kevlar Suit"
     PickupViewMesh=Mesh'Unreal.KevSuit'
     Charge=100
     ArmorAbsorption=80
     bIsAnArmor=True
     AbsorptionPriority=6
     PickupSound=Sound'Unreal.Pickups.SuitSnd'
     Icon=Texture'Unreal.Icons.I_kevlar'
     DrawType=DT_Mesh
     Mesh=Mesh'Unreal.KevSuit'
     CollisionRadius=20.000000
     CollisionHeight=30.000000
}
