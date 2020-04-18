/*=============================================================================
	UnCorObj.h: Standard core object definitions.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

/*-----------------------------------------------------------------------------
	UPackage.
-----------------------------------------------------------------------------*/

//
// A package.
//
class CORE_API UPackage : public UObject
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UPackage,UObject,0)

	// Variables.
	void* DllHandle;
	DWORD PackageFlags;

	// Constructors.
	UPackage();

	// UObject interface.
	void Destroy();
	void Serialize( FArchive& Ar );

	// UPackage interface.
	void Bind();
	void* GetDllExport( const char* ExportName, UBOOL Checked );
};

/*-----------------------------------------------------------------------------
	USubsystem.
-----------------------------------------------------------------------------*/

//
// A subsystem.
//
class CORE_API USubsystem : public UObject, public FExec
{
	DECLARE_ABSTRACT_CLASS(USubsystem,UObject,CLASS_Transient)
	NO_DEFAULT_CONSTRUCTOR(USubsystem)
};

/*-----------------------------------------------------------------------------
	ULanguage.
-----------------------------------------------------------------------------*/

//
// A language (special case placeholder class).
//
class CORE_API ULanguage : public UObject
{
	DECLARE_ABSTRACT_CLASS(ULanguage,UObject,CLASS_Transient)
	NO_DEFAULT_CONSTRUCTOR(ULanguage)
};

/*-----------------------------------------------------------------------------
	UTextBuffer.
-----------------------------------------------------------------------------*/

//
// An object that holds a bunch of text.  The text is contiguous and, if
// of nonzero length, is terminated by a NULL at the very last position.
//
class CORE_API UTextBuffer : public UObject, public FOutputDevice
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UTextBuffer,UObject,0)

	// Variables.
	INT Pos, Top;
	FString Text;

	// Constructors.
	UTextBuffer() {}

	// UObject interface.
	void Export( FOutputDevice& Out, const char* FileType, int Indent );
	void Serialize( FArchive& Ar );

	// FOutputDevice interface.
	void WriteBinary( const void* Data, int Length, EName MsgType=NAME_None );
};

/*----------------------------------------------------------------------------
	UFactory.
----------------------------------------------------------------------------*/

//
// An object responsible for creating and importing new objects.
//
class CORE_API UFactory : public UObject
{
	DECLARE_ABSTRACT_CLASS(UFactory,UObject,0)

	// Variables.
	UClass*         SupportedClass;
	TArray<FString> Formats;
	UBOOL            bCreateNew;

	// Constructor.
	UFactory();

	// UFactory interface.
	virtual UObject* Create( UClass* Class, UObject* InParent, FName Name, UObject* Context, const char* Type, const char*& Buffer, const char* BufferEnd, FFeedbackContext* Warn=NULL )=0;
};

/*----------------------------------------------------------------------------
	UExporter.
----------------------------------------------------------------------------*/

//
// An object responsible for exporting other objects to files.
//
class CORE_API UExporter : public UObject
{
	DECLARE_ABSTRACT_CLASS(UExporter,UObject,0)

	// Variables.
	UClass*         SupportedClass;
	TArray<FString> Formats;
	FString			FormatError;

	// Constructor.
	UExporter();

	// UExporter interface.
	virtual UBOOL Export( UObject* Object, const char* Type, FArchive& Ar, FFeedbackContext* Warn=NULL )=0;
};

/*----------------------------------------------------------------------------
	USystem.
----------------------------------------------------------------------------*/

class CORE_API USystem : public USubsystem
{
	DECLARE_CLASS(USystem,USubsystem,CLASS_Config)

	// Variables.
	char Paths[96][16];
	char SavePath[96];
	char CachePath[96];
	char CacheExt[32];
	FName Suppress[16];
	INT PurgeCacheDays;

	// Constructors.
	static void InternalClassInitializer( UClass* Class );

	// FOutputDevice interface.
	UBOOL Exec( const char* Cmd, FOutputDevice* Out );
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
