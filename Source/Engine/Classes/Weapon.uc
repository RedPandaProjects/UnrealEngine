//=============================================================================
// Parent class of all weapons.
//=============================================================================
class Weapon expands Inventory
	abstract
	localized
	intrinsic;

#exec Texture Import File=Textures\Weapon.pcx Name=S_Weapon Mips=Off Flags=2

//-----------------------------------------------------------------------------
// Weapon ammo/firing information:
// Two-element arrays here are defined for normal fire (0) and alt fire (1).
var() float   MaxTargetRange;    // Maximum distance to target.
var() class<ammo> AmmoName;          // Type of ammo used.
var() byte    ReloadCount;       // Amount of ammo depletion before reloading. 0 if no reloading is done.
var() int     PickupAmmoCount;   // Amount of ammo initially in pick-up item.
var travel ammo	AmmoType;		 // Inventory Ammo being used.
var	  bool	  bPointing;		 // Indicates weapon is being pointed
var() bool	  bInstantHit;		 // If true, instant hit rather than projectile firing weapon
var() bool	  bAltInstantHit;	 // If true, instant hit rather than projectile firing weapon for AltFire
var(WeaponAI) bool	  bWarnTarget;		 // When firing projectile, warn the target
var(WeaponAI) bool	  bAltWarnTarget;	 // When firing alternate projectile, warn the target
var   bool	  bWeaponUp;		 // Used in Active State
var   bool	  bChangeWeapon;	 // Used in Active State
var   bool 	  bLockedOn;
var(WeaponAI) bool	  bSplashDamage;	 // used by bot AI
//var	  bool	  bFireMem;
//var	  bool	  bAltFireMem;

var()   vector	FireOffset;		 // Offset from drawing location for projectile/trace start
var     class<projectile> ProjectileClass;
var     class<projectile> AltProjectileClass;
var		float	ProjectileSpeed;
var		float	AltProjectileSpeed;
var		float	AimError;		// Aim Error for bots (note this value doubled if instant hit weapon)
var()	float	ShakeMag;
var()	float	ShakeTime;
var()	float   ShakeVert;
var(WeaponAI)	float	AIRating;
var(WeaponAI)	float	RefireRate;
var(WeaponAI)	float	AltRefireRate;

//-----------------------------------------------------------------------------
// Sound Assignments
var() sound 	FireSound;
var() sound 	AltFireSound;
var() sound 	CockingSound;
var() sound 	SelectSound;
var() sound 	Misc1Sound;
var() sound 	Misc2Sound;
var() sound 	Misc3Sound;

var() Localized String[64] MessageNoAmmo;

var Rotator AdjustedAim;

// Network replication
//
replication
{
	// Things the server should send to the client.
	reliable if( Role==ROLE_Authority && bNetOwner )
		AmmoType, bPointing, bLockedOn;
}

//=============================================================================
// Inventory travelling across servers.

event TravelPostAccept()
{
	Super.TravelPostAccept();
	if ( Pawn(Owner) == None )
		return;
	if ( AmmoName != None )
	{
		AmmoType = Ammo(Pawn(Owner).FindInventoryType(AmmoName));
		if ( AmmoType == None )
		{		
			AmmoType = Spawn(AmmoName);	// Create ammo type required		
			Pawn(Owner).AddInventory(AmmoType);		// and add to player's inventory
			AmmoType.BecomeItem();
			AmmoType.AmmoAmount = PickUpAmmoCount; 
			AmmoType.GotoState('Idle2');
		}
	}
	if ( self == Pawn(Owner).Weapon )
	{
		if ( Owner.IsA('PlayerPawn') )
			SetHand(PlayerPawn(Owner).Handedness);
		BringUp();
	}
	else GoToState('Idle2');
}

//-------------------------------------------------------
// AI related functions

function PostBeginPlay()
{
	Super.PostBeginPlay();
	MaxDesireability = AIRating;
}

event float BotDesireability(Pawn Bot)
{
	local Weapon AlreadyHas;

	AlreadyHas = Weapon(Bot.FindInventoryType(class)); 
	if ( AlreadyHas != None )
	{
		if ( Level.Game.bCoopWeaponMode )
			return 0;
		if ( AlreadyHas.AmmoType == None )
			return 0.35 * MaxDesireability;

		if ( AlreadyHas.AmmoType.AmmoAmount > 0 )
			return FMax( 0.35 * MaxDesireability, 
					AlreadyHas.AmmoType.MaxDesireability * 0.25 
					* FMin( 8.0, AlreadyHas.AmmoType.MaxAmmo/AlreadyHas.AmmoType.AmmoAmount) ); 
	}
	if ( (Bot.Weapon == None) || (Bot.Weapon.AIRating < 0.3) )
		return 2*MaxDesireability;

	return MaxDesireability;
}

function float RateSelf( out int bUseAltMode )
{
	if ( (AmmoType != None) && (AmmoType.AmmoAmount <=0) )
		return -2;
	bUseAltMode = int(FRand() < 0.4);
	return (AIRating + FRand() * 0.05);
}

// return delta to combat style
function float SuggestAttackStyle()
{
	return 0.0;
}

function float SuggestDefenseStyle()
{
	return 0.0;
}

//-------------------------------------------------------

function bool HandlePickupQuery( inventory Item )
{
	local int oldAmmo;

	if (Item.Class == Class)
	{
		oldAmmo = AmmoType.AmmoAmount;
		if (AmmoType!=None && AmmoType.AddAmmo(PickupAmmoCount)) 
		{
			Pawn(Owner).ClientMessage(PickupMessage);
			if ( (OldAmmo == 0) && (Pawn(Owner).Weapon.class != item.class) && !Pawn(Owner).bNeverSwitchOnPickup )
			{
				if ( !WeaponSet(Pawn(Owner)) ) Item.PlaySound(Item.PickupSound);
			}
			else Item.PlaySound(Item.PickupSound);		   
		}
		Item.SetRespawn();   
		return true;
	}
	if ( Inventory == None )
		return false;

	return Inventory.HandlePickupQuery(Item);
}

// set which hand is holding weapon
// set which hand is holding weapon
function setHand(float Hand)
{
	local Rotator newRot;

	PlayerViewOffset.Y = Default.PlayerViewOffset.Y * Hand;
	if ( Hand == 0 )
	{
		PlayerViewOffset.X = Default.PlayerViewOffset.X * 0.88;
		PlayerViewOffset.Z = Default.PlayerViewOffset.Z * 1.12;
	}
	else
	{
		PlayerViewOffset.X = Default.PlayerViewOffset.X;
		PlayerViewOffset.Z = Default.PlayerViewOffset.Z;
	}
	PlayerViewOffset *= 100; //scale since network passes vector components as ints
	FireOffset.Y = Default.FireOffset.Y * Hand;
	newRot = Rotation;
	newRot.Roll = Default.Rotation.Roll * Hand;
	setRotation(newRot);
}

//
// Change weapon to that specificed by F matching inventory weapon's Inventory Group.
function Weapon WeaponChange( byte F )
{	
	local Weapon newWeapon;
	 
	if ( InventoryGroup == F )
	{
		if ( (AmmoType != None) && (AmmoType.AmmoAmount <= 0) )
		{
			if ( Inventory == None )
				newWeapon = None;
			else
				newWeapon = Inventory.WeaponChange(F);
			if ( newWeapon == None )
				Pawn(Owner).ClientMessage( class$MessageNoAmmo );		
			return newWeapon;
		}		
		else 
			return self;
	}
	else if ( Inventory == None )
		return None;
	else
		return Inventory.WeaponChange(F);
}

//
// Spawn a copy of this weapon and give it to the player Other.
// Either destroy original, or sets it up to be respawned.
// Also add Ammo to Other's inventory if it doesn't already exist
//
function inventory SpawnCopy( pawn Other )
{
	local Weapon newWeapon;

	newWeapon = Weapon(Super.SpawnCopy(Other));
	newWeapon.Instigator = Other;
	newWeapon.GiveAmmo(Other);
	newWeapon.SetSwitchPriority(Other);
	if ( !Other.bNeverSwitchOnPickup )
		newWeapon.WeaponSet(Other);
	newWeapon.AmbientGlow = 0;
	return newWeapon;
}

function SetSwitchPriority(pawn Other)
{
	local int i;
	local name temp, carried;

	if ( PlayerPawn(Other) != None )
	{
		for ( i=0; i<20; i++)
			if ( PlayerPawn(Other).WeaponPriority[i] == class.name )
			{
				AutoSwitchPriority = i;
				return;
			}
		// else, register this weapon
		carried = class.name;
		for ( i=AutoSwitchPriority; i<20; i++ )
		{
			if ( PlayerPawn(Other).WeaponPriority[i] == '' )
			{
				PlayerPawn(Other).WeaponPriority[i] = carried;
				return;
			}
			else if ( i<19 )
			{
				temp = PlayerPawn(Other).WeaponPriority[i];
				PlayerPawn(Other).WeaponPriority[i] = carried;
				carried = temp;
			}
		}
	}		
}

function GiveAmmo( Pawn Other )
{
	if ( AmmoName == None )
		return;
	AmmoType = Ammo(Other.FindInventoryType(AmmoName));
	if ( AmmoType != None )
		AmmoType.AddAmmo(PickUpAmmoCount);
	else
	{
		AmmoType = Spawn(AmmoName);	// Create ammo type required		
		Other.AddInventory(AmmoType);		// and add to player's inventory
		AmmoType.BecomeItem();
		AmmoType.AmmoAmount = PickUpAmmoCount; 
		AmmoType.GotoState('Idle2');
	}
}	

// Return the switch priority of the weapon (normally AutoSwitchPrioirity, but may be
// modified by environment (or by other factors for bots)
function float SwitchPriority() 
{
	local float temp;
	local int bTemp;

	if ( Owner.IsA('ScriptedPawn') )
		return RateSelf(bTemp);
	else if ( (AmmoType != None) && (AmmoType.AmmoAmount<=0) )
	{
		if ( Pawn(Owner).Weapon == self )
			return -0.5;
		else
			return -1;
	}
	else 
		return AutoSwitchPriority;
}

// Compare self to current weapon.  If better than current weapon, then switch
function bool WeaponSet(Pawn Other)
{
	local bool bSwitch,bHaveAmmo;
	local Inventory Inv;
	
	if ( Other.Weapon == self)
		return false;
	if ( Other.Weapon == None )
	{
		Other.PendingWeapon = self;
		Other.ChangedWeapon();
		return true;	
	}
	else if ( (Other.Weapon.SwitchPriority() < SwitchPriority()) 
		&& Other.Weapon.PutDown() )
	{
		Other.PendingWeapon = Self;
		GotoState('');
		return true;
	}
	else 
	{
		GoToState('');
		return false;
	}
}

function Weapon RecommendWeapon( out float rating, out int bUseAltMode )
{
	local Weapon Recommended;
	local float oldRating, oldFiring;
	local int oldMode;

	if ( Owner.IsA('PlayerPawn') )
		rating = SwitchPriority();
	else
	{
		rating = RateSelf(bUseAltMode);
		if ( (self == Pawn(Owner).Weapon) 
			&& ((AmmoType == None) || (AmmoType.AmmoAmount > 0)) )
			rating += 0.15; // tend to stick with same weapon
	}
	if ( inventory != None )
	{
		Recommended = inventory.RecommendWeapon(oldRating, oldMode);
		if ( (Recommended != None) && (oldRating > rating) )
		{
			rating = oldRating;
			bUseAltMode = oldMode;
			return Recommended;
		}
	}
	return self;
}

// Toss this weapon out
function DropFrom(vector StartLocation)
{
	if ( !SetLocation(StartLocation) )
		return; 
	//bFireMem = false;
	//bAltFireMem = false;
	AIRating = Default.AIRating;
	PickupAmmoCount = AmmoType.AmmoAmount;
	AmmoType.AmmoAmount = 0;
	Super.DropFrom(StartLocation);
}

//**************************************************************************************
//
// Firing functions and states
//

function CheckVisibility()
{
	if( Owner.bHidden && (Pawn(Owner).Health > 0) && (Pawn(Owner).Visibility < Pawn(Owner).Default.Visibility) )
	{
		Owner.bHidden = false;
		Pawn(Owner).Visibility = Pawn(Owner).Default.Visibility;
	}
}

function Fire( float Value )
{
	//bFireMem = false;
	//bAltFireMem = false;
	if (AmmoType.UseAmmo(1))
	{
		GotoState('NormalFire');
		if ( PlayerPawn(Owner) != None )
			PlayerPawn(Owner).ShakeView(ShakeTime, ShakeMag, ShakeVert);
		bPointing=True;
		if ( bInstantHit )
			TraceFire(0.0);
		else
			ProjectileFire(ProjectileClass, ProjectileSpeed, bWarnTarget);
		PlayFiring();
		if ( Owner.bHidden )
			CheckVisibility();
	}
}

function AltFire( float Value )
{
	//bFireMem = false;
	//bAltFireMem = false;
	if (AmmoType.UseAmmo(1))
	{
		GotoState('AltFiring');
		if ( PlayerPawn(Owner) != None )
			PlayerPawn(Owner).ShakeView(ShakeTime, ShakeMag, ShakeVert);
		bPointing=True;
		if ( bAltInstantHit )
			TraceFire(0.0);
		else
			ProjectileFire(AltProjectileClass, AltProjectileSpeed, bAltWarnTarget);
		PlayAltFiring();
		if ( Owner.bHidden )
			CheckVisibility();
	}
}

function PlayFiring()
{
	//Play firing animation and sound
}

function PlayAltFiring()
{
	//Play alt firing animation and sound
}

function Projectile ProjectileFire(class<projectile> ProjClass, float ProjSpeed, bool bWarn)
{
	local Vector Start, X,Y,Z;

	Owner.MakeNoise(Pawn(Owner).SoundDampening);
	GetAxes(Pawn(owner).ViewRotation,X,Y,Z);
	Start = Owner.Location + CalcDrawOffset() + FireOffset.X * X + FireOffset.Y * Y + FireOffset.Z * Z; 
	AdjustedAim = pawn(owner).AdjustAim(ProjSpeed, Start, AimError, True, bWarn);	
	return Spawn(ProjClass,,, Start,AdjustedAim);	
}

function TraceFire( float Accuracy )
{
	local vector HitLocation, HitNormal, StartTrace, EndTrace, X,Y,Z;
	local actor Other;

	Owner.MakeNoise(Pawn(Owner).SoundDampening);
	GetAxes(Pawn(owner).ViewRotation,X,Y,Z);
	StartTrace = Owner.Location + CalcDrawOffset() + FireOffset.X * X + FireOffset.Y * Y + FireOffset.Z * Z; 
	AdjustedAim = pawn(owner).AdjustAim(1000000, StartTrace, 2.75*AimError, False, False);	
	EndTrace = StartTrace + Accuracy * (FRand() - 0.5 )* Y * 1000
		+ Accuracy * (FRand() - 0.5 ) * Z * 1000 ;
	EndTrace += (10000 * vector(AdjustedAim)); 
	Other = Pawn(Owner).TraceShot(HitLocation,HitNormal,EndTrace,StartTrace);
	ProcessTraceHit(Other, HitLocation, HitNormal, vector(AdjustedAim),Y,Z);
}

function ProcessTraceHit(Actor Other, Vector HitLocation, Vector HitNormal, Vector X, Vector Y, Vector Z)
{
	//Spawn appropriate effects at hit location, any weapon lights, and damage hit actor
}

// Finish a firing sequence
function Finish()
{
	if ( bChangeWeapon )
	{
		GotoState('DownWeapon');
		return;
	}

	if ( PlayerPawn(Owner) == None )
	{
		//bFireMem = false;
		//bAltFireMem = false;
		if ( AmmoType.AmmoAmount<=0 )
		{
			Pawn(Owner).StopFiring();
			Pawn(Owner).SwitchToBestWeapon();
		}
		else if ( (Pawn(Owner).bFire != 0) && (FRand() < RefireRate) )
			Global.Fire(0);
		else if ( (Pawn(Owner).bAltFire != 0) && (FRand() < AltRefireRate) )
			Global.AltFire(0);	
		else 
		{
			Pawn(Owner).StopFiring();
			GotoState('Idle');
		}
		return;
	}
	if ( (AmmoType.AmmoAmount<=0) || (Pawn(Owner).Weapon != self) )
		GotoState('Idle');
	else if ( /*bFireMem ||*/ Pawn(Owner).bFire!=0 )
		Global.Fire(0);
	else if ( /*bAltFireMem ||*/ Pawn(Owner).bAltFire!=0 )
		Global.AltFire(0);
	else 
		GotoState('Idle');
}

///////////////////////////////////////////////////////
state NormalFire
{
	function Fire(float F) 
	{
		//bFireMem = true;
	}
	function AltFire(float F) 
	{
		//bAltFireMem = true;
	}

Begin:
	FinishAnim();
	Finish();
}

////////////////////////////////////////////////////////
state AltFiring
{
	function Fire(float F) 
	{
		//bFireMem = true;
	}
	function AltFire(float F) 
	{
		//bAltFireMem = true;
	}

Begin:
	FinishAnim();
	Finish();
}

//**********************************************************************************
// Weapon is up, but not firing
state Idle
{
	function AnimEnd()
	{
		PlayIdleAnim();
	}

	function bool PutDown()
	{
		GotoState('DownWeapon');
		return True;
	}

Begin:
	bPointing=False;
	if ( (AmmoType != None) && (AmmoType.AmmoAmount<=0) ) 
		Pawn(Owner).SwitchToBestWeapon();  //Goto Weapon that has Ammo
	if ( /*(bFireMem ||*/ Pawn(Owner).bFire!=0 ) Fire(0.0);
	if ( /*(bAltFireMem ||*/ Pawn(Owner).bAltFire!=0 ) AltFire(0.0);	
	Disable('AnimEnd');
	PlayIdleAnim();
}

//
// Bring newly active weapon up
// Bring newly active weapon up
state Active
{
	function Fire(float F) 
	{
		//bFireMem = true;
	}

	function AltFire(float F) 
	{
		//bAltFireMem = true;
	}

	function bool PutDown()
	{
		if ( bWeaponUp || (AnimFrame < 0.75) )
			GotoState('DownWeapon');
		else
			bChangeWeapon = true;
		return True;
	}

	function BeginState()
	{
		bChangeWeapon = false;
	}

Begin:
	FinishAnim();
	if ( bChangeWeapon )
		GotoState('DownWeapon');
	bWeaponUp = True;
	PlayPostSelect();
	FinishAnim();
	Finish();
}

//
// Putting down weapon in favor of a new one.
//
State DownWeapon
{
ignores Fire, AltFire;

	function bool PutDown()
	{
		return true; //just keep putting it down
	}

	function BeginState()
	{
		bChangeWeapon = false;
	}

Begin:
	TweenDown();
	FinishAnim();
	bOnlyOwnerSee = false;
	Pawn(Owner).ChangedWeapon();
}

function BringUp()
{
	if ( Owner.IsA('PlayerPawn') )
		PlayerPawn(Owner).SetDesiredFOV(PlayerPawn(Owner).Default.FOVAngle);	
	bWeaponUp = false;
	PlaySelect();
	GotoState('Active');
}

function bool PutDown()
{
	bChangeWeapon = true;
	return true; 
}

function TweenDown()
{
	if ( GetAnimGroup(AnimSequence) == 'Select' )
		TweenAnim( AnimSequence, AnimFrame * 0.4 );
	else
		PlayAnim('Down', 1.0, 0.05);
}

function TweenSelect()
{
	TweenAnim('Select',0.001);
}

function PlaySelect()
{
	PlayAnim('Select',1.0,0.0);
	Owner.PlaySound(SelectSound, SLOT_Misc, Pawn(Owner).SoundDampening);	
}

function PlayPostSelect()
{
}

function PlayIdleAnim()
{
}

defaultproperties
{
     MaxTargetRange=4096.000000
     ProjectileSpeed=1000.000000
     AltProjectileSpeed=1000.000000
     aimerror=550.000000
     shakemag=300.000000
     shaketime=0.100000
     shakevert=5.000000
     AIRating=0.100000
     RefireRate=0.500000
     AltRefireRate=0.500000
     MessageNoAmmo=" has no ammo."
     AutoSwitchPriority=1
     InventoryGroup=1
     PickupMessage="You got a weapon"
     RespawnTime=30.000000
     PlayerViewOffset=(X=30.000000,Z=-5.000000)
     MaxDesireability=0.500000
     Texture=Texture'Engine.S_Weapon'
     bNoSmooth=True
}
