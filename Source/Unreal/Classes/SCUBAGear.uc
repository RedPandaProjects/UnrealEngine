//=============================================================================
// SCUBAGear.
//=============================================================================
class SCUBAGear expands Pickup;

#exec AUDIO IMPORT FILE="Sounds\Pickups\scubada1.WAV" NAME="Scubada1" GROUP="Pickups"
#exec AUDIO IMPORT FILE="Sounds\Pickups\scubal1.WAV" NAME="Scubal1" GROUP="Pickups"
#exec AUDIO IMPORT FILE="Sounds\Pickups\scubal2.WAV" NAME="Scubal2" GROUP="Pickups"

#exec TEXTURE IMPORT NAME=I_Scuba FILE=TEXTURES\HUD\i_scuba.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=Scuba ANIVFILE=MODELS\Scuba_a.3D DATAFILE=MODELS\Scuba_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Scuba X=0 Y=-400 Z=-100 YAW=64
#exec MESH SEQUENCE MESH=Scuba SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=ASC1 FILE=MODELS\scuba.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=Scuba X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=Scuba NUM=1 TEXTURE=ASC1

var vector X,Y,Z;	

function inventory PrioritizeArmor( int Damage, name DamageType, vector HitLocation )
{
	if (DamageType == 'Breathe') 
	{
		if (Pawn(Owner)!=None && IsInState('activated')
			&& !Pawn(Owner).FootRegion.Zone.bPainZone) Pawn(Owner).PainTime=12;
		GotoState('Deactivated');
	}
	else if (DamageType == 'Drowned' && Damage==0) 
		GoToState('Activated');
	if (DamageType == 'Drowned')  Return Self;
	Super.PrioritizeArmor(Damage, DamageType, HitLocation);
}

function UsedUp()
{
	local Pawn OwnerPawn;

	OwnerPawn = Pawn(Owner);
	if ( (OwnerPawn != None) && !OwnerPawn.FootRegion.Zone.bPainZone && OwnerPawn.HeadRegion.Zone.bWaterZone )
		OwnerPawn.PainTime = 15;
	Owner.AmbientSound = None;
	Super.UsedUp();
}
	
state Activated
{
	function endstate()
	{

		Owner.PlaySound(DeactivateSound);
		Owner.AmbientSound = None;		
		bActive = false;		
	}

	function Timer()
	{
		local float LocalTime;
		local Bubble1 b;

		if ( Pawn(Owner) == None )
		{
			UsedUp();
			return;
		}
		Charge -= 1;
		if (Charge<-0) {
			Pawn(Owner).ClientMessage(ExpireMessage);			
			UsedUp();	
		}
		LocalTime += 0.1;
		LocalTime = LocalTime - int(LocalTime);
		if ( Pawn(Owner).HeadRegion.Zone.bWaterZone && !Pawn(Owner).FootRegion.Zone.bPainZone ) 
		{
			Pawn(Owner).PainTime = 1;
			if (FRand()<LocalTime) {
				GetAxes(Pawn(Owner).ViewRotation,X,Y,Z);	
				b = Spawn(class'Bubble1', Owner, '', Pawn(Owner).Location
					+ 20.0 * X - (FRand()*6+5) * Y - (FRand()*6+5) * Z );
				if ( b != None )
					b.DrawScale = FRand()*0.1+0.05;
			}
			if (FRand()<LocalTime) {
				GetAxes(Pawn(Owner).ViewRotation,X,Y,Z);	
				b = Spawn(class'Bubble1', Owner, '', Pawn(Owner).Location
					+ 20.0 * X + (FRand()*6+5) * Y - (FRand()*6+5) * Z );
				if ( b != None )
					b.DrawScale = FRand()*0.1+0.05;
			}
		}
	}
Begin:
	SetTimer(0.1,True);
	if (PlayerPawn(Owner).HeadRegion.Zone.bWaterZone) Owner.AmbientSound = ActivateSound;
	else Owner.AmbientSound = RespawnSound;		
}

state DeActivated
{
Begin:

}

defaultproperties
{
     bActivatable=True
     bDisplayableInv=True
     PickupMessage="You picked up the SCUBA gear"
     RespawnTime=20.000000
     PickupViewMesh=Mesh'Unreal.Scuba'
     Charge=1200
     PickupSound=Sound'Unreal.Pickups.GenPickSnd'
     ActivateSound=Sound'Unreal.Pickups.Scubal1'
     DeActivateSound=Sound'Unreal.Pickups.Scubada1'
     RespawnSound=Sound'Unreal.Pickups.Scubal2'
     Icon=Texture'Unreal.Icons.I_Scuba'
     RemoteRole=ROLE_DumbProxy
     Mesh=Mesh'Unreal.Scuba'
     bMeshCurvy=False
     CollisionRadius=18.000000
     CollisionHeight=15.000000
}
