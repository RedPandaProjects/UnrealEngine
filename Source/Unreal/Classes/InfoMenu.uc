//=============================================================================
// InfoMenu
//=============================================================================
class InfoMenu expands UnrealMenu;

#exec TEXTURE IMPORT NAME=TranslatorHUD3 FILE=models\TRANHUD3.PCX GROUP="Icons" FLAGS=2 MIPS=OFF

function DrawMenu(canvas Canvas)
{
	local float TempX, TempY;

	Canvas.bCenter = false;
	Canvas.Font = Canvas.MedFont;
	TempX = Canvas.ClipX;
	TempY = Canvas.ClipY;
	Canvas.Style = 2;	
	Canvas.SetPos(Canvas.ClipX/2-128, Canvas.ClipY/2-68);
	Canvas.DrawIcon(texture'TranslatorHUD3', 1.0);
	Canvas.SetOrigin(Canvas.ClipX/2-110,Canvas.ClipY/2-52);
	Canvas.SetClip(225,110);
	Canvas.SetPos(0,4);
	Canvas.Style = 1;	
	Canvas.DrawText(MenuList[1], False);
	DrawResponse(Canvas);
	Canvas.ClipX = TempX;
	Canvas.ClipY = TempY;
}

function DrawResponse(canvas Canvas);
function ProcessResponse();

defaultproperties
{
}
