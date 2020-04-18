//=============================================================================
// IntroNullHud.
//=============================================================================
class IntroNullHud expands UnrealHUD
	localized;
	
#exec OBJ LOAD FILE=textures\menugr.utx PACKAGE=Unreal.MenuGfx

var() localized string[64] ESCMessage;

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

	Canvas.Font = Canvas.MedFont;
	Canvas.SetPos(Canvas.ClipX/2.0-66,4);	
	Canvas.DrawText(ESCMessage, False);	

	Canvas.Style = 2;
	StartX = 0.5 * Canvas.ClipX - 128;	
	Canvas.SetPos(StartX,Canvas.ClipY-58);
	Canvas.DrawTile( Texture'MenuBarrier', 256, 64, 0, 0, 256, 64 );
	StartX = 0.5 * Canvas.ClipX - 128;
	Canvas.Style = 2;	
	Canvas.SetPos(StartX,Canvas.ClipY-52);
	Canvas.DrawIcon(texture'Logo2', 1.0);	

	if (Canvas.ClipX>790)
	{
		Canvas.SetPos(0,Canvas.ClipY-128);
		Canvas.DrawIcon(texture'DE', 1.0);		
		Canvas.SetPos(0,Canvas.ClipY-256);
		Canvas.DrawIcon(texture'GT', 1.0);			
		Canvas.SetPos(0,Canvas.ClipY-384);
		Canvas.DrawIcon(texture'Epic', 1.0);
	}	
	else if (Canvas.ClipX>390)
	{
		Canvas.SetPos(0,Canvas.ClipY-64);
		Canvas.DrawIcon(texture'DE2', 1.0);		
		Canvas.SetPos(0,Canvas.ClipY-128);
		Canvas.DrawIcon(texture'GT', 0.5);			
		Canvas.SetPos(0,Canvas.ClipY-192);
		Canvas.DrawIcon(texture'Epic2', 1.0);
	}
	
	Canvas.Style = 1;
}

defaultproperties
{
     ESCMessage="Press ESC to begin"
}
