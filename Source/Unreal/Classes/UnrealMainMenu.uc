//=============================================================================
// UnrealMainMenu
//=============================================================================
class UnrealMainMenu expands UnrealMenu
	localized;

var bool bBegun;

function bool ProcessSelection()
{
	local Menu ChildMenu;

	ChildMenu = None;
	if ( ! bBegun )
	{
		PlayEnterSound();
		bBegun = true;
	}

	if ( Selection == 1 )
		ChildMenu = spawn(class'UnrealGameMenu', owner);
	else if ( Selection == 2 )
		ChildMenu = spawn(class'UnrealMultiPlayerMenu', owner);
	else if ( Selection == 3 )
		ChildMenu = spawn(class'UnrealOptionsMenu', owner);
	else if ( Selection == 4 )
		ChildMenu = spawn(class'UnrealVideoMenu', owner);
	else	
		ChildMenu = spawn(class'UnrealQuitMenu', owner);

	if ( ChildMenu != None )
	{
		HUD(Owner).MainMenu = ChildMenu;
		ChildMenu.ParentMenu = self;
		ChildMenu.PlayerOwner = PlayerOwner;
	}
	return true;
}

function DrawMenu(canvas Canvas)
{
	local int StartX, StartY, Spacing;
	
	DrawBackGround(Canvas, false);

	Canvas.Style = 3;

	Spacing = Clamp(0.1 * Canvas.ClipY, 16, 48);
	StartX = Max(40, 0.5 * Canvas.ClipX - 96);
	StartY = Max(4, 0.5 * (Canvas.ClipY - 5 * Spacing - 128));

	// draw text
	DrawList(Canvas, true, Spacing, StartX, StartY); 

	// Draw help panel
	DrawHelpPanel(Canvas, StartY + MenuLength * Spacing + 4, 228);
}

defaultproperties
{
	 MenuList(1)="GAME"
	 MenuList(2)="MULTIPLAYER"
	 MenuList(3)="OPTIONS"
	 MenuList(4)="AUDIO/VIDEO"
	 MenuList(5)="QUIT"
     HelpMessage(1)="Hit enter to modify game options, including loading and saving games, changing difficulty level, and starting a BotMatch."
     HelpMessage(2)="Hit enter to modify Multiplayer setup options, including starting or joining a network game, and changing your appearance, name, or team."
     HelpMessage(3)="Hit enter to customize controls."
	 HelpMessage(4)="Change sound and display options"
     HelpMessage(5)="Hit enter to quit game."
     MenuLength=5
     Class=Unreal.UnrealMainMenu
}
