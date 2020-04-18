//=============================================================================
// Chunk.
//=============================================================================
class Chunk expands Projectile;

#exec AUDIO IMPORT FILE="Sounds\flak\flachit1.WAV" NAME="Hit1" GROUP="flak"
#exec AUDIO IMPORT FILE="Sounds\flak\flachit3.WAV" NAME="Hit3" GROUP="flak"
#exec AUDIO IMPORT FILE="Sounds\flak\flachit5.WAV" NAME="Hit5" GROUP="flak"
#exec AUDIO IMPORT FILE="Sounds\flak\chunkhit.WAV" NAME="ChunkHit" GROUP="flak"

var bool bDelayTime;

	function PostBeginPlay()
	{
		local rotator RandRot;

		RandRot = Rotation;
		RandRot.Pitch += FRand() * 2000 - 1000;
		RandRot.Yaw += FRand() * 2000 - 1000;
		RandRot.Roll += FRand() * 2000 - 1000;
		Velocity = Vector(RandRot) * (Speed + (FRand() * 200 - 100));
		if (Region.zone.bWaterZone)
			SetPhysics(PHYS_Falling);
		Super.PostBeginPlay();
	}

	function ProcessTouch (Actor Other, vector HitLocation)
	{
		if ( (Chunk(Other) == None) && (bDelayTime || (Other != Instigator)) )
		{
			if ( Level.NetMode == NM_Client )
			{
				Destroy();
				return;
			}
			speed = VSize(Velocity);
			If ( speed > 200 )
			{
				Other.TakeDamage(damage, instigator,HitLocation,
					(MomentumTransfer * Velocity/speed), 'shredded' );
				if ( (Level.NetMode == NM_StandAlone) && (FRand() < 0.5) )
					PlaySound(Sound 'ChunkHit',, 2.0,,1000);
			}
			Destroy();
		}
	}
	function Timer() 
	{
		Destroy();
	}

	simulated function Landed( Vector HitNormal )
	{
		SetPhysics(PHYS_None);
	}

	simulated function HitWall( vector HitNormal, actor Wall )
	{
		local float Rand;
		local SmallSpark s;

		if ( (Level.NetMode != NM_Client) && (Mover(Wall) != None) && Mover(Wall).bDamageTriggered )
		{
			Wall.TakeDamage( Damage, instigator, Location, MomentumTransfer * Normal(Velocity), '');
			Destroy();
			return;
		}
		if (!bDelayTime) 
		{
			bDelayTime=True;
			SetPhysics(PHYS_Falling);
			if ( (Level.Netmode != NM_DedicatedServer) && (FRand()<0.2) ) 
			{
				s = Spawn(Class'SmallSpark',,,Location+HitNormal*5,rotator(HitNormal));
				s.RemoteRole = ROLE_None;
			}
		}
		Velocity = 0.8*(( Velocity dot HitNormal ) * HitNormal * (-1.8 + FRand()*0.8) + Velocity);   // Reflect off Wall w/damping
		SetRotation(rotator(Velocity));
		speed = VSize(Velocity);
		if ( speed > 100 ) 
		{
			if ( Role == ROLE_Authority )
				MakeNoise(0.3);
			if ( Level.Netmode != NM_DedicatedServer )
			{
				Rand = FRand();
				if (Rand < 0.33)	PlaySound(sound 'Hit1', SLOT_Misc,0.6,,1000);	
				else if (Rand < 0.66) PlaySound(sound 'Hit3', SLOT_Misc,0.6,,1000);
				else PlaySound(sound 'Hit5', SLOT_Misc,0.6,,1000);
			}				
		}
		else 
		{
			bBounce = False;
			SetTimer(1.0,False);
		}
	}

	function zonechange(Zoneinfo NewZone)
	{
		if (NewZone.bWaterZone)
			SetPhysics(PHYS_Falling);			
	}

defaultproperties
{
     speed=2500.000000
     MaxSpeed=2700.000000
     Damage=17.000000
     MomentumTransfer=10000
     RemoteRole=ROLE_SimulatedProxy
     LifeSpan=3.000000
     bUnlit=True
     bNoSmooth=True
     bMeshCurvy=False
     bBounce=True
     NetPriority=6.000000
}
