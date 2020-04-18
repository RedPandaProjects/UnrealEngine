//=============================================================================
// TriggerLight.
// A lightsource which can be triggered on or off.
//=============================================================================
class TriggerLight expands Light;

//-----------------------------------------------------------------------------
// Variables.

var() float ChangeTime;        // Time light takes to change from on to off.
var() bool  bInitiallyOn;      // Whether it's initially on.
var() bool  bDelayFullOn;      // Delay then go full-on.
var() float RemainOnTime;      // How long the TriggerPound effect lasts

var   float InitialBrightness; // Initial brightness.
var   float Alpha, Direction;
var   actor SavedTrigger;
var   float poundTime;

//-----------------------------------------------------------------------------
// Engine functions.

// Called at start of gameplay.
function BeginPlay()
{
	// Remember initial light type and set new one.
	Disable( 'Tick' );
	InitialBrightness = LightBrightness;
	if( bInitiallyOn )
	{
		Alpha     = 1.0;
		Direction = 1.0;
	}
	else
	{
		Alpha     = 0.0;
		Direction = -1.0;
	}
}

// Called whenever time passes.
function Tick( float DeltaTime )
{
	Alpha += Direction * DeltaTime / ChangeTime;
	if( Alpha > 1.0 )
	{
		Alpha = 1.0;
		Disable( 'Tick' );
		if( SavedTrigger != None )
			SavedTrigger.EndEvent();
	}
	else if( Alpha < 0.0 )
	{
		Alpha = 0.0;
		Disable( 'Tick' );
		if( SavedTrigger != None )
			SavedTrigger.EndEvent();
	}
	if( !bDelayFullOn )
		LightBrightness = Alpha * InitialBrightness;
	else if( (Direction>0 && Alpha!=1) || Alpha==0 )
		LightBrightness = 0;
	else
		LightBrightness = InitialBrightness;
}

//-----------------------------------------------------------------------------
// Public states.

// Trigger turns the light on.
state() TriggerTurnsOn
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		if( SavedTrigger!=None )
			SavedTrigger.EndEvent();
		SavedTrigger = Other;
		SavedTrigger.BeginEvent();
		Direction = 1.0;
		Enable( 'Tick' );
	}
}

// Trigger turns the light off.
state() TriggerTurnsOff
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		if( SavedTrigger!=None )
			SavedTrigger.EndEvent();
		SavedTrigger = Other;
		SavedTrigger.BeginEvent();
		Direction = -1.0;
		Enable( 'Tick' );
	}
}

// Trigger toggles the light.
state() TriggerToggle
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		if( SavedTrigger!=None )
			SavedTrigger.EndEvent();
		SavedTrigger = Other;
		SavedTrigger.BeginEvent();
		Direction *= -1;
		Enable( 'Tick' );
	}
}

// Trigger controls the light.
state() TriggerControl
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		if( SavedTrigger!=None )
			SavedTrigger.EndEvent();
		SavedTrigger = Other;
		SavedTrigger.BeginEvent();
		if( bInitiallyOn ) Direction = -1.0;
		else               Direction = 1.0;
		Enable( 'Tick' );
	}
	function UnTrigger( actor Other, pawn EventInstigator )
	{
		if( SavedTrigger!=None )
			SavedTrigger.EndEvent();
		SavedTrigger = Other;
		SavedTrigger.BeginEvent();
		if( bInitiallyOn ) Direction = 1.0;
		else               Direction = -1.0;
		Enable( 'Tick' );
	}
}

state() TriggerPound {

	function Timer () {
	
		if (poundTime >= RemainOnTime) {
		
			Disable ('Timer');
		}
		poundTime += ChangeTime;
		Direction *= -1;
		SetTimer (ChangeTime, false);
	}

	function Trigger( actor Other, pawn EventInstigator )
	{

		if( SavedTrigger!=None )
			SavedTrigger.EndEvent();
		SavedTrigger = Other;
		SavedTrigger.BeginEvent();
		Direction = 1;
		poundTime = ChangeTime;			// how much time will pass till reversal
		SetTimer (ChangeTime, false);		// wake up when it's time to reverse
		Enable   ('Timer');
		Enable   ('Tick');
	}
}

defaultproperties
{
	bStatic=False
	bMovable=True
}
