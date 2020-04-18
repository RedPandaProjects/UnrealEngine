//=============================================================================
// flashlight
//=============================================================================
class Flashlight expands Pickup;

#exec AUDIO IMPORT FILE="Sounds\Pickups\FSHLITE1.WAV" NAME="FSHLITE1"   GROUP="Pickups"
#exec AUDIO IMPORT FILE="Sounds\Pickups\fshlite2.WAV" NAME="FSHLITE2"   GROUP="Pickups"
 
#exec TEXTURE IMPORT NAME=I_Flashlight FILE=TEXTURES\HUD\i_flash.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=Flashl ANIVFILE=MODELS\flashl_a.3D DATAFILE=MODELS\flashl_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Flashl X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=flashl SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=flashl SEQ=Still  STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JFlashl1 FILE=MODELS\flashl.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=Flashl X=0.02 Y=0.02 Z=0.04
#exec MESHMAP SETTEXTURE MESHMAP=flashl NUM=1 TEXTURE=Jflashl1

var FlashLightBeam s;
var float TimeChange;
var Vector HitNormal,HitLocation,EndTrace,StartTrace,X,Y,Z,NewHitLocation;

state Activated
{
	function endstate()
	{
		if (s!=None) s.Destroy();
		bActive = false;		
	}
	
	function Tick( float DeltaTime )
	{
		TimeChange += DeltaTime*10;
		if (TimeChange > 1) {
			if ( s == None )
			{
				UsedUp();
				return;
			}		
			Charge -= int(TimeChange);
			TimeChange = TimeChange - int(TimeChange);
		}
		
		if (s == None) Return;

		if ( Pawn(Owner) == None )
		{
			s.Destroy();
			UsedUp();
			return;		
		}
		if (Charge<-0) {
			s.Destroy();
			Pawn(Owner).ClientMessage(ExpireMessage);		
			UsedUp();		
		}
		
		if (Charge<400) s.LightBrightness=byte(Charge*0.6+10);

		GetAxes(Pawn(Owner).ViewRotation,X,Y,Z);	
		EndTrace = Pawn(Owner).Location + 10000* Vector(Pawn(Owner).ViewRotation);
		Trace(HitLocation,HitNormal,EndTrace,Pawn(Owner).Location, True);
		s.SetLocation(HitLocation-vector(Pawn(Owner).ViewRotation)*64);
//		s.LightRadius = fmin(Vsize(HitLocation-Pawn(Owner).Location)/200,14) + 2.0;
	}
	
	function BeginState()
	{
		TimeChange = 0;
		Owner.PlaySound(ActivateSound);		
		GetAxes(Pawn(Owner).ViewRotation,X,Y,Z);	
		EndTrace = Pawn(Owner).Location + 10000* Vector(Pawn(Owner).ViewRotation);
		Trace(HitLocation,HitNormal,EndTrace,Pawn(Owner).Location+Y*17);
		s = Spawn(class'FlashLightBeam',Owner, '', HitLocation+HitNormal*40);
		s.LightHue = LightHue;
		s.LightRadius = LightRadius;		
		if (Charge<400) s.LightBrightness=byte(Charge*0.6+10);	
		if (s==None) GoToState('DeActivated');	
	}
	
Begin:
}

state DeActivated
{
Begin:
	s.Destroy();
	Owner.PlaySound(DeActivateSound);
}

defaultproperties
{
     ExpireMessage="Flashlight batteries have died."
     bActivatable=True
     bDisplayableInv=True
     PickupMessage="You picked up the flashlight"
     RespawnTime=40.000000
     PickupViewMesh=Mesh'Unreal.Flashl'
     Charge=800
     PickupSound=Sound'Unreal.Pickups.GenPickSnd'
     ActivateSound=Sound'Unreal.Pickups.FSHLITE1'
     DeActivateSound=Sound'Unreal.Pickups.FSHLITE2'
     Icon=Texture'Unreal.Icons.I_Flashlight'
     RemoteRole=ROLE_DumbProxy
     Mesh=Mesh'Unreal.Flashl'
     AmbientGlow=96
     bMeshCurvy=False
     CollisionRadius=22.000000
     CollisionHeight=4.000000
     LightBrightness=100
     LightHue=33
     LightSaturation=187
     LightRadius=7
}
