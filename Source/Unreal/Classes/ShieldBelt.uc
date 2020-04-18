//=============================================================================
// ShieldBelt.
//=============================================================================
class ShieldBelt expands Pickup;

#exec AUDIO IMPORT FILE="Sounds\Pickups\SBELTA1.WAV"  NAME="BeltSnd"       GROUP="Pickups"
#exec AUDIO IMPORT FILE="Sounds\Pickups\pSBELTA2.WAV"  NAME="PSbelta2"       GROUP="Pickups"
#exec AUDIO IMPORT FILE="Sounds\Pickups\SBELThe2.WAV"  NAME="Sbelthe2"       GROUP="Pickups"

#exec TEXTURE IMPORT NAME=I_ShieldBelt FILE=TEXTURES\HUD\i_belt.PCX GROUP="Icons" MIPS=OFF
#exec TEXTURE IMPORT NAME=GoldSkin FILE=models\gold.PCX GROUP="None"

#exec MESH IMPORT MESH=ShieldBeltMesh ANIVFILE=MODELS\belt_a.3D DATAFILE=MODELS\belt_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=ShieldBeltMesh X=0 Y=120 Z=110 YAW=64
#exec MESH SEQUENCE MESH=ShieldBeltMesh SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Abelt1 FILE=MODELS\belt.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=ShieldBeltMesh X=0.025 Y=0.025 Z=0.05
#exec MESHMAP SETTEXTURE MESHMAP=ShieldBeltMesh NUM=1 TEXTURE=Abelt1

function Timer()
{
}

function ArmorImpactEffect(vector HitLocation)
{ 
	if ( Owner.IsA('PlayerPawn') )
	{
		PlayerPawn(Owner).ClientFlash(-0.05,vect(400,400,400));
		PlayerPawn(Owner).PlaySound(DeActivateSound, SLOT_None, 2.7*PlayerPawn(Owner).SoundDampening);
	}
}


function PickupFunction(Pawn Other)
{
	Other.Texture = texture'GoldSkin';
	Other.bMeshEnviroMap = True;
	SetTimer(1.0,True);
}

function Destroyed()
{
	if ( Owner != None )
		Owner.bMeshEnviroMap = False;
	if (MyMarker != None )
		MyMarker.markedItem = None;		
	// Remove from owner's inventory.
	if( Pawn(Owner)!=None )
		Pawn(Owner).DeleteInventory( Self );
}

defaultproperties
{
     bDisplayableInv=True
     PickupMessage="You got the Shield Belt"
     RespawnTime=60.000000
     PickupViewMesh=Mesh'Unreal.ShieldBeltMesh'
     ProtectionType1=ProtectNone
     ProtectionType2=ProtectNone
     Charge=100
     ArmorAbsorption=100
     bIsAnArmor=True
     AbsorptionPriority=10
     MaxDesireability=1.800000
     PickupSound=Sound'Unreal.Pickups.BeltSnd'
     DeActivateSound=Sound'Unreal.Pickups.Sbelthe2'
     Icon=Texture'Unreal.Icons.I_ShieldBelt'
     RemoteRole=ROLE_DumbProxy
     Mesh=Mesh'Unreal.ShieldBeltMesh'
     AmbientGlow=64
     bMeshCurvy=False
     CollisionRadius=20.000000
     CollisionHeight=5.000000
}
