//=============================================================================
// UnrealScoreBoard
//=============================================================================
class UnrealScoreBoard expands ScoreBoard;

var float 		Scores[16];
var string[20]  Names[16];
var float		LastRequestTime;
var byte		RequestOffset;
var byte		NumPlayers;
var bool		bRefreshed;

// server update scores
function UpdateScores()
{
	local Pawn aPawn, Moving, Temp;
	local Pawn Scoring[16];
	local int i;

	aPawn = Level.PawnList;
	NumPlayers = 0;

	// get a sorted list of the top 16 players
	while ( aPawn != None )
	{
		if ( aPawn.bIsPlayer )
		{
			if ( NumPlayers < 16 )
				NumPlayers = NumPlayers + 1;
			Moving = aPawn;
			for (i=0; i<NumPlayers; i++)
			{
				if ( Scoring[i] == None )
					Scoring[i] = Moving;
				else if ( Scoring[i].Score < Moving.Score )
				{
					Temp = Moving;
					Moving = Scoring[i];
					Scoring[i] = Temp;
				}
			}
		}		
		aPawn = aPawn.nextPawn;
	}

	for ( i=0; i<16; i++ )
	{
		if ( i < NumPlayers )
		{
			Names[i] = Scoring[i].PlayerName;
			Scores[i] = Scoring[i].Score;
		}
		else
			Names[i] = "";
	}
}

// UpdateNext - server returns next update, and a new request offset
function UpdateNext(string[20] CurrentName, int offset, PlayerPawn requester)
{
	if ( CurrentName == Names[offset] )
		requester.QuickRefreshScores(Scores[offset], offset, NumPlayers);
	else
		requester.ClientRefreshScores(Names[offset], Scores[offset], offset, NumPlayers);
}

// RefreshScores() - refresh based on new score received from server
function RefreshScores(string[20] NewName, float NewScore, byte newOffset, byte NewNum)
{
	NumPlayers = NewNum;
	Names[NewOffset] = NewName;
	Scores[NewOffset] = NewScore;
}

function QuickRefreshScores(float NewScore, byte newOffset, byte NewNum)
{
	NumPlayers = NewNum;
	Scores[NewOffset] = NewScore;
}

function ShowScores( canvas Canvas )
{
	local int i, num, max;

	if ( Level.TimeSeconds - LastRequestTime > 0.07 )
	{
		RequestOffset = RequestOffset + 1;
		if ( RequestOffset >= NumPlayers )
		{
			bRefreshed = true;
			RequestOffset = 0;
		}
		PlayerPawn(Owner).ServerRequestScores(Names[RequestOffset], RequestOffset);
		LastRequestTime = Level.TimeSeconds;
	}

	max = int(0.03725 * Canvas.ClipY);
	
	// Display it
	Canvas.Font = Canvas.MedFont;
	num = 0;

	if ( bRefreshed )
		for ( i=0; i<NumPlayers; i++ )
		{
			if ( Names[i] != "" )
			{
				Canvas.SetPos(0.2 * Canvas.ClipX, 0.2 * Canvas.ClipY + 16 * num );
				Canvas.DrawText(Names[i], False);
				Canvas.SetPos(0.7 * Canvas.ClipX, 0.2 * Canvas.ClipY + 16 * num );
				Canvas.DrawText(int(Scores[i]), False);
				num++;
				if ( num >= max )
					break;
			}
		}	
}

defaultproperties
{
	NumPlayers=16
}