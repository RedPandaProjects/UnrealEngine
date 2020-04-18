//=============================================================================
// OKMenu
//=============================================================================
class OKMenu expands InfoMenu
	localized;

var localized string[32] OKString;

function ProcessResponse()
{
	//process

	ExitMenu();
}

function DrawResponse(canvas Canvas)
{
	Canvas.SetPos(100,84);
	Canvas.DrawText(OKString, False);
}

function bool ProcessYes()
{
	ProcessResponse();
	return true;
}

function bool ProcessSelection()
{
	ProcessResponse();
	return true;
}

defaultproperties
{
     OKString="[OK]"
}