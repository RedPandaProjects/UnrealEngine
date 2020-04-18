//=============================================================================
// VoiceBox.
//=============================================================================
class VoiceBox expands Pickup;

#exec TEXTURE IMPORT NAME=I_VoiceBox FILE=TEXTURES\HUD\i_voice.PCX GROUP="Icons" MIPS=OFF

#exec AUDIO IMPORT FILE="Sounds\Pickups\VOICEB1.WAV" NAME="VoiceSnd" GROUP="Pickups"
#exec AUDIO IMPORT FILE="Sounds\automag\shot.WAV" NAME="shot" GROUP="AutoMag"
#exec AUDIO IMPORT FILE="Sounds\stinger\sshot10d.WAV" NAME="StingerFire" GROUP="Stinger"
#exec AUDIO IMPORT FILE="Sounds\flak\shot1.WAV" NAME="shot1" GROUP="flak"
#exec AUDIO IMPORT FILE="Sounds\flak\Explode1.WAV" NAME="Explode1" GROUP="flak"
#exec AUDIO IMPORT FILE="Sounds\flak\expl2.WAV" NAME="expl2" GROUP="flak"

#exec MESH IMPORT MESH=VoiceBoxMesh ANIVFILE=MODELS\voice_a.3D DATAFILE=MODELS\voice_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=VoiceBoxMesh X=0 Y=0 Z=-15 YAW=64
#exec MESH SEQUENCE MESH=VoiceBoxMesh SEQ=All   STARTFRAME=0  NUMFRAMES=10
#exec MESH SEQUENCE MESH=VoiceBoxMesh SEQ=Pulse STARTFRAME=0  NUMFRAMES=10
#exec TEXTURE IMPORT NAME=Ainv1 FILE=MODELS\inv.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=VoiceBoxMesh X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=VoiceBoxMesh NUM=1 TEXTURE=Ainv1

var vector X,Y,Z;
var() sound BattleSounds[10];	

state Activated  // Delete from inventory and toss in front of player.
{
	function HitWall (vector HitNormal, actor Wall)
	{
		Velocity = 0.5*(( Velocity dot HitNormal ) * HitNormal * (-2.0) + Velocity);   // Reflect off Wall w/damping
	//	PlaySound(Sound 'GrenadeFloor', SLOT_Misc, VSize(Velocity)/1300 );
		If (VSize(Velocity) < 20)
		{
			bBounce = False;
			bStasis = false;
			SetPhysics(PHYS_None);
			GoToState('Playing');
		}
	}
Begin:
	GetAxes(Pawn(Owner).ViewRotation,X,Y,Z);
	SetLocation(Pawn(Owner).Location+X*10+Y*8-Z*20);
    Disable('Touch');
	bBounce=True;	
	Velocity = Owner.Velocity + Vector(Owner.Rotation) * 150.0;
	Velocity.z += 240;         
	DesiredRotation = RotRand();
	RotationRate.Yaw = 20000*FRand() - 10000;
	RespawnTime = 0.0; //don't respawn
	SetPhysics(PHYS_Falling);
	RemoteRole = ROLE_DumbProxy;
	BecomePickup();
	bCollideWorld = true;
	Pawn(Owner).NextItem();
	if (Pawn(Owner).SelectedItem == Self) Pawn(Owner).SelectedItem=None;	
	Owner.PlaySound(ActivateSound);		
	Pawn(Owner).DeleteInventory(Self);	
	bStasis=false;
}


state Playing
{
	function Touch(Actor Other)
	{
		Super.Touch(Other);
	}

	function Timer()
	{
		local int i;
		
		MakeNoise(1.0);
		for (i=0 ; i<10 ; i++) 
			if (FRand()<0.05 && BattleSounds[i]!=None) PlaySound(BattleSounds[i], SLOT_None, FRand()/2+0.5);
		Charge--;
		if (Charge<=0) {
			spawn(class'SpriteBallExplosion',,,Location + vect(0,0,1)*16); 
			destroy();
		}
	}
	
	function BeginState()
	{
		bStasis = false;
		SetTimer(0.1,True);
		LoopAnim('Pulse');
	}
}

defaultproperties
{
     BattleSounds(0)=Sound'Unreal.AutoMag.shot'
     BattleSounds(1)=Sound'Unreal.flak.expl2'
     BattleSounds(2)=Sound'Unreal.AutoMag.shot'
     BattleSounds(3)=Sound'Unreal.AutoMag.shot'
     BattleSounds(4)=Sound'Unreal.flak.Explode1'
     BattleSounds(5)=Sound'Unreal.flak.expl2'
     BattleSounds(6)=Sound'Unreal.flak.shot1'
     BattleSounds(7)=Sound'Unreal.ASMD.TazerFire'
     BattleSounds(8)=Sound'Unreal.Stinger.Ricochet'
     BattleSounds(9)=Sound'Unreal.AutoMag.shot'
     bActivatable=True
     bDisplayableInv=True
     PickupMessage="You picked up the Voice Box"
     RespawnTime=30.000000
     PickupViewMesh=Mesh'Unreal.VoiceBoxMesh'
     Charge=100
     PickupSound=Sound'Unreal.Pickups.GenPickSnd'
     Icon=Texture'Unreal.Icons.I_VoiceBox'
     RemoteRole=ROLE_DumbProxy
     Mesh=Mesh'Unreal.VoiceBoxMesh'
     AmbientGlow=64
     bMeshCurvy=False
     CollisionRadius=18.000000
     CollisionHeight=8.000000
     bCollideWorld=True
}
