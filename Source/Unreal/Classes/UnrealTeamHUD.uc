//=============================================================================
// UnrealTeamHUD
//=============================================================================
class UnrealTeamHUD expands UnrealHUD
	config;

#exec TEXTURE IMPORT NAME=BlueSkull FILE=TEXTURES\HUD\i_skullb.PCX GROUP="Icons" MIPS=OFF
#exec TEXTURE IMPORT NAME=GreenSkull FILE=TEXTURES\HUD\i_skullg.PCX GROUP="Icons" MIPS=OFF
#exec TEXTURE IMPORT NAME=RedSkull FILE=TEXTURES\HUD\i_skullr.PCX GROUP="Icons" MIPS=OFF
#exec TEXTURE IMPORT NAME=YellowSkull FILE=TEXTURES\HUD\i_skully.PCX GROUP="Icons" MIPS=OFF

simulated function DrawFragCount(Canvas Canvas, int X, int Y)
{
	local texture SkullTexture;

	SkullTexture = texture'IconSkull';

	if ( Pawn(Owner).TeamName ~= "red" )
		SkullTexture = texture'RedSkull';
	else if ( Pawn(Owner).TeamName ~= "blue" )
		SkullTexture = texture'BlueSkull';
	else if ( Pawn(Owner).TeamName ~= "green" )
		SkullTexture = texture'GreenSkull';
	else if ( Pawn(Owner).TeamName ~= "yellow" )
		SkullTexture = texture'YellowSkull';

	DrawSkull(Canvas, X, Y, SkullTexture);
}

function DrawSkull(Canvas Canvas, int X, int Y, texture SkullTexture)
{ 
	Canvas.SetPos(X,Y);
	Canvas.DrawIcon(SkullTexture, 1.0);	
	Canvas.CurX -= 19;
	Canvas.CurY += 23;
	Canvas.Font = Font'TinyWhiteFont';	
	if (Pawn(Owner).Score<100) Canvas.CurX+=6;
	if (Pawn(Owner).Score<10) Canvas.CurX+=6;	
	if (Pawn(Owner).Score<0) Canvas.CurX-=6;
	Canvas.DrawText(int(Pawn(Owner).Score),False);
				
}