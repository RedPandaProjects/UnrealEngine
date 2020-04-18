//=============================================================================
// Pawn, the base class of all actors that can be controlled by players or AI.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Pawn expands Actor 
	abstract
	config
	intrinsic;

#exec Texture Import File=Textures\Pawn.pcx Name=S_Pawn Mips=Off Flags=2

//-----------------------------------------------------------------------------
// Pawn variables.

// General flags.
var bool		bBehindView;    // Outside-the-player view.
var bool        bIsPlayer;      // Pawn is a player or a player-bot.
var bool		bJustLanded;	// used by eyeheight adjustment
var bool		bUpAndOut;		// used by swimming 
var bool		bIsWalking;
var const bool	bHitSlopedWall;	// used by Physics
var travel bool	bNeverSwitchOnPickup;	// if true, don't automatically switch to picked up weapon

//AI flags
var(Combat) bool	bCanStrafe;
var(Orders) bool	bFixedStart;
var const bool		bReducedSpeed;		//used by movement intrinsices
var		bool		bCanJump;
var		bool 		bCanWalk;
var		bool		bCanSwim;
var		bool		bCanFly;
var		bool		bCanOpenDoors;
var		bool		bCanDoSpecial;
var		bool		bDrowning;
var const bool		bLOSflag;			// used for alternating LineOfSight traces
var 	bool 		bFromWall;
var		bool		bHunting;			// tells navigation code that pawn is hunting another pawn,
										//	so fall back to finding a path to a visible pathnode if none
										//	are reachable
var		bool		bAvoidLedges;		// don't get too close to ledges
var		bool		bJumpOffPawn;		
var		bool		bShootSpecial;
var		bool		bAutoActivate;

// Ticked pawn timers
var		float		SightCounter;	//Used to keep track of when to check player visibility
var		float           PainTime;			//used for getting PainTimer() messages (for Lava, no air, etc.)
var		float		SpeechTime;	

// Physics updating time monitoring (for AI monitoring reaching destinations)
var const	float		AvgPhysicsTime;

// Additional pawn region information.
var PointRegion FootRegion;
var PointRegion HeadRegion;

// Navigation AI
var 	float		MoveTimer;
var 	Actor		MoveTarget;		// set by movement intrinsics
var		vector	 	Destination;	// set by Movement intrinsics
var	 	vector		Focus;			// set by Movement intrinsics
var		float		DesiredSpeed;
var		float		MaxDesiredSpeed;
var(Combat) float	MeleeRange; // Max range for melee attack (not including collision radii)

// Player and enemy movement.
var(Movement) float      GroundSpeed;     // The maximum ground speed.
var(Movement) float      WaterSpeed;      // The maximum swimming speed.
var(Movement) float      AirSpeed;        // The maximum flying speed.
var(Movement) float		 AccelRate;		  // max acceleration rate
var(Movement) float		 JumpZ;      		// vertical acceleration w/ jump
var(Movement) float      MaxStepHeight;   // Maximum size of upward/downward step.

// AI basics.
var	 	float		MinHitWall;		// Minimum HitNormal dot Velocity.Normal to get a HitWall from the
									// physics
var() 	byte       	Visibility;      //How visible is the pawn? 0 = invisible. 
									// 128 = normal.  255 = highly visible.
var		float		Alertness; //-1 to 1 ->Used within specific states for varying reaction to stimuli 
var		float 		Stimulus; //Strenght of stimulus - Set when stimulus happens, used in Acquisition state 
var(AI) float		SightRadius;     //Maximum seeing distance.
var(AI) float		PeripheralVision;//Cosine of limits of peripheral vision.
var(AI) float		HearingThreshold;  //Minimum noise loudness for hearing
var		vector		LastSeenPos; 		// enemy position when I last saw enemy (auto updated if EnemyNotVisible() enabled)
var		vector		LastSeeingPos;		// position where I last saw enemy (auto updated if EnemyNotVisible enabled)
var	 	Pawn    	Enemy;

// Player info.
var globalconfig string[20]   PlayerName;    // Player name, or blank if none.
var globalconfig string[20]   TeamName;    // Desired Team name, or blank if none. (Only here to save for config
var travel byte	 		Team;      	// Player Team, 255 = None for player
var travel Weapon       Weapon;        // The pawn's current weapon.
var Weapon				PendingWeapon;	// Will become weapon once current weapon is put down
var travel Inventory	SelectedItem;	// currently selected inventory item

// Movement.
var rotator     	ViewRotation;  	// View rotation.
var vector			WalkBob;
var() float      	BaseEyeHeight; 	// Base eye height above collision center.
var float        	EyeHeight;     	// Current eye height, adjusted for bobbing and stairs.
var	const	vector	Floor;			// Normal of floor pawn is standing on (only used
									//	by PHYS_Spider)
var float			SplashTime;		// time of last splash

// View
var float        OrthoZoom;     // Orthogonal/map view zoom factor.
var() float      FovAngle;      // X field of view angle in degrees, usually 90.

// Player game statistics.
var int			DieCount, ItemCount, KillCount, SecretCount;
var float		score;

//Health
var() travel int      Health;          // Health: 100 = normal maximum

// Inherent Armor (for creatures).
var() name	ReducedDamageType; //Either a damagetype name or 'All', 'AllEnvironment' (Burned, Corroded, Frozen)
var() float ReducedDamagePct;

// Inventory to drop when killed (for creatures)
var() class<inventory> DropWhenKilled;

// Zone pain
var(Movement) float		UnderWaterTime;  	//how much time pawn can go without air (in seconds)

var(AI) enum EAttitude  //important - order in decreasing importance
{
	ATTITUDE_Fear,		//will try to run away
	ATTITUDE_Hate,		// will attack enemy
	ATTITUDE_Frenzy,	//will attack anything, indiscriminately
	ATTITUDE_Threaten,	// animations, but no attack
	ATTITUDE_Ignore,
	ATTITUDE_Friendly,
	ATTITUDE_Follow 	//accepts player as leader
} AttitudeToPlayer;	//determines how creature will react on seeing player (if in human form)

var(AI) enum EIntelligence //important - order in increasing intelligence
{
	BRAINS_NONE, //only reacts to immediate stimulus
	BRAINS_REPTILE, //follows to last seen position
	BRAINS_MAMMAL, //simple navigation (limited path length)
	BRAINS_HUMAN   //complex navigation, team coordination, use environment stuff (triggers, etc.)
}	Intelligence;

var(AI) float		Skill;			// skill, scaled by game difficulty (add difficulty to this value)	
var		actor		SpecialGoal;	// used by navigation AI
var		float		SpecialPause;

// Sound and noise management
var const 	vector 		noise1spot;
var const 	float 		noise1time;
var const	pawn		noise1other;
var const	float		noise1loudness;
var const 	vector 		noise2spot;
var const 	float 		noise2time;
var const	pawn		noise2other;
var const	float		noise2loudness;
var			float		LastPainSound;

// chained pawn list
var const	pawn		nextPawn;

// Common sounds
var(Sounds)	sound	HitSound1;
var(Sounds)	sound	HitSound2;
var(Sounds)	sound	Land;
var(Sounds)	sound	Die;
var(Sounds) sound	WaterStep;

// Input buttons.
var input byte
	bZoom, bRun, bLook, bDuck, bSnapLevel,
	bStrafe, bFire, bAltFire,
	bExtra0, bExtra1, bExtra2, bExtra3;

var(Combat) float CombatStyle; // -1 to 1 = low means tends to stay off and snipe, high means tends to charge and melee
var NavigationPoint home; //set when begin play, used for retreating and attitude checks
 
var name NextState; //for queueing states
var name NextLabel; //for queueing states

var float SoundDampening;
var float DamageScaling;

var(Orders) name AlarmTag; // tag of object to go to when see player
var(Orders) name SharedAlarmTag;
var	Decoration	carriedDecoration;

replication
{
	// Things the server should send to the client.
	reliable if( Role==ROLE_Authority )
		Weapon;
	reliable if( Role==ROLE_Authority && bNetOwner )
		 PlayerName, Team, TeamName, bIsPlayer, CarriedDecoration, SelectedItem,
		 GroundSpeed, WaterSpeed, AirSpeed, AccelRate, JumpZ, MaxStepHeight,
		 bBehindView;
	unreliable if( Role==ROLE_Authority && bNetOwner && bIsPlayer && bNetInitial )
		ViewRotation;
	unreliable if( Role==ROLE_Authority && bNetOwner )
		Health, MoveTarget, Score;

	// Functions.
	reliable if( Role==ROLE_Authority && RemoteRole==ROLE_AutonomousProxy )
		ClientDying, ClientReStart, ClientGameEnded, ClientSetRotation;
	unreliable if( Role==ROLE_Authority )
		ClientHearSound, ClientMessage;
	reliable if( Role<ROLE_Authority )
		NextItem, SwitchToBestWeapon, bExtra0, bExtra1, bExtra2, bExtra3;

	// Input.
	unreliable if( Role<ROLE_AutonomousProxy )
		bZoom, bRun, bLook, bDuck, bSnapLevel, bStrafe;
	unreliable always if( Role<=ROLE_AutonomousProxy )
		bFire, bAltFire;
}

// Latent Movement.
//Note that MoveTo sets the actor's Destination, and MoveToward sets the
//actor's MoveTarget.  Actor will rotate towards destination

intrinsic(500) final latent function MoveTo( vector NewDestination, optional float speed);
intrinsic(502) final latent function MoveToward(actor NewTarget, optional float speed);
intrinsic(504) final latent function StrafeTo(vector NewDestination, vector NewFocus);
intrinsic(506) final latent function StrafeFacing(vector NewDestination, actor NewTarget);
intrinsic(508) final latent function TurnTo(vector NewFocus);
intrinsic(510) final latent function TurnToward(actor NewTarget);

// Intrinsic AI functions
//LineOfSightTo() returns true if any of several points of Other is visible 
// (origin, top, bottom)
intrinsic(514) final function bool LineOfSightTo(actor Other); 
// CanSee() similar to line of sight, but also takes into account Pawn's peripheral vision
intrinsic(533) final function bool CanSee(actor Other); 
intrinsic(518) final function Actor FindPathTo(vector aPoint, optional bool bSinglePath, 
												optional bool bClearPaths);
intrinsic(517) final function Actor FindPathToward(actor anActor, optional bool bSinglePath, 
												optional bool bClearPaths);
intrinsic(525) final function NavigationPoint FindRandomDest(optional bool bClearPaths);

intrinsic(522) final function ClearPaths();
intrinsic(523) final function vector EAdjustJump();

//Reachable returns what part of direct path from Actor to aPoint is traversable
//using the current locomotion method
intrinsic(521) final function bool pointReachable(vector aPoint);
intrinsic(520) final function bool actorReachable(actor anActor);

/* PickWallAdjust()
Check if could jump up over obstruction (only if there is a knee height obstruction)
If so, start jump, and return current destination
Else, try to step around - return a destination 90 degrees right or left depending on traces
out and floor checks
*/
intrinsic(526) final function bool PickWallAdjust();
intrinsic(524) final function int FindStairRotation(float DeltaTime);
intrinsic(527) final latent function WaitForLanding();
intrinsic(540) final function actor FindBestInventoryPath(out float MinWeight, bool bPredictRespawns);

intrinsic(529) final function AddPawn();
intrinsic(530) final function RemovePawn();

// Pick best pawn target
intrinsic(531) final function pawn PickTarget(out float bestAim, out float bestDist, vector FireDir, vector projStart);
intrinsic(534) final function actor PickAnyTarget(out float bestAim, out float bestDist, vector FireDir, vector projStart);

// Force end to sleep
intrinsic function StopWaiting();

event MayFall(); //return true if allowed to fall - called by engine when pawn is about to fall

//
// Client gateway functions.
//
event ClientMessage( coerce string[255] S );

function AddVelocity( vector NewVelocity)
{
	if (Physics == PHYS_Walking)
		SetPhysics(PHYS_Falling);
	if ( (Velocity.Z > 380) && (NewVelocity.Z > 0) )
		NewVelocity.Z *= 0.5;
	Velocity += NewVelocity;
}

function ClientSetRotation( rotator NewRotation )
{
	local Pawn P;

	ViewRotation      = NewRotation;
	NewRotation.Pitch = 0;
	NewRotation.Roll  = 0;
	SetRotation( NewRotation );
}

function ClientDying(name DamageType, vector HitLocation)
{
	PlayDying(DamageType, HitLocation);
	GotoState('Dying');
}

function ClientReStart()
{
	//log("client restart");
	Velocity = vect(0,0,0);
	Acceleration = vect(0,0,0);
	BaseEyeHeight = Default.BaseEyeHeight;
	EyeHeight = BaseEyeHeight;
	
	if (Region.Zone.bWaterZone)
	{
		if (HeadRegion.Zone.bWaterZone)
				PainTime = UnderWaterTime;
		setPhysics(PHYS_Swimming);
		GotoState('PlayerSwimming');
	}
	else if ( (Level.NetMode == NM_StandAlone) && Level.Game.IsA('VRikersGame') )
		GotoState('PlayerWaking');
	else
		GotoState('PlayerWalking'); 
}

function ClientGameEnded()
{
	GotoState('GameEnded');
}

//=============================================================================
// Inventory related functions.

// toss out the weapon currently held
function TossWeapon()
{
	local vector X,Y,Z;
	if ( Weapon == None )
		return;
	GetAxes(Rotation,X,Y,Z);
	Weapon.DropFrom(Location + 0.8 * CollisionRadius * X + - 0.5 * CollisionRadius * Y); 
}	

// The player/bot wants to select next item
exec function NextItem()
{
	local Inventory Inv;

	if (SelectedItem==None) {
		SelectedItem = Inventory.SelectNext();
		Return;
	}
	if (SelectedItem.Inventory!=None)
		SelectedItem = SelectedItem.Inventory.SelectNext(); 
	else
		SelectedItem = Inventory.SelectNext();

	if ( SelectedItem == None )
		SelectedItem = Inventory.SelectNext();
}

// FindInventoryType()
// returns the inventory item of the requested class
// if it exists in this pawn's inventory 

function Inventory FindInventoryType( class DesiredClass )
{
	local Inventory Inv;

	for( Inv=Inventory; Inv!=None; Inv=Inv.Inventory )   
		if ( Inv.class == DesiredClass )
			return Inv;
	return None;
} 

// Add Item to this pawn's inventory. 
// Returns true if successfully added, false if not.
function bool AddInventory( inventory NewItem )
{
	// Skip if already in the inventory.
	local inventory Inv;
	
	for( Inv=Inventory; Inv!=None; Inv=Inv.Inventory )
		if( Inv == NewItem )
			return false;

	// The item should not have been destroyed if we get here.
	assert(NewItem!=None);

	// Add to front of inventory chain.
	NewItem.SetOwner(Self);
	NewItem.Inventory = Inventory;
	Inventory = NewItem;

	return true;
}

// Remove Item from this pawn's inventory, if it exists.
// Returns true if it existed and was deleted, false if it did not exist.
function bool DeleteInventory( inventory Item )
{
	// If this item is in our inventory chain, unlink it.
	local actor Link;
	if ( Item == Weapon )
		Weapon = None;
	if ( Item == SelectedItem )
		SelectedItem = None;
	for( Link = Self; Link!=None; Link=Link.Inventory )
	{
		if( Link.Inventory == Item )
		{
			Link.Inventory = Item.Inventory;
			break;
		}
	}
	Item.SetOwner(None);
}

// Just changed to pendingWeapon
function ChangedWeapon()
{
	//log("changed weapon");
	if (Weapon == PendingWeapon)
	{
		if ( Weapon == None )
			SwitchToBestWeapon();
		else if ( Weapon.GetStateName() == 'DownWeapon' ) 
			Weapon.BringUp();
		PendingWeapon = None;
		return;
	}
	if ( PendingWeapon == None )
		PendingWeapon = Weapon;
	PlayWeaponSwitch(PendingWeapon);
	if ( (PendingWeapon != None) && (PendingWeapon.Mass > 20) && (carriedDecoration != None) )
		DropDecoration();
	Weapon = PendingWeapon;
	if ( Weapon != None )
	{
		Weapon.BringUp();
		if ( (Level.Game != None) && (Level.Game.Difficulty > 1) )
			MakeNoise(0.1 * Level.Game.Difficulty);		
	}
	PendingWeapon = None;
}

//==============
// Encroachment
event bool EncroachingOn( actor Other )
{
	if ( (Other.Brush != None) || (Brush(Other) != None) )
		return true;
		
	if (!bIsPlayer && (Pawn(Other) != None))
		return true;
		
	return false;
}

event EncroachedBy( actor Other )
{
	if ( Pawn(Other) != None )
		gibbedBy(Other);
		
}

function gibbedBy(actor Other)
{
	local pawn instigatedBy;

	instigatedBy = pawn(Other);
	if (instigatedBy == None)
		instigatedBy = Other.instigator;
	health = -1000; //make sure gibs
	Died(instigatedBy, 'gibbed', Location);
}

event PlayerTimeOut()
{
	if (Health > 0)
		Died(None, 'suicided', Location);
}

//Base change - if new base is pawn or decoration, damage based on relative mass and old velocity
// Also, non-players will jump off pawns immediately
function JumpOffPawn()
{
	Velocity += 60 * VRand();
	Velocity.Z = 180;
	SetPhysics(PHYS_Falling);
}

singular event BaseChange()
{
	local float decorMass;

	if ( (base == None) && (Physics == PHYS_None) )
		SetPhysics(PHYS_Falling);
	else if (Pawn(Base) != None)
	{
		Base.TakeDamage( (1-Velocity.Z/400)* Mass/Base.Mass, Self,Location,0.5 * Velocity , 'stomped');
		JumpOffPawn();
	}
	else if ( (Decoration(Base) != None) && (Velocity.Z < -400) )
	{
		decorMass = FMax(Decoration(Base).Mass, 1);
		Base.TakeDamage((-2* Mass/decorMass * Velocity.Z/400), Self, Location, 0.5 * Velocity, 'stomped');
	}
}

event LongFall();

//=============================================================================
// Network related functions.


event Destroyed()
{
	local Inventory Inv;

	RemovePawn();

	for( Inv=Inventory; Inv!=None; Inv=Inv.Inventory )   
		Inv.Destroy();
	if ( bIsPlayer && (Level.Game != None) )
		Level.Game.logout(self);
	Super.Destroyed();
}

//=============================================================================
// functions.

//
// Intrinsic client-side functions.
//
intrinsic simulated event ClientHearSound ( actor Actor, int Id, sound S, vector SoundLocation, vector Parameters );

//
// Called immediately before gameplay begins.
//
event PreBeginPlay()
{
	AddPawn();
	Super.PreBeginPlay();
	if ( bDeleteMe )
		return;

	// Set instigator to self.
	Instigator = Self;
	DesiredRotation = Rotation;
	SplashTime = Level.TimeSeconds;
	SightCounter = 0.2 * FRand();  //offset randomly 
	if ( Level.Game != None )
		Skill += Level.Game.Difficulty; 
	Skill = FClamp(Skill, 0, 3);
	PreSetMovement();
	
	if ( DrawScale != Default.Drawscale )
	{
		SetCollisionSize(CollisionRadius*DrawScale/Default.DrawScale, CollisionHeight*DrawScale/Default.DrawScale);
		Health = Health * DrawScale/Default.DrawScale;
	}
		
	if (!bIsPlayer) 
		{
			EyeHeight = 0.8 * CollisionHeight; //FIXME - if all baseeyeheights set right, use them
			BaseEyeHeight = EyeHeight;
			if (Fatness == 0) //vary monster fatness slightly if at default
				Fatness = 120 + Rand(8) + Rand(8);
		}
	else if (PlayerName == "")
		PlayerName = "Player";
}

/* PreSetMovement()
default for walking creature.  Re-implement in subclass
for swimming/flying capability
*/
function PreSetMovement()
{
	if (JumpZ > 0)
		bCanJump = true;
	bCanWalk = true;
	bCanSwim = false;
	bCanFly = false;
	MinHitWall = -0.6;
	if (Intelligence > BRAINS_Reptile)
		bCanOpenDoors = true;
	if (Intelligence == BRAINS_Human)
		bCanDoSpecial = true;
}
	
//=============================================================================
// Animation playing - should be implemented in subclass, 
//
// PlayWaiting, PlayRunning, and PlayGutHit, PlayMovingAttack (if used)
// and PlayDying are required to be implemented in the subclass

function PlayRunning()
{
	////log("Error - PlayRunning should be implemented in subclass of "$class);
}

function PlayWalking()
{
	PlayRunning(); 
}

function PlayWaiting()
{
	////log("Error - PlayWaiting should be implemented in subclass");
}

function PlayMovingAttack()
{
	////log("Error - PlayMovingAttack should be implemented in subclass");
	//Note - must restart attack timer when done with moving attack
	PlayRunning();
}

function PlayWaitingAmbush()
{
	PlayWaiting();
}

function TweenToFighter(float tweentime)
{
}

function TweenToRunning(float tweentime)
{
	TweenToFighter(0.1);
}

function TweenToWalking(float tweentime)
{
	TweenToRunning(tweentime);
}

function TweenToPatrolStop(float tweentime)
{
	TweenToFighter(tweentime);
}

function TweenToWaiting(float tweentime)
{
	TweenToFighter(tweentime);
}

function PlayThreatening()
{
	TweenToFighter(0.1);
}

function PlayPatrolStop()
{
	PlayWaiting();
}

function PlayTurning()
{
	TweenToFighter(0.1);
}

function PlayBigDeath(name DamageType);
function PlayHeadDeath(name DamageType);
function PlayLeftDeath(name DamageType);
function PlayRightDeath(name DamageType);
function PlayGutDeath(name DamageType);

function PlayDying(name DamageType, vector HitLoc)
{
	local vector X,Y,Z, HitVec, HitVec2D;
	local float dotp;

	if ( Velocity.Z > 250 )
	{
		PlayBigDeath(DamageType);
		return;
	}
	
	if ( DamageType == 'Decapitated' )
	{
		PlayHeadDeath(DamageType);
		return;
	}
			
	GetAxes(Rotation,X,Y,Z);
	X.Z = 0;
	HitVec = Normal(HitLoc - Location);
	HitVec2D= HitVec;
	HitVec2D.Z = 0;
	dotp = HitVec2D dot X;

	//first check for head hit
	if ( HitLoc.Z - Location.Z > 0.5 * CollisionHeight )
	{
		if (dotp > 0)
			PlayHeadDeath(DamageType);
		else
			PlayGutDeath(DamageType);
		return;
	}
	
	if (dotp > 0.71) //then hit in front
		PlayGutDeath(DamageType);
	else
	{
		dotp = HitVec dot Y;
		if (dotp > 0.0)
			PlayLeftDeath(DamageType);
		else
			PlayRightDeath(DamageType);
	}
}

function PlayGutHit(float tweentime)
{
	log("Error - play gut hit must be implemented in subclass of "$class);
}

function PlayHeadHit(float tweentime)
{
	PlayGutHit(tweentime);
}

function PlayLeftHit(float tweentime)
{
	PlayGutHit(tweentime);
}

function PlayRightHit(float tweentime)
{
	PlayGutHit(tweentime);
}

function FireWeapon();

/* TraceShot - used by instant hit weapons, and monsters 
*/
function actor TraceShot(out vector HitLocation, out vector HitNormal, vector EndTrace, vector StartTrace)
{
	local vector realHit;
	local actor Other;
	Other = Trace(HitLocation,HitNormal,EndTrace,StartTrace,True);
	if ( Pawn(Other) != None )
	{
		realHit = HitLocation;
		if ( !Pawn(Other).AdjustHitLocation(HitLocation, EndTrace - StartTrace) )
			Other = Pawn(Other).TraceShot(HitLocation,HitNormal,EndTrace,realHit);
	}
	return Other;
}

/* Adjust hit location - adjusts the hit location in for pawns, and returns
true if it was really a hit, and false if not (for ducking, etc.)
*/
function bool AdjustHitLocation(out vector HitLocation, vector TraceDir)
{
	local float adjZ, maxZ;

	TraceDir = Normal(TraceDir);
	HitLocation = HitLocation + 0.4 * CollisionRadius * TraceDir;

	if ( (GetAnimGroup(AnimSequence) == 'Ducking') && (AnimFrame > -0.03) )
	{
		maxZ = Location.Z + 0.25 * CollisionHeight;
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
	}
	return true;
}
			
function PlayTakeHit(float tweentime, vector HitLoc, int damage)
{
	local vector X,Y,Z, HitVec, HitVec2D;
	local float dotp;
	
	GetAxes(Rotation,X,Y,Z);
	X.Z = 0;
	HitVec = Normal(HitLoc - Location);
	HitVec2D= HitVec;
	HitVec2D.Z = 0;
	dotp = HitVec2D dot X;

	//first check for head hit
	if ( HitLoc.Z - Location.Z > 0.5 * CollisionHeight )
	{
		if (dotp > 0)
			PlayHeadHit(tweentime);
		else
			PlayGutHit(tweentime);
		return;
	}
	
	if (dotp > 0.71) //then hit in front
		PlayGutHit( tweentime);
	else if (dotp < -0.71) // then hit in back
		PlayHeadHit(tweentime);
	else
	{
		dotp = HitVec dot Y;
		if (dotp > 0.0)
			PlayLeftHit(tweentime);
		else
			PlayRightHit(tweentime);
	}
}

function PlayVictoryDance()
{
	TweenToFighter(0.1);
}

function PlayOutOfWater()
{
	TweenToFalling();
}

function PlayDive();
function TweenToFalling();
function PlayInAir();
function PlayDuck();
function PlayCrawling();

function PlayLanded(float impactVel)
{
	local float landVol;
	//default - do nothing (keep playing existing animation)
	landVol = impactVel/JumpZ;
	landVol = 0.005 * Mass * landVol * landVol;
	PlaySound(Land, SLOT_Interact, FMin(20, landVol));
}

function PlayFiring();
function PlayWeaponSwitch(Weapon NewWeapon);
function TweenToSwimming(float tweentime);


//-----------------------------------------------------------------------------
// Sound functions
function PlayTakeHitSound(int Damage, name damageType, int Mult)
{
	if ( Level.TimeSeconds - LastPainSound < 0.25 )
		return;

	if (HitSound1 == None)return;
	LastPainSound = Level.TimeSeconds;
	if (FRand() < 0.5)
		PlaySound(HitSound1, SLOT_Pain, FMax(Mult * TransientSoundVolume, Mult * 2.0));
	else
		PlaySound(HitSound2, SLOT_Pain, FMax(Mult * TransientSoundVolume, Mult * 2.0));
}

function Gasp();

function DropDecoration()
{
	if (CarriedDecoration != None)
	{
		CarriedDecoration.bWasCarried = true;
		CarriedDecoration.SetBase(None);
		CarriedDecoration.SetPhysics(PHYS_Falling);
		CarriedDecoration.Velocity = Velocity + 10 * VRand();
		CarriedDecoration.Instigator = self;
		CarriedDecoration = None;
	}
}

function GrabDecoration()
{
	local vector lookDir, HitLocation, HitNormal, T1, T2, extent;
	local actor HitActor;

	if ( carriedDecoration == None )
	{
		//first trace to find it
		lookDir = vector(Rotation);
		lookDir.Z = 0;
		T1 = Location + BaseEyeHeight * vect(0,0,1) + lookDir * 0.8 * CollisionRadius;
		T2 = T1 + lookDir * 1.2 * CollisionRadius;
		HitActor = Trace(HitLocation, HitNormal, T2, T1, true);
		if ( HitActor == None )
		{
			T1 = T2 - (BaseEyeHeight + CollisionHeight - 2) * vect(0,0,1);
			HitActor = Trace(HitLocation, HitNormal, T1, T2, true);
		}
		else if ( HitActor == Level )
		{
			T2 = HitLocation - lookDir;
			T1 = T2 - (BaseEyeHeight + CollisionHeight - 2) * vect(0,0,1);
			HitActor = Trace(HitLocation, HitNormal, T1, T2, true);
		}	
		if ( (HitActor == None) || (HitActor == Level) )
		{
			extent.X = CollisionRadius;
			extent.Y = CollisionRadius;
			extent.Z = CollisionHeight;
			HitActor = Trace(HitLocation, HitNormal, Location + lookDir * 1.2 * CollisionRadius, Location, true, extent);
		}

		if ( Mover(HitActor) != None )
		{
			if ( Mover(HitActor).bUseTriggered )
				HitActor.Trigger( self, self );
		}		
		else if ( (Decoration(HitActor) != None)  && ((weapon == None) || (weapon.Mass < 20)) )
		{
			CarriedDecoration = Decoration(HitActor);
			if ( !CarriedDecoration.bPushable || (CarriedDecoration.Mass > 40) 
				|| (CarriedDecoration.StandingCount > 0) )
			{
				CarriedDecoration = None;
				return;
			}
			lookDir.Z = 0;				
			if ( CarriedDecoration.SetLocation(Location + (0.5 * CollisionRadius + CarriedDecoration.CollisionRadius) * lookDir) )
			{
				CarriedDecoration.SetPhysics(PHYS_None);
				CarriedDecoration.SetBase(self);
			}
			else
				CarriedDecoration = None;
		}
	}
}
	
function StopFiring();

/* AdjustAim()
ScriptedPawn version does adjustment for non-controlled pawns. 
PlayerPawn version does the adjustment for player aiming help.
Only adjusts aiming at pawns
allows more error in Z direction (full as defined by AutoAim - only half that difference for XY)
*/

function rotator AdjustAim(float projSpeed, vector projStart, int aimerror, bool bLeadTarget, bool bWarnTarget)
{
	return ViewRotation;
}

function rotator AdjustToss(float projSpeed, vector projStart, int aimerror, bool bLeadTarget, bool bWarnTarget)
{
	return ViewRotation;
}

function WarnTarget(Pawn shooter, float projSpeed, vector FireDir)
{
	// AI controlled creatures may duck
	// if not falling, and projectile time is long enough
	// often pick opposite to current direction (relative to shooter axis)
}

function SetMovementPhysics()
{
	//implemented in sub-class
}

function PlayHit(float Damage, vector HitLocation, name damageType, float MomentumZ)
{
}

function PlayDeathHit(float Damage, vector HitLocation, name damageType)
{
}

function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
{
	local int actualDamage;
	local bool bAlreadyDead;
	
	bAlreadyDead = (Health <= 0);

	if (Physics == PHYS_None)
		SetMovementPhysics();
	if (Physics == PHYS_Walking)
		momentum.Z = FMax(momentum.Z, 0.4 * VSize(momentum));
	if ( instigatedBy == self )
		momentum *= 0.6;
	momentum = momentum/Mass;
	AddVelocity( momentum ); 

	actualDamage = Level.Game.ReduceDamage(Damage, DamageType, self, instigatedBy);
	if ( bIsPlayer )
	{
		if (ReducedDamageType == 'All') //God mode
			actualDamage = 0;
		else if (Inventory != None) //then check if carrying armor
			actualDamage = Inventory.ReduceDamage(actualDamage, DamageType, HitLocation);
		else
			actualDamage = Damage;
	}
	else if ( (InstigatedBy != None) &&
				(InstigatedBy.IsA(Class.Name) || self.IsA(InstigatedBy.Class.Name)) )
		ActualDamage = ActualDamage * FMin(1 - ReducedDamagePct, 0.35); 
	else if ( (ReducedDamageType == 'All') || 
		((ReducedDamageType != '') && (ReducedDamageType == damageType)) )
		actualDamage = float(actualDamage) * (1 - ReducedDamagePct);
	
	Health -= actualDamage;
	if (CarriedDecoration != None)
		DropDecoration();
	if (Health > 0)
	{
		if (instigatedBy != None)
			damageAttitudeTo(instigatedBy);
		PlayHit(actualDamage, hitLocation, damageType, Momentum.Z);
	}
	else if ( !bAlreadyDead )
	{
		NextState = '';
		PlayDeathHit(actualDamage, hitLocation, damageType);
		if ( actualDamage > mass )
			Health = -1 * actualDamage;
		Enemy = instigatedBy;
		Died(instigatedBy, damageType, HitLocation);
	}
	else
	{
		Warn(self$" took regular damage while already dead");
		GotoState('Dying');
	}
	MakeNoise(1.0); 
}

function Died(pawn Killer, name damageType, vector HitLocation)
{
	local pawn OtherPawn;
	local actor A;

	if ( bDeleteMe )
		return; //already destroyed
	OtherPawn = Level.PawnList;
	while ( OtherPawn != None )
	{
		OtherPawn.Killed(Killer, self, damageType);
		OtherPawn = OtherPawn.nextPawn;
	}
	if ( CarriedDecoration != None )
		DropDecoration();
	level.game.Killed(Killer, self, damageType);
	//log(class$" dying");
	if( Event != '' )
		foreach AllActors( class 'Actor', A, Event )
			A.Trigger( Self, Killer );
	Level.Game.DiscardInventory(self);
	Velocity.Z *= 1.3;
	if ( (damageType != 'decapitated') && Gibbed() )
	{
		SpawnGibbedCarcass();
		if ( bIsPlayer )
			HidePlayer();
		else
			Destroy();
	}
	if ( RemoteRole == ROLE_AutonomousProxy )
		ClientDying(DamageType, HitLocation);
	PlayDying(DamageType, HitLocation);
	GotoState('Dying');
}

function bool Gibbed()
{
	return false;
}

function Carcass SpawnCarcass()
{
	log(self$" should never call base spawncarcass");
	return None;
}

function SpawnGibbedCarcass()
{
}
	
function HidePlayer()
{
	SetCollision(false, false, false);
	TweenToFighter(0.01);
	bHidden = true;
}

event HearNoise( float Loudness, Actor NoiseMaker);
event SeePlayer( actor Seen );
event UpdateEyeHeight( float DeltaTime );
event EnemyNotVisible();

function Killed(pawn Killer, pawn Other, name damageType)
{
	//implemented in subclass
}

//Typically implemented in subclass
function string[64] KillMessage(name damageType, pawn Other)
{
	local string[64] message;

	message = Level.Game.CreatureKillMessage(damageType, Other);
	return (message$class);
}

function damageAttitudeTo(pawn Other)
	{
	//should be implemented in subclass
	}

function Falling()
	{
		//SetPhysics(PHYS_Falling); //Note - physics changes type to PHYS_Falling by default
		//log(class$" Falling");
		PlayInAir();
	}


event Landed(vector HitNormal)
{
	SetMovementPhysics();
	if ( !IsAnimating() )
		PlayLanded(Velocity.Z);
	if (Velocity.Z < -1.4 * JumpZ)
		MakeNoise(-0.5 * Velocity.Z/(FMax(JumpZ, 150.0)));
	bJustLanded = true;
}

//FIXME?  right now I only check for painzones in foot zone changes
event FootZoneChange(ZoneInfo newFootZone)
{
	local actor HitActor;
	local vector HitNormal, HitLocation;
	local float splashSize;
	local actor splash;
	
	if ( Level.NetMode == NM_Client )
		return;
	if ( Level.TimeSeconds - SplashTime > 0.25 ) 
	{
		SplashTime = Level.TimeSeconds;
		if (Physics == PHYS_Falling)
			MakeNoise(1.0);
		else
			MakeNoise(0.3);
		if ( FootRegion.Zone.bWaterZone )
		{
			if ( !newFootZone.bWaterZone && (Role==ROLE_Authority) )
			{
				if ( FootRegion.Zone.ExitSound != None )
					PlaySound(FootRegion.Zone.ExitSound, SLOT_Interact, 1); 
				if ( FootRegion.Zone.ExitActor != None )
					Spawn(FootRegion.Zone.ExitActor,,,Location - CollisionHeight * vect(0,0,1));
			}
		}
		else if ( newFootZone.bWaterZone && (Role==ROLE_Authority) )
		{
			splashSize = FClamp(0.000025 * Mass * (300 - 0.5 * FMax(-500, Velocity.Z)), 1.0, 4.0 );
			if ( newFootZone.EntrySound != None )
			{
				HitActor = Trace(HitLocation, HitNormal, 
						Location - (CollisionHeight + 40) * vect(0,0,0.8), Location - CollisionHeight * vect(0,0,0.8), false);
				if ( HitActor == None )
					PlaySound(newFootZone.EntrySound, SLOT_Misc, 2 * splashSize);
				else 
					PlaySound(WaterStep, SLOT_Misc, 1.5 + 0.5 * splashSize);
			}
			if( newFootZone.EntryActor != None )
			{
				splash = Spawn(newFootZone.EntryActor,,,Location - CollisionHeight * vect(0,0,1));
				if ( splash != None )
					splash.DrawScale = splashSize;
			}
			//log("Feet entering water");
		}
	}
	
	if (FootRegion.Zone.bPainZone)
	{
		if ( !newFootZone.bPainZone && !HeadRegion.Zone.bWaterZone )
			PainTime = -1.0;
	}
	else if (newFootZone.bPainZone)
		PainTime = 0.01;
}
	
event HeadZoneChange(ZoneInfo newHeadZone)
{
	if ( Level.NetMode == NM_Client )
		return;
	if (HeadRegion.Zone.bWaterZone)
	{
		if (!newHeadZone.bWaterZone)
		{
			if ( bIsPlayer && (PainTime > 0) && (PainTime < 8) )
				Gasp();
			if ( Inventory != None )
				Inventory.ReduceDamage(0, 'Breathe', Location); //inform inventory of zone change
			bDrowning = false;
			if ( !FootRegion.Zone.bPainZone )
				PainTime = -1.0;
		}
	}
	else
	{
		if (newHeadZone.bWaterZone)
		{
			if ( !FootRegion.Zone.bPainZone )
				PainTime = UnderWaterTime;
			if ( Inventory != None )
				Inventory.ReduceDamage(0, 'Drowned', Location); //inform inventory of zone change
			//log("Can't breathe");
		}
	}
}

event SpeechTimer();

//Pain timer just expired.
//Check what zone I'm in (and which parts are)
//based on that cause damage, and reset PainTime
	
event PainTimer()
{
	local float depth;

	//log("Pain Timer");
	if ( (Health < 0) || (Level.NetMode == NM_Client) )
		return;
		
	if ( FootRegion.Zone.bPainZone )
	{
		depth = 0.4;
		if (Region.Zone.bPainZone)
			depth += 0.4;
		if (HeadRegion.Zone.bPainZone)
			depth += 0.2;

		if (FootRegion.Zone.DamagePerSec > 0)
		{
			if ( IsA('PlayerPawn') )
				Level.Game.SpecialDamageString = FootRegion.Zone.DamageString;
			TakeDamage(int(float(FootRegion.Zone.DamagePerSec) * depth), None, Location, vect(0,0,0), FootRegion.Zone.DamageType); 
		}
		else
			Health = Min(Default.Health, Health - depth * FootRegion.Zone.DamagePerSec);

		if (Health > 0)
			PainTime = 1.0;
	}
	else if ( HeadRegion.Zone.bWaterZone )
	{
		TakeDamage(5, None, Location, vect(0,0,0), 'drowned'); 
		if ( Health > 0 )
			PainTime = 2.0;
	}
}		

function bool CheckWaterJump(out vector WallNormal)
{
	local actor HitActor;
	local vector HitLocation, HitNormal, checkpoint, start, checkNorm, Extent;

	if (CarriedDecoration != None)
		return false;
	checkpoint = vector(Rotation);
	checkpoint.Z = 0.0;
	checkNorm = Normal(checkpoint);
	checkPoint = Location + CollisionRadius * checkNorm;
	Extent = CollisionRadius * vect(1,1,0);
	Extent.Z = CollisionHeight;
	HitActor = Trace(HitLocation, HitNormal, checkpoint, Location, true, Extent);
	if ( (HitActor != None) && (Pawn(HitActor) == None) )
	{
		WallNormal = -1 * HitNormal;
		start = Location;
		start.Z += 1.1 * MaxStepHeight;
		checkPoint = start + 2 * CollisionRadius * checkNorm;
		HitActor = Trace(HitLocation, HitNormal, checkpoint, start, true);
		if (HitActor == None)
			return true;
	}

	return false;
}

function bool SwitchToBestWeapon()
{
	local float rating;
	local int usealt;

	if ( Inventory == None )
		return false;

	PendingWeapon = Inventory.RecommendWeapon(rating, usealt);

	if ( PendingWeapon == None )
		return false;

	if ( Weapon == None )
		ChangedWeapon();
	else if ( Weapon != PendingWeapon )
		Weapon.PutDown();

	return (usealt > 0);
}

State Dying
{
ignores SeePlayer, EnemyNotVisible, HearNoise, KilledBy, Trigger, Bump, HitWall, HeadZoneChange, FootZoneChange, ZoneChange, Falling, WarnTarget, Died;

	function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
							Vector momentum, name damageType)
	{
		if ( bDeleteMe )
			return;
		Health = Health - Damage;
		Momentum = Momentum/Mass;
		AddVelocity( momentum ); 
		if ( !bHidden && Gibbed() )
		{
			bHidden = true;
			SpawnGibbedCarcass();
			if ( bIsPlayer )
				HidePlayer();
			else
				Destroy();
		}
	}

	function Timer()
	{
		if ( !bHidden )
		{
			bHidden = true;
			SpawnCarcass();
			if ( bIsPlayer )
				HidePlayer();
			else
				Destroy();
		}
	}

	event Landed(vector HitNormal)
	{
		SetPhysics(PHYS_None);
	}

	function BeginState()
	{
		SetTimer(0.3, false);
	}
}

state GameEnded
{
ignores SeePlayer, HearNoise, KilledBy, Bump, HitWall, HeadZoneChange, FootZoneChange, ZoneChange, Falling, TakeDamage, WarnTarget;

	function BeginState()
	{
		SetPhysics(PHYS_None);
		HidePlayer();
	}
}

defaultproperties
{
     OrthoZoom=+40000.000000
     FovAngle=+00090.000000
     Health=100
     Visibility=128
     SightRadius=+02500.000000
     HearingThreshold=+00001.000000
     AttitudeToPlayer=ATTITUDE_Hate
     Intelligence=BRAINS_MAMMAL
     MaxDesiredSpeed=+00001.000000
     GroundSpeed=+00320.000000
     WaterSpeed=+00200.000000
     AccelRate=+00500.000000
     JumpZ=+00325.000000
     MaxStepHeight=+00025.000000
     noise1time=-00010.000000
     noise2time=-00010.000000
     AvgPhysicsTime=+00000.100000
     SoundDampening=+00001.000000
     DamageScaling=+00001.000000
     bDirectional=True
     bCanTeleport=True
     bIsKillGoal=True
	 bStasis=True
	 bIsPawn=True
     SoundRadius=9
	 SoundVolume=240
	 TransientSoundVolume=+00002.000000
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
     bProjTarget=True
     bRotateToDesired=True
     RotationRate=(Pitch=4096,Yaw=50000,Roll=3072)
	 Texture=S_Pawn
     RemoteRole=ROLE_SimulatedProxy
	 AnimSequence=Fighter
     NetPriority=+00004.000000
}
