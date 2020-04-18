//=============================================================================
// ScoreBoard
//=============================================================================
class ScoreBoard expands Actor;

function ShowScores( canvas Canvas );
function UpdateScores();
function UpdateNext(string[20] CurrentName, int offset, PlayerPawn requester);
function RefreshScores(string[20] NewName, float NewScore, byte newOffset, byte NewNum);
function QuickRefreshScores(float NewScore, byte newOffset, byte NewNum);

function PreBeginPlay()
{
	// don't call parent prebeginplay
}

defaultproperties
{
	bHidden=true
}