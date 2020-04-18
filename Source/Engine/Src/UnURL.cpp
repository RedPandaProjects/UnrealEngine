/*=============================================================================
	UnURL.cpp: Various file-management functions.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"

/*-----------------------------------------------------------------------------
	FURL Statics.
-----------------------------------------------------------------------------*/

// Variables.
FString FURL::DefaultProtocol;
FString FURL::DefaultProtocolDescription;
FString FURL::DefaultName;
FString FURL::DefaultMap;
FString FURL::DefaultLocalMap;
FString FURL::DefaultHost;
FString FURL::DefaultPortal;
FString FURL::DefaultMapExt;
FString FURL::DefaultSaveExt;
INT		FURL::DefaultPort=0;

// Static init.
void FURL::Init()
{
	guard(FURL::Init);

	DefaultProtocol				= GetConfigStr( "URL", "Protocol" );
	DefaultProtocolDescription	= GetConfigStr( "URL", "ProtocolDescription" );
	DefaultName					= GetConfigStr( "URL", "Name" );
	DefaultMap					= GetConfigStr( "URL", "Map" );
	DefaultLocalMap				= GetConfigStr( "URL", "LocalMap" );
	DefaultHost					= GetConfigStr( "URL", "Host" );
	DefaultPortal				= GetConfigStr( "URL", "Portal" );
	DefaultMapExt				= GetConfigStr( "URL", "MapExt" );
	DefaultSaveExt				= GetConfigStr( "URL", "SaveExt" );
	DefaultPort					= appAtoi( GetConfigStr( "URL", "Port" ) );

	unguard;
}

ENGINE_API FArchive& operator<<( FArchive& Ar, FURL& U )
{
	guard(FURL<<);
	Ar << U.Protocol << U.Host << U.Map << U.Portal << U.Op << U.Port << U.Valid;
	return Ar;
	unguard;
}

template FArchive& operator<<( FArchive& Ar, TArray<char>& );

/*-----------------------------------------------------------------------------
	Internal.
-----------------------------------------------------------------------------*/

static UBOOL ValidNetChar( const char* c )
{
	if( appStrchr(c,' ') )
		return 0;
	else
		return 1;
}

/*-----------------------------------------------------------------------------
	Constructors.
-----------------------------------------------------------------------------*/

//
// Constuct a purely default, local URL from an optional filename.
//
FURL::FURL( const char* LocalFilename )
:	Protocol	( DefaultProtocol )
,	Host		( DefaultHost )
,	Map			( LocalFilename ? LocalFilename : DefaultMap )
,	Portal		( DefaultPortal )
,	Port		( DefaultPort )
,	Op			()
,	Valid		( 1 )
{}

//
// Construct a URL from text and an optional relative base.
//
FURL::FURL( FURL* Base, const char* TextURL, ETravelType Type )
:	Protocol	( DefaultProtocol )
,	Host		( DefaultHost )
,	Map			( DefaultMap )
,	Portal		( DefaultPortal )
,	Port		( DefaultPort )
,	Op			()
,	Valid		( 1 )
{
	guard(FURL::FURL);
	check(TextURL);

	// Make a copy.
	char Temp[1024], *URL=Temp;
	appStrncpy( Temp, TextURL, ARRAY_COUNT(Temp) );

	// Copy Base.
	if( Type==TRAVEL_Relative )
	{
		check(Base);
		Protocol = Base->Protocol;
		Host     = Base->Host;
		Map      = Base->Map;
		Portal   = Base->Portal;
		Port     = Base->Port;
	}
	if( Type==TRAVEL_Relative || Type==TRAVEL_Partial )
	{
		check(Base);
		for( INT i=0; i<Base->Op.Num(); i++ )
		{
			if
			(	appStricmp(*Base->Op(i),"PUSH")!=0
			&&	appStricmp(*Base->Op(i),"POP" )!=0
			&&	appStricmp(*Base->Op(i),"PEER")!=0
			&&	appStricmp(*Base->Op(i),"LOAD")!=0 )
				new(Op)FString(Base->Op(i));
		}
	}

	// Skip leading blanks.
	while( *URL == ' ' )
		URL++;

	// Options.
	char* s = appStrchr(URL,'?');
	if( s )
	{
		*s++ = 0;
		do
		{
			char* t = appStrchr(s,'?');
			if( t )
				*t++ = 0;
			if( !ValidNetChar( s ) )
			{
				*this = FURL();
				Valid = 0;
				return;
			}
			AddOption( s );
			s = t;
		} while( s );
	}

	// Portal.
	s = appStrchr(URL,'#');
	if( s )
	{
		*s++ = 0;
		char* t = appStrchr(s,'?');
		if( t )
		{
			*t++ = 0;
			URL = t;
		}
		if( !ValidNetChar( s ) )
		{
			*this = FURL();
			Valid = 0;
			return;
		}
		Portal = s;
	}

	// Handle pure filenames.
	UBOOL FarHost=0;
	if( appStrlen(URL)<2 || !appIsAlpha(URL[0]) || URL[1]!=':' )
	{
		// Parse protocol.
		if
		(	(appStrchr(URL,':')!=NULL)
		&&	(appStrchr(URL,':')>URL+1)
		&&	(appStrchr(URL,'.')==NULL || appStrchr(URL,':')<appStrchr(URL,'.')) )
		{
			char* s  = URL;
			URL      = appStrchr(URL,':');
			*URL++   = 0;
			Protocol = s;
		}

		// Parse optional leading slashes.
		if( *URL=='/' )
		{
			URL++;
			if( *URL++ != '/' )
			{
				*this = FURL();
				Valid = 0;
				return;
			}
			FarHost = 1;
			Host = "";
		}

		// Parse optional host name and port.
		const char* Dot = appStrchr(URL,'.');
		if
		(	(Dot)
		&&	(Dot-URL>0)
		&&	(appStrnicmp( Dot+1,*DefaultMapExt,  DefaultMapExt .Length() )!=0 || appIsAlnum(Dot[DefaultMapExt .Length()+1]) )
		&&	(appStrnicmp( Dot+1,*DefaultSaveExt, DefaultSaveExt.Length() )!=0 || appIsAlnum(Dot[DefaultSaveExt.Length()+1]) ) )
		{
			char* s = URL;
			URL     = appStrchr(URL,'/');
			if( URL )
				*URL++ = 0;
			char* t = appStrchr(s,':');
			if( t )
			{
				// Port.
				*t++ = 0;
				Port = appAtoi( t );
			}
			Host = s;
			if( appStricmp(*Protocol,*DefaultProtocol)==0 )
				Map = DefaultMap;
			else
				Map = "";
			FarHost = 1;
		}
	}

	// Parse optional map, portal and options.
	UBOOL FarMap=0;
	if( URL )
	{
		// Map.
		if( appStrlen(URL)>0 )
		{
			FarMap = 1;

			// Portal.
			char* t = appStrchr(URL,'/');
			if( t )
			{
				// Trailing slash.
				*t++ = 0;
				char* u = appStrchr(t,'/');
				if( u )
				{
					*u++ = 0;
					if( *u != 0 )
					{
						*this = FURL();
						Valid = 0;
						return;
					}
				}

				// Portal name.
				Portal = t;
			}

			// Map.
			Map = URL;
		}
	}

	// Validate everything.
	if
	(	!ValidNetChar(*Protocol  )
	||	!ValidNetChar(*Host      )
	||	!ValidNetChar(*Map       )
	||	!ValidNetChar(*Portal    )
	||	(!FarHost && !FarMap && !Op.Num()) )
	{
		*this = FURL();
		Valid = 0;
		return;
	}

	// Success.
	unguard;
}

/*-----------------------------------------------------------------------------
	Conversion to text.
-----------------------------------------------------------------------------*/

//
// Convert this URL to text.
//
void FURL::String( FString& Result, UBOOL FullyQualified ) const
{
	guard(FURL::String);

	// Emit protocol.
	if( Protocol!=DefaultProtocol || FullyQualified )
	{
		Result.Appendf( "%s", Protocol );
		Result.Appendf( ":" );
		if( Host!=DefaultHost )
			Result.Appendf( "//" );
	}

	// Emit host.
	if( Host!=DefaultHost || Port!=DefaultPort )
	{
		Result.Appendf( "%s", Host );
		if( Port!=DefaultPort )
		{
			Result.Appendf( ":" );
			Result.Appendf( "%i", Port );
		}
		Result.Appendf( "/" );
	}

	// Emit map.
	if( Map.Length() )
		Result.Appendf( "%s", Map );

	// Emit options.
	for( INT i=0; i<Op.Num(); i++ )
	{
		Result.Appendf( "?"         );
		Result.Appendf( "%s", Op(i) );
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Informational.
-----------------------------------------------------------------------------*/

//
// Return whether this URL corrsponds to an internal object, i.e. an Unreal
// level which this app can try to connect to locally or on the net. If this
// is fals, the URL refers to an object that a remote application like Internet
// Explorer can execute.
//
UBOOL FURL::IsInternal() const
{
	guard(FURL::IsInternal);
	return Protocol==DefaultProtocol;
	unguard;
}

//
// Return whether this URL corresponds to an internal object on this local 
// process. In this case, no Internet use is necessary.
//
UBOOL FURL::IsLocalInternal() const
{
	guard(FURL::IsLocalInternal);
	return IsInternal() && Host.Length()==0;
	unguard;
}

//
// Add a unique option to the URL, replacing any existing one.
//
void FURL::AddOption( const char* Str )
{
	guard(FURL::AddOption);
	int Match = appStrchr(Str,'=') ? appStrchr(Str,'=')+1-Str : appStrlen(Str)+1;
	for( int i=0; i<Op.Num(); i++ )
		if( appStrnicmp( *Op(i), Str, Match )==0 )
			break;
	if( i==Op.Num() )	new( Op )FString( Str );
	else				Op( i ) = Str;
	unguard;
}

//
// Parse options from the profile.
//
void FURL::GetConfigOptions( const char* Section, const char* Filename )
{
	guard(FURL::GetConfigOptions);
	char Text[32767], *Ptr=Text;
	GetConfigSection( Section, Text, ARRAY_COUNT(Text) );
	while( *Ptr )
	{
		AddOption( Ptr );
		Ptr += appStrlen(Ptr)+1;
	}
	unguard;
}

//
// See if the URL contains an option string.
//
UBOOL FURL::HasOption( const char* Test ) const
{
	guard(FURL::HasOption);
	for( int i=0; i<Op.Num(); i++ )
		if( appStricmp( *Op(i), Test )==0 )
			return 1;
	return 0;
	unguard;
}

//
// Return an option if it exists.
//
const char* FURL::GetOption( const char* Match, const char* Default ) const
{
	guard(FURL::GetOption);
	for( int i=0; i<Op.Num(); i++ )
		if( appStrnicmp( *Op(i), Match, appStrlen(Match) )==0 )
			return *Op(i) + appStrlen(Match);
	return Default;
	unguard;
}

/*-----------------------------------------------------------------------------
	Comparing.
-----------------------------------------------------------------------------*/

//
// Compare two URL's to see if they refer to the same exact thing.
//
UBOOL FURL::operator==( const FURL& Other ) const
{
	guard(FURL::operator==);
	if
	(	Protocol	!= Other.Protocol
	||	Host		!= Other.Host
	||	Map			!= Other.Map
	||	Port		!= Other.Port
	||  Op.Num()    != Other.Op.Num() )
		return 0;

	for( int i=0; i<Op.Num(); i++ )
		if( Op(i) != Other.Op(i) )
			return 0;

	return 1;
	unguard;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
