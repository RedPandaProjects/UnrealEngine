//=============================================================================
// UnrealGameOptionsMenu
//=============================================================================
class UnrealGameOptionsMenu expands UnrealMenu
	config
	localized;

var() localized string[64] AdvancedString;
var() localized string[128] AdvancedHelp;
var() config bool bCanModifyGore;
var() class<GameInfo> GameClass;
var	  GameInfo	GameType;

function Destroyed()
{
	Super.Destroyed();
	if ( GameType != Level.Game )
		GameType.Destroy();
}

function PostBeginPlay()
{
	Super.PostBeginPlay();
	if ( Level.Game.Class == GameClass )
		GameType = Level.Game;
	else
		GameType = Spawn(GameClass);
}

function MenuProcessInput( byte KeyNum, byte ActionNum )
{
	Super.MenuProcessInput(KeyNum, ActionNum);
	if ( !bCanModifyGore )
	{
		if ( KeyNum == EInputKey.IK_Up )
		{
			if ( Selection == 2 )
				Selection = 1;
		}
		else if ( KeyNum == EInputKey.IK_Down )
		{
			if ( Selection == 2 )
				Selection = 3;
		}
	}
}
function bool ProcessLeft()
{
	if ( Selection == 1 )
		GameType.SetGameSpeed(FMax(0.5, Level.TimeDilation - 0.1));
	else if ( (Selection == 2) && bCanModifyGore )
		GameType.bLowGore = !GameType.bLowGore;
	else 
		return false;

	return true;
}

function bool ProcessRight()
{
	if ( Selection == 1 )
		GameType.SetGameSpeed(FMin(2.0, Level.TimeDilation + 0.1));
	else if ( (Selection == 2) && bCanModifyGore )
		GameType.bLowGore = !GameType.bLowGore;
	else 
		return false;

	return true;
}

function bool ProcessSelection()
{
	local Menu ChildMenu;

	if ( (Selection == 2) && bCanModifyGore )
		Level.Game.bLowGore = !Level.Game.bLowGore;
	//else if ( Selection == MenuLength )
	//	ChildMenu = spawn(class'UnrealMenu', owner);
	else
		return false;

	if ( ChildMenu != None )
	{
		HUD(Owner).MainMenu = ChildMenu;
		ChildMenu.ParentMenu = self;
		ChildMenu.PlayerOwner = PlayerOwner;
	}
	return true;
}

function SaveConfigs()
{
	if ( GameType != None )
		GameType.SaveConfig();
	PlayerOwner.SaveConfig();
}

function DrawOptions(canvas Canvas, int StartX, int StartY, int Spacing)
{
	MenuList[1] = Default.MenuList[1];
	if ( bCanModifyGore )
		MenuList[2] = Default.MenuList[2];
	else
		MenuList[2] = "";
	//MenuList[MenuLength] = AdvancedString;
    //HelpMessage[MenuLength] = AdvancedHelp;
	DrawList(Canvas, false, Spacing, StartX, StartY);  
}

function DrawValues(canvas Canvas, int StartX, int StartY, int Spacing)
{
	local int s;

	s = 10 * (GameType.GameSpeed + 0.02);
	MenuList[1] = (""$(10 * s)$"%");
	if ( bCanModifyGore )
		MenuList[2] = string(GameType.bLowGore);
	else
		MenuList[2] = "";
	//MenuList[MenuLength] = "";
	DrawList(Canvas, false, Spacing, StartX + 160, StartY);  
}

function DrawMenu(canvas Canvas)
{
	local int StartX, StartY, Spacing;

	DrawBackGround(Canvas, false);

	StartX = Max(40, 0.5 * Canvas.ClipX - 115);

	if ( (MenuLength < 6) || (Canvas.ClipY > 240) )
	{
		DrawTitle(Canvas);
		Spacing = Clamp(0.04 * Canvas.ClipY, 12, 32);
		StartY = Max(40, 0.5 * (Canvas.ClipY - MenuLength * Spacing - 128));
	}
	else
	{
		Spacing = Clamp(0.04 * Canvas.ClipY, 11, 32);
		StartY = Max(4, 0.5 * (Canvas.ClipY - MenuLength * Spacing - 128));
	}
	DrawOptions(Canvas, StartX, StartY, Spacing);
	DrawValues(Canvas, StartX, StartY, Spacing);		
	DrawHelpPanel(Canvas, StartY + MenuLength * Spacing + 4, 228);
}

defaultproperties
{
	 bCanModifyGore=true
	 MenuList(1)="Game Speed"
	 MenuList(2)="Reduced Gore"
	 MenuTitle="GAME OPTIONS"
	 AdvancedString="Advanced Options"
	 AdvancedHelp="Edit advanced game configuration options."
     HelpMessage(1)="Adjust the speed at which time passes in the game."
     HelpMessage(2)="If true, reduces the gore in the game."
     MenuLength=2
     Class=Unreal.UnrealGameOptionsMenu
	 GameClass=SinglePlayer
}
