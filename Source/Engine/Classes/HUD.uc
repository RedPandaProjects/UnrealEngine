//=============================================================================
// HUD: Superclass of the heads-up display.
//=============================================================================
class HUD expands Actor
	abstract
	config
	intrinsic;

//=============================================================================
// Variables.

var config int HudMode;	
var config int Crosshair;
var() class<menu> MainMenuType;
var	Menu MainMenu;

//=============================================================================
// Status drawing.

simulated function PreRender( canvas Canvas );
simulated function PostRender( canvas Canvas );
simulated function InputNumber(byte F);
simulated function ChangeHud(int d);
simulated function ChangeCrosshair(int d);
simulated function DrawCrossHair( canvas Canvas, int StartX, int StartY);

defaultproperties
{
     bHidden=True
     RemoteRole=ROLE_SimulatedProxy
}
