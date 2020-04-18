//=============================================================================
// ServerUplink: Sends timely information to the Master Server based on the
//               game in progress.
//=============================================================================
class ServerUplink expands TcpLink config;

var() config bool        DoUplink;					// Should this dedicated server tell the master server?
var() config string[255] MasterServerAddress;		// The domain/ip address of the master server
var() config string[255] CommentString;				// SHORT!!! Comment about the game
var() config int 		 GamePort;  				// What is the default gameport?

var string[255] InformationString;					// Basic server data + {*
var string[16]  CommentEnd;							// Basically *
var string[255] PerPlayerData;						// Contains the information for one player
var string[16]	EndingString;						// Basically }

var string[255] ThisMachinesURL;
var string[16]  ServerVersion;
var string[16]  GameTypeString;
var string[64]  LevelFileName;
var string[255] TempStr;

var playerpawn 	UPlayer;
var playerpawn	UPlayerList[64];
var int         InfoNum;     
var int         PlayerNum;
var int         NumPlayers;
var bool		MakePlayerString;

var int UpdatePeriod;  			//15*60

var int MasterPort;				//=41357
var int VerifyPort;				//=41359
var int QueryPort;				//Gameport+1000

var float TimePassed;
var int   AmtSent;

var ServerVerify SVerify;		// References to the necessary helper scripts
var ServerQuery  SQuery;


//
// Take out any * signs and replace with +
//
function PruneCommentString()
{
	local bool fin;
	local int  index, index2;

	fin=false;
	while(!fin)
	{
		index = InStr( CommentString, "*" );
		if( index != -1 )
		{
			// Replace
			index2 = Len( CommentString );
			if( index+1 < index2 )
			{
				// Not the last character in the string
				CommentString = Left( CommentString, index ) $ "+" $
								Right( CommentString, index2-index-1 );
			}
			else
			{
				// It's the last character in the string
				CommentString = Left( CommentString, index ) $ "+";
				fin=true;
			}
			
		}
		else fin=true;
	}
}

//
// Creates the basic server information string
//
function MakeInformationString()
{
	LevelFileName	= String(Level.Parent.Name);

	// How many players exist in the game?
	NumPlayers=0;
	foreach AllActors( class 'playerpawn', UPlayer ) {
		NumPlayers++;
	}

	// Find out what kind of game is running
	GameTypeString = string(Level.Game.Class.Name); // this way, the game class name is not hardcoded -Tim

	// Write the main data header
	InformationString = ThisMachinesURL $ " " $ ServerVersion  $ " " $ 
						NumPlayers      $ " " $ GameTypeString $ " " $ 
						LevelFileName   $ " " $ GamePort       $ " " $ 
						QueryPort       $ " { *";
}


//
// Do the main initialization
//
function BeginPlay()
{
	local int index1, index2;
	local string[240] TempURL;

	if( !DoUplink )
	{
		Destroy();
		return;
	}

	UpdatePeriod = 15*60;		// every 15 minutes

	// Parse Level.GetAddressURL() for IP address and port number
	// Format: Ip.Address.com:PortNum/LevelName if the port isn't the
	// default.  Otherwise, its
	// 		   Ip.Address.com/LevelName

	TempStr = Level.GetAddressURL();
	index2 = InStr( TempStr, ":"  );			// End bound for IpAddress	

	if( index2 > 0 )
	{
		// Format with port included
		ThisMachinesURL = Mid( TempStr, 0, index2 );
		TempStr = Mid( TempStr, index2+1 );			// Get the substring starting at Port number
		index2 = InStr( TempStr, "/" );				// End bound for Port number
		if( index2 > 0 )
			GamePort = Int( Mid( TempStr, 0, index2 ) );
		// otherwise assume the intialized default, 7777
	}
	else
	{
		// Format with port excluded
		index2 = InStr( TempStr, "/" );			// End bound for IpAddress
		ThisMachinesURL = Mid( TempStr, 0, index2 );
		// GamePort will stay with the initialized default, 7777
	}

	// Ensure that the Master server address is in IP address form
	if( GetIpByName( MasterServerAddress, TempURL ) == 1 )
	{
		// Success
		MasterServerAddress = TempURL;
	}
		
	QueryPort    = GamePort + 1000;
	MasterPort   = 41357;
	VerifyPort   = 41359;


	ServerVersion = Level.EngineVersion;
	PruneCommentString();
	CommentEnd = "* ";
	EndingString = " } ";
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
// Wakes up every few minutes to send some information to the Master server.
//
function Timer()
{
	Disable('Timer');
	TimePassed=0;
	AmtSent=0;
	GotoState('TryToConnect');
}


//
// ------- States -------
//

auto state Initial
{
Begin:
	Disable('Tick');
	SetTimer(30, false);	// Wait 30 seconds for unreal to finish thrashing
	SQuery  = Spawn( Class 'ServerQuery'  );
		SQuery.QueryPort = QueryPort;
	SVerify = Spawn( Class 'ServerVerify' );
		SVerify.VerifyPort = VerifyPort;
}

state WaitPeriod
{
Begin:
	// Set it to wake up and re-send data
	Enable('Timer');
	SetTimer(UpdatePeriod, false);
}

state TryToConnect
{
	function Tick( float DeltaTime )
	{
		// Open a connection to the master server
		Open( MasterServerAddress, MasterPort );
		if( LinkState == TCP_Open )
		{
				GotoState('SendData');
		}
		TimePassed+=DeltaTime;
		if( TimePassed >= UpdatePeriod )
		{
			// Error, could not connect to master server
			log("ServerUplink: Critical, cannot establish link to master server.  Game will not be visible.");
			Disable('Tick');
			destroy();
		}
	}

Begin:
	//log("Connecting to " $ MasterServerAddress $ " over port " $ MasterPort);
	Enable('Tick');
}

state WaitWhileClosing
{
	function Timer()
	{
		Disable('Timer');
		Close();		
		Enable('Tick');
		GotoState('WaitPeriod');
	}

Begin:
	// This wait is necessary otherwise the close() command will
	// destroy the queued data that was sent.
	//log("Closing.");
	Disable('Tick');
	SetTimer(6,false);
}

state SendData
{
	function Tick( float deltaTime )
	{
		switch( InfoNum )
		{
			case 0:		// Basic Information
				//log("Basic Information");
				AmtSent += SendText( InformationString, AmtSent );
				if( AmtSent >= Len( InformationString ) )
				{
					AmtSent=0;
					InfoNum++;
				}
				break;
				
			case 1:		// Comment String
				//log("Comment String");
				AmtSent += SendText( CommentString, AmtSent );
				if( AmtSent >= Len( CommentString ) )
				{
					AmtSent=0;
					InfoNum++;
				}
				break;
			
			case 2: 	// Comment Ending
				//log("Comment Ending");
				AmtSent += SendText( CommentEnd, AmtSent );
				if( AmtSent >= Len( CommentEnd ) )
				{
					AmtSent=0;
					InfoNum++;
				}
				break;
			
			case 3: 	// Each player individually
				//log("Each player individually");
				if( NumPlayers > 0 )
				{
					// Should a new player string be created?
					if( MakePlayerString )
					{
						PerPlayerData = UPlayerList[PlayerNum].PlayerName $ " " $
										UPlayerList[PlayerNum].Skin $ " " $
										UPlayerList[PlayerNum].KillCount $ " " $
										UPlayerList[PlayerNum].DieCount $ " " $
										UPlayerList[PlayerNum].Team $ " ";
						AmtSent=0;
						MakePlayerString=false;
					}				
				
					AmtSent += SendText( PerPlayerData, AmtSent );
					if( AmtSent >= Len( PerPlayerData ) )
					{
						MakePlayerString=true;
						PlayerNum++;
						if( PlayerNum >= NumPlayers )
						{
							InfoNum++;
						}
					}
				}
				else InfoNum++;
				break;
		
			default:	// Ending
				//log("Ending");
				AmtSent += SendText( EndingString, AmtSent );
				if( AmtSent >= Len( EndingString ) )
				{
					AmtSent=0;
					GotoState('WaitWhileClosing');
				}
		}

		TimePassed += deltaTime;
		if( TimePassed >= UpdatePeriod )
		{
			// Considered unsuccessful, not finished sending after all this time.
			// Close the connection, try again immediately
			Close();
			GotoState('TryToConnect');
		}
	}
Begin:

	// Wait a moment, otherwise for some reason non-blocking mode malfunctions in send
	Disable('Tick');
	sleep(1);

	InfoNum=0;
	PlayerNum=0;
	MakePlayerString=true;
	MakeInformationString();
	GetPlayerReferences();

	//log("Sending data.");
	Enable('Tick');
}

defaultproperties
{
}

