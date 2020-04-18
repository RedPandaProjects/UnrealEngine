//=============================================================================
// SpecialEvent: Receives trigger messages and does some "special event"
// depending on the state.
//=============================================================================
class SpecialEvent expands Triggers
	localized;

#exec Texture Import File=Textures\TrigSpcl.pcx Name=S_SpecialEvent Mips=Off Flags=2

//-----------------------------------------------------------------------------
// Variables.

var() int        Damage;         // For DamagePlayer state.
var() name		 DamageType;
var() localized string[64] DamageString;
var() sound      Sound;          // For PlaySoundEffect state.
var() localized string[80] Message; // For all states.
var() bool       bBroadcast;     // To broadcast the message to all players.
var() bool       bPlayerViewRot; // Whether player can rotate the view while pathing.

//-----------------------------------------------------------------------------
// Functions.

function Trigger( actor Other, pawn EventInstigator )
{
	local pawn P;
	if( bBroadcast )
		BroadcastMessage(Message, true); // Broadcast message to all players.
	else if( EventInstigator!=None && len(Message)!=0 )
	{
		// Send message to instigator only.
		EventInstigator.ClientMessage( Message );
	}
}

//-----------------------------------------------------------------------------
// States.

// Just display the message.
state() DisplayMessage
{
}

// Damage the instigator who caused this event.
state() DamageInstigator
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		Global.Trigger( Self, EventInstigator );
		if ( Other.IsA('PlayerPawn') )
			Level.Game.SpecialDamageString = DamageString;
		Other.TakeDamage( Damage, EventInstigator, EventInstigator.Location, Vect(0,0,0), DamageType);
	}
}

// Kill the instigator who caused this event.
state() KillInstigator
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		Global.Trigger( Self, EventInstigator );
		if ( Other.IsA('PlayerPawn') )
			Level.Game.SpecialDamageString = DamageString;
		if( EventInstigator != None )
			EventInstigator.Died( None, DamageType, EventInstigator.Location );
	}
}

// Play a sound.
state() PlaySoundEffect
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		Global.Trigger( Self, EventInstigator );
		PlaySound( Sound );
	}
}

// Play a sound.
state() PlayersPlaySoundEffect
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		local pawn P;

		Global.Trigger( Self, EventInstigator );
		P = Level.PawnList;

		while ( P != None )
		{
			if ( P.bIsPlayer && P.IsA('PlayerPawn') )
				P.PlaySound( Sound, SLOT_None, 4, true, 4 );
			P = P.nextPawn;
		}
	}
}

// Place Ambient sound effect on player
state() PlayAmbientSoundEffect
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		Global.Trigger( Self, EventInstigator );
		EventInstigator.AmbientSound = AmbientSound;
	}
}


// Send the player on a spline path through the level.
state() PlayerPath
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		local InterpolationPoint i;
		Global.Trigger( Self, EventInstigator );
		if( EventInstigator!=None && EventInstigator.bIsPlayer )
		{
			foreach AllActors( class 'InterpolationPoint', i, Event )
			{
				if( i.Position == 0 )
				{
					EventInstigator.GotoState('');
					EventInstigator.SetCollision(True,false,false);
					EventInstigator.bCollideWorld = False;
					EventInstigator.Target = i;
					EventInstigator.SetPhysics(PHYS_Interpolating);
					EventInstigator.PhysRate = 1.0;
					EventInstigator.PhysAlpha = 0.0;
					EventInstigator.bInterpolating = true;
					EventInstigator.AmbientSound = AmbientSound;
				}
			}
		}
	}
}

defaultproperties
{
     Texture=Texture'Engine.S_SpecialEvent'
}
