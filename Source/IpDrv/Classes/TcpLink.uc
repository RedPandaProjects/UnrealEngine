//=============================================================================
// TcpLink: An Internet TCP/IP connection.
//=============================================================================
class TcpLink expands Info
	intrinsic
	transient;

//-----------------------------------------------------------------------------
// Variables.

var const string[80] URL;
var const byte  IP[4];
var const int   Port;
var float       KeepaliveSeconds;
var const float KeepaliveCounter;
var float       TimeoutSeconds;
var const float TimeoutCounter;
var const byte  TcpInternal[64];

var const int 	MainSocket;			// Socket
var const int   ConnectSocket;		// Socket
var const bool  bSocketInitialized;
var const bool  bSocketBound;
var const bool  bWSAInitialized;

var const enum ETcpLinkState
{
	TCP_Closed,		// Connection is closed.
	TCP_Resolving,	// Resolving a URL.
	TCP_Connecting,	// Connecting.
	TCP_Listening,	// Listening for connection.
	TCP_Open,		// Open and connected.
} LinkState;

var enum ETcpMode
{
	TMOD_Binary,	// Raw binary data.
	TMOD_Text,      // Raw text.
	TMOD_Line,      // Cr-Lf delineated lines of text.
} LinkMode;

//-----------------------------------------------------------------------------
// Internal functions.

// Listen for an incoming connection.
// Can only call when LinkState==TCP_Closed.
// Returns true if listening successfully, false
// if port is already in use.
intrinsic function bool Listen( int InPort );

// Try to connect to a remote host.
// Can only call when  LinkState==TCP_Closed.
intrinsic function bool Open( string[80] OpenURL, int InPort );

// Close the connection.
// On return, LinkState==TCP_Closed.
intrinsic function Close();

// Send text.
// If LinkMode==TMOD_TextLines, sends a terminating cr/lf.
// Otherwise, sends the text without termination.
intrinsic function int SendText( coerce string[240] Str, int offset );

intrinsic function int ReadText( out string[240] Str, int ReadLen );

// Send raw binary data.
intrinsic function int SendBinary( int Count, byte B[240] );

// Extract.
intrinsic function int ReadBinary( int Count, out byte B[240] );

// Returns last error which occured with winsock
intrinsic function int GetLastError();

// Mangles the given integer by a function, for handshaking purposes
intrinsic function int Encrypt( int Key );

// Given a string containing an IP address or Domain name, returns 
// the corresponding IP address (if successful)
intrinsic function int GetIPByName( coerce string[240] Domain, out string[240] IpAddr );

//-----------------------------------------------------------------------------
// Event notifications.

// Called when a Listen() is accepted.
event Accepted();

// Called when an Open() succeeds.
event Connected();

// Called whenever the connection
// state becomes TCP_Closed except when Close() is called.
event Closed();

// Called when data is received and LinkMode==TMOD_Binary.
event ReceivedBinary( int Count );

// Called when text is received and LinkMode==TMOD_Text.
event ReceivedText( string[240] S );

// Called when a line of text is received and LinkMode==TMOD_Lines.
event ReceivedLine( string[240] S );

defaultproperties
{
	bAlwaysTick=true
}
