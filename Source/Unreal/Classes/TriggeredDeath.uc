//=============================================================================
// TriggeredDeath.
// When triggered, kills the player, causing screen flashes and sounds.
//=============================================================================
class TriggeredDeath expands Triggers;

var() Sound  MaleDeathSound;
var() Sound  FemaleDeathSound;
var() float  StartFlashScale;
var() Vector StartFlashFog;
var() float  EndFlashScale;
var() Vector EndFlashFog;
var() float  ChangeTime;
var() Name   DeathName;
var() bool   bKillPlayer;	// should the player be killed?
var() bool   bDestroyItems;	// destroy any items which may touch it as well

var float TimePassed;
var UnrealPlayer Victim;

function BeginPlay()
{
	Victim = None;
}

function Touch( Actor Other )
{
	// Something has contacted the death trigger.
	// If it is an UnrealPlayer, have it screen flash and
	// die.
	if( UnrealPlayer( Other ) != None )
	{
		TimePassed = 0;
		Victim = UnrealPlayer( Other );
		if( Female( Other ) != None )
			Victim.PlaySound( FemaleDeathSound, SLOT_Talk );
		else
			Victim.PlaySound( MaleDeathSound, SLOT_Talk );
		Enable('Tick');
	}
	else if( bDestroyItems )
	{
		Other.Destroy();
	}
}


function Tick( float DeltaTime )
{
	local Float CurScale;
	local vector CurFog;
	local float  TimeRatio;

	if( Victim != None )
	{
		// Check the timing
		TimePassed += DeltaTime;
		if( TimePassed >= ChangeTime )
		{
			TimeRatio = 1;
			Disable('Tick');
			if( bKillPlayer )
			{
				Victim.level.game.Killed(None, Victim, DeathName);
				Victim.HidePlayer();
				Victim.Level.Game.DiscardInventory(Victim);
				Victim.Health = -1;
				Victim.Score -= 1;
				Victim.GotoState('Dying');
			}
			Victim.ClientFlash( EndFlashScale, 1000 * EndFlashFog );
		}
		else TimeRatio = TimePassed/ChangeTime;

		// Continue the screen flashing
		CurScale = (EndFlashScale-StartFlashScale)*TimeRatio + StartFlashScale;
		CurFog   = (EndFlashFog  -StartFlashFog  )*TimeRatio + StartFlashFog;

		Victim.ClientFlash( CurScale, 1000 * CurFog );
	}
	else Disable('Tick');
}

defaultproperties
{
}


