/*=============================================================================
	UnChan.h: Unreal datachannel class.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Types.
-----------------------------------------------------------------------------*/

// Types of channels.
enum EChannelType
{
	CHTYPE_None			= 0,  // Invalid type.
	CHTYPE_Control		= 1,  // Connection control.
	CHTYPE_Actor  		= 2,  // Actor-update channel.
	CHTYPE_File         = 3,  // Binary file transfer.
	CHTYPE_MAX          = 32, // Maximum.
};

// Channel state.
enum EChannelState
{
	UCHAN_Open          = 0, // Channel is open.
	UCHAN_Closing       = 1, // Channel is closing and awaiting acknowledgement.
};

// Up to this many reliable channel bunches may be buffered.
enum {RELIABLE_BUFFER=32};

// Sequence numbers.
enum EBunchSequences
{
	SEQ_None			= 0, // No sequence number.
	SEQ_Initial			= 1, // Initial sequence.
	SEQ_First			= 2, // First wraparound sequence.
};

// Channel flags.
enum EChannelFlags
{
	CHF_Mask           = 0x07ff, // Mask to get channel number.
	CHF_Ack            = 0x1000, // Acknowledging a packet.
	CHF_Close          = 0x2000, // Requesting to open a connection to the remote.
	CHF_Reliable       = 0x4000, // This packet is reliable and sequenced.
	CHF_AckNak         = 0x8000, // When applied on top of Ack, represents negative acknowledgement.
};

/*-----------------------------------------------------------------------------
	Channel type registry.
-----------------------------------------------------------------------------*/

// Global channel type functions.
BYTE GAutoRegisterChannel( INT ChType, FChannel* (*Constructor)( UNetConnection* InConnection, INT InChIndex, UBOOL InOpenedLocally ) );
UBOOL GIsKnownChannelType( INT Type );

// Goes inside class definition,
#define DECLARE_CHTYPE(chtype,chclass) \
public: \
	enum {ChannelType=chtype}; \
	static FChannel* Construct( UNetConnection* InConnection, INT InChIndex, UBOOL InOpenedLocally ) \
		{return new chclass( InConnection, InChIndex, InOpenedLocally );}

// Channel type autoregistration macro.
#define IMPLEMENT_CHTYPE(chclass) \
	BYTE autoregisterI##chclass = GAutoRegisterChannel( chclass::ChannelType, chclass::Construct );

// Channel table.
extern ENGINE_API FChannel* (*GChannelConstructors[CHTYPE_MAX])( UNetConnection* InConnection, INT InChIndex, UBOOL InOpenedLocally );

/*-----------------------------------------------------------------------------
	FChannel base class.
-----------------------------------------------------------------------------*/

//
// Base class of communication channels.
//
class ENGINE_API FChannel : public FOutputDevice
{
public:
	// Variables.
	UNetConnection*	Connection;     // Owner connection.
	INT             ChIndex;		// Index of this channel.
	INT				OpenedLocally;  // Whether channel was opened locally or by remote.
	EChannelState	State;			// State of the channel.
	EChannelType	ChType;			// Type of this channel.
	DOUBLE			CloseTime;      // Time initial close-request was sent.

	// Unsorted record of outgoing unacked data, NULL entries are blanked.
	FOutBunch*      OutRec[RELIABLE_BUFFER];
	DOUBLE			OutTime[RELIABLE_BUFFER];

	// Unsorted record of incoming queued up data with dependencies on unreceived data.
	FInBunch*       InRec[RELIABLE_BUFFER];

	// FOutputDevice dummy interface.
	void WriteBinary( const void* Data, int Length, EName MsgType ) {}

	// Constructor.
	FChannel( INT InChType, UNetConnection* InConnection, INT InChannelIndex, INT InOpenedLocally );
	virtual ~FChannel();

	// FChannel interface.
	virtual void Close();
	virtual char* Describe( char* String256 );
	virtual void ReceivedBunch( FInBunch& Bunch )=0;
	virtual void Tick();

	// General channel functions.
	void ReceivedAck( _WORD Sequence );
	void ReceivedNak( _WORD Sequence );
	void ReceivedRawBunch( FInBunch& Bunch, UBOOL bFirstTime );
	void SendClose();
	void SendRawBunch( FOutBunch& Bunch );
	UBOOL SendBunch( FOutBunch& Bunch, UBOOL Merge );
	UBOOL IsNetReady( UBOOL Saturate );
	void AssertInSequenced();
	INT ReserveOutgoingIndex( UBOOL bClose );
	INT MaxSend();
};

/*-----------------------------------------------------------------------------
	FControlChannel base class.
-----------------------------------------------------------------------------*/

//
// A channel for exchanging text.
//
class ENGINE_API FControlChannel : public FChannel
{
	DECLARE_CHTYPE(CHTYPE_Control,FControlChannel);

	// Constructor.
	FControlChannel( UNetConnection* InConnection, INT InChannelIndex, INT InOpenedLocally );
	~FControlChannel();

	// UChannel interface.
	void ReceivedBunch( FInBunch& Bunch );

	// FOutputDevice interface.
	void WriteBinary( const void *Data, int Length, EName MsgType );

	// FControlChannel interface.
	char* Describe( char* String256 );
};

/*-----------------------------------------------------------------------------
	FActorChannel.
-----------------------------------------------------------------------------*/

//
// A channel for exchanging actor properties.
//
class ENGINE_API FActorChannel : public FChannel
{
	DECLARE_CHTYPE(CHTYPE_Actor,FActorChannel);

	// Variables.
	ULevel*	Level;			// Level this actor channel is associated with.
	AActor* Actor;			// Actor this corresponds to.
	BYTE*	Recent;			// Most recently sent values.
	DOUBLE	RelevantTime;	// Last time this actor was relevant to client.
	DOUBLE	LastUpdateTime;	// Last time this actor was replicated.

	// Constructor.
	FActorChannel( UNetConnection* InConnection, INT InChannelIndex, INT InOpenedLocally );
	~FActorChannel();

	// UChannel interface.
	void ReceivedBunch( FInBunch& Bunch );
	void Close();

	// FActorChannel interface.
	char* Describe( char* String256 );
	void ReplicateActor( UBOOL FullReplication );
};

/*-----------------------------------------------------------------------------
	File transfer channel.
-----------------------------------------------------------------------------*/

//
// A channel for exchanging binary files.
//
class ENGINE_API FFileChannel : public FChannel
{
	DECLARE_CHTYPE(CHTYPE_File,FFileChannel);

	// Variables.
	char	Filename[256];	 // Filename being transfered.
	char    PrettyName[256]; // Pretty name of file.
	char	Error[256];		 // Error.
	FILE*	File;			 // File being transfered.
	INT		Transfered;		 // Bytes transfered.
	INT		PackageIndex;	 // Index of package in Map.

	// Constructor.
	FFileChannel( UNetConnection* InConnection, INT InChannelIndex, INT InOpenedLocally );
	~FFileChannel();

	// UChannel interface.
	void ReceivedBunch( FInBunch& Bunch );

	// FFileChannel interface.
	char* Describe( char* String256 );
	void Tick();
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
