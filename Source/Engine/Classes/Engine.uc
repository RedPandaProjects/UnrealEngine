//=============================================================================
// Engine: The base class of the global application object classes.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Engine expands Subsystem
	intrinsic
	config
	transient;

// Drivers.
var(Drivers) config class<RenderDevice>   GameRenderDevice;
var(Drivers) config class<AudioSubsystem> AudioDevice;
var(Drivers) config class<Console>        Console;
var(Drivers) config class<NetDriver>      NetworkDevice;
var(Drivers) config class<Language>       Language;

// Variables.
var primitive Cylinder;
var const client Client;
var const renderbase Render;
var const audiosubsystem Audio;
var int TickCycles, GameCycles, ClientCycles;
var(Settings) config int CacheSizeMegs;
var(Settings) config bool UseSound;

defaultproperties
{
	CacheSizeMegs=2
	UseSound=True
}
