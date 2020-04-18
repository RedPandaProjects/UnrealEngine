//=============================================================================
// CreatureCarcass.
//=============================================================================
class CreatureCarcass expands Carcass;

#exec MESH IMPORT MESH=CowBody1 ANIVFILE=MODELS\g_cow2_a.3D DATAFILE=MODELS\g_cow2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=CowBody1 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=CowBody1 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=CowBody1 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JGCow1  FILE=MODELS\Nc_1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=CowBody1 X=0.06 Y=0.06 Z=0.12
#exec MESHMAP SETTEXTURE MESHMAP=CowBody1 NUM=1 TEXTURE=JGCow1

#exec MESH IMPORT MESH=CowBody2 ANIVFILE=MODELS\g_cowb_a.3D DATAFILE=MODELS\g_cowb_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=CowBody2 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=CowBody2 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=CowBody2 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JGCow1  FILE=MODELS\Nc_1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=CowBody2 X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=CowBody2 NUM=1 TEXTURE=JGCow1

#exec MESH IMPORT MESH=LiverM ANIVFILE=MODELS\g_gut1_a.3D DATAFILE=MODELS\g_gut1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=LiverM X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=LiverM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=LiverM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jparts1  FILE=MODELS\g_parts.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=LiverM X=0.02 Y=0.02 Z=0.04
#exec MESHMAP SETTEXTURE MESHMAP=LiverM NUM=1 TEXTURE=Jparts1

#exec AUDIO IMPORT FILE="Sounds\Gibs\biggib1.WAV" NAME="Gib1" GROUP="Gibs"
#exec AUDIO IMPORT FILE="Sounds\Gibs\biggib2.WAV" NAME="Gib4" GROUP="Gibs"
#exec AUDIO IMPORT FILE="Sounds\Gibs\biggib3.WAV" NAME="Gib5" GROUP="Gibs"
#exec AUDIO IMPORT FILE="Sounds\Gibs\gib1.WAV" NAME="Gib2" GROUP="Gibs"
#exec AUDIO IMPORT FILE="Sounds\Gibs\gib3.WAV" NAME="Gib3" GROUP="Gibs"
#exec AUDIO IMPORT FILE="Sounds\Gibs\bthump1.WAV" NAME="Thump" GROUP="Gibs"

var() mesh	bodyparts[8];
var() float Trails[8];
var() float ZOffset[8];
var() bool bGreenBlood;
var() sound GibOne;
var() sound GibTwo;
var() sound LandedSound;
var	  bool bThumped;
var	  bool bPermanent;
var	  bool bCorroding;
var   ZoneInfo DeathZone;
var	  float	ReducedHeightFactor;

	function PostBeginPlay()
	{
		if ( !bDecorative )
		{
			DeathZone = Region.Zone;
			DeathZone.NumCarcasses++;
		}
		Super.PostBeginPlay();
		if ( Physics == PHYS_None )
			SetCollision(bCollideActors, false, false);
	}

	function Destroyed()
	{
		if ( !bDecorative )
			DeathZone.NumCarcasses--;
		Super.Destroyed();
	}

	function Initfor(actor Other)
	{
		local rotator carcRotation;

		if ( bDecorative )
		{
			DeathZone = Region.Zone;
			DeathZone.NumCarcasses++;
		}
		bDecorative = false;
		bMeshCurvy = Other.bMeshCurvy;	
		bMeshEnviroMap = Other.bMeshEnviroMap;	
		Mesh = Other.Mesh;
		Skin = Other.Skin;
		Texture = Other.Texture;
		Fatness = Other.Fatness;
		DrawScale = Other.DrawScale;
		SetCollisionSize(Other.CollisionRadius + 4, Other.CollisionHeight);
		if ( !SetLocation(Location) )
			SetCollisionSize(CollisionRadius - 4, CollisionHeight);

		DesiredRotation = other.Rotation;
		DesiredRotation.Roll = 0;
		DesiredRotation.Pitch = 0;
		AnimSequence = Other.AnimSequence;
		AnimFrame = Other.AnimFrame;
		AnimRate = Other.AnimRate;
		TweenRate = Other.TweenRate;
		AnimMinRate = Other.AnimMinRate;
		AnimLast = Other.AnimLast;
		bAnimLoop = Other.bAnimLoop;
		SimAnim.X = 10000 * AnimFrame;
		SimAnim.Y = 10000 * AnimRate;
		SimAnim.Z = 1000 * TweenRate;
		SimAnim.W = 10000 * AnimLast;
		bAnimFinished = Other.bAnimFinished;
		Velocity = other.Velocity;
		Mass = Other.Mass;
		if ( Buoyancy < 0.8 * Mass )
			Buoyancy = 0.9 * Mass;
	}

	function TakeDamage( int Damage, Pawn InstigatedBy, Vector Hitlocation, 
							Vector Momentum, name DamageType)
	{	
		local BloodSpurt b;
	
		b = Spawn(class'BloodSpurt',,,HitLocation,rot(16384,0,0));
		if ( bGreenBlood )
			b.GreenBlood();		
		if ( !bPermanent )
		{
			if ( (DamageType == 'Corroded') && (Damage >= 100) )
			{
				bCorroding = true;
				GotoState('Corroding');
			}
			else
				Super.TakeDamage(Damage, instigatedBy, HitLocation, Momentum, DamageType);
		}
	}

	function CreateReplacement()
	{
		local CreatureChunks carc;
		
		if (bHidden)
			return;
		if ( bodyparts[0] != None )
			carc = Spawn(class 'CreatureChunks',,, Location + ZOffset[0] * CollisionHeight * vect(0,0,1)); 
		if (carc != None)
		{
			carc.TrailSize = Trails[0];
			carc.Mesh = bodyparts[0];
			carc.Initfor(self);
			carc.Velocity = velocity; //no rand
			carc.Bugs = Bugs;
			if ( Bugs != None )
				Bugs.SetBase(carc);
			Bugs = None;
		}
		else if ( Bugs != None )
			Bugs.Destroy();
	}

	function ChunkUp(int Damage)
	{
		if ( bPermanent )
			return;
		CreateReplacement();
		if ( !Level.Game.bLowGore )
			ClientExtraChunks();
		SetPhysics(PHYS_None);
		bHidden = true;
		SetCollision(false,false,false);
		bProjTarget = false;
		GotoState('Gibbing');
	}

	function ClientExtraChunks()
	{
		local CreatureChunks carc;
		local bloodpuff Blood;
		local bloodspurt b;
		local int n;
		
		n = 1;
		while ( (n<8) && (bodyparts[n] != none) )
		{
			carc = Spawn(class 'CreatureChunks',,, Location + ZOffset[n] * CollisionHeight * vect(0,0,1));
			if (carc != None)
			{
				carc.TrailSize = Trails[n];
				carc.Mesh = bodyparts[n];
				carc.Initfor(self);
			}
			n++;
		}
		b = Spawn(class 'Bloodspurt',,,,rot(16384,0,0));
		if ( bGreenBlood )
			b.GreenBlood();
		if ( Level.bHighDetailMode && !bGreenBlood )
		{
			Blood = spawn(class'BloodPuff',,, Location);
			Blood.drawscale = 0.2 * CollisionRadius;
		}
	}

	function Landed(vector HitNormal)
	{
		local rotator finalRot;
		local float OldHeight;
		local BloodSpurt b;

		if ( (Velocity.Z < -1000) && !bPermanent )
		{
			ChunkUp(200);
			return;
		}

		finalRot = Rotation;
		finalRot.Roll = 0;
		finalRot.Pitch = 0;
		setRotation(finalRot);
//		if ( !bDecorative )
//		{
//			b = Spawn(class 'Bloodspurt',,,,rot(16384,0,0));
//			if ( bGreenBlood ) b.GreenBlood();			
//		}
		SetPhysics(PHYS_None);
	 	SetCollision(bCollideActors, false, false);
		if ( HitNormal.Z < 0.99 )
			ReducedHeightFactor = 0.1;
		if ( HitNormal.Z < 0.93 )
			ReducedHeightFactor = 0.0;
		if ( !IsAnimating() )
			LieStill();
	}

	function AnimEnd()
	{
		if ( Physics == PHYS_None )
			LieStill();
		else if ( Region.Zone.bWaterZone )
		{
			bThumped = true;
			LieStill();
		}
	}

	function LieStill()
	{
		if ( !bThumped && !bDecorative )
			LandThump();
		if ( !bReducedHeight )
			ReduceCylinder();
	}

	function ThrowOthers()
	{
		local float dist, shake;
		local pawn Thrown;
		local PlayerPawn aPlayer;
		local vector Momentum;

		Thrown = Level.PawnList;
		While ( Thrown != None )
		{
			aPlayer = PlayerPawn(Thrown);
			if ( aPlayer != None )
			{	
				dist = VSize(Location - aPlayer.Location);
				shake = FMax(500, 1500 - dist);
				aPlayer.ShakeView( FMax(0, 0.35 - dist/20000),shake, 0.015 * shake );
				if ( (aPlayer.Physics == PHYS_Walking) && (dist < 1500) )
				{
					Momentum = -0.5 * aPlayer.Velocity + 100 * VRand();
					Momentum.Z =  7000000.0/((0.4 * dist + 350) * aPlayer.Mass);
					aPlayer.AddVelocity(Momentum);
				}
			}
		Thrown = Thrown.nextPawn;
		}
	}

	function LandThump()
	{
		local float impact;

		if ( Physics == PHYS_None)
		{
			bThumped = true;
			if ( Role == ROLE_Authority )
			{
				impact = 0.75 + Velocity.Z * 0.004;
				impact = Mass * impact * impact * 0.015;
				PlaySound(LandedSound,, impact);
				if ( Mass >= 500 )
					ThrowOthers();
			}
		}
	}

	function ReduceCylinder()
	{
		local float OldHeight;

		bReducedHeight = true;
		SetCollision(bCollideActors,False,False);
		OldHeight = CollisionHeight;
		if ( ReducedHeightFactor < Default.ReducedHeightFactor )
			SetCollisionSize(CollisionRadius, CollisionHeight * ReducedHeightFactor);
		else
			SetCollisionSize(CollisionRadius + 4, CollisionHeight * ReducedHeightFactor);
		PrePivot = vect(0,0,1) * (OldHeight - CollisionHeight); 
		if ( !SetLocation(Location - PrePivot) )
		{
			SetCollisionSize(CollisionRadius - 4, CollisionHeight);
			if ( !SetLocation(Location - PrePivot) )
			{
				SetCollisionSize(CollisionRadius, OldHeight);
				SetCollision(false, false, false);
				PrePivot = vect(0,0,0);
			}
		}
		PrePivot = PrePivot + vect(0,0,2);
		Mass = Mass * 0.8;
		Buoyancy = Buoyancy * 0.8;
	}

	function HitWall(vector HitNormal, actor Wall)
	{
		local BloodSpurt b;
	
		b = Spawn(class 'Bloodspurt',,,,Rotator(HitNormal));
		if ( bGreenBlood )	
			b.GreenBlood();		
		Velocity = 0.7 * (Velocity - 2 * HitNormal * (Velocity Dot HitNormal));
		Velocity.Z *= 0.9;
		if ( Abs(Velocity.Z) < 120 )
		{
			bBounce = false;
			Disable('HitWall');
		}
	}

auto state Dying
{
	ignores TakeDamage;

Begin:
	if ( bCorroding )
		GotoState('Corroding');
	if ( bDecorative && !bReducedHeight )
	{
		ReduceCylinder();
		SetPhysics(PHYS_None);
	}
	Sleep(0.2);
	if ( bCorroding )
		GotoState('Corroding');
	GotoState('Dead');
}

state Dead 
{
	function AddFliesAndRats()
	{
		if ( (flies > 0) && (Bugs == None) && (Level.NetMode == NM_Standalone) )
		{
			Bugs = Spawn(class 'DeadBodySwarm');
			if (Bugs != None)
			{
				Bugs.SetBase(Self);
				DeadBodySwarm(Bugs).swarmsize = flies * (FRand() + 0.5);
				DeadBodySwarm(Bugs).swarmradius = collisionradius;
			}
		}
	}

	function CheckZoneCarcasses()
	{
		local CreatureCarcass C, Best;

		if ( !bDecorative && (DeathZone.NumCarcasses > DeathZone.MaxCarcasses) )
		{
			Best = self;
			ForEach AllActors(class'CreatureCarcass', C)
				if ( (C != Self) && !C.bDecorative && (C.DeathZone == DeathZone) && !C.IsAnimating() )
				{
					if ( Best == self )
						Best = C;
					else if ( !C.PlayerCanSeeMe() )
					{
						Best = C;
						break;
					}
				}
			Best.Destroy();
		}
	}

	function BeginState()
	{
		if ( bDecorative || bPermanent 
			|| (IsA('HumanCarcass') && (Level.NetMode == NM_Standalone) && Level.Game.IsA('SinglePlayer')) )
			lifespan = 0.0;
		else
			SetTimer(FMax(12.0, 30.0 - 2 * DeathZone.NumCarcasses), false); 
	}

}

state Gibbing
{
	ignores Landed, HitWall, AnimEnd, TakeDamage, ZoneChange;

	function GibSound()
	{
		local float decision;

		decision = FRand();
		if (decision < 0.2)
			PlaySound(GibOne, SLOT_Interact, 0.06 * Mass);
		else if ( decision < 0.35 )
			PlaySound(GibTwo, SLOT_Interact, 0.06 * Mass);
		else if ( decision < 0.5 )
			PlaySound(sound'Gib3', SLOT_Interact, 0.06 * Mass);
		else if ( decision < 0.8 )
			PlaySound(sound'Gib4', SLOT_Interact, 0.06 * Mass);
		else 
			PlaySound(sound'Gib5', SLOT_Interact, 0.06 * Mass);
	}

Begin:
	Sleep(0.2);
	GibSound();
	if ( !bPlayerCarcass )
		Destroy();
}

state Corroding
{
	ignores Landed, HitWall, AnimEnd, TakeDamage, ZoneChange;

	function Tick( float DeltaTime )
	{
		local int NewFatness; 
		local float splashSize;
		local actor splash;

		NewFatness = fatness - 80 * DeltaTime;
		if ( NewFatness < 85 )
		{
			if ( Region.Zone.bWaterZone && Region.Zone.bDestructive )
			{
				splashSize = FClamp(0.0002 * Mass * (250 - 0.5 * FMax(-600,Velocity.Z)), 1.0, 4.0 );
				if ( Region.Zone.ExitSound != None )
					PlaySound(Region.Zone.ExitSound, SLOT_Interact, splashSize);
				if ( Region.Zone.ExitActor != None )
				{
					splash = Spawn(Region.Zone.ExitActor); 
					if ( splash != None )
						splash.DrawScale = splashSize;
				}
			}			
			Destroy();
		}
		fatness = Clamp(NewFatness, 0, 255);
	}
	
	function BeginState()
	{
		Disable('Tick');
	}
	
Begin:
	Sleep(0.5);
	Enable('Tick');	
}

defaultproperties
{
     bodyparts(0)=Mesh'Unreal.CowBody1'
     bodyparts(1)=Mesh'Unreal.CowBody1'
     bodyparts(2)=Mesh'Unreal.CowBody2'
     bodyparts(3)=Mesh'Unreal.CowBody2'
     bodyparts(4)=Mesh'Unreal.LiverM'
     bodyparts(5)=Mesh'Unreal.LiverM'
     Trails(0)=0.500000
     Trails(1)=0.500000
     Trails(2)=0.500000
     Trails(3)=0.500000
     Trails(4)=0.500000
     Trails(5)=0.500000
     Trails(6)=0.500000
     Trails(7)=0.500000
     ZOffset(0)=0.500000
     ZOffset(1)=-0.500000
     GibOne=Sound'Unreal.Gibs.Gib1'
     GibTwo=Sound'Unreal.Gibs.Gib2'
     LandedSound=Sound'Unreal.Gibs.Thump'
     ReducedHeightFactor=0.300000
     flies=4
     bSlidingCarcass=True
     TransientSoundVolume=3.000000
     bBlockActors=True
     bBlockPlayers=True
	 NetPriority=+6.00000
}
