/*=============================================================================
	UnURL.h: Unreal URL class.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	FURL.
-----------------------------------------------------------------------------*/

//
// A uniform resource locator.
//
class ENGINE_API FURL
{
public:
	// Variables.
	FString Protocol;	// Protocol, i.e. "unreal" or "http".
	FString Host;		// Optional hostname, i.e. "204.157.115.40" or "unreal.epicgames.com", blank if local.
	FString Map;		// Map name, i.e. "SkyCity", default is "Index".
	FString Portal;		// Portal to enter through, default is "".
	TArray<FString> Op;	// Options.
	INT		Port;       // Optional host port.
	UBOOL	Valid;		// Whether parsed successfully.

	// Statics.
	static FString DefaultProtocol;
	static FString DefaultProtocolDescription;
	static FString DefaultName;
	static FString DefaultMap;
	static FString DefaultLocalMap;
	static FString DefaultHost;
	static FString DefaultPortal;
	static FString DefaultMapExt;
	static FString DefaultSaveExt;
	static INT DefaultPort;

	// Constructors.
	FURL( const char* Filename=NULL );
	FURL( FURL* Base, const char* TextURL, ETravelType Type );

	// Functions.
	UBOOL IsInternal() const;
	UBOOL IsLocalInternal() const;
	UBOOL HasOption( const char* Test ) const;
	const char* GetOption( const char* Match, const char* Default ) const;
	void GetConfigOptions( const char* Section, const char* Filename=NULL );
	void AddOption( const char* Str );
	void String( FString& Result, UBOOL FullyQualified=0 ) const;
	static void Init();
	ENGINE_API friend FArchive& operator<<( FArchive& Ar, FURL& U );

	// Operators.
	UBOOL operator==( const FURL& Other ) const;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
