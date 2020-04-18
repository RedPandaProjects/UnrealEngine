//=============================================================================
// Player: Corresponds to a real player (a local camera or remote net player).
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Player expands Object
	intrinsic;

//-----------------------------------------------------------------------------
// Player properties.

// Internal.
var intrinsic const int vfOut;

// The actor this player controls.
var transient const playerpawn Actor;
var transient const console Console;
var transient const dynamicstring TravelItems;

defaultproperties
{
}
