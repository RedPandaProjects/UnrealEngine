//=============================================================================
// ServerVerify: Authentication for the game server.
//=============================================================================
class ServerVerify expands TcpLink;

// Waits for the Master Server to connect and send the code.  The code is then
// mangled by the encrypt function, and returned to the Master Server.  Once this
// occurs, this script becomes inactive.

var int 		VerifyPort;			// Assumes this is set by the ServerUplink object
var string[32] 	CodeString;			// Stores the code passed from master server
var string[240]	RcvTemp;			// Buffer for recieving text
var int        	InitialCode;
var int        	ModifiedCode;
var int         AmtTrans;
var int         i,l;

auto state Listening
{
	function Tick( float DeltaTime )
	{
		if( Listen( VerifyPort ) )
		{
			// Operation successful, but did it connect?
			if( LinkState == TCP_Open )
			{
				// Connected, assume to Master Server
				GotoState('Recieving');
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

state Recieving
{
	function Tick( float DeltaTime )
	{
		// Expects to recieve exactly 10 characters
		AmtTrans  += ReadText( RcvTemp, 10-AmtTrans );
		CodeString = CodeString $ RcvTemp;
		if( AmtTrans >= 10 )
		{
			// The code was recieved.  Convert and send the 
			// corresponding response back.
			InitialCode  = Int    ( CodeString );
			ModifiedCode = Encrypt( InitialCode );
			CodeString   = String ( ModifiedCode );
			// Make sure that the string is exactly 10 characters long,
			// excluding the NULL.
			l = Len( CodeString );
			if( l < 9 )
			{
				// Pad it with space characters
				for( i=0; i<9-l; i++ )
				{
					CodeString = CodeString $ " ";				
				}				
			}
			GotoState('Sending');
		}
	}

Begin:
	AmtTrans   = 0;
	CodeString = "";
}

state Sending
{
	function Tick( float DeltaTime )
	{
		// The Master Server expects exactly 10 characters
		AmtTrans += SendText( CodeString, AmtTrans );
		if( AmtTrans >= 10 )
		{
			// The code was fully sent.
			// Disconnect, set the timer, go to bed, wake up later and repeat.
			GotoState('WaitWhileClosing');
		}
	}

Begin:
	AmtTrans = 0;
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
	SetTimer(6,false);
}

state Inactive
{
Begin:
	Disable( 'Tick' );
}

defaultproperties
{
}