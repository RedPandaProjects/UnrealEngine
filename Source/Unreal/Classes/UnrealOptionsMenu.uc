//=============================================================================
// UnrealOptionsMenu
//=============================================================================
class UnrealOptionsMenu expands UnrealMenu
	localized;

#exec Texture Import File=Textures\hud1.pcx Name=Hud1 MIPS=OFF
#exec Texture Import File=Textures\hud2.pcx Name=Hud2 MIPS=OFF
#exec Texture Import File=Textures\hud3.pcx Name=Hud3 MIPS=OFF
#exec Texture Import File=Textures\hud4.pcx Name=Hud4 MIPS=OFF
#exec Texture Import File=Textures\hud5.pcx Name=Hud5 MIPS=OFF
#exec Texture Import File=Textures\hud6.pcx Name=Hud6 MIPS=OFF

var() texture HUDIcon[6];
var   string[64] MenuValues[20];
var	  bool bJoystick;

function bool ProcessYes()
{
	if ( Selection == 1 )
		PlayerOwner.ChangeAutoAim(0.93);
	else if ( Selection == 2 )
	{
		bJoystick = true;
		PlayerOwner.ConsoleCommand("set windrv.windowsclient usejoystick "$int(bJoystick));
	}
	else if ( Selection == 4 )
		PlayerOwner.bInvertMouse = True;
	else if ( Selection == 5 )
		PlayerOwner.ChangeSnapView(True);
	else if ( Selection == 6 )
		PlayerOwner.ChangeAlwaysMouseLook(True);
	else if ( Selection == 7 )
		PlayerOwner.ChangeStairLook(True);
	else 
		return false;

	return true;
}

function bool ProcessNo()
{
	if ( Selection == 1 )
		PlayerOwner.ChangeAutoAim(1);
	else if ( Selection == 2 )
	{
		bJoystick = false;
		PlayerOwner.ConsoleCommand("set windrv.windowsclient usejoystick "$int(bJoystick));
	}
	else if ( Selection == 4 )
		PlayerOwner.bInvertMouse = False;
	else if ( Selection == 5 )
		PlayerOwner.ChangeSnapView(False);
	else if ( Selection == 6 )
		PlayerOwner.ChangeAlwaysMouseLook(False);
	else if ( Selection == 7 )
		PlayerOwner.ChangeStairLook(False);

	else 
		return false;

	return true;
}

function bool ProcessLeft()
{
	if ( Selection == 1 )
	{
		if ( PlayerOwner.MyAutoAim == 1 )
			PlayerOwner.ChangeAutoAim(0.93);
		else
			PlayerOwner.ChangeAutoAim(1);
	}
	else if ( Selection == 2 )
	{
		bJoystick = !bJoystick;
		PlayerOwner.ConsoleCommand("set windrv.windowsclient usejoystick "$int(bJoystick));
	}
	else if ( Selection == 3 )
		PlayerOwner.UpdateSensitivity(FMax(1,PlayerOwner.MouseSensitivity - 1));
	else if ( Selection == 4 )
		PlayerOwner.bInvertMouse = !PlayerOwner.bInvertMouse;
	else if ( Selection == 5 )
		PlayerOwner.ChangeSnapView(!PlayerOwner.bSnapToLevel);
	else if ( Selection == 6 )
		PlayerOwner.ChangeAlwaysMouseLook(!PlayerOwner.bAlwaysMouseLook);
	else if ( Selection == 7 )
		PlayerOwner.ChangeStairLook(!PlayerOwner.bLookUpStairs);
	else if ( Selection == 8 )
		PlayerOwner.ChangeCrossHair();
	else if ( Selection == 9 )
	{
		if ( PlayerOwner.Handedness == 1 )
			PlayerOwner.ChangeSetHand("Right");
		else if ( PlayerOwner.Handedness == 0 )
			PlayerOwner.ChangeSetHand("Left");
		else
			PlayerOwner.ChangeSetHand("Center");
	}
	else if ( Selection == 10 )
	{
		if ( PlayerOwner.DodgeClickTime > 0 )
			PlayerOwner.ChangeDodgeClickTime(-1);
		else
			PlayerOwner.ChangeDodgeClickTime(0.25);
	}
	else if ( Selection == 13 )
		PlayerOwner.myHUD.ChangeHUD(-1);
	else if ( Selection == 14 )
		PlayerOwner.UpdateBob(PlayerOwner.Bob - 0.004);
	else 
		return false;

	return true;
}

function bool ProcessRight()
{
	if ( Selection == 1 )
	{
		if ( PlayerOwner.MyAutoAim == 1 )
			PlayerOwner.ChangeAutoAim(0.93);
		else
			PlayerOwner.ChangeAutoAim(1);
	}
	else if ( Selection == 2 )
	{
		bJoystick = !bJoystick;
		PlayerOwner.ConsoleCommand("set windrv.windowsclient usejoystick "$int(bJoystick));
	}
	else if ( Selection == 3 )
		PlayerOwner.UpdateSensitivity(PlayerOwner.MouseSensitivity + 1);
	else if ( Selection == 4 )
		PlayerOwner.bInvertMouse = !PlayerOwner.bInvertMouse;
	else if ( Selection == 5 )
		PlayerOwner.ChangeSnapView(!PlayerOwner.bSnapToLevel);
	else if ( Selection == 6 )
		PlayerOwner.ChangeAlwaysMouseLook(!PlayerOwner.bAlwaysMouseLook);
	else if ( Selection == 7 )
		PlayerOwner.ChangeStairLook(!PlayerOwner.bLookUpStairs);
	else if ( Selection == 8 )
		PlayerOwner.MyHUD.ChangeCrossHair(-1);
	else if ( Selection == 9 )
	{
		if ( PlayerOwner.Handedness == -1 )
			PlayerOwner.ChangeSetHand("Left");
		else if ( PlayerOwner.Handedness == 0 )
			PlayerOwner.ChangeSetHand("Right");
		else
			PlayerOwner.ChangeSetHand("Center");
	}
	else if ( Selection == 10 )
	{
		if ( PlayerOwner.DodgeClickTime > 0 )
			PlayerOwner.ChangeDodgeClickTime(-1);
		else
			PlayerOwner.ChangeDodgeClickTime(0.25);
	}
	else if ( Selection == 13 )
		PlayerOwner.myHUD.ChangeHUD(1);
	else if ( Selection == 14 )
		PlayerOwner.UpdateBob(PlayerOwner.Bob + 0.004);
	else
		return false;

	return true;
}

function bool ProcessSelection()
{
	local Menu ChildMenu;

	if ( Selection == 1 )
	{
		if ( PlayerOwner.MyAutoAim == 1 )
			PlayerOwner.ChangeAutoAim(0.93);
		else
			PlayerOwner.ChangeAutoAim(1);
	}
	else if ( Selection == 2 )
	{
		bJoystick = !bJoystick;
		PlayerOwner.ConsoleCommand("set windrv.windowsclient usejoystick "$int(bJoystick));
	}
	else if ( Selection == 4 )
		PlayerOwner.bInvertMouse = !PlayerOwner.bInvertMouse;
	else if ( Selection == 5 )
		PlayerOwner.ChangeSnapView(!PlayerOwner.bSnapToLevel);
	else if ( Selection == 6 )
		PlayerOwner.ChangeAlwaysMouseLook(!PlayerOwner.bAlwaysMouseLook);
	else if ( Selection == 7 )
		PlayerOwner.ChangeStairLook(!PlayerOwner.bLookUpStairs);
	else if ( Selection == 8 )
		PlayerOwner.ChangeCrossHair();
	else if ( Selection == 9 )
	{
		if ( PlayerOwner.Handedness == 1 )
			PlayerOwner.SetHand("Right");
		else if ( PlayerOwner.Handedness == 0 )
			PlayerOwner.SetHand("Left");
		else
			PlayerOwner.SetHand("Center");
	}
	else if ( Selection == 10 )
	{
		if ( PlayerOwner.DodgeClickTime > 0 )
			PlayerOwner.ChangeDodgeClickTime(-1);
		else
			PlayerOwner.ChangeDodgeClickTime(0.25);
	}
	else if ( Selection == 13 )
		PlayerOwner.myHUD.ChangeHUD(1);
	else if ( Selection == 11 )
		ChildMenu = spawn(class'UnrealKeyboardMenu', owner);
	else if ( Selection == 12 )
		ChildMenu = spawn(class'UnrealWeaponMenu', owner);
	else if ( Selection == 15 )
		PlayerOwner.ConsoleCommand("PREFERENCES");
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
	PlayerOwner.myHUD.SaveConfig();
	PlayerOwner.SaveConfig();
}

function DrawValues(canvas Canvas, Font RegFont, int Spacing, int StartX, int StartY)
{
	local int i;

	Canvas.Font = RegFont;
	for (i=0; i< (MenuLength); i++ )
	{
		SetFontBrightness( Canvas, (i == Selection - 1) );
		Canvas.SetPos(StartX, StartY + Spacing * i);
		Canvas.DrawText(MenuValues[i + 1], false);
	}
	Canvas.DrawColor = Canvas.Default.DrawColor;
}

function DrawMenu(canvas Canvas)
{
	local int StartX, StartY, Spacing, i, HelpPanelX;

	DrawBackGround(Canvas, (Canvas.ClipY < 250));

	HelpPanelX = 228;

	Spacing = Clamp(0.04 * Canvas.ClipY, 11, 32);
	StartX = Max(40, 0.5 * Canvas.ClipX - 120);

	if ( Canvas.ClipY > 240 )
	{
		DrawTitle(Canvas);
		StartY = Max(36, 0.5 * (Canvas.ClipY - MenuLength * Spacing - 128));
	}
	else
		StartY = Max(8, 0.5 * (Canvas.ClipY - MenuLength * Spacing - 128));

	// draw text
	DrawList(Canvas, false, Spacing, StartX, StartY);  
	MenuValues[1] = string( PlayerOwner.MyAutoAim < 1 );
	bJoystick =	bool(PlayerOwner.ConsoleCommandResult("get windrv.windowsclient usejoystick"));
	MenuValues[2] = string(bJoystick);
	MenuValues[3] = string(int(PlayerOwner.MouseSensitivity));
	MenuValues[4] = string(PlayerOwner.bInvertMouse);
	MenuValues[5] = string(PlayerOwner.bSnapToLevel);
	MenuValues[6] = string(PlayerOwner.bAlwaysMouseLook);
	MenuValues[7] = string(PlayerOwner.bLookUpStairs);
	if ( PlayerOwner.Handedness == 1 )
		MenuValues[9] = LeftString;
	else if ( PlayerOwner.Handedness == 0 )
		MenuValues[9] = CenterString;
	else
		MenuValues[9] = RightString;
	if ( PlayerOwner.DodgeClickTime > 0 )
		MenuValues[10] = EnabledString;
	else
		MenuValues[10] = DisabledString;
	MenuValues[13] = string(PlayerOwner.MyHUD.HudMode);
	DrawValues(Canvas, Canvas.MedFont, Spacing, StartX+160, StartY);

	// draw icons
	DrawSlider(Canvas, StartX + 155, StartY + 13 * Spacing + 1, 1000 * PlayerOwner.Bob, 0, 4);

	PlayerOwner.MyHUD.DrawCrossHair(Canvas, StartX + 160, StartY + 7 * Spacing - 3 );
	Canvas.SetPos(StartX+168, Canvas.ClipY-125 );
	if (Selection==13)	
	{
		if (Canvas.ClipY > 380 && PlayerOwner.MyHUD.HudMode<=6 && HUDIcon[PlayerOwner.MyHud.HudMode]!=None)
			Canvas.DrawIcon(HUDIcon[PlayerOwner.MyHUD.HudMode],1.0);
		Canvas.Font = Canvas.MedFont;
		HelpPanelX = 150;
	}

	// Draw help panel
	DrawHelpPanel(Canvas, StartY + MenuLength * Spacing, HelpPanelX);
}

defaultproperties
{
     HUDIcon(0)=Unreal.Hud1
     HUDIcon(1)=Unreal.Hud2
     HUDIcon(2)=Unreal.Hud3
     HUDIcon(3)=Unreal.Hud4
     HUDIcon(4)=Unreal.Hud5
     HUDIcon(5)=Unreal.Hud6
	 MenuTitle="OPTIONS MENU"
	 MenuList(1)="Auto Aim"
	 MenuList(2)="Joystick Enabled"
	 MenuList(3)="Mouse Sensitivity"
	 MenuList(4)="Invert Mouse"
	 MenuList(5)="LookSpring"
	 MenuList(6)="Always MouseLook"
	 MenuList(7)="Auto Slope Look"
	 MenuList(8)="Crosshair"
	 MenuList(9)="Weapon Hand"
	 MenuList(10)="Dodging"
	 MenuList(11)="Customize Controls"
	 MenuList(12)="Prioritize Weapons"
	 MenuList(13)="HUD Configuration"
	 MenuList(14)="View Bob"
	 MenuList(15)="Advanced Options"
     HelpMessage(1)="Enable or disable vertical aiming help."
	 HelpMessage(2)="Toggle enabling of joystick."
     HelpMessage(3)="Adjust the mouse sensitivity, or how far you have to move the mouse to produce a given motion in the game."
     HelpMessage(4)="Invert the mouse X axis.  When true, pushing the mouse forward causes you to look down rather than up."
     HelpMessage(5)="If true, when you let go of the mouselook key the view will automatically center itself."
     HelpMessage(6)="If true, the mouse is always used for looking up and down, with no need for a mouselook key."
     HelpMessage(7)="If true, when not mouse-looking your view will automatically be adjusted to look up and down slopes and stairs."
     HelpMessage(8)="Choose the crosshair appearing at the center of your screen"
     HelpMessage(9)="Select where your weapon will appear."
     HelpMessage(10)="If enabled, double-clicking on the movement keys (forward, back, strafe left, and strafe right) will cause you to do a fast dodge move."
     HelpMessage(11)="Hit enter to customize keyboard, mouse, and joystick configuration."
     HelpMessage(12)="Hit enter to prioritize weapon switching order."
     HelpMessage(13)="Use the left and right arrow keys to select a Heads Up Display configuration."
     HelpMessage(14)="Adjust the amount of bobbing when moving."
	 HelpMessage(15)="Open advanced preferences configuration menu."
     MenuLength=15
     Class=Unreal.UnrealOptionsMenu
}
