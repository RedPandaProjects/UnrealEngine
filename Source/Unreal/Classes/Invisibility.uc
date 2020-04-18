//=============================================================================
// Invisibility.
//=============================================================================
class Invisibility expands PickUp;

#exec TEXTURE IMPORT NAME=I_Invisibility FILE=TEXTURES\HUD\i_Invis.PCX GROUP="Icons" MIPS=OFF

#exec AUDIO IMPORT FILE="Sounds\Pickups\Scloak1.WAV" NAME="Invisible" GROUP="Pickups"

#exec MESH IMPORT MESH=InvisibilityMesh ANIVFILE=MODELS\Invis_a.3D DATAFILE=MODELS\Invis_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=InvisibilityMesh X=0 Y=0 Z=-15
#exec MESH SEQUENCE MESH=InvisibilityMesh SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=InvisibilityMesh SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JInvisibility1 FILE=MODELS\Invis.PCX GROUP=Skins 
#exec OBJ LOAD FILE=Textures\fireeffect26.utx  PACKAGE=Unreal.Effect26
#exec MESHMAP SCALE MESHMAP=InvisibilityMesh X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=InvisibilityMesh NUM=1 TEXTURE=JInvisibility1
#exec MESHMAP SETTEXTURE MESHMAP=InvisibilityMesh NUM=0 TEXTURE=Unreal.Effect26.FireEffect26

var byte TempVis;


function Invisibility (bool Vis)
{ 
	if (Pawn(Owner)==None) Return;

	if( Vis )
	{
		PlaySound(ActivateSound,,4.0);
		if ( PlayerPawn(Owner) != None )		
			PlayerPawn(Owner).ClientAdjustGlow(-0.15, vect(156.25,156.25,351.625));
		Pawn(Owner).Visibility = 10;
		Pawn(Owner).bHidden=True;
		if ( Pawn(Owner).Weapon != None )
			Pawn(Owner).Weapon.bOnlyOwnerSee=False;			
	}
	else
	{
		PlaySound(DeActivateSound);
		if ( PlayerPawn(Owner) != None )		
			PlayerPawn(Owner).ClientAdjustGlow(0.15, vect(-156.25,-156.25,-351.625));	
		Pawn(Owner).Visibility = Pawn(Owner).Default.Visibility;
		if ( Pawn(Owner).health > 0 )
			Pawn(Owner).bHidden=False;
		Pawn(Owner).Weapon.bOnlyOwnerSee=True;
	}
}

state Activated
{
	function endstate()
	{
		Invisibility(False);
		bActive = false;		
	}

	function Timer()
	{
		Charge -= 1;
		Owner.bHidden=True;		
		Pawn(Owner).Visibility = 10;		
		if (Charge<-0) 
		{	
			Pawn(Owner).ClientMessage(ExpireMessage);	
			UsedUp();
		}
	}

	function BeginState()
	{
		Invisibility(True);
		SetTimer(0.5,True);
	}
}

state DeActivated
{
Begin:
}

defaultproperties
{
     ExpireMessage="Invisibility has worn off."
     bAutoActivate=True
     bActivatable=True
     bDisplayableInv=True
     PickupMessage="You have Invisibility"
     RespawnTime=100.000000
     PickupViewMesh=Mesh'Unreal.InvisibilityMesh'
     Charge=100
     MaxDesireability=1.200000
     PickupSound=Sound'Unreal.Pickups.GenPickSnd'
     ActivateSound=Sound'Unreal.Pickups.Invisible'
     Icon=Texture'Unreal.Icons.I_Invisibility'
     RemoteRole=ROLE_DumbProxy
     Mesh=Mesh'Unreal.InvisibilityMesh'
     AmbientGlow=96
     bMeshCurvy=False
     CollisionRadius=15.000000
     CollisionHeight=17.000000
}
