//=============================================================================
// RazorBlade.
//=============================================================================
class RazorBlade expands Projectile;
 
#exec MESH IMPORT MESH=razorb ANIVFILE=MODELS\razorb_a.3D DATAFILE=MODELS\razorb_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=razorb X=0 Y=0 Z=0 YAW=-64
#exec MESH SEQUENCE MESH=razorb SEQ=All    STARTFRAME=0   NUMFRAMES=15
#exec MESH SEQUENCE MESH=razorb SEQ=Spin  STARTFRAME=0   NUMFRAMES=15
#exec TEXTURE IMPORT NAME=Jrazor1 FILE=MODELS\razor.PCX 
#exec OBJ LOAD FILE=textures\FireEffect54.utx PACKAGE=Unreal.Effect54
#exec MESHMAP SCALE MESHMAP=razorb X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=razorb NUM=1 TEXTURE=Jrazor1
#exec MESHMAP SETTEXTURE MESHMAP=razorb NUM=0 TEXTURE=Unreal.Effect54.FireEffect54

#exec AUDIO IMPORT FILE="Sounds\Razor\fly15.WAV" NAME="RazorHum" GROUP="RazorJack"
#exec AUDIO IMPORT FILE="Sounds\Razor\bladehit.wav" NAME="BladeHit" GROUP="RazorJack"
#exec AUDIO IMPORT FILE="Sounds\Razor\bladethunk.wav" NAME="BladeThunk" GROUP="RazorJack"
#exec AUDIO IMPORT FILE="Sounds\Razor\start9b.WAV" NAME="StartBlade" GROUP="RazorJack"

var int NumWallHits;
var bool bCanHitInstigator, bHitWater;
var int CurPitch,PitchStart,YawStart,CurYaw;
var rotator CurRotation;

/////////////////////////////////////////////////////
auto state Flying
{
	function ProcessTouch (Actor Other, Vector HitLocation)
	{
		if ( bCanHitInstigator || (Other != Instigator) ) 
		{
			if ( Other.IsA('Pawn') && (HitLocation.Z - Other.Location.Z > 0.62 * Other.CollisionHeight)
				&& (instigator.IsA('PlayerPawn') || (instigator.skill > 1))
				&& (!Other.IsA('ScriptedPawn') || !ScriptedPawn(Other).bIsBoss) )
				Other.TakeDamage(3.5 * damage, instigator,HitLocation,
					(MomentumTransfer * Normal(Velocity)), 'decapitated' );
			else			 
				Other.TakeDamage(damage, instigator,HitLocation,
					(MomentumTransfer * Normal(Velocity)), 'shredded' );
			PlaySound(MiscSound, SLOT_Misc, 2.0);
			destroy();
		}
	}

	simulated function ZoneChange( Zoneinfo NewZone )
	{
		local Splash w;
		
		if (!NewZone.bWaterZone || bHitWater) Return;

		bHitWater = True;
		w = Spawn(class'Splash',,,,rot(16384,0,0));
		w.DrawScale = 0.5;
		w.RemoteRole = ROLE_None;
		Velocity=0.6*Velocity;
	}

	simulated function SetRoll(vector NewVelocity) 
	{
		local rotator newRot;	
	
		newRot = rotator(NewVelocity);	
		SetRotation(newRot);	
	}

	simulated function HitWall (vector HitNormal, actor Wall)
	{
		
		bCanHitInstigator = true;
		if ( (RemoteRole == ROLE_DumbProxy) || (Level.Netmode != NM_DedicatedServer) )
			PlaySound(ImpactSound, SLOT_Misc, 2.0);
		if ( Level.NetMode != NM_Client )
		{
			if ( (Mover(Wall) != None) && Mover(Wall).bDamageTriggered )
			{
				Wall.TakeDamage( Damage, instigator, Location, MomentumTransfer * Normal(Velocity), '');
				Destroy();
				return;
			}
			NumWallHits++;
			SetTimer(0, False);
			MakeNoise(0.3);
			if ( NumWallHits > 5 )
				Destroy();
		}
		Velocity -= 2 * ( Velocity dot HitNormal) * HitNormal;  
		SetRoll(Velocity);
		LoopAnim('Spin',1.0);		
	}

	function BeginState()
	{
		local vector X;

		X = vector(Rotation);	
		Velocity = Speed * X;     // Impart ONLY forward vel
		LoopAnim('Spin',1.0);
		PlaySound(SpawnSound, SLOT_None,4.2);
		if ( Level.NetMode == NM_Standalone )
			SoundPitch = 200 + 50 * FRand();			

		if (Instigator.HeadRegion.Zone.bWaterZone)
			bHitWater = True;	
	}

Begin:
	Sleep(0.2);
	bCanHitInstigator = true;
}

defaultproperties
{
     speed=1300.000000
     MaxSpeed=1200.000000
     Damage=30.000000
     MomentumTransfer=15000
     SpawnSound=Sound'Unreal.Razorjack.StartBlade'
     ImpactSound=Sound'Unreal.Razorjack.BladeHit'
     MiscSound=Sound'Unreal.Razorjack.BladeThunk'
     RemoteRole=ROLE_SimulatedProxy
     LifeSpan=6.000000
     AnimSequence=spin
     Mesh=Mesh'Unreal.razorb'
     AmbientGlow=167
     bUnlit=True
     bMeshCurvy=False
     SoundRadius=12
     SoundVolume=128
     SoundPitch=200
     AmbientSound=Sound'Unreal.Razorjack.RazorHum'
     bBounce=True
     NetPriority=6.000000
}
