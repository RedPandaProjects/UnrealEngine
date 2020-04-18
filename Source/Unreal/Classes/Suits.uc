//=============================================================================
// Suits.
//=============================================================================
class Suits expands Pickup;

#exec TEXTURE IMPORT NAME=I_Suit FILE=TEXTURES\HUD\i_Suit.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=Suit ANIVFILE=MODELS\bsuit_a.3D DATAFILE=MODELS\bsuit_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Suit X=0 Y=100 Z=40 YAW=64 ROLL=64
#exec MESH SEQUENCE MESH=Suit SEQ=All STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Asuit1 FILE=MODELS\bsuit.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=Suit X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=Suit NUM=1 TEXTURE=Asuit1

function PickupFunction(Pawn Other)
{
	local inventory inv2;
	if (Other.Inventory==None) Return;
	
	for( Inv2=Other.Inventory; Inv2!=None; Inv2=Inv2.Inventory ) 
		if (Suits(Inv2)!=None && Inv2.class!=class) Inv2.Destroy();
}

defaultproperties
{
     bDisplayableInv=True
     RespawnTime=30.000000
     PickupViewMesh=Mesh'Unreal.Suit'
     ProtectionType1=ProtectNone
     ProtectionType2=ProtectNone
     Icon=Texture'Unreal.Icons.I_Suit'
     RemoteRole=ROLE_DumbProxy
     DrawType=DT_None
     AmbientGlow=64
     bMeshCurvy=False
     CollisionRadius=26.000000
     CollisionHeight=39.000000
}
