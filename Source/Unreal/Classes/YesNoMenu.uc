//=============================================================================
// YesNoMenu
//=============================================================================
class YesNoMenu expands InfoMenu
	localized;

var localized string[32] YesSelString;
var localized string[32] NoSelString;
var bool bResponse;

function ProcessResponse()
{
	//process based on state of bResponse

	ExitMenu();
}

function DrawResponse(canvas Canvas)
{
	Canvas.SetPos(72,84);
	if ( bResponse )
		Canvas.DrawText(YesSelString, False);
	else
		Canvas.DrawText(NoSelString, False);
}

function bool ProcessYes()
{
	bResponse = true;
	ProcessResponse();
	return true;
}

function bool ProcessNo()
{
	bResponse = false;
	ProcessResponse();
	return true;
}

function bool ProcessLeft()
{
	bResponse = !bResponse;
	return true;
}

function bool ProcessRight()
{
	bResponse = !bResponse;
	return true;
}

function bool ProcessSelection()
{
	ProcessResponse();
	return true;
}

defaultproperties
{
     YesSelString="[YES]  No"
     NoSelString=" Yes  [NO]"
}