//=============================================================================
// Bots
//=============================================================================
class Bots expands ScriptedPawn
	abstract;

var(Sounds) sound 	drown;
var(Sounds) sound	breathagain;
var(Sounds) sound	Footstep1;
var(Sounds) sound	Footstep2;
var(Sounds) sound	Footstep3;
var(Sounds) sound	HitSound3;
var(Sounds) sound	HitSound4;
var(Sounds) sound	Die2;
var(Sounds) sound	Die3;
var(Sounds) sound	Die4;
var(Sounds) sound	GaspSound;
var(Sounds) sound	UWHit1;
var(Sounds) sound	UWHit2;
var(Sounds) sound   LandGrunt;

var bool bNoShootDecor;
var bool bGathering;
var bool bCamping;
var Weapon EnemyDropped;
var float PlayerKills;
var float PlayerDeaths;
var float LastInvFind;

function eAttitude AttitudeTo(Pawn Other)
{
	if (Other.bIsPlayer)
	{
		if ( Level.Game.IsA('TeamGame') && (Team == Other.Team) )
			return ATTITUDE_Friendly;
		else 
		{
			if (RelativeStrength(Other) > Aggressiveness)
				AttitudeToPlayer = ATTITUDE_Fear;
			else if (AttitudeToPlayer == ATTITUDE_Fear)
				AttitudeToPlayer = ATTITUDE_Hate;
		}
		return AttitudeToPlayer;
	}
	else 
		return Super.AttitudeTo(Other);
}

//-----------------------------------------------------------------------------
// Sound functions 

function AdjustSkill(bool bWinner)
{
	if ( bWinner )
	{
		PlayerKills += 1;
		skill -= 1/Min(PlayerKills, 20);
		skill = FClamp(skill, 0, 3);
	}
	else
	{
		PlayerDeaths += 1;
		skill += 1/Min(PlayerDeaths, 20);
		skill = FClamp(skill, 0, 3);
	}
}

simulated function PlayFootStep()
{
	local sound step;
	local float decision;

	if ( Role < ROLE_Authority )
		return;
	if ( FootRegion.Zone.bWaterZone )
	{
		PlaySound(sound 'LSplash', SLOT_Interact, 1, false, 1500.0, 1.0);
		return;
	}

	decision = FRand();
	if ( decision < 0.34 )
		step = Footstep1;
	else if (decision < 0.67 )
		step = Footstep2;
	else
		step = Footstep3;

	if ( DesiredSpeed <= 0.5 )
		PlaySound(step, SLOT_Interact, 0.5, false, 400.0, 1.0);
	else 
		PlaySound(step, SLOT_Interact, 1, false, 1200.0, 1.0);
}

function PlayDyingSound()
{
	local float rnd;

	if ( HeadRegion.Zone.bWaterZone )
	{
		if ( FRand() < 0.5 )
			PlaySound(UWHit1, SLOT_Pain,2.0,,,Frand()*0.2+0.9);
		else
			PlaySound(UWHit2, SLOT_Pain,2.0,,,Frand()*0.2+0.9);
		return;
	}

	rnd = FRand();
	if (rnd < 0.25)
		PlaySound(Die, SLOT_Talk,2.0);
	else if (rnd < 0.5)
		PlaySound(Die2, SLOT_Talk,2.0);
	else if (rnd < 0.75)
		PlaySound(Die3, SLOT_Talk,2.0);
	else 
		PlaySound(Die4, SLOT_Talk,2.0);
}

function PlayTakeHitSound(int damage, name damageType, int Mult)
{
	if ( Level.TimeSeconds - LastPainSound < 0.25 )
		return;
	LastPainSound = Level.TimeSeconds;

	if ( HeadRegion.Zone.bWaterZone )
	{
		if ( damageType == 'Drowned' )
			PlaySound(drown, SLOT_Pain, 1.5);
		else if ( FRand() < 0.5 )
			PlaySound(UWHit1, SLOT_Pain,2.0,,,Frand()*0.15+0.9);
		else
			PlaySound(UWHit2, SLOT_Pain,2.0,,,Frand()*0.15+0.9);
		return;
	}
	damage *= FRand();

	if (damage < 8) 
		PlaySound(HitSound1, SLOT_Pain,2.0,,,Frand()*0.2+0.9);
	else if (damage < 25)
	{
		if (FRand() < 0.5) PlaySound(HitSound2, SLOT_Pain,2.0,,,Frand()*0.15+0.9);			
		else PlaySound(HitSound3, SLOT_Pain,2.0,,,Frand()*0.15+0.9);
	}
	else
		PlaySound(HitSound4, SLOT_Pain,2.0,,,Frand()*0.15+0.9);			
}

function CallForHelp()
{
	local Pawn P;

	P = Level.PawnList;
	while ( P != None )
	{
		if ( P.IsA('Bots') && (P.Team == Team) )
			Bots(P).HandleHelpMessageFrom(self);
		P = P.nextPawn;
	}
}

function HandleHelpMessageFrom(Pawn Other);

function Gasp()
{
	if ( PainTime < 2 )
		PlaySound(GaspSound, SLOT_Talk, 2.0);
	else
		PlaySound(BreathAgain, SLOT_Talk, 2.0);
}

function PlayAcquisitionSound()
{
}

function PlayFearSound()
{
}

function PlayRoamingSound()
{
}

function PlayThreateningSound()
{
}

//-----------------------------------------------------------------------------
// Bot functions

function string[64] KillMessage(name damageType, pawn Other)
{
	return ( Level.Game.PlayerKillMessage(damageType, Other)$PlayerName );
}

function PreBeginPlay()
{
	bIsPlayer = true;
	if (Orders == '')
		Orders = 'Roaming';
	Super.PreBeginPlay();
}
	
function SetFall()
{
	if (Enemy != None)
	{
		NextState = 'Attacking'; //default
		NextLabel = 'Begin';
		TweenToFalling();
		NextAnim = AnimSequence;
		GotoState('FallingState');
	}
}

event UpdateEyeHeight(float DeltaTime)
{
	local float smooth, bound;
	
	smooth = FMin(1.0, 10.0 * DeltaTime/Level.TimeDilation);
	// smooth up/down stairs
	If ( (Physics == PHYS_Walking) && !bJustLanded)
	{
		EyeHeight = (EyeHeight - Location.Z + OldLocation.Z) * (1 - smooth) + BaseEyeHeight * smooth;
		bound = -0.5 * CollisionHeight;
		if (EyeHeight < bound)
			EyeHeight = bound;
		else
		{
			bound = CollisionHeight + FMin(FMax(0.0,(OldLocation.Z - Location.Z)), MaxStepHeight); 
			 if ( EyeHeight > bound )
				EyeHeight = bound;
		}
	}
	else
	{
		smooth = FMax(smooth, 0.35); //FIXME - was 0.43, what should it be?
		bJustLanded = false;
		EyeHeight = EyeHeight * ( 1 - smooth) + BaseEyeHeight * smooth;
	}

	// also update viewrotation
	ViewRotation = Rotation;
}

/* Adjust hit location - adjusts the hit location in for pawns, and returns
true if it was really a hit, and false if not (for ducking, etc.)
*/
function bool AdjustHitLocation(out vector HitLocation, vector TraceDir)
{
	local float adjZ, maxZ;

	TraceDir = Normal(TraceDir);
	HitLocation = HitLocation + 0.5 * CollisionRadius * TraceDir;
	if ( BaseEyeHeight == Default.BaseEyeHeight )
		return true;

	maxZ = Location.Z + EyeHeight + 0.25 * CollisionHeight;
	if ( HitLocation.Z > maxZ )
	{
		if ( TraceDir.Z >= 0 )
			return false;
		adjZ = (maxZ - HitLocation.Z)/TraceDir.Z;
		HitLocation.Z = maxZ;
		HitLocation.X = HitLocation.X + TraceDir.X * adjZ;
		HitLocation.Y = HitLocation.Y + TraceDir.Y * adjZ;
		if ( VSize(HitLocation - Location) > CollisionRadius )	
			return false;
	}
	return true;
}

function bool CanFireAtEnemy()
{
	local vector HitLocation, HitNormal,X,Y,Z, projStart;
	local actor HitActor;
	
	if ( Weapon == None )
		return false;
	
	GetAxes(Rotation,X,Y,Z);
	projStart = Location + Weapon.CalcDrawOffset() + Weapon.FireOffset.X * X + 1.2 * Weapon.FireOffset.Y * Y + Weapon.FireOffset.Z * Z;
	if ( Weapon.IsA('ASMD') || Weapon.IsA('Minigun') || Weapon.IsA('Rifle') ) //instant hit
		HitActor = Trace(HitLocation, HitNormal, Enemy.Location + Enemy.CollisionHeight * vect(0,0,0.7), projStart, true);
	else
		HitActor = Trace(HitLocation, HitNormal, 
				projStart + 220 * Normal(Enemy.Location + Enemy.CollisionHeight * vect(0,0,0.7) - Location), 
				projStart, true);

	if ( HitActor == Enemy )
		return true;
	if ( (HitActor != None) && (VSize(HitLocation - Location) < 200) )
		return false;
	if ( (Pawn(HitActor) != None) && (AttitudeTo(Pawn(HitActor)) > ATTITUDE_Ignore) )
		return false;

	return true;
}

function bool Gibbed()
{
	return ( (Health < -80) || ((Health < -40) && (FRand() < 0.65)) );
}

function ChangedWeapon()
{
	local int usealt;

	if ( Weapon == PendingWeapon )
	{
		if ( Weapon == None )
			SwitchToBestWeapon();
		else if ( Weapon.GetStateName() == 'DownWeapon' ) 
			Weapon.GotoState('Idle');
		PendingWeapon = None;
	}
	else
		Super.ChangedWeapon();

	if ( Weapon != None )
	{
		if ( (bFire > 0) || (bAltFire > 0) )
		{
			Weapon.RateSelf(usealt);
			if ( usealt == 0 )
			{
				bAltFire = 0;
				bFire = 1;
				Weapon.Fire(1.0);
			}
			else
			{
				bAltFire = 0;
				bFire = 1;
				Weapon.AltFire(1.0);
			}
		}
		Weapon.SetHand(0);
	}
}

function PreSetMovement()
{
	if ( Skill == 3 )
	{
		PeripheralVision = -0.1;
		RotationRate.Yaw = 100000;
	}
	else
	{
		PeripheralVision = 0.7 - 0.35 * skill;
		RotationRate.Yaw = 30000 + 16000 * skill;
	}
	if (JumpZ > 0)
		bCanJump = true;
	bCanWalk = true;
	bCanSwim = true;
	bCanFly = false;
	MinHitWall = -0.5;
	bCanOpenDoors = true;
	bCanDoSpecial = true;
	if ( skill <= 1 )
	{
		bCanDuck = false;
		MaxDesiredSpeed = 0.8 + 0.1 * skill;
	}
	else
	{
		MaxDesiredSpeed = 1;
		bCanDuck = true;
	}
}

function PainTimer()
{
	local float depth;
	if (Health < 0)
		return;

	if (FootRegion.Zone.bPainZone)
		Super.PainTimer();
	else if (HeadRegion.Zone.bWaterZone)
	{
		if (bDrowning)
			self.TakeDamage(5, None, Location, vect(0,0,0), 'drowned'); 
		else
		{
			bDrowning = true;
			GotoState('FindAir');
		}
		if (Health > 0)
			PainTime = 2.0;
	}
}	

function AnnoyedBy(Pawn Other)
{
}

function TryToDuck(vector duckDir, bool bReversed)
{
	local vector HitLocation, HitNormal, Extent;
	local actor HitActor;
	local bool bSuccess, bDuckLeft;

	//log("duck");			
	duckDir.Z = 0;
	bDuckLeft = !bReversed;
	Extent.X = CollisionRadius;
	Extent.Y = CollisionRadius;
	Extent.Z = CollisionHeight;
	HitActor = Trace(HitLocation, HitNormal, Location + 240 * duckDir, Location, false, Extent);
	bSuccess = ( (HitActor == None) || (VSize(HitLocation - Location) > 150) );
	if ( !bSuccess )
	{
		bDuckLeft = !bDuckLeft;
		duckDir *= -1;
		HitActor = Trace(HitLocation, HitNormal, Location + 240 * duckDir, Location, false, Extent);
		bSuccess = ( (HitActor == None) || (VSize(HitLocation - Location) > 150) );
	}
	if ( !bSuccess )
		return;
	
	if ( HitActor == None )
		HitLocation = Location + 240 * duckDir; 

	HitActor = Trace(HitLocation, HitNormal, HitLocation - MaxStepHeight * vect(0,0,1), HitLocation, false, Extent);
	if (HitActor == None)
		return;
		
	//log("good duck");

	SetFall();
	Velocity = duckDir * 400;
	Velocity.Z = 160;
	PlayDodge(bDuckLeft);
	SetPhysics(PHYS_Falling);
	GotoState('FallingState','Ducking');
}

function PlayDodge(bool bDuckLeft)
{
	PlayDuck();
}

//FIXME - here decide when to pause/start firing based on weapon,etc
function PlayCombatMove()
{	
	PlayRunning();
	if ( skill > 2 )
		bReadyToAttack = true;
	if ( bMovingRangedAttack && bReadyToAttack && bCanFire )
	{
		if ( NeedToTurn(Enemy.Location) )
		{
			bAltFire = 0;
			bFire = 0;
		}
		else 
			FireWeapon(); 
	}		
	else 
	{
		bFire = 0;
		bAltFire = 0;
	}
}

function PlayMeleeAttack()
{
	//log("play melee attack");
	Acceleration = AccelRate * VRand();
	TweenToWaiting(0.15); 
	FireWeapon();
}

function PlayRangedAttack()
{
	//log("play ranged attack");
	TweenToWaiting(0.11);
	FireWeapon();
}

function PlayMovingAttack()
{
	PlayRunning();
	FireWeapon();
}

function PlayOutOfWater()
{
	PlayDuck();
}

function FireWeapon()
{
	local bool bUseAltMode;

	bUseAltMode = SwitchToBestWeapon();
	bShootSpecial = false;


	if( Weapon!=None )
	{
		if ( (Weapon.AmmoType != None) && (Weapon.AmmoType.AmmoAmount <= 0) )
		{
			bReadyToAttack = true;
			return;
		}

		ViewRotation = Rotation;
		if ( bUseAltMode )
		{
			bFire = 0;
			bAltFire = 1;
			Weapon.AltFire(1.0);
		}
		else
		{
			bFire = 1;
			bAltFire = 0;
			Weapon.Fire(1.0);
		}
		PlayFiring();
	}
	if ( Target != Enemy )
		Target = Enemy;
}

function WhatToDoNext(name LikelyState, name LikelyLabel)
{
	bFire = 0;
	bAltFire = 0;
	bReadyToAttack = false;
	Enemy = None;
	if ( OldEnemy != None )
	{
		Enemy = OldEnemy;
		OldEnemy = None;
		GotoState('Attacking');
	}
	else if ( (LikelyState != 'Waiting') && (LikelyState != '') )
		GotoState(LikelyState, LikelyLabel);
	else
	{
		OrderObject = None;
		OrderTag = '';
		GotoState('Roaming');
		if ( Skill > 2.7 )
			bReadyToAttack = true; 
	}
}

function Killed(pawn Killer, pawn Other, name damageType)
{
	if (Other == Enemy)
	{
		bFire = 0;
		bAltFire = 0;
		bReadyToAttack = ( skill > 3 * FRand() );
		EnemyDropped = Enemy.Weapon;
		Enemy = None;
		GotoState('Attacking');
	}
	//log(Other$" killed");
}	

function ReSetSkill()
{
	bLeadTarget = (1.5 * FRand() < Skill);
	if ( Skill == 0 )
	{
		Health = 80;
		ReFireRate = 0.75 * Default.ReFireRate;
	}
	else
		ReFireRate = Default.ReFireRate * (1 - 0.25 * skill);

	PreSetMovement();
}

//===============================================================================
// Bot states 

state GameEnded
{
ignores SeePlayer, EnemyNotVisible, HearNoise, TakeDamage, Died, Bump, Trigger, HitWall, HeadZoneChange, FootZoneChange, ZoneChange, Falling, WarnTarget;

}

state Dying
{
ignores SeePlayer, EnemyNotVisible, HearNoise, Died, Bump, Trigger, HitWall, HeadZoneChange, FootZoneChange, ZoneChange, Falling, WarnTarget;

	function ReStartPlayer()
	{
		if( bHidden && Level.Game.RestartPlayer(self) )
		{
			Velocity = vect(0,0,0);
			Acceleration = vect(0,0,0);
			ViewRotation = Rotation;
			ReSetSkill();
			SetPhysics(PHYS_Falling);
			GotoState('Roaming');
		}
	}
	
	function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
							Vector momentum, name damageType)
	{
		if ( !bHidden )
			Super.TakeDamage(Damage, instigatedBy, hitlocation, momentum, damageType);
	}
	
	function BeginState()
	{
		SetTimer(0, false);
		Enemy = None;
		bFire = 0;
		bAltFire = 0;
	}

Begin:
	Sleep(0.2);
	if ( !bHidden )
	{
		SpawnCarcass();
		HidePlayer();
	}
TryAgain:
	Sleep(0.25 + DeathMatchGame(Level.Game).NumBots * FRand());
	ReStartPlayer();
	Goto('TryAgain');
}


state FallingState 
{
ignores Bump, Hitwall, HearNoise, WarnTarget;

	function Timer()
	{
		if ( Enemy != None )
		{
			bReadyToAttack = true;
			if ( CanFireAtEnemy() )
				GotoState('FallingState', 'FireWhileFalling');
		}
	}

	function Landed(vector HitNormal)
	{
		//Note - physics changes type to PHYS_Walking by default for landed pawns
		//log("Player landed w/ "$Velocity);
		PlayLanded(Velocity.Z);
		if (Velocity.Z < -1.4 * JumpZ)
		{
			MakeNoise(-0.5 * Velocity.Z/(FMax(JumpZ, 150.0)));
			if (Velocity.Z <= -1100)
			{
				if ( (Velocity.Z < -2000) && (ReducedDamageType != 'All') )
				{
					health = -1000; //make sure gibs
					Died(None, 'fell', Location);
				}
				else if ( Role == ROLE_Authority )
					TakeDamage(-0.15 * (Velocity.Z + 1050), None, Location, vect(0,0,0), 'fell');
			}
			GotoState('FallingState', 'Landed');
		}
		else 
			GotoState('FallingState', 'Done');
	}

	function BeginState()
	{
		Super.BeginState();
		if ( (bFire > 0) || (bAltFire > 0) || (Skill == 3) )
			SetTimer(0.01, false);
	}

FireWhileFalling:
	if ( Physics != PHYS_Falling )
		Goto('Done');
	TurnToward(Enemy);
	if ( CanFireAtEnemy() )
		FireWeapon();
	Sleep(0.9 + 0.2 * FRand());
	Goto('FireWhileFalling');
}			

state MeleeAttack
{
ignores SeePlayer, HearNoise, Bump;

	function KeepAttacking()
	{
		if ( (Enemy == None) || (Enemy.Health <= 0)
			|| (VSize(Enemy.Location - Location) > (0.9 * MeleeRange + Enemy.CollisionRadius + CollisionRadius)) ) 
			GotoState('Attacking');
		else 
		{
			bReadyToAttack = true;
			SetTimer(TimeBetweenAttacks, false);
			GotoState('TacticalMove', 'NoCharge');
		}
	}
	
	function BeginState()
	{
		Target = Enemy;
		Disable('AnimEnd');
		bCanJump = false;
	}
	
	function EndState()
	{
		bCanJump = (JumpZ > 0);
	}			
}

state RangedAttack
{
ignores SeePlayer, HearNoise;

	function KeepAttacking()
	{
		if ( bFiringPaused )
			return;
		if ( (Enemy == None) || (Enemy.Health <= 0) || !LineOfSightTo(Enemy) )
		{
			bFire = 0;
			bAltFire = 0; 
			GotoState('Attacking');
		}
		else if ( Skill > 3.5 * FRand() - 0.5 )
		{
			bReadyToAttack = true;
			GotoState('TacticalMove');
		}	
	}


	function AnimEnd()
	{
		local float decision;

		decision = FRand() - 0.27 * skill - 0.1;
		if ( (bFire == 0) && (bAltFire == 0) )
			decision = decision - 0.5;
		if ( decision < 0 )
			GotoState('RangedAttack', 'DoneFiring');
		else
		{
			PlayWaiting();
			FireWeapon();
		}
	}

	function BeginState()
	{
		Target = Enemy;
		Disable('AnimEnd');
		if ( bFiringPaused )
		{
			SetTimer(SpecialPause, false);
			SpecialPause = 0;
		}
	}
}

state Hunting
{
ignores EnemyNotVisible; 

	function AnimEnd()
	{
		PlayRunning();
		bFire = 0;
		bAltFire = 0;
		bReadyToAttack = true;
		Disable('AnimEnd');
	}


	function Bump(actor Other)
	{
		//log(Other.class$" bumped "$class);
		if (Pawn(Other) != None)
		{
			if (Enemy == Other)
				bReadyToAttack = True; //can melee right away
			SetEnemy(Pawn(Other));
			LastSeenPos = Enemy.Location;
		}
		setTimer(2.0, false);
		Disable('Bump');
	}
	
}

//FIXME - improve FindAir (use paths)
state FindAir
{
ignores SeePlayer, HearNoise, Bump;

	function HeadZoneChange(ZoneInfo newHeadZone)
	{
		Global.HeadZoneChange(newHeadZone);
		if (!newHeadZone.bWaterZone)
			GotoState('Attacking');
	}

	function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
	{
		Super.TakeDamage(Damage, instigatedBy, hitlocation, momentum, damageType);
		if ( health <= 0 )
			return;
		if ( NextState == 'TakeHit' )
		{
			NextState = 'FindAir'; 
			NextLabel = 'TakeHit';
			GotoState('TakeHit'); 
		}
	}

	function HitWall(vector HitNormal, actor Wall)
	{
		//change directions
		Destination = 200 * (Normal(Destination - Location) + HitNormal);
	}

	function AnimEnd() 
	{
		if (Enemy != None)
			PlayCombatMove();
		else
			PlayRunning();
	}

	function Timer()
	{
		bReadyToAttack = True;
		settimer(0.5, false);
	}

	function EnemyNotVisible()
	{
		////log("enemy not visible");
		bReadyToAttack = false;
	}

/* PickDestination()
*/
	function PickDestination(bool bNoCharge)
	{
		Destination = VRand();
		Destination.Z = 1;
		Destination = Location + 200 * Destination;				
	}

Begin:
	//log("Find air");
	TweenToRunning(0.2);
	Enable('AnimEnd');
	PickDestination(false);

DoMove:	
	if ( Enemy == None )
		MoveTo(Destination);
	else
	{
		bCanFire = true;
		StrafeFacing(Destination, Enemy);	
	}
	GotoState('Attacking');

TakeHit:
	TweenToRunning(0.15);
	Goto('DoMove');

}

state TacticalMove
{
ignores SeePlayer, HearNoise;

	function EnemyNotVisible()
	{
		if ( !bGathering && (aggressiveness > relativestrength(enemy)) )
		{
			if (ValidRecovery())
				GotoState('TacticalMove','RecoverEnemy');
			else
				GotoState('Attacking');
		}
		Disable('EnemyNotVisible');
	}

	function PickDestination(bool bNoCharge)
	{
		local inventory Inv, BestInv, SecondInv;
		local float Bestweight, NewWeight, MaxDist, SecondWeight;

		// possibly pick nearby inventory
		// higher skill bots will always strafe, lower skill
		// both do this less, and strafe less

		if ( !bReadyToAttack && (TimerRate == 0.0) )
			SetTimer(0.7, false);
		if ( LastInvFind - Level.TimeSeconds < 2.5 - 0.5 * skill )
		{
			Super.PickDestination(bNoCharge);
			return;
		}

		LastInvFind = Level.TimeSeconds;
		bGathering = false;
		BestWeight = 0;
		MaxDist = 600 + 70 * skill;
		foreach visiblecollidingactors(class'Inventory', Inv, MaxDist)
			if ( (Inv.IsInState('PickUp')) && (Inv.MaxDesireability/200 > BestWeight)
				&& (Inv.Location.Z < Location.Z + MaxStepHeight + CollisionHeight)
				&& (Inv.Location.Z > FMin(Location.Z, Enemy.Location.Z) - CollisionHeight) )
			{
				NewWeight = inv.BotDesireability(self)/VSize(Inv.Location - Location);
				if ( NewWeight > BestWeight )
				{
					SecondWeight = BestWeight;
					BestWeight = NewWeight;
					SecondInv = BestInv;
					BestInv = Inv;
				}
			}

		if ( BestInv == None )
		{
			Super.PickDestination(bNoCharge);
			return;
		}

		if ( TryToward(BestInv, BestWeight) )
			return;

		if ( SecondInv == None )
		{
			Super.PickDestination(bNoCharge);
			return;
		}

		if ( TryToward(SecondInv, SecondWeight) )
			return;

		Super.PickDestination(bNoCharge);
	}

	function bool TryToward(inventory Inv, float Weight)
	{
		if ( (Weight < 0.001) && ((Weight < 0.001 - 0.0002 * skill) 
				|| !Enemy.LineOfSightTo(Inv)) )
			return false;

		if ( ActorReachable(Inv) )
		{
			Destination = Inv.Location;
			bGathering = true;
			if ( 2.7 * FRand() < skill )
				GotoState('TacticalMove','DoStrafeMove');
			else
				GotoState('TacticalMove','DoDirectMove');
			return true;
		}

		return false;
	}

	function PainTimer()
	{
		if ( (FootRegion.Zone.bPainZone) && (FootRegion.Zone.DamagePerSec > 0) )
			GotoState('Retreating');
		Super.PainTimer();
	}

}

/* Retreating for a bot is going toward an item while still engaged with an enemy, but fearing that enemy (so
no desire to remain engaged)
   TacticalGet is for going to an item while engaged, and remaining engaged. TBD
   Roaming is going to items w/ no enemy. TBD
*/

state Retreating
{
ignores EnemyNotVisible;

	function SeePlayer(Actor SeenPlayer)
	{
		if ( (SeenPlayer == Enemy) || LineOfSightTo(Enemy) )
			return;
		if ( SetEnemy(Pawn(SeenPlayer)) )
		{
			LastSeenPos = SeenPlayer.Location;
			MakeNoise(1.0);
			GotoState('Attacking');
		}
	}

	function HearNoise(float Loudness, Actor NoiseMaker)
	{
		if ( (NoiseMaker.instigator == Enemy) || LineOfSightTo(Enemy) )
			return;

		if ( SetEnemy(NoiseMaker.instigator) )
		{
			LastSeenPos = 0.5 * (NoiseMaker.Location + VSize(NoiseMaker.Location - Location) * vector(Rotation));
			MakeNoise(1.0);
			GotoState('Attacking');
		}
	}

	function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
							Vector momentum, name damageType)
	{
		Global.TakeDamage(Damage, instigatedBy, hitlocation, momentum, damageType);
		if ( health <= 0 )
			return;
		if (NextState == 'TakeHit')
		{
			NextState = 'Retreating'; 
			NextLabel = 'TakeHit';
			GotoState('TakeHit'); 
		}
		else if ( !bCanFire && (skill > 3 * FRand()) )
			GotoState('Retreating', 'Moving');
	}

	function Timer()
	{
		bReadyToAttack = True;
		Enable('Bump');
	}
	
	function SetFall()
	{
		NextState = 'Retreating'; 
		NextLabel = 'Landed';
		NextAnim = AnimSequence;
		GotoState('FallingState'); 
	}

	function HitWall(vector HitNormal, actor Wall)
	{
		if (Physics == PHYS_Falling)
			return;
		if ( Wall.IsA('Mover') && Mover(Wall).HandleDoor(self) )
		{
			if ( SpecialPause > 0 )
				Acceleration = vect(0,0,0);
			GotoState('Retreating', 'SpecialNavig');
			return;
		}
		Focus = Destination;
		if (PickWallAdjust())
			GotoState('Retreating', 'AdjustFromWall');
		else
			MoveTimer = -1.0;
	}

	function PickDestination()
	{
	 	local inventory Inv, BestInv, SecondInv;
		local float Bestweight, NewWeight, invDist, MaxDist, SecondWeight;
		local actor BestPath;
		local bool bTriedFar;

		if ( !bReadyToAttack && (TimerRate == 0.0) )
			SetTimer(0.7, false);

		// do I still fear my enemy?
		if ( (Enemy == None) || (AttitudeTo(Enemy) > ATTITUDE_Fear) )
		{
			GotoState('Attacking');
			return;
		}

		bestweight = 0;

		//first look at nearby inventory < 500 dist
		// FIXME reduce favoring of stuff nearer/visible to enemy
		MaxDist = 500 + 70 * skill;
		foreach visiblecollidingactors(class'Inventory', Inv, MaxDist)
			if ( (Inv.IsInState('PickUp')) && (Inv.MaxDesireability/200 > BestWeight)
				&& (Inv.Location.Z < Location.Z + MaxStepHeight + CollisionHeight)
				&& (Inv.Location.Z > FMin(Location.Z, Enemy.Location.Z) - CollisionHeight) )
			{
				NewWeight = inv.BotDesireability(self)/VSize(Inv.Location - Location);
				if ( NewWeight > BestWeight )
				{
					SecondWeight = BestWeight;
					BestWeight = NewWeight;
					SecondInv = BestInv;
					BestInv = Inv;
				}
			}

		 // see if better long distance inventory 
		if ( BestWeight < 0.2 )
		{ 
			bTriedFar = true;
			BestPath = FindBestInventoryPath(BestWeight, false);
			if ( BestPath != None )
			{
				MoveTarget = BestPath;
				return;
			}
		}

		 // if nothing, then tactical move
		if ( (BestInv != None) && ActorReachable(BestInv) )
		{
			MoveTarget = BestInv;
			return;
		}

		if ( (SecondInv != None) && ActorReachable(SecondInv) )
		{
			MoveTarget = BestInv;
			return;
		}
		if ( !bTriedFar )
		{ 
			BestWeight = 0;
			BestPath = FindBestInventoryPath(BestWeight, false);
			if ( BestPath != None )
			{
				MoveTarget = BestPath;
				return;
			}
		}

		LastInvFind = Level.TimeSeconds;
		GotoState('TacticalMove', 'NoCharge');
	}

	function Bump(actor Other)
	{
		local vector VelDir, OtherDir;
		local float speed;

		//log(Other.class$" bumped "$class);
		if (Pawn(Other) != None)
		{
			if ( (Other == Enemy) || SetEnemy(Pawn(Other)) )
			{
				bReadyToAttack = true;
				GotoState('Attacking');
			}
			return;
		}
		if ( TimerRate <= 0 )
			setTimer(1.0, false);
		speed = VSize(Velocity);
		if ( speed > 1 )
		{
			VelDir = Velocity/speed;
			VelDir.Z = 0;
			OtherDir = Other.Location - Location;
			OtherDir.Z = 0;
			OtherDir = Normal(OtherDir);
			if ( (VelDir Dot OtherDir) > 0.9 )
			{
				Velocity.X = VelDir.Y;
				Velocity.Y = -1 * VelDir.X;
				Velocity *= FMax(speed, 200);
			}
		}
		Disable('Bump');
	}

	function AnimEnd() 
	{
		if ( bCanFire && LineOfSightTo(Enemy) )
			PlayCombatMove();
		else
			PlayRunning();
	}

	function BeginState()
	{
		CallForHelp();
		bCanFire = false;
		SpecialGoal = None;
		SpecialPause = 0.0;
	}

Begin:
	//log(class$" retreating");
	if ( (TimerRate == 0.0) || (bReadyToAttack && (FRand() < 0.4)) )
	{
		SetTimer(TimeBetweenAttacks, false);
		bReadyToAttack = false;
	}
	TweenToRunning(0.15);
	WaitForLanding();
	
RunAway:
	PickDestination();
SpecialNavig:
	if (SpecialPause > 0.0)
	{
		if ( LineOfSightTo(Enemy) )
		{
			bFiringPaused = true;
			NextState = 'Retreating';
			NextLabel = 'Moving';
			GotoState('RangedAttack');
		}
		Disable('AnimEnd');
		Acceleration = vect(0,0,0);
		TweenToPatrolStop(0.3);
		Sleep(SpecialPause);
		SpecialPause = 0.0;
		Enable('AnimEnd');
		TweenToRunning(0.1);
	}
Moving:
	if ( MoveTarget == None )
	{
		Sleep(0.0);
		Goto('RunAway');
	}
	if ( MoveTarget.IsA('InventorySpot') 
		&& (InventorySpot(MoveTarget).markedItem.GetStateName() == 'Pickup') )
			MoveTarget = InventorySpot(MoveTarget).markedItem;
	if ( (skill < 3) && (!LineOfSightTo(Enemy) ||
		(Skill - 2 * FRand() + (Normal(Enemy.Location - Location - vect(0,0,1) * (Enemy.Location.Z - Location.Z)) 
			Dot Normal(MoveTarget.Location - Location - vect(0,0,1) * (MoveTarget.Location.Z - Location.Z))) < 0)) )
	{
		bCanFire = false;
		MoveToward(MoveTarget);
	}
	else
	{
		bCanFire = true;
		StrafeFacing(MoveTarget.Location, Enemy);
	}
	Goto('RunAway');

Landed:
	AnimEnd();
	Goto('Moving');

TakeHit:
	TweenToRunning(0.12);
	Goto('Moving');

AdjustFromWall:
	StrafeTo(Destination, Focus); 
	MoveTo(Destination);
	Goto('Moving');
}

state Roaming
{
ignores EnemyNotVisible;

	function HandleHelpMessageFrom(Pawn Other)
	{
		if ( (Health > 60) && (Weapon.AIRating > 3) && (Other.Team == Team)
			&& (Other.Enemy != None)
			&& (VSize(Other.Enemy.Location - Location) < 1500) )
		{
			SetEnemy(Other.Enemy);
			GotoState('Attacking');
		}
	}

	function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
							Vector momentum, name damageType)
	{
		Global.TakeDamage(Damage, instigatedBy, hitlocation, momentum, damageType);
		if ( health <= 0 )
			return;
		if (NextState == 'TakeHit')
		{
			NextState = 'Attacking'; 
			NextLabel = '';
			GotoState('TakeHit'); 
		}
		else if ( !bCanFire && (skill > 3 * FRand()) )
			GotoState('Attacking');
	}

	function Timer()
	{
		bReadyToAttack = True;
		Enable('Bump');
	}
	
	function SetFall()
	{
		NextState = 'Roaming'; 
		NextLabel = 'Landed';
		NextAnim = AnimSequence;
		GotoState('FallingState'); 
	}

	function HitWall(vector HitNormal, actor Wall)
	{
		if (Physics == PHYS_Falling)
			return;
		if ( Wall.IsA('Mover') && Mover(Wall).HandleDoor(self) )
		{
			if ( SpecialPause > 0 )
				Acceleration = vect(0,0,0);
			GotoState('Roaming', 'SpecialNavig');
			return;
		}
		Focus = Destination;
		if (PickWallAdjust())
			GotoState('Roaming', 'AdjustFromWall');
		else
			MoveTimer = -1.0;
	}

	function PickDestination()
	{
		local inventory Inv, BestInv, KnowPath;
		local float Bestweight, NewWeight, DroppedDist;
		local actor BestPath;
		local actor HitActor;
		local vector HitNormal, HitLocation;
		local decoration Dec;

		if ( (EnemyDropped != None) && !EnemyDropped.bDeleteMe 
			&& (EnemyDropped.Owner == None) )
		{
			DroppedDist = VSize(EnemyDropped.Location - Location);
			if ( (DroppedDist < 800) && ActorReachable(EnemyDropped) )
			{
				BestWeight = EnemyDropped.BotDesireability(self); 		
				if ( BestWeight > 0.4 )
				{
					MoveTarget = EnemyDropped;
					EnemyDropped = None;
					return; 
				}
				BestInv = EnemyDropped;
				BestWeight = BestWeight/DroppedDist;
				KnowPath = BestInv;
			}	
			else
				BestWeight = 0;
		}	
		else
			BestWeight = 0;

		EnemyDropped = None;
									
		//first look at nearby inventory < 600 dist
		foreach visiblecollidingactors(class'Inventory', Inv, 600)
			if ( (Inv.IsInState('PickUp')) && (Inv.MaxDesireability/50 > BestWeight)
				&& (Inv.Location.Z < Location.Z + MaxStepHeight + CollisionHeight) )
			{
				NewWeight = inv.BotDesireability(self)/VSize(Inv.Location - Location);
				if ( NewWeight > BestWeight )
				{
					BestWeight = NewWeight;
					BestInv = Inv;
				}
			}

		if ( (BestInv != None) && ActorReachable(BestInv) )
		{
			MoveTarget = BestInv;
			return;
		}
		else if ( KnowPath != None )
		{
			MoveTarget = KnowPath;
			return;
		}

		BestWeight = 0;

		// if none found, check for decorations with inventory
		if ( !bNoShootDecor )
			foreach visiblecollidingactors(class'Decoration', Dec, 500)
				if ( Dec.Contents != None )
				{
					bNoShootDecor = true;
					Target = Dec;
					GotoState('Roaming', 'ShootDecoration');
					return;
				}

		bNoShootDecor = false;

		// look for long distance inventory 
		BestPath = FindBestInventoryPath(BestWeight, (skill >= 2));
		if ( BestPath != None )
		{
			MoveTarget = BestPath;
			return;
		}

		 // if nothing, then wander or camp
		if ( FRand() < 0.35 )
			GotoState('Wandering');
		else
			GotoState('Roaming', 'Camp');
	}

	function Bump(actor Other)
	{
		local vector VelDir, OtherDir;
		local float speed;

		//log(Other.class$" bumped "$class);
		if (Pawn(Other) != None)
		{
			if ( (Other == Enemy) || SetEnemy(Pawn(Other)) )
			{
				bReadyToAttack = true;
				GotoState('Attacking');
			}
			return;
		}
		if ( TimerRate <= 0 )
			setTimer(1.0, false);
		speed = VSize(Velocity);
		if ( speed > 1 )
		{
			VelDir = Velocity/speed;
			VelDir.Z = 0;
			OtherDir = Other.Location - Location;
			OtherDir.Z = 0;
			OtherDir = Normal(OtherDir);
			if ( (VelDir Dot OtherDir) > 0.9 )
			{
				Velocity.X = VelDir.Y;
				Velocity.Y = -1 * VelDir.X;
				Velocity *= FMax(speed, 200);
			}
		}
		Disable('Bump');
	}

	function AnimEnd() 
	{
		if ( bCamping )
			PlayWaiting();
		else
			PlayRunning();
	}

	function BeginState()
	{
		bNoShootDecor = false;
		bCanFire = false;
		SpecialGoal = None;
		SpecialPause = 0.0;
	}

	function EndState()
	{
		Super.EndState();
		bCamping = false;
	}

Camp:
	bCamping = true;
	Acceleration = vect(0,0,0);
	TweenToWaiting(0.15);
	if ( NearWall(200) )
	{
		PlayTurning();
		TurnTo(Focus);
	}
	Sleep(3.5 + FRand() - skill);
	if ( (Weapon != None) && (Weapon.AIRating > 0.4) && (3 * FRand() > skill + 1) )
		Goto('Camp');
Begin:
	bCamping = false;
	TweenToRunning(0.1);
	WaitForLanding();
	
RunAway:
	PickDestination();
SpecialNavig:
	if (SpecialPause > 0.0)
	{
		Disable('AnimEnd');
		Acceleration = vect(0,0,0);
		TweenToPatrolStop(0.3);
		Sleep(SpecialPause);
		SpecialPause = 0.0;
		Enable('AnimEnd');
		TweenToRunning(0.1);
	}
Moving:
	if ( MoveTarget == None )
	{
		Acceleration = vect(0,0,0);
		Sleep(0.1);
		Goto('RunAway');
	}
	if ( MoveTarget.IsA('InventorySpot') )
	{
		if ( InventorySpot(MoveTarget).markedItem.GetStateName() == 'Pickup' )
			MoveTarget = InventorySpot(MoveTarget).markedItem;
		else if ( VSize(Location - MoveTarget.Location) < CollisionRadius )
			Goto('Camp');
	}
	bCamping = false;
	MoveToward(MoveTarget);
	Goto('RunAway');

TakeHit:
	TweenToRunning(0.12);
	Goto('Moving');

Landed:
	AnimEnd();
	Goto('Moving');

AdjustFromWall:
	bCamping = false;
	StrafeTo(Destination, Focus); 
	MoveTo(Destination);
	Goto('Moving');

ShootDecoration:
	TurnToward(Target);
	if ( Target != None )
	{
		FireWeapon();
		bAltFire = 0;
		bFire = 0;
	}
	Goto('RunAway');
}

state Wandering
{
ignores EnemyNotVisible;

Begin:
	//log(class$" Wandering");

Wander: 
	WaitForLanding();
	PickDestination();
	TweenToWalking(0.2);
	FinishAnim();
	PlayWalking();
	
Moving:
	Enable('HitWall');
	MoveTo(Destination, WalkingSpeed);
Pausing:
	Acceleration = vect(0,0,0);
	if ( NearWall(200) )
	{
		PlayTurning();
		TurnTo(Focus);
	}
	if (FRand() < 0.3)
		PlayRoamingSound();
	Enable('AnimEnd');
	NextAnim = '';
	TweenToPatrolStop(0.2);
	Sleep(1.0);
	Disable('AnimEnd');
	FinishAnim();
	GotoState('Roaming');

ContinueWander:
	FinishAnim();
	PlayWalking();
	if ( !bQuiet && (FRand() < 0.3) )
		PlayRoamingSound();
	if (FRand() < 0.2)
		Goto('Turn');
	Goto('Wander');

Turn:
	Acceleration = vect(0,0,0);
	PlayTurning();
	TurnTo(Location + 20 * VRand());
	Goto('Pausing');

AdjustFromWall:
	StrafeTo(Destination, Focus); 
	Destination = Focus; 
	Goto('Moving');
}

defaultproperties
{
     SightRadius=+03000.000000
     Aggressiveness=+00000.200000
     ReFireRate=+00000.900000
     bHasRangedAttack=True
     bMovingRangedAttack=True
     BaseEyeHeight=+00023.000000
     UnderWaterTime=+00020.000000
     bCanStrafe=True
	 bAutoActivate=True
     MeleeRange=+00050.000000
     Intelligence=BRAINS_HUMAN
     GroundSpeed=+00400.000000
     AirSpeed=+00400.000000
     AccelRate=+02048.000000
     MaxStepHeight=+00025.000000
     CombatStyle=+00000.20000
     DrawType=DT_Mesh
     LightBrightness=70
     LightHue=40
     LightSaturation=128
     LightRadius=6
	 bStasis=false
     Buoyancy=+00100.000000
     RotationRate=(Pitch=3072,Yaw=30000,Roll=2048)
     NetPriority=+00008.000000
}
