//=============================================================================
// Amplifier.
//=============================================================================
class Amplifier expands Pickup;

#exec AUDIO IMPORT FILE="Sounds\Pickups\HEALTH1.WAV" NAME="HEALTH1" GROUP="Pickups"
#exec AUDIO IMPORT FILE="Sounds\Pickups\ampl1.WAV" NAME="AmpAct" GROUP="Pickups"

#exec TEXTURE IMPORT NAME=I_Amp FILE=TEXTURES\HUD\i_amp.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=AmplifierM ANIVFILE=MODELS\amp_a.3D DATAFILE=MODELS\amp_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=AmplifierM X=0 Y=0 Z=50 YAW=0
#exec MESH SEQUENCE MESH=AmplifierM SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JAmplifier1 FILE=MODELS\Amp.PCX GROUP="Skins" 
#exec MESHMAP SCALE MESHMAP=AmplifierM X=0.08 Y=0.08 Z=0.16
#exec MESHMAP SETTEXTURE MESHMAP=AmplifierM NUM=1 TEXTURE=JAmplifier1

var() float AmpMultiplier;
var() sound AmpSound;

event TravelPreAccept()
{
	local inventory w;

	Super.TravelPreAccept();
	w = Pawn(Owner).FindInventoryType(class'DispersionPistol');
	if ( w != None )
		DispersionPistol(w).Amp = self;
	w = Pawn(Owner).FindInventoryType(class'ASMD');
	if ( w != None )
		ASMD(w).Amp = self;
}


function inventory SpawnCopy( pawn Other )
{
	local inventory Copy;
	local Inventory I;

	Copy = Super.SpawnCopy(Other);
	I = Other.FindInventoryType(class'DispersionPistol');
	if ( DispersionPistol(I) != None )
		DispersionPistol(I).amp = Amplifier(Copy);

	I = Other.FindInventoryType(class'ASMD');
	if ( ASMD(I) != None )
		ASMD(I).amp = Amplifier(Copy);

	return Copy;
}

function float UseCharge(float Amount)
{
	Return 1.0;
}

function UsedUp()
{
	local Inventory I;

	I = Pawn(Owner).FindInventoryType(class'DispersionPistol');
	if ( DispersionPistol(I) != None )
		DispersionPistol(I).amp = None;

	I = Pawn(Owner).FindInventoryType(class'ASMD');
	if ( ASMD(I) != None )
		ASMD(I).amp = None;

	Super.UsedUp();
}

state Activated
{

	function float UseCharge(float Amount)
	{
		local float TempCharge;
		if (AmpMultiplier<1.0) AmpMultiplier=1.0;
	
		if (Charge < Amount) {
			TempCharge = Charge;
			Charge=0;
			Return (AmpMultiplier-1.0)*TempCharge/Amount+1.0;
		}
		Charge = Charge - Amount;
		return AmpMultiplier;
	}


	function Timer()
	{
		Charge -= 2;
		if (Charge<=0) 
		{
			UsedUp();		
		}
	}

	function EndState()
	{
		if ( Owner.IsA('PlayerPawn') )
			PlayerPawn(Owner).ClientAdjustGlow(0.1,vect(-100,-20,0));
		Owner.AmbientSound=None;
		Owner.LightType=LT_None;
		Owner.AmbientGlow=0;		
		bActive = false;		
	}
Begin:
	SetTimer(1.0,True);
	PlaySound(ActivateSound);
	Owner.AmbientSound=AmpSound;
	if ( Owner.IsA('PlayerPawn') )
		PlayerPawn(Owner).ClientAdjustGlow(-0.1,vect(100,20,0));
	Owner.LightType=LT_Steady;	
	Owner.LightRadius=6;
	Owner.LightEffect=LE_NonIncidence;
	Owner.LightSaturation=40;
	Owner.LightHue=6;
	Owner.LightBrightness=255;
	Owner.AmbientGlow=255;
}

state DeActivated
{
Begin:

}

defaultproperties
{
     AmpMultiplier=4.000000
     AmpSound=Sound'Unreal.Pickups.AmpAct'
     bCanActivate=True
     ExpireMessage="Amplifier is out of power."
     bAutoActivate=True
     bActivatable=True
     bDisplayableInv=True
     PickupMessage="You got the Energy Amplifier"
     RespawnTime=90.000000
     PickupViewMesh=Mesh'Unreal.AmplifierM'
     Charge=1000
     MaxDesireability=1.200000
     PickupSound=Sound'Unreal.Pickups.GenPickSnd'
     ActivateSound=Sound'Unreal.Pickups.HEALTH1'
     Icon=Texture'Unreal.Icons.I_Amp'
     Mesh=Mesh'Unreal.AmplifierM'
     bMeshCurvy=False
     CollisionRadius=20.000000
     CollisionHeight=13.500000
}
