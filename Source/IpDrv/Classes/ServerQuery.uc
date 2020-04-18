//=============================================================================
// ServerQuery: GameSpy connects to this script to recieve server and 
//              game info.
//=============================================================================
class ServerQuery expands TcpLink;


var string[8]  Version;
var string[8]  NumPlayers;
var string[32] GameType;
var string[64] MapName;

var string[64] PlayerName;
var string[16] PlayerSkin;
var string[16] Kills;
var string[16] Deaths;
var string[16] TeamNumber;

var int 				QueryPort;			// Assume this becomes set by the ServerUplink class
var playerpawn 			UPlayer;
var playerpawn			UPlayerList[64];
var int           		iNumPlayers;
var GameInfo			GInfo;
var int 				InfoNum;
var int                 PlayerNum;
var float				TimePassed;			// IDIOT!!! don't make it an int!!
var string[240]			TempString;
var int					UpdatePeriod;
var int					AmtSent;
var int					AmtRcvd;
var string[32]			CommandString;
var string[64]			RcvTemp;
var string[192]			ResponseString;

function BeginPlay()
{
	UpdatePeriod = 40;		// No more than 40 seconds
}

//
// Initializes the main server information strings.   Pad with terminating whitespace.
//
function MakeServerInfo()
{
	Version = Level.EngineVersion;
	iNumPlayers=0;
	foreach AllActors( class 'playerpawn', UPlayer )
	{
		iNumPlayers++;		
	}
	NumPlayers = " " $ iNumPlayers $ " ";

	// Find out what kind of game is running
	GInfo = Level.Game;
	GameType = string(Level.Game.Class.Name) $ " "; // this way, the game class name is not hardcoded -Tim
	MapName = String(Level.Parent.Name) $ " ";	
}

//
// Stores all the player references in an array, so that they can be cycled through in Tick.
//
function GetPlayerReferences()
{
	local int i;
	i=0;
	foreach AllActors( class 'playerpawn', UPlayer )
	{
		UPlayerList[i] = UPlayer;
		i++;
	}
}

//
// Given an input command (quit) or variable name, returns the appropriate value/string
//
function int InterpretCommand( string[32] inputStr, out string[192] outputStr )
{
	local int index;

	index = InStr( inputStr, "0q" );		// That's Zero-Q
	if( index == 0 )
	{
		// Quit command recognized
		return 0;
	}

	// Otherwise assume this is a query of a variable inside the Game class
	outputStr = Level.Game.GetPropertyText( inputStr );
	return 1;
}


//
// ------- States -------
//


auto state Listening
{
	function Tick( float DeltaTime )
	{
		if( Listen( QueryPort ) )
		{
			// Operation successful, but did it connect?
			if( LinkState == TCP_Open )
			{
				// Connected, assume to GameSpy Server
				GotoState('SendingServerInfo');
			}
		}
		else
		{
			// Some error occured.  Repeat anyway.
		}
	}

Begin:
	Enable( 'Tick' );
}

state SendingServerInfo
{
	function Tick( float DeltaTime )
	{
		switch( InfoNum )
		{
			case 0:	// Version
				AmtSent += SendText( Version, AmtSent );
				if( AmtSent >= Len( Version ) )
				{
					AmtSent=0;
					InfoNum++;
				}
				break;

			case 1: // NumPlayers
				AmtSent += SendText( NumPlayers, AmtSent );
				if( AmtSent >= Len( NumPlayers ) )
				{
					AmtSent=0;
					InfoNum++;
				}
				break;
			
			case 2:	// GameType
				AmtSent += SendText( GameType, AmtSent );
				if( AmtSent >= Len( GameType ) )
				{
					AmtSent=0;
					InfoNum++;
					if( iNumPlayers == 0 )
					{
						// Next element will be the last, so append terminator }
						MapName = MapName $ " }";
					}
				}
				break;
			
			default: // MapName
				AmtSent += SendText( MapName, AmtSent );
				if( AmtSent >= Len( MapName ) )
				{
					AmtSent=0;
					GotoState('SendingPlayerInfo');
				}
		}

		TimePassed += DeltaTime;
		if( TimePassed >= UpdatePeriod )
		{
			// Considered unsuccessful, not finished sending after all this time.
			// Close the connection.
			Close();
			GotoState('Listening');
		}
	}

Begin:
	InfoNum=0;
	MakeServerInfo();
	GetPlayerReferences();
	TimePassed=0;
	AmtSent=0;
}

state SendingPlayerInfo
{
	function Tick( float DeltaTime )
	{
		switch( InfoNum )
		{
			case 0:	// Name
				AmtSent += SendText( PlayerName, AmtSent );
				if( AmtSent >= Len( PlayerName ) )
				{
					AmtSent=0;
					InfoNum++;
				}
				break;
	
			case 1:	// Skin
				AmtSent += SendText( PlayerSkin, AmtSent );
				if( AmtSent >= Len( PlayerSkin ) )
				{
					AmtSent=0;
					InfoNum++;
				}
				break;
					
			case 2: // Kills
				AmtSent += SendText( Kills, AmtSent );
				if( AmtSent >= Len( Kills ) )
				{
					AmtSent=0;
					InfoNum++;
				}
				break;
			
			case 3:	// Deaths
				AmtSent += SendText( Deaths, AmtSent );
				if( AmtSent >= Len( Deaths ) )
				{
					AmtSent=0;
					InfoNum++;
				}
				break;
	
			default: // TeamNumber
				AmtSent += SendText( TeamNumber, AmtSent );
				if( AmtSent >= Len( TeamNumber ) )
				{
					AmtSent=0;
					InfoNum=0;
					PlayerNum++;
					
					if( PlayerNum < iNumPlayers )
					{
						// Load the data of the next player
						PlayerName = UPlayerList[PlayerNum].PlayerName $ " ";
						PlayerSkin = UPlayerList[PlayerNum].Skin $ " ";
						Kills      = UPlayerList[PlayerNum].KillCount $ " ";
						Deaths     = UPlayerList[PlayerNum].DieCount $ " ";
						TeamNumber = UPlayerList[PlayerNum].Team $ " ";
						if( PlayerNum == iNumPlayers-1 )
						{
							// This is the last player in the list
							// Add a terminator to the last field
							TeamNumber = TeamNumber $ "} ";
						}
					}
					else
					{
						// Finished Sending basic data, now accept extended queries
						GotoState('RecvQuery');
					}
				}
		}	

		TimePassed += DeltaTime;
		if( TimePassed >= UpdatePeriod )
		{
			// Considered unsuccessful, not finished sending after all this time.
			// Close the connection.
			GotoState('WaitWhileClosing');
		}
	}
	
Begin:
	if( iNumPlayers > 0 )
	{
		PlayerNum=0;
		InfoNum=0;
		AmtSent=0;
		PlayerName = UPlayerList[0].PlayerName $ " ";
		PlayerSkin = UPlayerList[0].Skin $ " ";
		Kills      = UPlayerList[0].KillCount $ " ";
		Deaths     = UPlayerList[0].DieCount $ " ";
		TeamNumber = UPlayerList[0].Team $ " ";
		if( PlayerNum == iNumPlayers-1 )
		{
			// This is the last player in the list
			// Add a terminator to the last field
			TeamNumber = TeamNumber $ "} ";
		}
	}
	else
	{
		// Nothing to be sent, skip this phase of transmission
		Disable('Tick');
		GotoState('RecvQuery');
	}
}


state RecvQuery
{
	//
	// Recieve the command from the querying application
	//
	function Tick( float DeltaTime )
	{
		local int index;
		
		AmtRcvd += ReadText( TempString, 31-AmtRcvd );
		if( AmtRcvd > 31 )
		{
			// Too much data, abort
			GotoState('WaitWhileClosing');
		}

		index = InStr( TempString, "*" );				// Check for terminator *

		if( index >= 0 )
		{
			// Remove the *-terminator
			TempString = Left( TempString, index );
			CommandString = CommandString $ TempString;

			// Command finished, interpret it and send something back
			index = InterpretCommand( CommandString, ResponseString );
			if( index == 0 )
			{
				// Quit command recieved
				GotoState('WaitWhileClosing');
			}
			else
			{
				// Send back the value of the variable
				GotoState('AnswerQuery');
			}
		}
		else
		{
			CommandString = CommandString $ RcvTemp;
		}

		TimePassed += DeltaTime;
		if( TimePassed >= UpdatePeriod )
		{
			// Considered unsuccessful, not finished receiving after all this time.
			// Close the connection.
			GotoState('WaitWhileClosing');
		}
	}
Begin:
	Enable('Tick');
	CommandString = "";
	AmtRcvd=0;
}


state AnswerQuery
{
	//
	// Send back the value of the queried variable in ResponseString
	//
	function Tick( float DeltaTime )
	{
		AmtSent += SendText( ResponseString, AmtSent );
		if( AmtSent >= Len( ResponseString ) )
		{
			// Finished sending, return to receive mode
			GotoState('RecvQuery');
		}

		TimePassed += deltaTime;
		if( TimePassed >= UpdatePeriod )
		{
			// Considered unsuccessful, not finished sending after all this time.
			// Close the connection.
			GotoState('WaitWhileClosing');
		}
	}
Begin:
	AmtSent=0;
}


state WaitWhileClosing
{
	function Timer()
	{
		Disable('Timer');
		Close();		
		Enable('Tick');
		GotoState('Listening');
	}

Begin:
	// This wait is necessary otherwise the close() command will
	// destroy the queued data that was sent.
	Disable('Tick');
	Enable('Timer');
	SetTimer(6,false);
}

defaultproperties
{
}