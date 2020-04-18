//=============================================================================
// CreatureChunks.
//=============================================================================
class CreatureChunks expands Carcass;

#exec MESH IMPORT MESH=CowBody1 ANIVFILE=MODELS\g_cow2_a.3D DATAFILE=MODELS\g_cow2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=CowBody1 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=CowBody1 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=CowBody1 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JGCow1  FILE=MODELS\Nc_1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=CowBody1 X=0.06 Y=0.06 Z=0.12
#exec MESHMAP SETTEXTURE MESHMAP=CowBody1 NUM=1 TEXTURE=JGCow1

#exec AUDIO IMPORT FILE="Sounds\Gibs\gibP1.WAV" NAME="gibP1" GROUP="Gibs"
#exec AUDIO IMPORT FILE="Sounds\Gibs\gibP3.WAV" NAME="gibP3" GROUP="Gibs"
#exec AUDIO IMPORT FILE="Sounds\Gibs\gibP4.WAV" NAME="gibP4" GROUP="Gibs"
#exec AUDIO IMPORT FILE="Sounds\Gibs\gibP5.WAV" NAME="gibP5" GROUP="Gibs"
#exec AUDIO IMPORT FILE="Sounds\Gibs\gibP6.WAV" NAME="gibP6" GROUP="Gibs"

var	  Bloodtrail	trail;
var	  float			TrailSize;
var   bool			bGreenBlood;

function PostBeginPlay()
{

	if ( Region.Zone.bDestructive || ((Level.Game != None) && Level.Game.bLowGore) )
		Destroy();
	else 
		Super.PostBeginPlay();
}

simulated function ZoneChange( ZoneInfo NewZone )
{
	local float splashsize;
	local actor splash;

	if ( NewZone.bWaterZone )
	{
		if ( trail != None )
		{
			if ( Level.bHighDetailMode )
				bUnlit = false;
			trail.Destroy();
		}
		if ( Mass <= Buoyancy )
			SetCollisionSize(0,0);
		if ( bSplash && !Region.Zone.bWaterZone && (Abs(Velocity.Z) < 80) )
			RotationRate *= 0.6;
		else if ( !Region.Zone.bWaterZone && (Velocity.Z < -200) )
		{
			// else play a splash
			splashSize = FClamp(0.0001 * Mass * (250 - 0.5 * FMax(-600,Velocity.Z)), 1.0, 3.0 );
			if ( NewZone.EntrySound != None )
				PlaySound(NewZone.EntrySound, SLOT_Interact, splashSize);
			if ( NewZone.EntryActor != None )
			{
				splash = Spawn(NewZone.EntryActor); 
				if ( splash != None )
					splash.DrawScale = splashSize;
			}
		}
		bSplash = true;
	}

	if ( NewZone.bDestructive || (NewZone.bPainZone  && (NewZone.DamagePerSec > 0)) )
		Destroy();
}
	
function Destroyed()
{
	if ( trail != None )
		trail.Destroy();
	Super.Destroyed();
}

function Initfor(actor Other)
{
	local vector RandDir;

	bDecorative = false;
	DrawScale = Other.DrawScale;
	if ( DrawScale != 1.0 )
		SetCollisionSize(CollisionRadius * 0.5 * (1 + DrawScale), CollisionHeight * 0.5 * (1 + DrawScale));
	RotationRate.Yaw = Rand(200000) - 100000;
	RotationRate.Pitch = Rand(200000 - Abs(RotationRate.Yaw)) - 0.5 * (200000 - Abs(RotationRate.Yaw));
	RandDir = 700 * FRand() * VRand();
	RandDir.Z = 200 * FRand() - 50;
	Velocity = (0.2 + FRand()) * (other.Velocity + RandDir);
	If (Region.Zone.bWaterZone)
		Velocity *= 0.5;
	if ( TrailSize > 0 )
	{
		if ( CreatureCarcass(Other) != None )
			bGreenBlood = CreatureCarcass(Other).bGreenBlood;
		else if ( (CreatureChunks(Other) != None) )
			bGreenBlood = CreatureChunks(Other).bGreenBlood;
	}
			
	if ( FRand() < 0.3 )
		Buoyancy = 1.06 * Mass; // float corpse
	else
		Buoyancy = 0.94 * Mass;
}

function ChunkUp(int Damage)
{
	local BloodSpurt b;

	if (bHidden)
		return;
	b = Spawn(class 'Bloodspurt',,,,rot(16384,0,0));
	if ( bGreenBlood )		
		b.GreenBlood();	
	if (bPlayerCarcass)
	{
		bHidden = true;
		SetPhysics(PHYS_None);
		SetCollision(false,false,false);
		bProjTarget = false;
	}
	else
		destroy();
}

function ClientExtraChunks()
{
	local Bloodspurt b;

	If ( Level.NetMode == NM_DedicatedServer )
		return;
	b = Spawn(class 'Bloodspurt',,,,rot(16384,0,0));
	if ( bGreenBlood )
		b.GreenBlood();
	b.RemoteRole = ROLE_None;
}

simulated function Landed(vector HitNormal)
{
	local rotator finalRot;
	local BloodSpurt b;

	if ( trail != None )
	{
		if ( Level.bHighDetailMode )
			bUnlit = false;
		trail.Destroy();
		trail = None;
	}
	finalRot = Rotation;
	finalRot.Roll = 0;
	finalRot.Pitch = 0;
	setRotation(finalRot);
	if ( Level.NetMode != NM_DedicatedServer )
	{
		b = Spawn(class 'Bloodspurt',,,,rot(16384,0,0));
		if ( bGreenBlood )
			b.GreenBlood();		
		b.RemoteRole = ROLE_None;
	}
	SetPhysics(PHYS_None);
	SetCollision(true, false, false);
}

simulated function HitWall(vector HitNormal, actor Wall)
{
	local float speed, decision;
	local BloodSpurt b;

	Velocity = 0.8 * (Velocity - 2 * HitNormal * (Velocity Dot HitNormal));
	Velocity.Z = FMin(Velocity.Z * 0.8, 700);
	speed = VSize(Velocity);
	if ( (speed < 250) && (trail != None) )
	{
		if ( Level.bHighDetailMode )
			bUnlit = false;
		trail.Destroy();
		trail = None;
	}
	else if ( speed > 350 )
	{
		if ( speed > 700 )
			velocity *= 0.8;
		if (  Level.NetMode != NM_DedicatedServer )
		{
			decision = FRand();
			if ( decision < 0.2 )
				PlaySound(sound 'gibP1');
			else if ( decision < 0.4 )
				PlaySound(sound 'gibP3');
			else if ( decision < 0.6 )
				PlaySound(sound 'gibP4');
			else if ( decision < 0.8 )
				PlaySound(sound 'gibP5');
			else 
				PlaySound(sound 'gibP6');
		}
	}
	if ( (trail == None) && (Level.NetMode != NM_DedicatedServer) )
	{
		b = Spawn(class 'Bloodspurt',,,,Rotator(HitNormal));
		if ( bGreenBlood )
			b.GreenBlood();		
		b.RemoteRole = ROLE_None;
	}

	if ( speed < 120 )
	{
		bBounce = false;
		Disable('HitWall');
	}
}

function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
{
	SetPhysics(PHYS_Falling);
	bBobbing = false;
	Velocity += momentum/Mass;
	CumulativeDamage += Damage;
	If ( Damage > FMin(15, Mass) || (CumulativeDamage > Mass) )
		ChunkUp(Damage);
}

auto state Dying
{
	ignores TakeDamage;

Begin:
	if ( bDecorative )
		SetPhysics(PHYS_None);
	else if ( (TrailSize > 0) && !Region.Zone.bWaterZone )
	{
		trail = Spawn(class'BloodTrail',self);
//		trail.DrawScale = TrailSize;
		if ( bGreenBlood )
			trail.GreenBlood();
	}
	Sleep(0.35);
	SetCollision(true, false, false);
	GotoState('Dead');
}	

state Dead 
{
	function BeginState()
	{
		if ( bDecorative )
			lifespan = 0.0;
		else
			SetTimer(5.0, false);
	}
}

defaultproperties
{
     TrailSize=3.000000
     RemoteRole=ROLE_SimulatedProxy
     LifeSpan=20.000000
     Mesh=Mesh'Unreal.CowBody1'
     bCollideActors=False
     bBounce=True
     bFixedRotationDir=True
	 bUnlit=True
     Mass=30.000000
     Buoyancy=27.000000
     RotationRate=(Pitch=30000,Roll=30000)
}
