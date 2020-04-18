//=============================================================================
// UnrealMultiPlayerMenu
//=============================================================================
class UnrealMultiPlayerMenu expands UnrealMenu
	localized;

function bool ProcessSelection()
{
	local Menu ChildMenu;

	ChildMenu = None;

	if ( Selection == 1 )
		ChildMenu = spawn(class'UnrealJoinGameMenu', owner);
	else if ( Selection == 2 )
		ChildMenu = spawn(class'UnrealServerMenu', owner);
	else
		ChildMenu = spawn(class'UnrealPlayerMenu', owner);

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

	DrawBackGround(Canvas, False);
	
	Spacing = Clamp(0.1 * Canvas.ClipY, 16, 48);
	StartX = Max(40, 0.5 * Canvas.ClipX - 96);
	StartY = Max(8, 0.5 * (Canvas.ClipY - 5 * Spacing - 128));

	// draw text
	DrawList(Canvas, true, Spacing, StartX, StartY); 

	// Draw help panel
	if ( Canvas.ClipY > 300 )
		DrawHelpPanel(Canvas, StartY + MenuLength * Spacing + 16, 228);
	else
		DrawHelpPanel(Canvas, StartY + MenuLength * Spacing, 228);
}

defaultproperties
{
	 MenuList(1)="JOIN GAME"
	 MenuList(2)="START GAME"
	 MenuList(3)="PLAYER SETUP"
     HelpMessage(1)="Join a network game."
     HelpMessage(2)="Set up and start a network game."
     HelpMessage(3)="Configure appearance, name, and team name."
     MenuLength=3
     Class=Unreal.UnrealMultiPlayerMenu
}
