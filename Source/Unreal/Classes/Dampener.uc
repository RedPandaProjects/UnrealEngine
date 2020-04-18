//=============================================================================
// Dampener.
//=============================================================================
class Dampener expands PickUp;

#exec AUDIO IMPORT FILE="Sounds\Pickups\Dampndea.WAV" NAME="dampndea"       GROUP="Pickups"
#exec AUDIO IMPORT FILE="Sounds\Pickups\DAMPNER1.WAV" NAME="DampSnd"       GROUP="Pickups"

#exec TEXTURE IMPORT NAME=I_Dampener FILE=TEXTURES\HUD\i_Damp.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=DampenerM ANIVFILE=MODELS\acoust_a.3D DATAFILE=MODELS\acoust_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=DampenerM X=0 Y=0 Z=-50 YAW=64
#exec MESH SEQUENCE MESH=DampenerM SEQ=All STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Ainv1 FILE=MODELS\inv.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=DampenerM X=0.03 Y=0.03 Z=0.06
#exec MESHMAP SETTEXTURE MESHMAP=DampenerM NUM=1 TEXTURE=Ainv1

//
// Player has activated the item.
//
// Count down the charge, turn acoustic damping on.  If charge runs out, remove from 
// inventory and destroy, resetting sound to normal
//
state Activated
{
	function Timer()
	{
		Charge -= 1;
		if (Charge<=0) {
			if ( Owner != None )
			{
				Owner.PlaySound(DeActivateSound);
				if ( Owner.IsA('Pawn') )
					Pawn(Owner).SoundDampening = 1.0;
			}
			UsedUp();		
		}
	}
	function EndState()
	{
		if ( Owner.IsA('Pawn') )
			Pawn(Owner).SoundDampening = 1.0;
		bActive = false;		
	}
Begin:
	SetTimer(0.1,True);
	Owner.PlaySound(ActivateSound);	
	Pawn(Owner).SoundDampening = 0.1;	
}

state DeActivated
{
Begin:
	if ( Owner != None )
	{
		Owner.PlaySound(DeActivateSound);
		if ( Owner.IsA('Pawn') )
			Pawn(Owner).SoundDampening = 1.0;
	}
}

defaultproperties
{
     ExpireMessage="Acoustic dampener has run out."
     bAutoActivate=True
     bActivatable=True
     bDisplayableInv=True
     PickupMessage="You got the Acoustic Dampener"
     RespawnTime=30.000000
     PickupViewMesh=Mesh'Unreal.DampenerM'
     Charge=200
     PickupSound=Sound'Unreal.Pickups.GenPickSnd'
     ActivateSound=Sound'Unreal.Pickups.dampndea'
     DeActivateSound=Sound'Unreal.Pickups.DampSnd'
     Icon=Texture'Unreal.Icons.I_Dampener'
     RemoteRole=ROLE_DumbProxy
     Mesh=Mesh'Unreal.DampenerM'
     AmbientGlow=69
     bMeshCurvy=False
     CollisionRadius=15.000000
     CollisionHeight=10.000000
}
