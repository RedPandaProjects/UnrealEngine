//=============================================================================
// Spectator.
//=============================================================================
class Spectator expands PlayerPawn;

exec function BehindView( Bool B )
{
}

exec function Fly()
{
	UnderWaterTime = -1;	
	SetCollision(false, false, false);
	bCollideWorld = true;
	log("Fly");
	GotoState('CheatFlying');

	ClientRestart();
}

function ClientReStart()
{
	//log("client restart");
	Velocity = vect(0,0,0);
	Acceleration = vect(0,0,0);
	BaseEyeHeight = Default.BaseEyeHeight;
	EyeHeight = BaseEyeHeight;
	
	GotoState('CheatFlying');
}

function PlayerTimeOut()
{
	if (Health > 0)
		Died(None, 'dropped', Location);
}

exec function Grab()
{
}

// Send a message to all players.
exec function Say( string[255] S )
{
	if ( !Level.Game.bMuteSpectators )
		BroadcastMessage( PlayerName$":"$S, true );
}

//=============================================================================
// functions.

exec function RestartLevel()
{
}

// This pawn was possessed by a player.
function Possess()
{
	bIsPlayer = true;
	DodgeClickTime = FMin(0.3, DodgeClickTime);
	EyeHeight = BaseEyeHeight;
	NetPriority = 8;
	Weapon = None;
	Inventory = None;
	Fly();
}

function PostBeginPlay()
{
	if (Level.LevelEnterText != "" )
		ClientMessage(Level.LevelEnterText);
	bIsPlayer = true;
	FlashScale = vect(1,1,1);
}

//=============================================================================
// Inventory-related input notifications.

// The player wants to switch to weapon group numer I.
exec function SwitchWeapon (byte F )
{
}

exec function NextItem()
{
}

exec function PrevItem()
{
}

exec function ActivateItem()
{
}

exec function Fire( optional float F )
{
}

exec function AltFire( optional float F )
{
}

//=================================================================================

function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
{
}

defaultproperties
{
     Visibility=0
     AttitudeToPlayer=ATTITUDE_Friendly
	 bHidden=true
     bCollideActors=False
     bCollideWorld=False
     bBlockActors=False
     bBlockPlayers=False
     bProjTarget=False
}