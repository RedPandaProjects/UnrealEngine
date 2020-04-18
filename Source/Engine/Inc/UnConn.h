/*=============================================================================
	UnConn.h: Unreal network connection base class.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

class UNetDriver;

/*-----------------------------------------------------------------------------
	UNetConnection.
-----------------------------------------------------------------------------*/

//
// State of a connection.
//
enum EConnectionState
{
	USOCK_Invalid   = 0, // Connection is invalid, possibly uninitialized.
	USOCK_Closed    = 1, // Connection permanently closed.
	USOCK_Pending	= 2, // Connection is awaiting connection.
	USOCK_Open      = 3, // Connection is open.
};

//
// A network connection.
//
class ENGINE_API UNetConnection : public UPlayer
{
	DECLARE_ABSTRACT_CLASS(UNetConnection,UPlayer,CLASS_Transient|CLASS_Config)
	NO_DEFAULT_CONSTRUCTOR(UNetConnection)

	// Constants.
	enum{ MAX_PROTOCOL_VERSION = 1     }; // Maximum protocol version supported.
	enum{ MIN_PROTOCOL_VERSION = 1     }; // Minimum protocol version supported.
	enum{ MAX_PACKET_SIZE      = 512   }; // Absolute maximum size of a packet.
	enum{ IDEAL_PACKET_SIZE    = 192   }; // Ideal size of a packet.
	enum{ MAX_CHANNELS         = 2047  }; // Maximum channels that can be open.

	// Connection information.
	UNetDriver*		Driver;				  // Owning driver.
	EConnectionState State;				  // State this connection is in.
	FURL			URL;				  // URL of the other side.

	// Negotiated parameters.
	INT				ProtocolVersion;	  // Protocol version we're communicating with (<=PROTOCOL_VERSION).
	INT				MaxPacket;			  // Maximum packet size.
	INT				ByteLimit;			  // Maximum bytes per second.
	INT				SimLatency;			  // Simulated latency in milliseconds.
	INT 			SimPacketLoss;		  // Simulated packet loss, 0=none, 100=all.
	INT				Challenge;			  // Server-generated challenge.
	FString			RequestURL;			  // URL requested by client

	// Internal.
	DOUBLE			LastReceiveTime;	  // Last time a packet was received, for timeout checking.
	DOUBLE			LastSendTime;		  // Last time a packet was sent, for keepalives.
	DOUBLE			LastTickTime;		  // Last time of polling.
	INT				QueuedBytes;		  // Bytes assumed to be queued up.
	INT				LastBunchStart;		  // Most recently sent bunch.
	INT				LastBunchEnd;		  // Most recently sent bunch.
	INT				LastOutIndex;		  // Most recent outgoing index.
	INT				SentBunchStart;		  // Most recently sent bunch end.

	// Packet.
	BYTE	OutData[MAX_PACKET_SIZE];     // Outgoing packet.
	INT		OutNum;						  // Number of bytes in outgoing packet.

	// Channel table.
	FChannel*  Channels     [ MAX_CHANNELS ];
	_WORD      OutReliable  [ MAX_CHANNELS ];
	_WORD      OutSequence  [ MAX_CHANNELS ];
	_WORD      LastInRetired[ MAX_CHANNELS ];
	_WORD      LastInRcvd   [ MAX_CHANNELS ];

	// Constructors and destructors.
	UNetConnection( UNetDriver* Driver );

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();

	// UPlayer interface.
	void ReadInput( APlayerPawn* Actor, FLOAT DeltaSeconds );

	// FOutputDevice interface.
	void WriteBinary( const void* Data, int Length, EName MsgType=NAME_None );

	// UNetConnection interface.
	UNetDriver* GetDriver() {return Driver;}
	virtual char* Describe( char* String256 )=0;
	virtual void FlushNet( UBOOL Duplicate=0 );
	virtual void Tick();
	virtual void AssertValid();
	virtual UBOOL IsNetReady();
	class FControlChannel* GetControlChannel();
	FChannel* CreateChannel( enum EChannelType Type, UBOOL bOpenedLocally, _WORD ChannelIndex=MAXWORD );
	void ReceivedPacket( BYTE* Data, INT Size );
	void SendAck( _WORD ChIndex, _WORD Sequence );
	void SendNak( _WORD ChIndex, _WORD Sequence );
	class FActorChannel* GetActorChannel( AActor* Actor );
	void ReceiveFile( INT PackageIndex );
	void SlowAssertValid()
	{
#if CHECK_ALL
		AssertValid();
#endif
	}
};

/*-----------------------------------------------------------------------------
	Channel iterators.
-----------------------------------------------------------------------------*/

//
// Simple channel iterator.
//
class FChannelIterator
{
public:
	// Functions.
	FChannelIterator( UNetConnection* InConn )
	:	Conn( InConn), Index( -1 )
	{
		++*this;
	}
	void operator++()
	{
		debug(Index<UNetConnection::MAX_CHANNELS);
		while( ++Index<UNetConnection::MAX_CHANNELS && !Conn->Channels[Index] );
	}
	operator UBOOL()
	{
		return Index<UNetConnection::MAX_CHANNELS;
	}
	INT GetIndex()
	{
		return Index;
	}
	FChannel* operator* ()
	{
		debug(Index<UNetConnection::MAX_CHANNELS);
		return Conn->Channels[Index];
	}
	FChannel* operator-> ()
	{
		debug(Index<UNetConnection::MAX_CHANNELS);
		return Conn->Channels[Index];
	}
private:
	// Variables.
	INT Index;
	UNetConnection* Conn;
};

//
// Typed channel iterator.
//
template<class T> class FTypedChannelIterator
{
public:
	// Functions.
	FTypedChannelIterator( UNetConnection* InConn )
	:	Conn( InConn), Index( -1 )
	{
		++*this;
	}
	void operator++()
	{
		debug(Index<UNetConnection::MAX_CHANNELS);
		while
		(	++Index<UNetConnection::MAX_CHANNELS
		&&	(!Conn->Channels[Index] || Conn->Channels[Index]->ChType!=T::ChannelType) );
	}
	operator UBOOL()
	{
		return Index<UNetConnection::MAX_CHANNELS;
	}
	INT GetIndex()
	{
		return Index;
	}
	T* operator* ()
	{
		debug(Index<UNetConnection::MAX_CHANNELS);
		debug(Conn->Channels[Index])
		return (T*)Conn->Channels[Index];
	}
	T* operator-> ()
	{
		debug(Index<UNetConnection::MAX_CHANNELS);
		debug(Conn->Channels[Index])
		return (T*)Conn->Channels[Index];
	}
private:
	// Variables.
	INT Index;
	UNetConnection* Conn;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
