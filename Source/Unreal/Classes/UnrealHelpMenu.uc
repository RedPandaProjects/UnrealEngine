//=============================================================================
// UnrealHelpMenu
//=============================================================================
class UnrealHelpMenu expands UnrealMenu
	config
	localized;

function bool ProcessSelection()
{
	local Menu ChildMenu;

	if ( Selection == 1 )
		PlayerOwner.ConsoleCommand("START ..\\help\\trouble.htm");
	else
		return false;

	return true;
}

function DrawMenu(canvas Canvas)
{
	local int StartX, StartY, Spacing, i, HelpPanelX;

	DrawBackGround(Canvas, (Canvas.ClipY < 250));

	HelpPanelX = 228;

	Spacing = Clamp(0.06 * Canvas.ClipY, 12, 32);
	StartX = Max(40, 0.5 * Canvas.ClipX - 120);

	DrawTitle(Canvas);
	StartY = Max(36, 0.5 * (Canvas.ClipY - MenuLength * Spacing - 128));

	// draw text
	DrawList(Canvas, false, Spacing, StartX, StartY);  

	// Draw help panel
	DrawHelpPanel(Canvas, StartY + MenuLength * Spacing, HelpPanelX);
}

defaultproperties
{
	 bRetail=true
	 MenuTitle="HELP"
	 MenuList(1)="TroubleShooting"
     HelpMessage(1)="Open the troubleshooting document."
     MenuLength=1
}
