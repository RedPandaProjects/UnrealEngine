//=============================================================================
// IntroNullHud.
//=============================================================================
class SpectatorHud expands UnrealHUD;

#exec OBJ LOAD FILE=textures\menugr.utx PACKAGE=Unreal.MenuGfx

simulated function PostRender( canvas Canvas )
{
	local float StartX;

	HUDSetup(canvas);

	if ( (PlayerPawn(Owner) != None) && PlayerPawn(Owner).bShowMenu  )
	{
		if ( MainMenu == None )
			CreateMenu();
		if ( MainMenu != None )
			MainMenu.DrawMenu(Canvas);
		return;
	}
	else if ( PlayerPawn(Owner).ProgressTimeOut > Level.TimeSeconds )
		DisplayProgressMessage(Canvas);

	if (Canvas.ClipY<290) Return;

	Canvas.Style = 2;
	StartX = 0.5 * Canvas.ClipX - 128;	
	Canvas.SetPos(StartX,Canvas.ClipY-58);
	Canvas.DrawTile( Texture'MenuBarrier', 256, 64, 0, 0, 256, 64 );
	StartX = 0.5 * Canvas.ClipX - 128;
	Canvas.Style = 2;	
	Canvas.SetPos(StartX,Canvas.ClipY-52);
	Canvas.DrawIcon(texture'Logo2', 1.0);	
	Canvas.Style = 1;
}

defaultproperties
{
}
