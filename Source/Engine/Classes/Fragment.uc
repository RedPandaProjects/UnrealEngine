//=============================================================================
// Fragment.
//=============================================================================
class Fragment expands Projectile;

var() MESH Fragments[11];
var int numFragmentTypes;
var bool bFirstHit;

function PostBeginPlay()
{
	if ( Region.Zone.bDestructive )
		Destroy();
	else
		Super.PostBeginPlay();
}

simulated function CalcVelocity(vector Momentum, float ExplosionSize)
{
	Velocity = VRand()*(ExplosionSize+FRand()*150.0+100.0 + VSize(Momentum)/80); 
}

simulated function HitWall (vector HitNormal, actor HitWall)
{
	Velocity = 0.5*(( Velocity dot HitNormal ) * HitNormal * (-2.0) + Velocity);   // Reflect off Wall w/damping
	speed = VSize(Velocity);	
	if (bFirstHit && speed<400) 
	{
		bFirstHit=False;
		bRotatetoDesired=True;
		bFixedRotationDir=False;
		DesiredRotation.Pitch=0;	
		DesiredRotation.Yaw=FRand()*65536;
		DesiredRotation.roll=0;
	}
	RotationRate.Yaw = RotationRate.Yaw*0.75;
	RotationRate.Roll = RotationRate.Roll*0.75;
	RotationRate.Pitch = RotationRate.Pitch*0.75;
	if ( (speed < 60) && (HitNormal.Z > 0.7) )
	{
		SetPhysics(PHYS_none);
		bBounce = false;
		GoToState('Dying');
	}
	else If (speed > 50) 
	{
		if (FRand()<0.5) PlaySound(ImpactSound, SLOT_None, 0.5+FRand()*0.5,, 300, 0.85+FRand()*0.3);
		else PlaySound(MiscSound, SLOT_None, 0.5+FRand()*0.5,, 300, 0.85+FRand()*0.3);
	}
}

auto state Flying
{
	simulated function timer()
	{
		GoToState('Dying');
	}


	simulated function Touch(actor Other)
	{
		if (Pawn(Other)==None) Return;
		if (!Pawn(Other).bIsPlayer) Destroy();
	}


	simulated singular function ZoneChange( ZoneInfo NewZone )
	{
		local float splashsize;
		local actor splash;

		if ( NewZone.bWaterZone )
		{
			Velocity = 0.2 * Velocity;
			splashSize = 0.0005 * (250 - 0.5 * Velocity.Z);
			if ( Level.NetMode != NM_DedicatedServer )
			{
				if ( NewZone.EntrySound != None )
					PlaySound(NewZone.EntrySound, SLOT_Interact, splashSize);
				if ( NewZone.EntryActor != None )
				{
					splash = Spawn(NewZone.EntryActor); 
					if ( splash != None )
						splash.DrawScale = 4 * splashSize;
				}
			}
			if (bFirstHit) 
			{
				bFirstHit=False;
				bRotatetoDesired=True;
				bFixedRotationDir=False;
				DesiredRotation.Pitch=0;	
				DesiredRotation.Yaw=FRand()*65536;
				DesiredRotation.roll=0;
			}
			
			RotationRate = 0.2 * RotationRate;
			GotoState('Dying');
		}
		if ( NewZone.bPainZone && (NewZone.DamagePerSec > 0) )
			Destroy();
	}

	simulated function BeginState()
	{
		RandSpin(125000);
		if (RotationRate.Pitch>-10000&&RotationRate.Pitch<10000) 
			RotationRate.Pitch=10000;
		if (RotationRate.Roll>-10000&&RotationRate.Roll<10000) 
			RotationRate.Roll=10000;			
		Mesh = Fragments[int(FRand()*numFragmentTypes)];
		if ( Level.NetMode == NM_Standalone )
			LifeSpan = 20 + 40 * FRand();
		SetTimer(5.0,True);			
	}
}

state Dying
{
	simulated function HitWall (vector HitNormal, actor HitWall)
	{
		Velocity = 0.5*(( Velocity dot HitNormal ) * HitNormal * (-2.0) + Velocity);   // Reflect off Wall w/damping
		speed = VSize(Velocity);	
		if (bFirstHit && speed<400) 
		{
			bFirstHit=False;
			bRotatetoDesired=True;
			bFixedRotationDir=False;
			DesiredRotation.Pitch=0;	
			DesiredRotation.Yaw=FRand()*65536;
			DesiredRotation.roll=0;
		}
		RotationRate.Yaw = RotationRate.Yaw*0.75;
		RotationRate.Roll = RotationRate.Roll*0.75;
		RotationRate.Pitch = RotationRate.Pitch*0.75;
		if ( (Velocity.Z < 50) && (HitNormal.Z > 0.7) )
		{
			SetPhysics(PHYS_none);
			bBounce = false;
		}
		else If (speed > 80) 
		{
			if (FRand()<0.5) PlaySound(ImpactSound, SLOT_None, 0.5+FRand()*0.5,, 300, 0.85+FRand()*0.3);
			else PlaySound(MiscSound, SLOT_None, 0.5+FRand()*0.5,, 300, 0.85+FRand()*0.3);
		}
	}

	function TakeDamage( int Dam, Pawn instigatedBy, Vector hitlocation, 
							Vector momentum, name damageType)
	{
		Destroy();
	}

	simulated function timer()
	{
		if (!PlayerCanSeeMe()) 
			Destroy();
	}

	simulated function BeginState()
	{
		SetTimer(1.5,True);
		SetCollision(true, false, false);
	}
}

defaultproperties
{
     bFirstHit=True
	 bNetOptional=True
     bMeshCurvy=False
     CollisionRadius=+00018.000000
     CollisionHeight=+00004.000000
     Physics=PHYS_Falling
     bBounce=True
     bFixedRotationDir=True
	 bCollideActors=false
     LifeSpan=+00020.000000
     RemoteRole=ROLE_SimulatedProxy
     NetPriority=+00002.000000
}
