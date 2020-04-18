//=============================================================================
// PlayerPawn.
// player controlled pawns
// Note that Pawns which implement functions for the PlayerTick messages
// must handle player control in these functions
//=============================================================================
class PlayerPawn expands Pawn
	config
	intrinsic;

// Player info.
var const player Player;
var	globalconfig string[64] Password;	// for restarting coop savegames

var	travel	  float DodgeClickTimer; // max double click interval for dodge move
var(Movement) globalconfig float	DodgeClickTime;
var(Movement) globalconfig float Bob;
var float bobtime;

// Camera info.
var int ShowFlags;
var int RendMap;
var int Misc1;
var int Misc2;
var actor ViewTarget;
var vector FlashScale, FlashFog;
var HUD	myHUD;
var ScoreBoard Scoring;
var class<hud> HUDType;
var class<scoreboard> ScoringType;
var textbuffer CarryInfo;

var float DesiredFlashScale, ConstantGlowScale;
var vector DesiredFlashFog, ConstantGlowFog;
var float DesiredFOV;

// Music info.
var music Song;
var byte  SongSection;
var byte  CdTrack;
var EMusicTransition Transition;

var float shaketimer; // player uses this for shaking view
var int shakemag;	// max magnitude in degrees of shaking
var float shakevert; // max vertical shake magnitude
var float maxshake;
var float verttimer;
var(Pawn) class<carcass> CarcassType;
var travel globalconfig float MyAutoAim;
var travel globalconfig float Handedness;
var(Sounds) sound JumpSound;

// Player control flags
var bool		bAdmin;
var() globalconfig bool 		bLookUpStairs;	// look up/down stairs (player)
var() globalconfig bool		bSnapToLevel;	// Snap to level eyeheight when not mouselooking
var() globalconfig bool		bAlwaysMouseLook;
var globalconfig bool 		bKeyboardLook;	// no snapping when true
var bool		bWasForward;	// used for dodge move 
var bool		bWasBack;
var bool		bWasLeft;
var bool		bWasRight;
var bool		bEdgeForward;
var bool		bEdgeBack;
var bool		bEdgeLeft;
var bool 		bEdgeRight;
var bool		bIsCrouching;
var	bool		bShakeDir;			
var bool		bAnimTransition;
var bool		bCountJumps;
var bool		bIsTurning;
var bool		bFrozen;
var globalconfig bool	bInvertMouse;
var bool		bShowScores;
var bool		bShowMenu;
var bool		bSpecialMenu;
var bool		bWokeUp;
var bool		bPressedJump;
var bool		bUpdatePosition;
var bool		bDelayedCommand;
var bool		bRising;
var bool		bReducedVis;

var class<menu> SpecialMenu;
var string[255] DelayedCommand;
var globalconfig float		MouseSensitivity;

var globalconfig name	WeaponPriority[20]; //weapon class priorities (9 is highest)

var globalconfig int NetSpeed;
var float SmoothMouseX, SmoothMouseY;

// Input axes.
var input float 
	aBaseX, aBaseY, aBaseZ,
	aMouseX, aMouseY,
	aForward, aTurn, aStrafe, aUp, 
	aLookUp, aExtra4, aExtra3, aExtra2,
	aExtra1, aExtra0;

// Move Buffering
var SavedMove SavedMoves;
var SavedMove FreeMoves;
var(Networking) globalconfig float ClientNetMinDelta;		//minimum delta time for moves sent to server (network play only )
var(Networking) globalconfig float ServerNetMinDelta;	//minimum netmindelta allowed by the server
var float CurrentTimeStamp;
var float LastUpdateTime;

// Progess Indicator
var string[128] ProgressMessage;
var string[128] ProgressMessageTwo;
var float ProgressTimeOut;

replication
{
	// Things the server should send to the client.
	reliable if( Role==ROLE_Authority && bNetOwner )
		ViewTarget, ScoringType, HUDType, ServerNetMinDelta;

	// Things the client should send to the server
	reliable if ( Role<ROLE_Authority )
		WeaponPriority, Password;

	// Functions client can call.
	unreliable if( Role<ROLE_Authority )
		Fire, AltFire, ServerRequestScores;
	reliable if( Role<ROLE_Authority )
		ShowPath, RememberSpot, Say, RestartLevel, SwitchWeapon, Pause, SetPause, ServerSetHandedness,
		PrevItem, ActivateItem, ShowInventory, Grab, ServerFeignDeath, ServerSetWeaponPriority,
		ChangeName, ChangeTeam, God, Suicide, ViewClass, ViewPlayer, ServerSetSloMo, ServerAddBots,
		PlayersOnly, ThrowWeapon, ServerRestartPlayer, NeverSwitchOnPickup, 
		PrevWeapon, NextWeapon, ServerReStartGame, ServerUpdateWeapons, Taunt, ServerChangeSkin,
		SwitchLevel, SwitchCoopLevel, Kick, KillAll, Summon;
	unreliable if ( Role== ROLE_AutonomousProxy )
		ServerMove;
	unreliable if( Role<ROLE_AutonomousProxy )
		Fly, Walk, Ghost, Jump, FeignDeath;

	// Functions server can call.
	reliable if( Role==ROLE_Authority )
		ClientAdjustGlow, ClientTravel, ClientSetMusic, SetDesiredFOV;
	unreliable if( Role==ROLE_Authority )
		SetFOVAngle, ClientShake, ClientFlash, ClientRefreshScores, QuickRefreshScores;
	unreliable if( Role==ROLE_Authority && (RemoteRole==ROLE_AutonomousProxy) )
		ClientAdjustPosition;

	// Input variables.
	unreliable if( Role<ROLE_AutonomousProxy )
		aBaseX, aBaseY, aBaseZ,
		aMouseX, aMouseY,
		aForward, aTurn, aStrafe, aUp, 
		aLookUp, aExtra4, aExtra3, aExtra2,
		aExtra1, aExtra0;
}

//
// Intrinsic client-side functions.
//
intrinsic event ClientTravel( string[240] URL, ETravelType TravelType, bool bItems );
intrinsic event ClientMessage( coerce string[255] S );
intrinsic(537) final function ConsoleCommand( string[255] Command );
intrinsic(542) final function string[255] ConsoleCommandResult ( string[255] Command );
intrinsic(544) final function ResetKeyboard();
intrinsic(546) final function UpdateURL(string[128] NewOption);

// Intrinsic server-side functions
//
intrinsic(3971) final function AutonomousPhysics(float DeltaSeconds);

// ServerMove() - pass accel in components so it doesn't get rounded

function ServerMove(float TimeStamp, float AccelX, float AccelY, float AccelZ, 
					float LocX, float LocY, float LocZ,
					byte MoveFlags, eDodgeDir DodgeMove, rotator Rot, int ViewPitch, int ViewYaw)
{
	local float DeltaTime, clientErr;
	local rotator DeltaRot;
	local vector Accel, LocDiff;

	Accel.X = AccelX;
	Accel.Y = AccelY;
	Accel.Z = AccelZ;

	if ( CurrentTimeStamp > TimeStamp )
		return;

	DeltaTime = TimeStamp - CurrentTimeStamp;
	DeltaRot = (Rotation - Rot);
	CurrentTimeStamp = TimeStamp;
	SetRotation(Rot);
	ViewRotation.Pitch = ViewPitch;
	ViewRotation.Yaw = ViewYaw;
	ViewRotation.Roll = 0;

	if ( Level.Pauser == "" )
		MoveAutonomous(DeltaTime, MoveFlags, DodgeMove, Accel, DeltaRot);
	if ( Level.TimeSeconds - LastUpdateTime > 0.15 )
		ClientErr = 10000;
	else
	{
		LocDiff.X = Location.X - LocX;
		LocDiff.Y = Location.Y - LocY;
		LocDiff.Z = Location.Z - LocZ;
		ClientErr = LocDiff Dot LocDiff;
	}

	if ( ClientErr > 1 )
	{
		//log("Client Error at "$TimeStamp$" is "$ClientErr$" with Velocity "$Velocity$" LocDiff "$LocDiff$" Physics "$Physics);
		LastUpdateTime = Level.TimeSeconds;
		ClientAdjustPosition(TimeStamp, GetStateName(), Physics, Location.X, Location.Y, Location.Z, Velocity.X, Velocity.Y, Velocity.Z);
	}
	//log("Server moved "$self$" stamp "$TimeStamp$" location "$Location$" Acceleration "$Acceleration$" Velocity "$Velocity);
}	

function ProcessMove ( float DeltaTime, vector newAccel, eDodgeDir DodgeMove, rotator DeltaRot)
{
	Acceleration = newAccel;
}

final function MoveAutonomous(float DeltaTime, byte MoveFlags, eDodgeDir DodgeMove, vector newAccel, rotator DeltaRot)
{
	bRun = MoveFlags & 1;
	bDuck = MoveFlags & 2;
	bPressedJump = ((MoveFlags & 4) > 0);

	HandleWalking();
	ProcessMove(DeltaTime, newAccel, DodgeMove, DeltaRot);	
	AutonomousPhysics(DeltaTime);
}

// ClientAdjustPosition - pass newloc and newvel in components so it doesn't get rounded

function ClientAdjustPosition(float TimeStamp, name newState, EPhysics newPhysics,
							 float NewLocX, float NewLocY, float NewLocZ, float NewVelX, float NewVelY, float NewVelZ)
{
	local Decoration Carried;
	local vector OldLoc, NewLocation;

	if ( CurrentTimeStamp > TimeStamp )
		return;
	CurrentTimeStamp = TimeStamp;

	NewLocation.X = NewLocX;
	NewLocation.Y = NewLocY;
	NewLocation.Z = NewLocZ;
	Velocity.X = NewVelX;
	Velocity.Y = NewVelY;
	Velocity.Z = NewVelZ;
	//log("Client adjust "$self$" stamp "$TimeStamp$" location "$Location$" new location "$NewLocation);

	Carried = CarriedDecoration;
	OldLoc = Location;
	SetLocation(NewLocation);
	if ( Carried != None )
	{
		CarriedDecoration = Carried;
		CarriedDecoration.SetLocation(NewLocation + CarriedDecoration.Location - OldLoc);
		CarriedDecoration.SetPhysics(PHYS_None);
		CarriedDecoration.SetBase(self);
	}
	SetPhysics(newPhysics);
	//FIXME - don't do this state update if client dead???
	if ( !IsInState(newState) )
		GotoState(newState);

	bUpdatePosition = true;
}

function ClientUpdatePosition()
{
	local SavedMove CurrentMove;
	local int realbRun, realbDuck;
	local bool bRealJump;

	bUpdatePosition = false;
	realbRun= bRun;
	realbDuck = bDuck;
	bRealJump = bPressedJump;
	CurrentMove = SavedMoves;
	while ( CurrentMove != None )
	{
		if ( CurrentMove.TimeStamp <= CurrentTimeStamp )
		{
			SavedMoves = CurrentMove.NextMove;
			CurrentMove.NextMove = FreeMoves;
			FreeMoves = CurrentMove;
			FreeMoves.Clear();
			CurrentMove = SavedMoves;
		}
		else
		{
			MoveAutonomous(CurrentMove.Delta, CurrentMove.MoveFlags, CurrentMove.DodgeMove, CurrentMove.Acceleration, rot(0,0,0));
			CurrentMove = CurrentMove.NextMove;
		}
	}
	bDuck = realbDuck;
	bRun = realbRun;
	bPressedJump = bRealJump;
	//log("Client adjusted "$self$" stamp "$CurrentTimeStamp$" location "$Location$" dodge "$DodgeDir);
}

final function SavedMove GetFreeMove()
{
	local SavedMove s;

	if ( FreeMoves == None )
		return Spawn(class'SavedMove');
	else
	{
		s = FreeMoves;
		FreeMoves = FreeMoves.NextMove;
		s.NextMove = None;
		return s;
	}	
}

final function ReplicateMove(float DeltaTime, vector NewAccel, eDodgeDir DodgeMove, rotator DeltaRot)
{
	local SavedMove NewMove;
	local float MinDelta;
	local bool bMustSend;

	MinDelta = ClientNetMinDelta;
	MinDelta = FClamp(MinDelta, ServerNetMinDelta, 0.1);
	if ( SavedMoves == None )
	{
		SavedMoves = GetFreeMove();
		NewMove = SavedMoves;
	}
	else
	{
		NewMove = SavedMoves;
		while ( NewMove.NextMove != None )
			NewMove = NewMove.NextMove;
		if ( NewMove.bSent )
		{
			NewMove.NextMove = GetFreeMove();
			NewMove = NewMove.NextMove;
		}
	}

	if ( NewMove.Delta > 0 )
	{
		bMustSend = true; //send at least every other packet
		NewMove.Acceleration = (NewMove.Delta * NewMove.Acceleration + DeltaTime * NewAccel)/(NewMove.Delta + DeltaTime);
	}
	else
		NewMove.Acceleration = NewAccel;

	NewMove.DodgeMove = DodgeMove;
	NewMove.TimeStamp = Level.TimeSeconds;
	NewMove.Delta += DeltaTime;
	if ( bRun > 0 )
		NewMove.MoveFlags = NewMove.MoveFlags | 1;
	if ( bDuck > 0 )
		NewMove.MoveFlags = NewMove.MoveFlags | 2;
	if ( bPressedJump )
		NewMove.MoveFlags = NewMove.MoveFlags | 4;

	ProcessMove(DeltaTime, NewAccel, DodgeMove, DeltaRot);
	AutonomousPhysics(DeltaTime);
	if ( bMustSend || (NewMove.Delta >= MinDelta) || bPressedJump || (NewMove.DodgeMove != DODGE_None)
			|| ((Velocity != vect(0,0,0)) && ((NewMove.Acceleration Dot Velocity) <= 0)) )
	{ 
		NewMove.bSent = true;
		ServerMove(NewMove.TimeStamp, NewMove.Acceleration.X, NewMove.Acceleration.Y, NewMove.Acceleration.Z,
					Location.X, Location.Y, Location.Z,
					NewMove.MoveFlags, NewMove.DodgeMove, Rotation, ViewRotation.Pitch, ViewRotation.Yaw);
	}
	//log("Replicated "$self$" stamp "$NewMove.TimeStamp$" location "$Location$" dodge "$NewMove.DodgeMove$" to "$DodgeDir);
}

final function HandleWalking()
{
	local rotator carried;

	bIsWalking = (bRun != 0) || (bDuck != 0); 
	if ( (Role == ROLE_Authority) && (standingcount == 0) ) 
		CarriedDecoration = None;
	if ( CarriedDecoration != None ) //verify its still in front
	{
		bIsWalking = true;
		if ( Role == ROLE_Authority )
		{
			carried = Rotator(CarriedDecoration.Location - Location);
			carried.Yaw = ((carried.Yaw & 65535) - (Rotation.Yaw & 65535)) & 65535;
			if ( (carried.Yaw > 3072) && (carried.Yaw < 62463) )
				DropDecoration();
		}
	}
}

event Destroyed()
{
	Super.Destroyed();
	if ( myHud != None )
		myHud.Destroy();
	if ( Scoring != None )
		Scoring.Destroy();
}

function ServerReStartGame()
{
	Level.Game.RestartGame();
}

function PlayHit(float Damage, vector HitLocation, name damageType, float MomentumZ)
{
	Level.Game.SpecialDamageString = "";
}

function SetFOVAngle(float newFOV)
{
	FOVAngle = newFOV;
}

function QuickRefreshScores(float NewScore, byte offset, byte NewNum)
{
	if ( Scoring != None )
		Scoring.QuickRefreshScores(NewScore, offset, NewNum);
}

function ClientRefreshScores(string[32] NewName, float NewScore, byte offset, byte NewNum)
{
	if ( Scoring != None )
		Scoring.RefreshScores(NewName, NewScore, offset, NewNum);
}

function ServerRequestScores(string[32] CurrentName, byte offset)
{
	if ( Level.Game.Scores != None )
		Level.Game.Scores.UpdateNext(CurrentName, offset, self);
}
	 
function ClientFlash( float scale, vector fog )
{
	DesiredFlashScale = scale;
	DesiredFlashFog = 0.001 * fog;
}

function ClientAdjustGlow( float scale, vector fog )
{
	ConstantGlowScale += scale;
	ConstantGlowFog += 0.001 * fog;
}

function ClientShake(vector shake)
{
	if ( (shakemag < shake.X) || (shaketimer <= 0.01 * shake.Y) )
	{
		shakemag = shake.X;
		shaketimer = 0.01 * shake.Y;	
		maxshake = 0.01 * shake.Z;
		verttimer = 0;
		ShakeVert = -1.1 * maxshake;
	}
}

function ShakeView( float shaketime, float RollMag, float vertmag)
{
	local vector shake;

	shake.X = RollMag;
	shake.Y = 100 * shaketime;
	shake.Z = 100 * vertmag;
	ClientShake(shake);
}

function ClientSetMusic( music NewSong, byte NewSection, byte NewCdTrack, EMusicTransition NewTransition )
{
	Song        = NewSong;
	SongSection = NewSection;
	CdTrack     = NewCdTrack;
	Transition  = NewTransition;
}

function ServerFeignDeath()
{
	PendingWeapon = Weapon;
	if ( Weapon != None )
		Weapon.PutDown();
	GotoState('FeigningDeath');
}

function ServerSetHandedness( float hand)
{
	Handedness = hand;
	if ( Weapon != None )
		Weapon.SetHand(Handedness);
}

function ServerReStartPlayer()
{
	//log("calling restartplayer in dying with netmode "$Level.NetMode);
	if ( Level.NetMode == NM_Client )
		return;
	if( Level.Game.RestartPlayer(self) )
	{
		//log("server restart client");
		Level.Game.StartPlayer(self);
		ClientReStart();
	}
	else
		log("Restartplayer failed");
}

function ServerChangeSkin(coerce string[64] SkinName)
{
	local texture NewSkin;

	if ( Level.Game.bCanChangeSkin )
	{
		NewSkin = texture(DynamicLoadObject(SkinName, class'Texture'));
		if ( NewSkin != None )
			Skin = NewSkin;
		else
		{
			NewSkin = texture(DynamicLoadObject(string(Mesh)$"Skins."$SkinName, class'Texture'));
			if ( NewSkin != None )
				Skin = NewSkin;
		}
	}
}

//*************************************************************************************
// Normal gameplay execs
// Type the name of the exec function at the console to execute it
	
exec function Jump( optional float F )
{
	bPressedJump = true;
}

exec function CauseEvent( name N )
{
	local actor A;
	if( (bAdmin || (Level.Netmode == NM_Standalone)) && (N != '') )
		foreach AllActors( class 'Actor', A, N )
			A.Trigger( Self, Self );
}

exec function Taunt( name Sequence )
{
	if ( GetAnimGroup(Sequence) == 'Gesture' ) 
		PlayAnim(Sequence, 0.7, 0.2);
}

exec function FeignDeath()
{
}

exec function Grab()
{
	if (CarriedDecoration == None)
		GrabDecoration();
	else
		DropDecoration();
}

// Send a message to all players.
exec function Say( string[255] S )
{
	BroadcastMessage( PlayerName$":"$S, true );
}

exec function RestartLevel()
{
	if( bAdmin || Level.Netmode==NM_Standalone )
		ClientTravel( "?restart", TRAVEL_Relative, false );
}

exec function LocalTravel( string[128] URL )
{
	if( bAdmin || Level.Netmode==NM_Standalone )
		ClientTravel( URL, TRAVEL_Relative, true );
}

exec function ThrowWeapon()
{
	if( Level.NetMode == NM_Client )
		return;
	if( Weapon==None || Weapon.Class==Level.Game.DefaultWeapon )
		return;
	Weapon.Velocity = Vector(ViewRotation) * 500 + vect(0,0,220);
	TossWeapon();
	if ( Weapon == None )
		SwitchToBestWeapon();
}

exec function SetDesiredFOV(float F)
{
	DesiredFOV = FClamp(F, 1, 170);
}

/* PrevWeapon()
- switch to previous inventory group weapon
*/
exec function PrevWeapon()
{
	local int prevGroup;
	local Inventory inv;
	local Weapon realWeapon;

	if( bShowMenu || Level.Pauser!="" )
		return;
	if ( Weapon == None )
	{
		SwitchToBestWeapon();
		return;
	}
	prevGroup = 0;
	realWeapon = Weapon;
	if ( PendingWeapon != None )
		Weapon = PendingWeapon;
	PendingWeapon = None;
	
	for (inv=Inventory; inv!=None; inv=inv.Inventory)
		if ( inv.IsA('Weapon') && (inv.InventoryGroup < Weapon.InventoryGroup) && (Weapon(inv).AmmoType.AmmoAmount>0) 
			&& (inv.InventoryGroup >= prevGroup) )
		{
			prevGroup = inv.InventoryGroup;
			PendingWeapon = Weapon(inv);
		}

	prevGroup = Weapon.InventoryGroup;
	if ( PendingWeapon == None )
		for (inv=Inventory; inv!=None; inv=inv.Inventory)
			if ( inv.IsA('Weapon') && (inv.InventoryGroup > prevGroup)  && Weapon(inv).AmmoType.AmmoAmount>0)
			{
				prevGroup = inv.InventoryGroup;
				PendingWeapon = Weapon(inv);
			}

	Weapon = realWeapon;
	if ( PendingWeapon == None )
		return;

	if ( Weapon != PendingWeapon )
		Weapon.PutDown();
}

/* NextWeapon()
- switch to next inventory group weapon
*/
exec function NextWeapon()
{
	local int nextGroup;
	local Inventory inv;
	local Weapon realWeapon;

	if( bShowMenu || Level.Pauser!="" )
		return;
	if ( Weapon == None )
	{
		SwitchToBestWeapon();
		return;
	}
	nextGroup = 100;
	realWeapon = Weapon;
	if ( PendingWeapon != None )
		Weapon = PendingWeapon;
	PendingWeapon = None;

	for (inv=Inventory; inv!=None; inv=inv.Inventory)
		if ( inv.IsA('Weapon') && (inv.InventoryGroup > Weapon.InventoryGroup) && (Weapon(inv).AmmoType.AmmoAmount>0) 
			&& (inv.InventoryGroup < nextGroup) )
		{
			nextGroup = inv.InventoryGroup;
			PendingWeapon = Weapon(inv);
		}

	nextGroup = Weapon.InventoryGroup;
	if ( PendingWeapon == None )
		for (inv=Inventory; inv!=None; inv=inv.Inventory)
			if ( inv.IsA('Weapon') && (inv.InventoryGroup < nextGroup) && Weapon(inv).AmmoType.AmmoAmount>0) 
			{
				nextGroup = inv.InventoryGroup;
				PendingWeapon = Weapon(inv);
			}

	Weapon = realWeapon;
	if ( PendingWeapon == None )
		return;

	if ( Weapon != PendingWeapon )
		Weapon.PutDown();
}

exec function QuickSave()
{
	if ( (Health > 0) 
		&& (Level.NetMode == NM_Standalone)
		&& !Level.Game.IsA('DeathMatchGame') )
	{
		ClientMessage("Quick Saving");
		ConsoleCommand("SaveGame 9");
	}
}

exec function QuickLoad()
{
	ClientTravel( "?load=9", TRAVEL_Absolute, false);
}

exec function Kick( string[32] S ) 
{
	local Pawn aPawn;
	if ( !bAdmin )
		return;
	aPawn = Level.PawnList;
	while ( aPawn != None )
	{
		if ( aPawn.bIsPlayer && (aPawn.PlayerName ~= S) )
			aPawn.Destroy();
		aPawn = aPawn.nextPawn;
	}
}
  
// Try to set the pause state; returns success indicator.
function bool SetPause( BOOL bPause )
{
	if( Level.Game.bPauseable || bAdmin || Level.Netmode==NM_Standalone )
	{
		if( bPause )
			Level.Pauser=PlayerName;
		else
			Level.Pauser="";
		return True;
	}
	else return False;
}

exec function SetNetSpeed( int newspeed )
{
	ChangeNetSpeed(newspeed);
	SaveConfig();
}

function  ChangeNetSpeed( int newspeed )
{
	if (newspeed > 1000 )
		NetSpeed = newspeed;
	else if ( newspeed == 0 )
		NetSpeed = 2600; // 28.8 modem
	else if ( newspeed == 1 )
		NetSpeed = 3600; // 56K modem
	else
		NetSpeed = 20000; // LAN
}

// Try to pause the game.
exec function Pause()
{
	if ( bShowMenu )
		return;
	if( !SetPause(Level.Pauser=="") )
		ClientMessage( "Game is not pauseable" );
}

// Activate specific inventory item
exec function ActivateInventoryItem( class InvItem )
{
	local Inventory Inv;

	Inv = FindInventoryType(InvItem);
	if ( Inv != None )
		Inv.Activate();
}

// Translator Hotkey
exec function ActivateTranslator()
{
	if ( bShowMenu || Level.Pauser!="" )
		return;
	If (Inventory!=None) Inventory.ActivateTranslator(False);
}

// Translator Hotkey
exec function ActivateHint()
{
	if ( bShowMenu || Level.Pauser!="" )
		return;
	If (Inventory!=None) Inventory.ActivateTranslator(True);
}

// HUD
exec function ChangeHud()
{
	if ( myHud != None )
		myHUD.ChangeHud(1);
	myHUD.SaveConfig();
}

// Crosshair
exec function ChangeCrosshair()
{
	if ( myHud != None ) 
		myHUD.ChangeCrosshair(1);
	myHUD.SaveConfig();
}


event PreRender( canvas Canvas )
{
	if ( myHud != None )	
		myHUD.PreRender(Canvas);
	else if ( (Viewport(Player) != None) && (HUDType != None) )
		myHUD = spawn(HUDType, self);
}

event PostRender( canvas Canvas )
{
	if ( myHud != None )	
		myHUD.PostRender(Canvas);
	else if ( (Viewport(Player) != None) && (HUDType != None) )
		myHUD = spawn(HUDType, self);
}

//=============================================================================
// Inventory-related input notifications.

// Handle function keypress for F1-F10.
exec function FunctionKey( byte Num )
{
}

// The player wants to switch to weapon group numer I.
exec function SwitchWeapon (byte F )
{
	local weapon newWeapon;

	if ( bShowMenu || Level.Pauser!="" )
	{
		if ( myHud != None )
			myHud.InputNumber(F);
		return;
	}
	if ( Inventory == None )
		return;
	if ( (Weapon != None) && (Weapon.Inventory != None) )
		newWeapon = Weapon.Inventory.WeaponChange(F);
	else
		newWeapon = None;	
	if ( newWeapon == None )
		newWeapon = Inventory.WeaponChange(F);
	if ( newWeapon == None )
		return;

	if ( Weapon == None )
	{
		PendingWeapon = newWeapon;
		ChangedWeapon();
	}
	else if ( (Weapon != newWeapon) && Weapon.PutDown() )
		PendingWeapon = newWeapon;
}

// The player wants to select previous item
exec function PrevItem()
{
	local Inventory Inv, LastItem;

	if ( bShowMenu || Level.Pauser!="" )
		return;
	if (SelectedItem==None) {
		SelectedItem = Inventory.SelectNext();
		Return;
	}
	if (SelectedItem.Inventory!=None) 
		for( Inv=SelectedItem.Inventory; Inv!=None; Inv=Inv.Inventory ) {
			if (Inv==None) Break;
			if (Inv.bActivatable) LastItem=Inv;
		}
	for( Inv=Inventory; Inv!=SelectedItem; Inv=Inv.Inventory ) {
		if (Inv==None) Break;
		if (Inv.bActivatable) LastItem=Inv;
	}
	if (LastItem!=None) {
		SelectedItem = LastItem;
		ClientMessage(SelectedItem.class$" Selected");
	}
}

// The player wants to active selected item
exec function ActivateItem()
{
	if( bShowMenu || Level.Pauser!="" )
		return;
	if (SelectedItem!=None) 
		SelectedItem.Activate();
}

// The player wants to fire.
exec function Fire( optional float F )
{
	if( bShowMenu || Level.Pauser!="" )
		return;
	if( Weapon!=None )
	{
		Weapon.bPointing = true;
		Weapon.Fire(F);
		PlayFiring();
	}
}

// The player wants to alternate-fire.
exec function AltFire( optional float F )
{
	if( bShowMenu || Level.Pauser!="" )
		return;
	if( Weapon!=None )
	{
		Weapon.bPointing = true;
		Weapon.AltFire(F);
		PlayFiring();
	}
}

//Player Jumped
function DoJump( optional float F )
{
	if ( CarriedDecoration != None )
		return;
	if ( !bIsCrouching && (Physics == PHYS_Walking) )
	{
		if ( Role == ROLE_Authority )
			PlaySound(JumpSound, SLOT_Talk, 1.5, true, 1200, 1.0 );
		if ( (Level.Game != None) && (Level.Game.Difficulty > 0) )
			MakeNoise(0.1 * Level.Game.Difficulty);
		PlayInAir();
		Velocity.Z = JumpZ;
		if ( Base != Level )
			Velocity.Z += Base.Velocity.Z; 
		SetPhysics(PHYS_Falling);
		if ( bCountJumps && (Role == ROLE_Authority) )
			Inventory.OwnerJumped();
	}
}

exec function Suicide()
{
	KilledBy( None );
}

exec function AlwaysMouseLook( Bool B )
{
	ChangeAlwaysMouseLook(B);
	SaveConfig();
}

function ChangeAlwaysMouseLook(Bool B)
{
	bAlwaysMouseLook = B;
	if ( bAlwaysMouseLook )
	{
		bSnapToLevel = false;
		bLookUpStairs = false;
	}
}
	
exec function SnapView( bool B )
{
	ChangeSnapView(B);
	SaveConfig();
}

function ChangeSnapView( bool B )
{
	bSnapToLevel = B;
	if ( bSnapToLevel )
		bAlwaysMouseLook = false;
}
	
exec function StairLook( bool B )
{
	ChangeStairLook(B);
	SaveConfig();
}

function ChangeStairLook( bool B )
{
	bLookUpStairs = B;
	if ( bLookUpStairs )
		bAlwaysMouseLook = false;
}

exec function SetDodgeClickTime( float F )
{
	ChangeDodgeClickTime(F);
	SaveConfig();
}

function ChangeDodgeClickTime( float F )
{
	DodgeClickTime = FMin(0.3, F);
}

exec function SetName( coerce string[20] S )
{
	ChangeName(S);
	SaveConfig();
}

function ChangeName( coerce string[20] S )
{
	Level.Game.ChangeName(self, S, true);
}

exec function SetTeam( coerce string[20] S )
{
	ChangeTeam(S);
	SaveConfig();
}

function ChangeTeam( coerce string[20] S )
{
	if ( Level.Game.ChangeTeam(self, S) )
		TeamName = S;
}
	
exec function SetAutoAim( float F )
{
	ChangeAutoAim(F);
	SaveConfig();
}

function ChangeAutoAim( float F )
{
	MyAutoAim = FMax(Level.Game.AutoAim, F);
}

exec function PlayersOnly()
{
	if ( Level.Netmode != NM_Standalone )
		return;

	Level.bPlayersOnly = !Level.bPlayersOnly;
}

exec function SetHand( string[32] S )
{
	ChangeSetHand(S);
	SaveConfig();
}

function ChangeSetHand( string[32] S )
{
	if ( S ~= "Left" )
		Handedness = 1;
	else if ( S~= "Right" )
		Handedness = -1;
	else if ( S ~= "Center" )
		Handedness = 0;
	ServerSetHandedness(Handedness);
}

exec function ViewPlayer( string[32] S )
{
	local pawn other;

	other = Level.pawnList;
	while ( other != None )
	{
		if ( other.bIsPlayer && (other.PlayerName ~= S) )
			break;
		other = other.nextPawn;
	}

	if ( (other != None) && Level.Game.CanSpectate(self, other) )
	{
		ClientMessage("Now viewing from "$other);
		if ( other == self)
			ViewTarget = None;
		else
			ViewTarget = other;
	}
	else
		ClientMessage("Failed to change view");

	bBehindView = ( ViewTarget != None );
}

exec function ViewClass( class<actor> aClass )
{
	local actor other, first;

	first = None;
	ForEach AllActors( aClass, other )
	{
		if ( first == None )
			first = other;
		if ( other == ViewTarget ) 
			first = None;
	}  

	if ( (first != None) && Level.Game.CanSpectate(self, other) )
	{
		ClientMessage("Now viewing from "$other);
		if ( first == self )
			ViewTarget = None;
		else
			ViewTarget = first;
	}
	else
	{
		ClientMessage("Failed to change view");
		ViewTarget = None;
	}

	bBehindView = ( ViewTarget != None );
}

exec function NeverSwitchOnPickup( bool B )
{
	bNeverSwitchOnPickup = B;
	SaveConfig();
}

exec function InvertMouse( bool B )
{
	bInvertMouse = B;
	SaveConfig();
}

exec function SwitchLevel( string[128] URL )
{
	if( bAdmin || Level.NetMode==NM_Standalone )
		Level.ServerTravel( URL, false );
}

exec function SwitchCoopLevel( string[128] URL )
{
	if( bAdmin || Level.NetMode==NM_Standalone )
		Level.ServerTravel( URL, true );
}

exec function ShowScores()
{
	bShowScores = !bShowScores;
}
 
exec function ShowMenu()
{
	WalkBob = vect(0,0,0);
	bShowMenu = true; // menu is responsible for turning this off
	Player.Console.GotoState('Menuing');
		
	if( Level.Netmode == NM_Standalone )
		SetPause(true);
}

exec function ShowLoadMenu()
{
	ShowMenu();
}

exec function AddBots(int N)
{
	ServerAddBots(N);
}

function ServerAddBots(int N)
{
	local int i;

	if ( !bAdmin && (Level.Netmode != NM_Standalone) )
		return;

	if ( !Level.Game.IsA('DeathMatchGame') )
		return;

	for ( i=0; i<N; i++ )
		Level.Game.AddBot();
}

	
//*************************************************************************************
// Special purpose/cheat execs

exec function SetProgressMessage( string[255] S )
{
	ProgressMessage = S;
}

exec function SetProgressMessageTwo( string[255] S )
{
	ProgressMessageTwo = S;
}

exec function SetProgressTime( float T )
{
	ProgressTimeOut = T + Level.TimeSeconds;
}

exec event ShowUpgradeMenu();

exec function Amphibious()
{
	if ( !bAdmin && (Level.Netmode != NM_Standalone) )
		return;
	UnderwaterTime = +999999.0;
}
	
exec function Fly()
{
	if ( !bAdmin && (Level.Netmode != NM_Standalone) )
		return;
		
	UnderWaterTime = Default.UnderWaterTime;	
	ClientMessage("You feel much lighter");
	SetCollision(true, true , true);
	bCollideWorld = true;
	GotoState('CheatFlying');
}

exec function Walk()
{	
	if ( !bAdmin && (Level.Netmode != NM_Standalone) )
		return;

	StartWalk();
}

function StartWalk()
{
	UnderWaterTime = Default.UnderWaterTime;	
	SetCollision(true, true , true);
	SetPhysics(PHYS_Walking);
	bCollideWorld = true;
	ClientReStart();	
}

exec function Ghost()
{
	if ( !bAdmin && (Level.Netmode != NM_Standalone) )
		return;
	
	UnderWaterTime = -1.0;	
	ClientMessage("You feel ethereal");
	SetCollision(false, false, false);
	bCollideWorld = false;
	GotoState('CheatFlying');
}

exec function ShowInventory()
{
	local Inventory Inv;
	
	if( Weapon!=None )
		log( "   Weapon: " $ Weapon.Class );
	for( Inv=Inventory; Inv!=None; Inv=Inv.Inventory ) 
		log( "Inv: "$Inv );
	if ( SelectedItem != None )
		log( "Selected Item " $SelectedItem$"  Charge "$SelectedItem.Charge );
}

exec function AllAmmo()
{
	local Inventory Inv;

	if ( !bAdmin && (Level.Netmode != NM_Standalone) )
		return;

	for( Inv=Inventory; Inv!=None; Inv=Inv.Inventory ) 
		if (Ammo(Inv)!=None) 
		{
			Ammo(Inv).AmmoAmount  = 999;
			Ammo(Inv).MaxAmmo  = 999;				
		}
}	


exec function Invisible(bool B)
{
	if ( !bAdmin && (Level.Netmode != NM_Standalone) )
		return;

	if (B)
	{
		bHidden = true;
		Visibility = 0;
	}
	else
	{
		bHidden = false;
		Visibility = Default.Visibility;
	}	
}
	
exec function God()
{
	if ( !bAdmin && (Level.Netmode != NM_Standalone) )
		return;

	if ( ReducedDamageType == 'All' )
	{
		ReducedDamageType = '';
		ClientMessage("God mode off");
		return;
	}

	ReducedDamageType = 'All'; 
	ClientMessage("God Mode on");
}

exec function BehindView( Bool B )
{
	bBehindView = B;
}

exec function SetBob(float F)
{
	UpdateBob(F);
	SaveConfig();
}

function UpdateBob(float F)
{
	Bob = FClamp(F,0,0.032);
}

exec function SetSensitivity(float F)
{
	UpdateSensitivity(F);
	SaveConfig();
}

function UpdateSensitivity(float F)
{
	MouseSensitivity = FMax(0,F);
}

exec function SloMo( float T )
{
		ServerSetSloMo(T);
}

function ServerSetSloMo(float T)
{
	if ( bAdmin || (Level.Netmode == NM_Standalone) )
	{
		Level.Game.SetGameSpeed(T);
		Level.Game.SaveConfig(); 
	}
}

exec function SetJumpZ( float F )
{
	if ( !bAdmin && (Level.Netmode != NM_Standalone) )
		return;
	JumpZ = F;
}

exec function SetSpeed( float F )
{
	if ( !bAdmin && (Level.Netmode != NM_Standalone) )
		return;
	GroundSpeed = Default.GroundSpeed * f;
	WaterSpeed = Default.WaterSpeed * f;
}

exec function KillAll(class<actor> aClass)
{
	local Actor A;

	if ( !bAdmin && (Level.Netmode != NM_Standalone) )
		return;
	ForEach AllActors(class 'Actor', A)
		if ( ClassIsChildOf(A.class, aClass) )
			A.Destroy();
}

exec function KillPawns()
{
	local Pawn P;
	
	if ( !bAdmin && (Level.Netmode != NM_Standalone) )
		return;
	ForEach AllActors(class 'Pawn', P)
		if (PlayerPawn(P) == None)
			P.Destroy();
}

exec function Summon( string[128] ClassName )
{
	local class<actor> NewClass;
	if( !bAdmin && (Level.Netmode != NM_Standalone) )
		return;
	log( "Fabricate " $ ClassName );
	NewClass = class<actor>( DynamicLoadObject( ClassName, class'Class' ) );
	if( NewClass!=None )
		Spawn( NewClass,,,Location + 72 * Vector(Rotation) + vect(0,0,1) * 15 );
}

//==============
// Navigation Aids
exec function ShowPath()
{
	//find next path to remembered spot
	local Actor node;
	node = FindPathTo(Destination);
	if (node != None)
	{
		////log("found path");
		Spawn(class 'WayBeacon', self, '', node.location);
	}
	//else
		////log("didn't find path");
}

exec function RememberSpot()
{
	//remember spot
	Destination = Location;
}
	
//=============================================================================
// Input related functions.

// Postprocess the player's input.

event PlayerInput( float DeltaTime )
{
	local float SmoothTime, MouseScale;
	
	if ( bShowMenu && (myHud != None) ) 
	{
		if ( myHud.MainMenu != None )
			myHud.MainMenu.MenuTick( DeltaTime );
		// clear inputs
		bEdgeForward = false;
		bEdgeBack = false;
		bEdgeLeft = false;
		bEdgeRight = false;
		bWasForward = false;
		bWasBack = false;
		bWasLeft = false;
		bWasRight = false;
		aStrafe = 0;
		aTurn = 0;
		aForward = 0;
		aLookUp = 0;
		return;
	}
	else if ( bDelayedCommand )
	{
		bDelayedCommand = false;
		ConsoleCommand(DelayedCommand);
	}
				
	// Check for Dodge move
	// flag transitions
	bEdgeForward = (bWasForward ^^ (aBaseY > 0));
	bEdgeBack = (bWasBack ^^ (aBaseY < 0));
	bEdgeLeft = (bWasLeft ^^ (aStrafe > 0));
	bEdgeRight = (bWasRight ^^ (aStrafe < 0));
	bWasForward = (aBaseY > 0);
	bWasBack = (aBaseY < 0);
	bWasLeft = (aStrafe > 0);
	bWasRight = (aStrafe < 0);
	
	if (aLookUp != 0)
		bKeyboardLook = true;
	if (bSnapLevel != 0)
		bKeyboardLook = false;

	// Smooth and amplify mouse movement
	SmoothTime = FMin(0.2, 3 * DeltaTime);
	MouseScale = MouseSensitivity * DesiredFOV * 0.0111;
	aMouseX = aMouseX * MouseScale;
	aMouseY = aMouseY * MouseScale;

	if ( (SmoothMouseX == 0) || (aMouseX == 0) 
			|| ((SmoothMouseX > 0) != (aMouseX > 0)) )
		SmoothMouseX = aMouseX;
	else if ( (SmoothMouseX < 0.85 * aMouseX) || (SmoothMouseX > 1.17 * aMouseX) )
		SmoothMouseX = 5 * SmoothTime * aMouseX + (1 - 5 * SmoothTime) * SmoothMouseX;
	else
		SmoothMouseX = SmoothTime * aMouseX + (1 - SmoothTime) * SmoothMouseX;

	if ( (SmoothMouseY == 0) || (aMouseY == 0) 
			|| ((SmoothMouseY > 0) != (aMouseY > 0)) )
		SmoothMouseY = aMouseY;
	else if ( (SmoothMouseY < 0.85 * aMouseY) || (SmoothMouseY > 1.17 * aMouseY) )
		SmoothMouseY = 5 * SmoothTime * aMouseY + (1 - 5 * SmoothTime) * SmoothMouseY;
	else
		SmoothMouseY = SmoothTime * aMouseY + (1 - SmoothTime) * SmoothMouseY;

	aMouseX = 0;
	aMouseY = 0;

	// Remap raw x-axis movement.
	if( bStrafe!=0 )
	{
		// Strafe.
		aStrafe += aBaseX + SmoothMouseX;
		aBaseX   = 0;
	}
	else
	{
		// Forward.
		aTurn  += aBaseX + SmoothMouseX;
		aBaseX  = 0;
	}

	// Remap mouse y-axis movement.
	if( bAlwaysMouseLook || (bLook!=0) )
	{
		// Look up/down.
		bKeyboardLook = false;
		if ( bInvertMouse )
			aLookUp -= SmoothMouseY;
		else
			aLookUp += SmoothMouseY;
	}
	else
	{
		// Move forward/backward.
		aForward += SmoothMouseY;
	}

	// Remap other y-axis movement.
	aForward += aBaseY;
	aBaseY    = 0;

	// Handle walking.
	HandleWalking();
}

//=============================================================================
// functions.

event UpdateEyeHeight(float DeltaTime)
{
	local float smooth, bound;
	
	smooth = FMin(1.0, 10.0 * DeltaTime/Level.TimeDilation);
	// smooth up/down stairs
	If( (IsInState('PlayerSwimming') || Physics==PHYS_Walking) && !bJustLanded )
	{
		EyeHeight = (EyeHeight - Location.Z + OldLocation.Z) * (1 - smooth) + ( ShakeVert + BaseEyeHeight) * smooth;
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
		EyeHeight = EyeHeight * ( 1 - smooth) + (BaseEyeHeight + ShakeVert) * smooth;
	}

	// teleporters affect your FOV, so adjust it back down
	if ( FOVAngle != DesiredFOV )
	{
		if ( FOVAngle > DesiredFOV )
			FOVAngle = FOVAngle - FMax(7, 0.9 * DeltaTime * (FOVAngle - DesiredFOV)); 
		else 
			FOVAngle = FOVAngle - FMin(-7, 0.9 * DeltaTime * (FOVAngle - DesiredFOV)); 
		if ( Abs(FOVAngle - DesiredFOV) <= 10 )
			FOVAngle = DesiredFOV;
	} 
}

event PlayerTimeOut()
{
	if (Health > 0)
		Died(None, 'suicided', Location);
}

// Just changed to pendingWeapon
function ChangedWeapon()
{
	Super.ChangedWeapon();
	if ( Weapon != None )
		Weapon.SetHand(Handedness);
}

function JumpOffPawn()
{
	Velocity += 60 * VRand();
	Velocity.Z = 120;
	SetPhysics(PHYS_Falling);
}

event TravelPostAccept()
{
	if ( Health <= 0 )
		Health = Default.Health;
}

// This pawn was possessed by a player.
event Possess()
{
	local byte i;

	if ( Level.Netmode == NM_Client )
	{
		// replicate client weapon preferences to server
		ServerSetHandedness(Handedness);
		for ( i=0; i<ArrayCount(WeaponPriority); i++ )
			ServerSetWeaponPriority(i, WeaponPriority[i]);
	}
	ServerUpdateWeapons();
	bIsPlayer = true;
	DodgeClickTime = FMin(0.3, DodgeClickTime);
	EyeHeight = BaseEyeHeight;
	NetPriority = 8;
	StartWalk();
}

function ServerSetWeaponPriority(byte i, name WeaponName )
{
	WeaponPriority[i] = WeaponName;
}

// This pawn was unpossessed by a player.
event UnPossess()
{
	log(Self$" being unpossessed");
	if ( myHUD != None )
		myHUD.Destroy();
	bIsPlayer = false;
	EyeHeight = 0.8 * CollisionHeight;
}

function Carcass SpawnCarcass()
{
	local carcass carc;

	carc = Spawn(CarcassType);
	carc.Initfor(self);
	if (Player != None)
		carc.bPlayerCarcass = true;
	MoveTarget = carc; //for Player 3rd person views
	return carc;
}

function bool Gibbed()
{
	if ( (Health < -80) || ((Health < -40) && (FRand() < 0.65)) )
		return true;
	return false;
}

function SpawnGibbedCarcass()
{
	local carcass carc;

	carc = Spawn(CarcassType);
	carc.Initfor(self);
	carc.ChunkUp(-1 * Health);
	MoveTarget = carc; //for Player 3rd person views
}

event PlayerTick( float Time );

//
// Called immediately before gameplay begins.
//
event PreBeginPlay()
{
	bIsPlayer = true;
	Super.PreBeginPlay();
}

event PostBeginPlay()
{
	Super.PostBeginPlay();
	if (Level.LevelEnterText != "" )
		ClientMessage(Level.LevelEnterText);
	if ( Level.NetMode != NM_Client )
	{
		HUDType = Level.Game.HUDType;
		ScoringType = Level.Game.ScoreboardType;
		MyAutoAim = FMax(MyAutoAim, Level.Game.AutoAim);
	}
	bIsPlayer = true;
	DodgeClickTime = FMin(0.3, DodgeClickTime);
	EyeHeight = BaseEyeHeight;
	if ( Level.Game.IsA('SinglePlayer') && (Level.NetMode == NM_Standalone) )
		FlashScale = vect(0,0,0);
}

function ServerUpdateWeapons()
{
	local inventory Inv;

	For ( Inv=Inventory; Inv!=None; Inv=Inv.Inventory )
		if ( Inv.IsA('Weapon') )
			Weapon(Inv).SetSwitchPriority(self); 
}

//=============================================================================
// Animation playing - should be implemented in subclass, 
//

function PlayDodge()
{
	PlayDuck();
}

function PlayTurning();

function PlaySwimming()
{
	PlayRunning();
}

function PlayFeignDeath();
function PlayRising();

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

/* AdjustAim()
Calls this version for player aiming help.
Aimerror not used in this version.
Only adjusts aiming at pawns
*/

function rotator AdjustAim(float projSpeed, vector projStart, int aimerror, bool bLeadTarget, bool bWarnTarget)
{
	local vector FireDir, AimSpot, HitNormal, HitLocation;
	local actor BestTarget;
	local float bestAim, bestDist;
	local actor HitActor;
	
	FireDir = vector(ViewRotation);
	HitActor = Trace(HitLocation, HitNormal, projStart + 4000 * FireDir, projStart, true);
	if ( (HitActor != None) && HitActor.bProjTarget )
	{
		if ( bWarnTarget && HitActor.IsA('Pawn') )
			Pawn(HitActor).WarnTarget(self, projSpeed, FireDir);
		return ViewRotation;
	}

	bestAim = FMin(0.93, MyAutoAim);
	BestTarget = PickTarget(bestAim, bestDist, FireDir, projStart);

	if ( bWarnTarget && (Pawn(BestTarget) != None) )
		Pawn(BestTarget).WarnTarget(self, projSpeed, FireDir);	

	if ( (Level.Game.Difficulty > 2) || bAlwaysMouseLook || ((BestTarget != None) && (bestAim < MyAutoAim)) || (MyAutoAim >= 1) )
		return ViewRotation;
	
	if ( BestTarget == None )
	{
		bestAim = MyAutoAim;
		BestTarget = PickAnyTarget(bestAim, bestDist, FireDir, projStart);
		if ( BestTarget == None )
			return ViewRotation;
	}

	AimSpot = projStart + FireDir * bestDist;
	AimSpot.Z = BestTarget.Location.Z + 0.3 * BestTarget.CollisionHeight;

	return rotator(AimSpot - projStart);
}

function Falling()
	{
		//SetPhysics(PHYS_Falling); //Note - physics changes type to PHYS_Falling by default
		//log(class$" Falling");
		PlayInAir();
	}

function Landed(vector HitNormal)
{
	//Note - physics changes type to PHYS_Walking by default for landed pawns
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
	}
	else if ( (Level.Game != None) && (Level.Game.Difficulty > 1) && (Velocity.Z > 0.5 * JumpZ) )
		MakeNoise(0.1 * Level.Game.Difficulty);				
	bJustLanded = true;
}

function Died(pawn Killer, name damageType, vector HitLocation)
{
	//Assert( Role = ROLE_Authority );
	// encroach problem may cause this
	// so temp
	if (Role != ROLE_Authority)
	{
		log("Non-authority tried to die");
		return;
	}

	Super.Died(Killer, damageType, HitLocation);	
}

function eAttitude AttitudeTo(Pawn Other)
{
	if (Other.bIsPlayer)
		return AttitudeToPlayer;
	else 
		return Other.AttitudeToPlayer;
}


function string[64] KillMessage(name damageType, pawn Other)
{
	return ( Level.Game.PlayerKillMessage(damageType, Other)$PlayerName );
}
	
//=============================================================================
// Player Control

function KilledBy( pawn EventInstigator )
{
	Health = 0;
	Score -= 1;
	Died( EventInstigator, 'suicided', Location );
}

// Player view.
// Compute the rendering viewpoint for the player.
//

event PlayerCalcView( out actor ViewActor, out vector CameraLocation, out rotator CameraRotation )
{
	local vector View,HitLocation,HitNormal;
	local float ViewDist, WallOutDist;

	if ( ViewTarget != None )
	{
		ViewActor = ViewTarget;
		CameraLocation = ViewTarget.Location;
		CameraRotation = ViewTarget.Rotation;
		if ( Pawn(ViewTarget) != None )
		{
			if ( PlayerPawn(ViewTarget) != None )
				CameraRotation = PlayerPawn(ViewTarget).ViewRotation;
			CameraLocation.Z += Pawn(ViewTarget).EyeHeight;
		}
		return;
	}

	// View rotation.
	ViewActor = Self;
	CameraRotation = ViewRotation;

	if( bBehindView ) //up and behind
	{
	    ViewDist    = 100;
		WallOutDist = 30;
		View = vect(1,0,0) >> CameraRotation;
		if( Trace( HitLocation, HitNormal, Location - (ViewDist+WallOutDist) * vector(CameraRotation), Location ) != None )
			ViewDist = FMin( (Location - HitLocation) Dot View, ViewDist );
		CameraLocation -= (ViewDist - WallOutDist) * View;
	}
	else
	{
		// First-person view.
		CameraLocation = Location;
		CameraLocation.Z += EyeHeight;
		CameraLocation += WalkBob;
	}
}

function ViewFlash(float DeltaTime)
{
	local vector goalFog;
	local float goalscale, delta;

	delta = FMin(0.1, DeltaTime);
	goalScale = 1 + DesiredFlashScale + ConstantGlowScale + HeadRegion.Zone.ViewFlash.X; 
	goalFog = DesiredFlashFog + ConstantGlowFog + HeadRegion.Zone.ViewFog;
	DesiredFlashScale -= DesiredFlashScale * 2 * delta;  
	DesiredFlashFog -= DesiredFlashFog * 2 * delta;
	FlashScale.X += (goalScale - FlashScale.X) * 10 * delta;
	FlashFog += (goalFog - FlashFog) * 10 * delta;

	if ( FlashScale.X > 0.981 )
		FlashScale.X = 1;
	FlashScale = FlashScale.X * vect(1,1,1);

	if ( FlashFog.X < 0.019 )
		FlashFog.X = 0;
	if ( FlashFog.Y < 0.019 )
		FlashFog.Y = 0;
	if ( FlashFog.Z < 0.019 )
		FlashFog.Z = 0;
}

function ViewShake(float DeltaTime)
{
	if (shaketimer > 0.0) //shake view
	{
		shaketimer -= DeltaTime;
		if ( verttimer == 0 )
		{
			verttimer = 0.1;
			ShakeVert = -1.1 * maxshake;
		}
		else
		{
			verttimer -= DeltaTime;
			if ( verttimer < 0 )
			{
				verttimer = 0.2 * FRand();
				shakeVert = (2 * FRand() - 1) * maxshake;  
			}
		}
		ViewRotation.Roll = ViewRotation.Roll & 65535;
		if (bShakeDir)
		{
			ViewRotation.Roll += Int( 10 * shakemag * FMin(0.1, DeltaTime));
			bShakeDir = (ViewRotation.Roll > 32768) || (ViewRotation.Roll < (0.5 + FRand()) * shakemag);
			if ( (ViewRotation.Roll < 32768) && (ViewRotation.Roll > 1.3 * shakemag) )
			{
				ViewRotation.Roll = 1.3 * shakemag;
				bShakeDir = false;
			}
			else if (FRand() < 3 * DeltaTime)
				bShakeDir = !bShakeDir;
		}
		else
		{
			ViewRotation.Roll -= Int( 10 * shakemag * FMin(0.1, DeltaTime));
			bShakeDir = (ViewRotation.Roll > 32768) && (ViewRotation.Roll < 65535 - (0.5 + FRand()) * shakemag);
			if ( (ViewRotation.Roll > 32768) && (ViewRotation.Roll < 65535 - 1.3 * shakemag) )
			{
				ViewRotation.Roll = 65535 - 1.3 * shakemag;
				bShakeDir = true;
			}
			else if (FRand() < 3 * DeltaTime)
				bShakeDir = !bShakeDir;
		}
	}
	else
	{
		ShakeVert = 0;
		ViewRotation.Roll = ViewRotation.Roll & 65535;
		if (ViewRotation.Roll < 32768)
		{
			if ( ViewRotation.Roll > 0 )
				ViewRotation.Roll = Max(0, ViewRotation.Roll - (Max(ViewRotation.Roll,500) * 10 * FMin(0.1,DeltaTime)));
		}
		else
		{
			ViewRotation.Roll += ((65536 - Max(500,ViewRotation.Roll)) * 10 * FMin(0.1,DeltaTime));
			if ( ViewRotation.Roll > 65534 )
				ViewRotation.Roll = 0;
		}
	} 
}

function UpdateRotation(float DeltaTime, float maxPitch)
{
	local rotator newRotation;
	
	DesiredRotation = ViewRotation; //save old rotation
	ViewRotation.Pitch += 32.0 * DeltaTime * aLookUp;
	ViewRotation.Pitch = ViewRotation.Pitch & 65535;
	If ((ViewRotation.Pitch > 18000) && (ViewRotation.Pitch < 49152))
	{
		If (aLookUp > 0) 
			ViewRotation.Pitch = 18000;
		else
			ViewRotation.Pitch = 49152;
	}
	ViewRotation.Yaw += 32.0 * DeltaTime * aTurn;
	ViewShake(deltaTime);
	ViewFlash(deltaTime);
		
	newRotation = Rotation;
	newRotation.Yaw = ViewRotation.Yaw;
	newRotation.Pitch = ViewRotation.Pitch;
	If ( (newRotation.Pitch > maxPitch * RotationRate.Pitch) && (newRotation.Pitch < 65536 - maxPitch * RotationRate.Pitch) )
	{
		If (ViewRotation.Pitch < 32768) 
			newRotation.Pitch = maxPitch * RotationRate.Pitch;
		else
			newRotation.Pitch = 65536 - maxPitch * RotationRate.Pitch;
	}
	setRotation(newRotation);
}

function SwimAnimUpdate();

// Player movement.
// Player Standing, walking, running, falling.
state PlayerWalking
{
ignores SeePlayer, HearNoise, Bump;

	exec function FeignDeath()
	{
		if ( Physics == PHYS_Walking )
		{
			ServerFeignDeath();
			Acceleration = vect(0,0,0);
			GotoState('FeigningDeath');
		}
	}

	function ZoneChange( ZoneInfo NewZone )
	{
		if (NewZone.bWaterZone)
		{
			setPhysics(PHYS_Swimming);
			GotoState('PlayerSwimming');
		}
	}

	function AnimEnd()
	{
		local name MyAnimGroup;

		bAnimTransition = false;
		if (Physics == PHYS_Walking)
		{
			if (bIsCrouching)
			{
				if ( !bIsTurning && ((Velocity.X * Velocity.X + Velocity.Y * Velocity.Y) < 1000) )
					PlayDuck();	
				else
					PlayCrawling();
			}
			else
			{
				MyAnimGroup = GetAnimGroup(AnimSequence);
				if ((Velocity.X * Velocity.X + Velocity.Y * Velocity.Y) < 1000)
				{
					if ( MyAnimGroup == 'Waiting' )
						PlayWaiting();
					else
					{
						bAnimTransition = true;
						TweenToWaiting(0.2);
					}
				}	
				else if (bIsWalking)
				{
					if ( (MyAnimGroup == 'Waiting') || (MyAnimGroup == 'Landing') || (MyAnimGroup == 'Gesture') || (MyAnimGroup == 'TakeHit')  )
					{
						TweenToWalking(0.1);
						bAnimTransition = true;
					}
					else 
						PlayWalking();
				}
				else
				{
					if ( (MyAnimGroup == 'Waiting') || (MyAnimGroup == 'Landing') || (MyAnimGroup == 'Gesture') || (MyAnimGroup == 'TakeHit')  )
					{
						bAnimTransition = true;
						TweenToRunning(0.1);
					}
					else
						PlayRunning();
				}
			}
		}
	}

	function Landed(vector HitNormal)
	{
		Global.Landed(HitNormal);
		if (DodgeDir == DODGE_Active)
		{
			DodgeDir = DODGE_Done;
			DodgeClickTimer = 0.0;
			Velocity *= 0.1;
		}
		else
			DodgeDir = DODGE_None;
	}

	function Dodge(eDodgeDir DodgeMove)
	{
		local vector X,Y,Z;

		if ( bIsCrouching || (Physics != PHYS_Walking) )
			return;

		GetAxes(Rotation,X,Y,Z);
		if (DodgeMove == DODGE_Forward)
			Velocity = 1.5*GroundSpeed*X + (Velocity Dot Y)*Y;
		else if (DodgeMove == DODGE_Back)
			Velocity = -1.5*GroundSpeed*X + (Velocity Dot Y)*Y; 
		else if (DodgeMove == DODGE_Left)
			Velocity = 1.5*GroundSpeed*Y + (Velocity Dot X)*X; 
		else if (DodgeMove == DODGE_Right)
			Velocity = -1.5*GroundSpeed*Y + (Velocity Dot X)*X; 

		Velocity.Z = 160;
		if ( Role == ROLE_Authority )
			PlaySound(JumpSound, SLOT_Talk, 1.0, true, 800, 1.0 );
		PlayDodge();
		DodgeDir = DODGE_Active;
		SetPhysics(PHYS_Falling);
	}
	
	function ProcessMove(float DeltaTime, vector NewAccel, eDodgeDir DodgeMove, rotator DeltaRot)	
	{
		local vector OldAccel;

		OldAccel = Acceleration;
		Acceleration = NewAccel;
		bIsTurning = ( Abs(DeltaRot.Yaw/DeltaTime) > 5000 );
		if ( (DodgeMove == DODGE_Active) && (Physics == PHYS_Falling) )
			DodgeDir = DODGE_Active;	
		else if ( (DodgeMove != DODGE_None) && (DodgeMove < DODGE_Active) )
			Dodge(DodgeMove);

		if ( bPressedJump )
			DoJump();
		if ( (Physics == PHYS_Walking) && (GetAnimGroup(AnimSequence) != 'Dodge') )
		{
			if (!bIsCrouching)
			{
				if (bDuck != 0)
				{
					bIsCrouching = true;
					PlayDuck();
				}
			}
			else if (bDuck == 0)
			{
				OldAccel = vect(0,0,0);
				bIsCrouching = false;
			}

			if ( !bIsCrouching )
			{
				if ( (!bAnimTransition || (AnimFrame > 0)) && (GetAnimGroup(AnimSequence) != 'Landing') )
				{
					if ( Acceleration != vect(0,0,0) )
					{
						if ( (GetAnimGroup(AnimSequence) == 'Waiting') || (GetAnimGroup(AnimSequence) == 'Gesture') || (GetAnimGroup(AnimSequence) == 'TakeHit') )
						{
							bAnimTransition = true;
							TweenToRunning(0.1);
						}
					}
			 		else if ( (Velocity.X * Velocity.X + Velocity.Y * Velocity.Y < 1000) 
						&& (GetAnimGroup(AnimSequence) != 'Gesture') ) 
			 		{
			 			if ( GetAnimGroup(AnimSequence) == 'Waiting' )
			 			{
							if ( bIsTurning && (AnimFrame >= 0) ) 
							{
								bAnimTransition = true;
								PlayTurning();
							}
						}
			 			else if ( !bIsTurning ) 
						{
							bAnimTransition = true;
							TweenToWaiting(0.2);
						}
					}
				}
			}
			else
			{
				if ( (OldAccel == vect(0,0,0)) && (Acceleration != vect(0,0,0)) )
					PlayCrawling();
			 	else if ( !bIsTurning && (Acceleration == vect(0,0,0)) && (AnimFrame > 0.1) )
					PlayDuck();
			}
		}
	}
			
	event PlayerTick( float DeltaTime )
	{
		if ( bUpdatePosition )
			ClientUpdatePosition();

		PlayerMove(DeltaTime);
	}

	function PlayerMove( float DeltaTime )
	{
		local vector X,Y,Z, NewAccel;
		local EDodgeDir OldDodge;
		local eDodgeDir DodgeMove;
		local rotator OldRotation;
		local float Speed2D;
		local bool	bSaveJump;

		GetAxes(Rotation,X,Y,Z);

		aForward *= 0.4;
		aStrafe  *= 0.4;
		aLookup  *= 0.24;
		aTurn    *= 0.24;

		// Update acceleration.
		NewAccel = aForward*X + aStrafe*Y; 
		NewAccel.Z = 0;
		// Check for Dodge move
		if ( DodgeDir == DODGE_Active )
			DodgeMove = DODGE_Active;
		else
			DodgeMove = DODGE_None;
		if (DodgeClickTime > 0.0)
		{
			if ( DodgeDir < DODGE_Active )
			{
				OldDodge = DodgeDir;
				DodgeDir = DODGE_None;
				if (bEdgeForward && bWasForward)
					DodgeDir = DODGE_Forward;
				if (bEdgeBack && bWasBack)
					DodgeDir = DODGE_Back;
				if (bEdgeLeft && bWasLeft)
					DodgeDir = DODGE_Left;
				if (bEdgeRight && bWasRight)
					DodgeDir = DODGE_Right;
				if ( DodgeDir == DODGE_None)
					DodgeDir = OldDodge;
				else if ( DodgeDir != OldDodge )
					DodgeClickTimer = DodgeClickTime + 0.5 * DeltaTime;
				else 
					DodgeMove = DodgeDir;
			}
	
			if (DodgeDir == DODGE_Done)
			{
				DodgeClickTimer -= DeltaTime;
				if (DodgeClickTimer < -0.35) 
				{
					DodgeDir = DODGE_None;
					DodgeClickTimer = DodgeClickTime;
				}
			}		
			else if ((DodgeDir != DODGE_None) && (DodgeDir != DODGE_Active))
			{
				DodgeClickTimer -= DeltaTime;			
				if (DodgeClickTimer < 0)
				{
					DodgeDir = DODGE_None;
					DodgeClickTimer = DodgeClickTime;
				}
			}
		}
				
		if ( (Physics == PHYS_Walking) && (GetAnimGroup(AnimSequence) != 'Dodge') )
		{
			//if walking, look up/down stairs - unless player is rotating view
			if ( !bKeyboardLook && (bLook == 0) )
			{
				if ( bLookUpStairs )
					ViewRotation.Pitch = FindStairRotation(deltaTime);
				else if ( bSnapToLevel )
				{
					ViewRotation.Pitch = ViewRotation.Pitch & 65535;
					if (ViewRotation.Pitch > 32768)
						ViewRotation.Pitch -= 65536;
					ViewRotation.Pitch = ViewRotation.Pitch * (1 - 12 * FMin(0.0833, deltaTime));
					if ( Abs(ViewRotation.Pitch) < 1000 )
						ViewRotation.Pitch = 0;	
				}
			}

			Speed2D = Sqrt(Velocity.X * Velocity.X + Velocity.Y * Velocity.Y);
			//add bobbing when walking
			if ( !bShowMenu )
			{
				if ( Speed2D < 10 )
					BobTime += 0.2 * DeltaTime;
				else
					BobTime += DeltaTime * (0.3 + 0.7 * Speed2D/GroundSpeed);
				WalkBob = Y * 0.65 * Bob * Speed2D * sin(6.0 * BobTime);
				if ( Speed2D < 10 )
					WalkBob.Z = Bob * 30 * sin(12 * BobTime);
				else
					WalkBob.Z = Bob * Speed2D * sin(12 * BobTime);
			}
		}	
		else if ( !bShowMenu )
		{ 
			BobTime = 0;
			WalkBob = WalkBob * (1 - FMin(1, 8 * deltatime));
		}

		// Update rotation.
		OldRotation = Rotation;
		UpdateRotation(DeltaTime, 1);

		if ( bPressedJump && ((GetAnimGroup(AnimSequence) == 'Dodge') || (GetAnimGroup(AnimSequence) == 'Landing')) )
		{
			bSaveJump = true;
			bPressedJump = false;
		}
		else
			bSaveJump = false;

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, NewAccel, DodgeMove, OldRotation - Rotation);
		else
			ProcessMove(DeltaTime, NewAccel, DodgeMove, OldRotation - Rotation);
		bPressedJump = bSaveJump;
	}

	function BeginState()
	{
		WalkBob = vect(0,0,0);
		DodgeDir = DODGE_None;
		bIsCrouching = false;
		bIsTurning = false;
		bPressedJump = false;
		if (Physics != PHYS_Falling) SetPhysics(PHYS_Walking);
		if ( !IsAnimating() )
			PlayWaiting();
	}
	
	function EndState()
	{
		WalkBob = vect(0,0,0);
		bIsCrouching = false;
	}
}

state FeigningDeath
{
ignores SeePlayer, HearNoise, Bump, Fire, AltFire;

	function ZoneChange( ZoneInfo NewZone )
	{
		if (NewZone.bWaterZone)
		{
			setPhysics(PHYS_Swimming);
			GotoState('PlayerSwimming');
		}
	}

	exec function Taunt( name Sequence )
	{
	}

	function AnimEnd()
	{
		if ( Role == ROLE_Authority )
		{
			GotoState('PlayerWalking');
			ChangedWeapon();
		}
	}
	
	function Landed(vector HitNormal)
	{
		if ( Role == ROLE_Authority )
			PlaySound(Land, SLOT_Interact, 0.3, false, 800, 1.0);
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
					TakeDamage(-0.15 * (Velocity.Z + 1050), Self, Location, vect(0,0,0), 'fell');
			}
		}
		else if ( (Level.Game != None) && (Level.Game.Difficulty > 1) && (Velocity.Z > 0.5 * JumpZ) )
			MakeNoise(0.1 * Level.Game.Difficulty);				
		bJustLanded = true;				
	}

	function Rise()
	{
		if ( !bRising )
		{
			Enable('AnimEnd');
			BaseEyeHeight = Default.BaseEyeHeight;
			bRising = true;
			PlayRising();
		}
	}

	function ProcessMove(float DeltaTime, vector NewAccel, eDodgeDir DodgeMove, rotator DeltaRot)	
	{
		if ( bPressedJump || (NewAccel.Z > 0) )
			Rise();
		Acceleration = vect(0,0,0);
	}

	event PlayerTick( float DeltaTime )
	{
		if ( bUpdatePosition )
			ClientUpdatePosition();
		
		PlayerMove(DeltaTime);
	}

	function PlayerMove( float DeltaTime)
	{
		local rotator currentRot;
		local vector NewAccel;

		aLookup  *= 0.24;
		aTurn    *= 0.24;

		// Update acceleration.
		if ( !IsAnimating() && (aForward != 0) || (aStrafe != 0) )
			NewAccel = vect(0,0,1);
		else
			NewAccel = vect(0,0,0);

		// Update view rotation.
		currentRot = Rotation;
		UpdateRotation(DeltaTime, 1);
		SetRotation(currentRot);

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, NewAccel, DODGE_None, Rot(0,0,0));
		else
			ProcessMove(DeltaTime, NewAccel, DODGE_None, Rot(0,0,0));
		bPressedJump = false;
	}

	function PlayTakeHit(float tweentime, vector HitLoc, int Damage)
	{
		if ( IsAnimating() )
		{
			Enable('AnimEnd');
			Global.PlayTakeHit(tweentime, HitLoc, Damage);
		}
	}
	
	function PlayDying(name DamageType, vector HitLocation)
	{
		BaseEyeHeight = Default.BaseEyeHeight;
	}
	
	function ChangedWeapon()
	{
		Weapon = None;
	}

	function BeginState()
	{
		if ( carriedDecoration != None )
			DropDecoration();
		BaseEyeHeight = -0.5 * CollisionHeight;
		bIsCrouching = false;
		bPressedJump = false;
		bRising = false;
		Disable('AnimEnd');
		PlayFeignDeath();
	}
}

// Player movement.
// Player Swimming
state PlayerSwimming
{
ignores SeePlayer, HearNoise, Bump;

	function Landed(vector HitNormal)
	{
		//log(class$" Landed while swimming");
		PlayLanded(Velocity.Z);
		if (Velocity.Z < -1.2 * JumpZ)
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
					TakeDamage(-0.1 * (Velocity.Z + 1050), Self, Location, vect(0,0,0), 'fell');
			}
		}
		else if ( (Level.Game != None) && (Level.Game.Difficulty > 1) && (Velocity.Z > 0.5 * JumpZ) )
			MakeNoise(0.1 * Level.Game.Difficulty);				
		bJustLanded = true;
		if ( Region.Zone.bWaterZone )
			SetPhysics(PHYS_Swimming);
		else
		{
			GotoState('PlayerWalking');
			AnimEnd();
		}
	}

	function AnimEnd()
	{
		local vector X,Y,Z;
		GetAxes(Rotation, X,Y,Z);
		if ( (Acceleration Dot X) <= 0 )
		{
			if ( GetAnimGroup(AnimSequence) == 'TakeHit' )
			{
				bAnimTransition = true;
				TweenToWaiting(0.2);
			} 
			else
				PlayWaiting();
		}	
		else
		{
			if ( GetAnimGroup(AnimSequence) == 'TakeHit' )
			{
				bAnimTransition = true;
				TweenToSwimming(0.2);
			} 
			else
				PlaySwimming();
		}
	}
	
	function ZoneChange( ZoneInfo NewZone )
	{
		local actor HitActor;
		local vector HitLocation, HitNormal, checkpoint;

		if (!NewZone.bWaterZone)
		{
			SetPhysics(PHYS_Falling);
			if (bUpAndOut && CheckWaterJump(HitNormal)) //check for waterjump
			{
				velocity.Z = 330 + 2 * CollisionRadius; //set here so physics uses this for remainder of tick
				PlayDuck();
				GotoState('PlayerWalking');
			}				
			else if (!FootRegion.Zone.bWaterZone || (Velocity.Z > 160) )
			{
				GotoState('PlayerWalking');
				AnimEnd();
			}
			else //check if in deep water
			{
				checkpoint = Location;
				checkpoint.Z -= (CollisionHeight + 6.0);
				HitActor = Trace(HitLocation, HitNormal, checkpoint, Location, false);
				if (HitActor != None)
				{
					GotoState('PlayerWalking');
					AnimEnd();
				}
				else
				{
					Enable('Timer');
					SetTimer(0.7,false);
				}
			}
			//log("Out of water");
		}
		else
		{
			Disable('Timer');
			SetPhysics(PHYS_Swimming);
		}
	}

	function ProcessMove(float DeltaTime, vector NewAccel, eDodgeDir DodgeMove, rotator DeltaRot)	
	{
		local vector X,Y,Z, Temp;

		GetAxes(ViewRotation,X,Y,Z);
		Acceleration = NewAccel;

		if ( !bAnimTransition && (GetAnimGroup(AnimSequence) != 'Gesture') )
		{
			if ((X Dot Acceleration) <= 0)
	 		{
		 		 if ( GetAnimGroup(AnimSequence) != 'Waiting' )
					TweenToWaiting(0.1);
			}
			else if ( GetAnimGroup(AnimSequence) == 'Waiting' )
				TweenToSwimming(0.1);
		}
		bUpAndOut = ((X Dot Acceleration) > 0) && ((Acceleration.Z > 0) || (ViewRotation.Pitch > 2048));
		if ( bUpAndOut && !Region.Zone.bWaterZone && CheckWaterJump(Temp) ) //check for waterjump
		{
			velocity.Z = 330 + 2 * CollisionRadius; //set here so physics uses this for remainder of tick
			PlayDuck();
			GotoState('PlayerWalking');
		}				
	}

	event PlayerTick( float DeltaTime )
	{
		if ( bUpdatePosition )
			ClientUpdatePosition();
		
		PlayerMove(DeltaTime);
	}

	function PlayerMove(float DeltaTime)
	{
		local rotator oldRotation;
		local vector X,Y,Z, NewAccel;
		local float Speed2D;
	
		GetAxes(ViewRotation,X,Y,Z);

		aForward *= 0.2;
		aStrafe  *= 0.1;
		aLookup  *= 0.24;
		aTurn    *= 0.24;
		aUp		 *= 0.1;  
		
		NewAccel = aForward*X + aStrafe*Y + aUp*vect(0,0,1); 
	
		//add bobbing when swimming
		if ( !bShowMenu )
		{
			Speed2D = Sqrt(Velocity.X * Velocity.X + Velocity.Y * Velocity.Y);
			WalkBob = Y * Bob *  0.5 * Speed2D * sin(4.0 * Level.TimeSeconds);
			WalkBob.Z = Bob * 1.5 * Speed2D * sin(8.0 * Level.TimeSeconds);
		}

		SwimAnimUpdate();

		// Update rotation.
		oldRotation = Rotation;
		UpdateRotation(DeltaTime, 2);

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, NewAccel, DODGE_None, OldRotation - Rotation);
		else
			ProcessMove(DeltaTime, NewAccel, DODGE_None, OldRotation - Rotation);
		bPressedJump = false;
	}

	function Timer()
	{
		if ( !Region.Zone.bWaterZone && (Role == ROLE_Authority) )
		{
			//log("timer out of water");
			GotoState('PlayerWalking');
			AnimEnd();
		}
	
		Disable('Timer');
	}
	
	function BeginState()
	{
		Disable('Timer');
		if ( !IsAnimating() )
			TweenToWaiting(0.3);
		//log("player swimming");
	}
}

state PlayerFlying
{
ignores SeePlayer, HearNoise, Bump;
		
	function AnimEnd()
	{
		PlaySwimming();
	}
	
	event PlayerTick( float DeltaTime )
	{
		if ( bUpdatePosition )
			ClientUpdatePosition();
	
		PlayerMove(DeltaTime);
	}

	function PlayerMove(float DeltaTime)
	{
		local rotator newRotation;
		local vector X,Y,Z;

		GetAxes(Rotation,X,Y,Z);

		aForward *= 0.2;
		aStrafe  *= 0.2;
		aLookup  *= 0.24;
		aTurn    *= 0.24;

		Acceleration = aForward*X + aStrafe*Y;  
		// Update rotation.
		UpdateRotation(DeltaTime, 2);

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, Acceleration, DODGE_None, rot(0,0,0));
		else
			ProcessMove(DeltaTime, Acceleration, DODGE_None, rot(0,0,0));
	}
	
	function BeginState()
	{
		SetPhysics(PHYS_Flying);
		if  ( !IsAnimating() ) PlayWalking();
		//log("player flying");
	}
}

state CheatFlying
{
ignores SeePlayer, HearNoise, Bump, TakeDamage;
		
	function AnimEnd()
	{
		PlaySwimming();
	}
	
	function ProcessMove(float DeltaTime, vector NewAccel, eDodgeDir DodgeMove, rotator DeltaRot)	
	{
		Acceleration = NewAccel;
		MoveSmooth(Acceleration * DeltaTime);
	}

	event PlayerTick( float DeltaTime )
	{
		if ( bUpdatePosition )
			ClientUpdatePosition();

		PlayerMove(DeltaTime);
	}

	function PlayerMove(float DeltaTime)
	{
		local rotator newRotation;
		local vector X,Y,Z;

		GetAxes(ViewRotation,X,Y,Z);

		aForward *= 0.1;
		aStrafe  *= 0.1;
		aLookup  *= 0.24;
		aTurn    *= 0.24;
		aUp		 *= 0.1;
	
		Acceleration = aForward*X + aStrafe*Y + aUp*vect(0,0,1);  

		UpdateRotation(DeltaTime, 1);

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, Acceleration, DODGE_None, rot(0,0,0));
		else
			ProcessMove(DeltaTime, Acceleration, DODGE_None, rot(0,0,0));
	}

	function BeginState()
	{
		EyeHeight = BaseEyeHeight;
		SetPhysics(PHYS_None);
		if  ( !IsAnimating() ) PlaySwimming();
		// log("cheat flying");
	}
}

//===============================================================================
state PlayerWaking
{
ignores SeePlayer, HearNoise, KilledBy, Bump, HitWall, HeadZoneChange, FootZoneChange, ZoneChange, SwitchWeapon, Falling;

	function Timer()
	{
		BaseEyeHeight = Default.BaseEyeHeight;
	}

	event PlayerTick( float DeltaTime )
	{
		if ( bUpdatePosition )
			ClientUpdatePosition();

		PlayerMove(DeltaTime);
	}

	function PlayerMove(Float DeltaTime)
	{
		ViewFlash(deltaTime * 0.5);
		if ( TimerRate == 0 )
		{
			ViewRotation.Pitch -= DeltaTime * 12000;
			if ( ViewRotation.Pitch < 0 )
			{
				ViewRotation.Pitch = 0;
				GotoState('PlayerWalking');
			}
		}

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, vect(0,0,0), DODGE_None, rot(0,0,0));
		else
			ProcessMove(DeltaTime, vect(0,0,0), DODGE_None, rot(0,0,0));
	}

	function BeginState()
	{
		if ( bWokeUp )
		{
			ViewRotation.Pitch = 0;
			SetTimer(0, false);
			return;
		}
		BaseEyeHeight = 0;
		EyeHeight = 0;
		SetTimer(3.0, false);
		bWokeUp = true;
	}
}

state Dying
{
ignores SeePlayer, HearNoise, KilledBy, Bump, HitWall, HeadZoneChange, FootZoneChange, ZoneChange, SwitchWeapon, Falling;

	exec function Fire( optional float F )
	{
		if ( (Level.NetMode == NM_Standalone) && !Level.Game.IsA('DeathMatchGame') )
		{
			if ( bFrozen )
				return;
			ShowLoadMenu();
		}
		else
			ServerReStartPlayer();
	}
	
	exec function AltFire( optional float F )
	{
		Fire(F);
	}

	function PlayerCalcView( out actor ViewActor, out vector CameraLocation, out rotator CameraRotation )
	{
		local vector View,HitLocation,HitNormal, FirstHit, spot;
		local float DesiredDist, ViewDist, WallOutDist;
		local actor HitActor;
		// View rotation.
		CameraRotation = ViewRotation;
		DesiredFOV = Default.FOVAngle;		
		ViewActor = self;
		if( bBehindView ) //up and behind (for death scene)
		{
		    ViewDist = 190;
			if (MoveTarget != None)
				spot = MoveTarget.Location;
			else
				spot = Location;
			View = vect(1,0,0) >> CameraRotation;
			HitActor = Trace( HitLocation, HitNormal, 
					spot - ViewDist * vector(CameraRotation), spot, false, vect(12,12,2));
			if ( HitActor != None )
				CameraLocation = HitLocation;
			else
				CameraLocation = spot - ViewDist * View;
		}
		else
		{
			// First-person view.
			CameraLocation = Location;
			CameraLocation.Z += Default.BaseEyeHeight;
		}
	}

	event PlayerTick( float DeltaTime )
	{
		if ( bUpdatePosition )
			ClientUpdatePosition();
		
		PlayerMove(DeltaTime);
	}

	function PlayerMove(float DeltaTime)
	{
		local vector X,Y,Z;

		if ( !bFrozen )
		{
			if ( bPressedJump )
				Fire(0);
			GetAxes(ViewRotation,X,Y,Z);
			// Update view rotation.
			aLookup  *= 0.24;
			aTurn    *= 0.24;
			ViewRotation.Yaw += 32.0 * DeltaTime * aTurn;
			ViewRotation.Pitch += 32.0 * DeltaTime * aLookUp;
			ViewRotation.Pitch = ViewRotation.Pitch & 65535;
			If ((ViewRotation.Pitch > 18000) && (ViewRotation.Pitch < 49152))
			{
				If (aLookUp > 0) 
					ViewRotation.Pitch = 18000;
				else
					ViewRotation.Pitch = 49152;
			}
			if ( Role < ROLE_Authority ) // then save this move and replicate it
				ReplicateMove(DeltaTime, vect(0,0,0), DODGE_None, rot(0,0,0));
			bPressedJump = false;
		}
		ViewShake(DeltaTime);
		ViewFlash(DeltaTime);
	}

	function FindGoodView()
	{
		local vector cameraLoc;
		local rotator cameraRot;
		local int tries, besttry;
		local float bestdist, newdist;
		local int startYaw;
		local actor ViewActor;
		
		//fixme - try to pick view with killer visible
		//fixme - also try varying starting pitch
		////log("Find good death scene view");
		ViewRotation.Pitch = 56000;
		tries = 0;
		besttry = 0;
		bestdist = 0.0;
		startYaw = ViewRotation.Yaw;
		
		for (tries=0; tries<16; tries++)
		{
			cameraLoc = Location;
			PlayerCalcView(ViewActor, cameraLoc, cameraRot);
			newdist = VSize(cameraLoc - Location);
			if (newdist > bestdist)
			{
				bestdist = newdist;	
				besttry = tries;
			}
			ViewRotation.Yaw += 4096;
		}
			
		ViewRotation.Yaw = startYaw + besttry * 4096;
	}
	
	function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
							Vector momentum, name damageType)
	{
		if ( !bHidden )
			Super.TakeDamage(Damage, instigatedBy, hitlocation, momentum, damageType);
	}
	
	function Timer()
	{
		bFrozen = false;
		bShowScores = true;
		bPressedJump = false;
	}
	
	function BeginState()
	{
		bBehindView = true;
		bFrozen = true;
		bPressedJump = false;
		FindGoodView();
		if ( (Role == ROLE_Authority) && !bHidden )
			Super.Timer(); //FIXME - when server can do timers, delay this slightly
		SetTimer(1.0, false);
		//log(Self$" entering dying with remote role "$RemoteRole$" and role "$Role$" in state "$state);
	}
	
	function EndState()
	{
		bBehindView = false;
		bShowScores = false;
		//Log(self$" exiting dying with remote role "$RemoteRole$" and role "$Role);
	}
}

state GameEnded
{
ignores SeePlayer, HearNoise, KilledBy, Bump, HitWall, HeadZoneChange, FootZoneChange, ZoneChange, Falling, TakeDamage;

	exec function Fire( optional float F )
	{
		if ( !bFrozen )
			ServerReStartGame();
	}
	
	exec function AltFire( optional float F )
	{
		if ( !bFrozen )
			ServerReStartGame();
	}

	function PlayerCalcView( out actor ViewActor, out vector CameraLocation, out rotator CameraRotation )
	{
		local vector View,HitLocation,HitNormal, FirstHit;
		local float DesiredDist, ViewDist, WallOutDist;
		local actor HitActor;
		// View rotation.
		CameraRotation = ViewRotation;

		ViewActor = self;
		if( bBehindView ) 
		{
		    ViewDist = 300;
			View = vect(1,0,0) >> CameraRotation;
			HitActor = Trace( HitLocation, HitNormal, 
					Location - ViewDist * vector(CameraRotation), Location, false, vect(12,12,2));
			if ( HitActor != None )
				CameraLocation = HitLocation;
			else
				CameraLocation = Location - ViewDist * View;
		}
	}

	event PlayerTick( float DeltaTime )
	{
		if ( bUpdatePosition )
			ClientUpdatePosition();

		PlayerMove(DeltaTime);
	}

	function PlayerMove(float DeltaTime)
	{
		local vector X,Y,Z;
		
		GetAxes(ViewRotation,X,Y,Z);
		// Update view rotation.
		if ( bPressedJump && !bFrozen )
			ServerReStartGame();
		aLookup  *= 0.24;
		aTurn    *= 0.24;
		ViewRotation.Yaw += 32.0 * DeltaTime * aTurn;
		ViewRotation.Pitch += 32.0 * DeltaTime * aLookUp;
		ViewRotation.Pitch = ViewRotation.Pitch & 65535;
		If ((ViewRotation.Pitch > 18000) && (ViewRotation.Pitch < 49152))
		{
			If (aLookUp > 0) 
				ViewRotation.Pitch = 18000;
			else
				ViewRotation.Pitch = 49152;
		}
		ViewShake(DeltaTime);
		ViewFlash(DeltaTime);

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, vect(0,0,0), DODGE_None, rot(0,0,0));
		else
			ProcessMove(DeltaTime, vect(0,0,0), DODGE_None, rot(0,0,0));
		bPressedJump = false;
	}

	function FindGoodView()
	{
		local vector cameraLoc;
		local rotator cameraRot;
		local int tries, besttry;
		local float bestdist, newdist;
		local int startYaw;
		local actor ViewActor;
		
		ViewRotation.Pitch = 56000;
		tries = 0;
		besttry = 0;
		bestdist = 0.0;
		startYaw = ViewRotation.Yaw;
		
		for (tries=0; tries<16; tries++)
		{
			cameraLoc = Location;
			PlayerCalcView(ViewActor, cameraLoc, cameraRot);
			newdist = VSize(cameraLoc - Location);
			if (newdist > bestdist)
			{
				bestdist = newdist;	
				besttry = tries;
			}
			ViewRotation.Yaw += 4096;
		}
			
		ViewRotation.Yaw = startYaw + besttry * 4096;
	}
	
	function Timer()
	{
		bFrozen = false;
	}
	
	function BeginState()
	{
		bBehindView = true;
		bShowScores = true;
		bFrozen = true;
		FindGoodView();
		SetTimer(1.5, false);
		SetPhysics(PHYS_None);
		HidePlayer();
	}
}

defaultproperties
{
     DodgeClickTime=0.250000
     Bob=0.016000
     FlashScale=(X=1.000000,Y=1.000000,Z=1.000000)
     DesiredFOV=90.000000
     CdTrack=255
     MyAutoAim=0.930000
     Handedness=-1.000000
     bAlwaysMouseLook=True
	 NetSpeed=2600
     MouseSensitivity=3.000000
     WeaponPriority(0)=DispersionPistol
     WeaponPriority(1)=AutoMag
     WeaponPriority(2)=Stinger
     WeaponPriority(3)=DispersionPower1
     WeaponPriority(4)=ASMD
     WeaponPriority(5)=DispersionPower2
     WeaponPriority(6)=Eightball
     WeaponPriority(7)=FlakCannon
     WeaponPriority(8)=DispersionPower3
     WeaponPriority(9)=GESBioRifle
     WeaponPriority(10)=Razorjack
     WeaponPriority(11)=Rifle
     WeaponPriority(12)=DispersionPower4
     WeaponPriority(13)=Minigun
     WeaponPriority(14)=DispersionPower5
     ClientNetMinDelta=0.02500
     ServerNetMinDelta=0.02500
     bIsPlayer=True
     bCanJump=True
     DesiredSpeed=0.300000
     SightRadius=4100.000000
     bTravel=True
     bStasis=False
}
