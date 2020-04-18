/*=============================================================================
	UnLinker.h: Unreal object linker.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	FObjectExport.
-----------------------------------------------------------------------------*/

//
// Information about an exported object.
//
struct CORE_API FObjectExport
{
	// Persistent.
	INT         ClassIndex;
	INT         ParentIndex;
	INT			PackageIndex;
	FName		ObjectName;
	DWORD		ObjectFlags;
	INT         SerialSize;
	INT         SerialOffset;

	// Internal.
	FName		OldGroup;//oldver
	FName		ClassPackage;
	FName		ClassName;
	UObject*	_Object;

	// Functions.
	FObjectExport()
	: OldGroup( NAME_None )
	{}
	FObjectExport( UObject* InObject )
	:	ClassIndex		( 0										 )
	,	ParentIndex		( 0										 )
	,	PackageIndex	( 0                                      )
	,	ObjectName		( InObject->GetFName()					 )
	,	ObjectFlags		( InObject->GetFlags() & RF_Load		 )
	,	SerialSize		( 0										 )
	,	SerialOffset	( 0										 )
	,	_Object			( InObject								 )
	,	ClassPackage	( InObject->GetClass()->GetParent()->GetFName())
	,	ClassName		( InObject->GetClass()->GetFName()		 )
	,	OldGroup		( NAME_None )
	{}
	friend FArchive& operator<<( FArchive& Ar, FObjectExport& E )
	{
		guard(FObjectExport<<);

		Ar << AR_INDEX(E.ClassIndex);
		Ar << AR_INDEX(E.ParentIndex);
		if( Ar.Ver() >= 50 )
			Ar << E.PackageIndex;
		else //oldver
			E.PackageIndex = 0;
		Ar << E.ObjectName;
		Ar << E.ObjectFlags;
		Ar << AR_INDEX(E.SerialSize);
		if( E.SerialSize )
			Ar << AR_INDEX(E.SerialOffset);
		if( Ar.IsLoading() )
			E._Object = NULL;

		return Ar;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	FObjectImport.
-----------------------------------------------------------------------------*/

//
// Information about an imported object.
//
struct CORE_API FObjectImport
{
	// Persistent.
	FName			ClassPackage;
	FName			ClassName;
	FName			_ObjectPackage;//oldver
	INT				PackageIndex;
	FName			ObjectName;

	// Internal.
	UObject*		Object;
	ULinkerLoad*	SourceLinker;
	INT             SourceIndex;

	// Functions.
	FObjectImport()
	{}
	FObjectImport( UObject* InObject )
	:	ClassPackage	( InObject->GetClass()->GetParent()->GetFName())
	,	ClassName		( InObject->GetClass()->GetFName()		 )
	,	PackageIndex	( 0                                      )
	//oldver
	,	_ObjectPackage	( InObject->GetParent() ? InObject->GetParent()->GetFName() : NAME_None )
	,	ObjectName		( InObject->GetFName()					 )
	,	Object			( InObject								 )
	,	SourceLinker	( NULL									 )
	,	SourceIndex		( -1									 )
	{}
	friend FArchive& operator<<( FArchive& Ar, FObjectImport& I )
	{
		guard(FObjectImport<<);

		Ar << I.ClassPackage << I.ClassName;
		if( Ar.Ver()>=50 )
		{
			Ar << I.PackageIndex;
			I._ObjectPackage = NAME_DevGarbage;//oldver
		}
		else//oldver
		{
			Ar << I._ObjectPackage;
			I.PackageIndex = 0;
		}
		Ar << I.ObjectName;
		if( Ar.IsLoading() )
		{
			I.SourceIndex = -1;
			I.Object      = NULL;
		}
		return Ar;

		unguard;
	}
};

/*----------------------------------------------------------------------------
	File loaders and savers.
----------------------------------------------------------------------------*/

//
// Ansi file saver.
//
class FArchiveFileSave : public FArchive
{
public:
	char Filename[256];
	FArchiveFileSave( const char* InFilename )
	: File(NULL), Pos(0), BufferCount(0)
	{
		guard(FArchiveFileSave::FArchiveFileSave);
		appStrcpy( Filename, InFilename );
		File = appFopen( Filename, "w+b" );
		if( File == NULL )
			appThrowf( LocalizeError("OpenFailed") );
		unguard;
	}
	FArchiveFileSave()
	: File(NULL), Pos(0), BufferCount(0)
	{}
	~FArchiveFileSave()
	{
		guard(FArchiveFileSave::~FArchiveFileSave);
		Flush();
		if( File )
			appFclose( File );
		File = NULL;
		unguard;
	}
	virtual void Seek( INT InPos )
	{
		Flush();
		if( appFseek(File,InPos,USEEK_SET) != 0 )
			appThrowf( LocalizeError("SeekFailed") );
		Pos = InPos;
	}
	INT Tell()
	{
		return Pos;
	}
	virtual FArchive& Serialize( void* V, INT Length )
	{
		Pos += Length;
		INT Copy;
		while( Length > (Copy=ARRAY_COUNT(Buffer)-BufferCount) )
		{
			appMemcpy( Buffer+BufferCount, V, Copy );
			BufferCount += Copy;
			Length      -= Copy;
			V            = (BYTE*)V + Copy;
			Flush();
		}
		if( Length )
		{
			appMemcpy( Buffer+BufferCount, V, Length );
			BufferCount += Length;
		}
		return *this;
	}
//!!protected:
	void Flush()
	{
		if( BufferCount )
			if( appFwrite( Buffer, 1, BufferCount, File ) != BufferCount )
				throw( LocalizeError("WriteFailed"), Filename );
		BufferCount=0;
	}
	INT   BufferCount;
	BYTE  Buffer[4096];
	INT   Pos;
	FILE* File;
};

//
// File status.
//
enum {LoadBufferSize=4096};
struct FFileStatus
{
	INT SavedPos;
};

//
// Ansi file loader.
//
class FArchiveFileLoad : public FArchive
{
public:
	char Filename[256];
	INT Pos;
	FArchiveFileLoad( const char* InFilename )
	: File(NULL)
	, Pos(0)
	{
		guard(FArchiveFileLoad::FArchiveFileLoad);
		appStrcpy( Filename, InFilename );
		File = appFopen( Filename, "rb" );	
		if( File == NULL )
			appThrowf( LocalizeError("OpenFailed") );
		appFseek( File, 0, USEEK_END );
		Eof = appFtell( File );
		appFseek( File, 0, USEEK_SET );
		unguard;
	}
	FArchiveFileLoad()
	: File(NULL)
	{}
	~FArchiveFileLoad()
	{
		guard(FArchiveFileLoad::~FArchiveFileLoad);
		if( File )
			appFclose( File );
		File = NULL;
		unguard;
	}
	void Seek( INT InPos, INT InReadAhead=0 )
	{
		guard(FArchiveFileLoad::Seek);
		check(InPos>=0);
		check(InPos<=Eof);
		INT Result = appFseek(File,InPos,USEEK_SET);
		if( Result!=0 )
			appErrorf( "Seek Failed %i/%i (%i): %i %i", InPos, Eof, Pos, Result, appFerror(File) );
		unguard;
		Pos = InPos;
	}
	INT Tell()
	{
		return appFtell( File );
	}
	void Push( FFileStatus& St, BYTE* NewBuffer )
	{
		St.SavedPos = appFtell( File );
	}
	void Pop( FFileStatus& St )
	{
		guardSlow(FArchiveFileLoad::Pop);
		INT Result = appFseek( File, St.SavedPos, USEEK_SET );
		if( Result!=0 )
			appErrorf( "Seek Failed %i/%i (%i): %i %i", St.SavedPos, Eof, Pos, Result, appFerror(File) );
		Pos = St.SavedPos;
		unguardSlow;
	}
	FArchive& Serialize( void* V, INT Length )
	{
		INT Count = appFread( V, Length, 1, File );
		if( Count!=1 && Length!=0 )
			appErrorf( "appFread failed: Count=%i Length=%i Error=%i", Count, Length, appFerror(File) );
		Pos += Length;
		check(Pos<=Eof);
		return *this;
	}
//!!private:
	FILE* File;
	INT Eof;
};

/*----------------------------------------------------------------------------
	Items stored in Unrealfiles.
----------------------------------------------------------------------------*/

//
// Unrealfile summary, stored at top of file.
//
struct FPackageFileSummary
{
	// Variables.
	INT		Tag;
	INT		FileVersion;
	DWORD	PackageFlags;
	INT		NameCount,		NameOffset;
	INT		ExportCount,	ExportOffset;
	INT     ImportCount,	ImportOffset;
	INT		HeritageCount,	HeritageOffset;

	// Constructor.
	FPackageFileSummary()
	{
		appMemset( this, 0, sizeof(*this) );
	}

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FPackageFileSummary& Sum )
	{
		guard(FUnrealfileSummary<<);

		Ar << Sum.Tag;
		Ar << Sum.FileVersion;

		if( Sum.FileVersion >= 34 ) Ar << Sum.PackageFlags;//oldver
		else if( Ar.IsLoading() ) Sum.PackageFlags=0;

		Ar << Sum.NameCount     << Sum.NameOffset;
		Ar << Sum.ExportCount   << Sum.ExportOffset;
		Ar << Sum.ImportCount   << Sum.ImportOffset;
		Ar << Sum.HeritageCount << Sum.HeritageOffset;

		return Ar;
		unguard;
	}
};

/*----------------------------------------------------------------------------
	ULinker.
----------------------------------------------------------------------------*/

//
// A file linker.
//
class CORE_API ULinker : public UObject
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ULinker,UObject,CLASS_Transient)
	NO_DEFAULT_CONSTRUCTOR(ULinker)

	// Constants.
	enum {FNAME_SIZE=128};						// Maximum size of a filename.

	// Variables.
	UObject*				LinkerRoot;			// The linker's root object.
	FPackageFileSummary		Summary;			// File summary.
	TArray<FName>			NameMap;			// Maps file name indices to name table indices.
	TArray<FObjectImport>	ImportMap;			// Maps file object indices >=0 to external object names.
	TArray<FObjectExport>	ExportMap;			// Maps file object indices >=0 to external object names.
	TArray<FGuid>			Heritage;			// List of packages we're backwards compatible with.
	INT						Success;			// Whether the object was constructed successfully.
	DWORD					ContextFlags;		// Load flag mask.

	// Constructors.
	ULinker( UObject* InRoot, const char* InFilename )
	:	LinkerRoot( InRoot )
	,	Summary()
	,	Success( 123456 )
	,	ContextFlags( 0 )
	{
		check(LinkerRoot);
		check(InFilename);

		// Set context flags.
		if( (GIsEditor ) ) ContextFlags |= RF_LoadForEdit;
		if( (GIsClient ) ) ContextFlags |= RF_LoadForClient;
		if( (GIsServer ) ) ContextFlags |= RF_LoadForServer;
	}

	// UObject interface.
	void Serialize( FArchive& Ar )
	{
		guard(ULinker::Serialize);
		Super::Serialize( Ar );

		// Prevent garbage collecting of linker's names and package.
		Ar << NameMap << LinkerRoot;
		for( INT i=0; i<ExportMap.Num(); i++ )
		{
			FObjectExport& E = ExportMap(i);
			Ar << E.ObjectName << E.OldGroup << E.ClassPackage << E.ClassName;
		}
		for( i=0; i<ImportMap.Num(); i++ )
		{
			FObjectImport& I = ImportMap(i);
			Ar << *(UObject**)&I.SourceLinker;
			Ar << I.ClassPackage << I.ClassName << I._ObjectPackage;
		}

		unguard;
	}
};

/*----------------------------------------------------------------------------
	ULinkerLoad.
----------------------------------------------------------------------------*/

//
// A file loader.
//
class ULinkerLoad : public ULinker, public FArchiveFileLoad
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ULinkerLoad,ULinker,CLASS_Transient)
	NO_DEFAULT_CONSTRUCTOR(ULinkerLoad)

	// Friends.
	friend class UObject;
	friend class FPackageMap;

	// Variables.
	DWORD LoadFlags;
	INT FileSize;
	CHAR Status[256];

	// Constructor; all errors here throw exceptions which are fully recoverable.
	ULinkerLoad( UObject* InParent, const char* InFilename, DWORD InLoadFlags )
	:	ULinker( InParent, InFilename )
	,	FArchiveFileLoad( InFilename )
	,	LoadFlags( InLoadFlags )
	{
		guard(ULinkerLoad::ULinkerLoad);
		debugf( "Loading: %s", InParent->GetFullName() );

		// Error if linker already loaded.
		for( INT i=0; i<GObj.Loaders.Num(); i++ )
			if( GObj.GetLoader(i)->LinkerRoot == LinkerRoot )
				appThrowf( LocalizeError("LinkerExists"), LinkerRoot->GetClassName() );

		// Begin.
		GSystem->StatusUpdatef( 0, 0, LocalizeProgress("Loading"), Filename );
		FileSize = appFSize(Filename);

		// Set status info.
		guard(InitAr);
		ArVer       = PACKAGE_FILE_VERSION;
		ArIsLoading = 1;
		ArForEdit   = GIsEditor;
		ArForClient = 1;
		ArForServer = 1;
		unguard;

		// Read summary from file.
		guard(LoadSummary);
		*this << Summary;
		ArVer = Summary.FileVersion;
		if( Cast<UPackage>(LinkerRoot) )
			Cast<UPackage>(LinkerRoot)->PackageFlags = Summary.PackageFlags;
		unguard;
		//if( Summary.FileVersion < 61 )
		//	debugf("!!!!!!!!!!!!!!!!!!!!!%s %i",Filename,Summary.FileVersion);

		// Check tag.
		guard(CheckTag);
		if( Summary.Tag != PACKAGE_FILE_TAG )
		{
			GSystem->Warnf( LocalizeError("BinaryFormat"), Filename );
			throw( LocalizeError("Aborted") );
		}
		unguard;

		// Validate the summary.
		guard(CheckVersion);
		if( Summary.FileVersion < PACKAGE_MIN_VERSION )
			if( !GSystem->YesNof( LocalizeQuery("OldVersion"), Filename ) )
				throw( LocalizeError("Aborted") );
		unguard;

		// Allocate everything according to summary.
		ImportMap   .AddZeroed( Summary.ImportCount   );
		ExportMap   .AddZeroed( Summary.ExportCount   );
		NameMap		.AddZeroed( Summary.NameCount     );
		Heritage    .AddZeroed( Summary.HeritageCount );

		// Load heritage map.
		guard(LoadHeritage);
		if( Summary.HeritageCount > 0 )
		{
			Seek( Summary.HeritageOffset );
			for( INT i=0; i<Summary.HeritageCount; i++ )
				*this << Heritage( i );
		}
		unguard;

		// Load and map names.
		guard(LoadNames);
		if( Summary.NameCount > 0 )
		{
			//debugf( NAME_Log, "Reading name table: %i names", Summary.NameCount );
			Seek( Summary.NameOffset );
			for( INT i=0; i<Summary.NameCount; i++ )
			{
				// Read the name entry from the file.
				FNameEntry NameEntry;
				*this << NameEntry;

				// Add it to the name table if it's needed in this context.				
				NameMap(i) = (NameEntry.Flags & ContextFlags) ? FName( NameEntry.Name, FNAME_Add ) : NAME_None;
			}
		}
		unguard;

		// Load import map.
		guard(LoadImportMap);
		if( Summary.ImportCount > 0 )
		{
			//debugf( NAME_Log, "Reading import table: %i objects", Summary.ImportCount );
			Seek( Summary.ImportOffset );
			for( int i=0; i<Summary.ImportCount; i++ )
				*this << ImportMap( i );
		}
		unguard;

		// Load export map.
		guard(LoadExportMap);
		if( Summary.ExportCount > 0 )
		{
			//debugf( NAME_Log, "Reading export table: %i objects", Summary.ExportCount );
			Seek( Summary.ExportOffset );
			for( INT i=0; i<Summary.ExportCount; i++ )
				*this << ExportMap( i );
		}
		unguard;

		// Generate export in-memory info.
		guard(GenerateExportInfo);
		for( INT i=0; i<Summary.ExportCount; i++ )
		{
			FObjectExport& Export = ExportMap(i);
			if( Export.ClassIndex < 0 )
			{
				if( Ver() >= 50 )
				{
					FObjectImport& Import = ImportMap( -Export.ClassIndex-1 );
					check(Import.PackageIndex<0);
					Export.ClassPackage = ImportMap( -Import.PackageIndex-1 ).ObjectName;
				}
				else//oldver
				{
					Export.ClassPackage = ImportMap( -Export.ClassIndex-1 )._ObjectPackage;
				}
				Export.ClassName = ImportMap( -Export.ClassIndex-1 ).ObjectName;
			}
			else if( Export.ClassIndex > 0 )
			{
				Export.ClassPackage = LinkerRoot->GetFName();
				Export.ClassName    = ExportMap( Export.ClassIndex-1 ).ObjectName;
			}
			else
			{
				Export.ClassPackage = NAME_Core;
				Export.ClassName    = NAME_Class;
			}
		}
		unguard;

		// Add this linker to the object manager's linker array.
		GObj.Loaders.AddItem( this );
		try
		{
			// Validate all imports and map them to their remote linkers.
			guard(ValidateImports);
			for( INT i=0; i<Summary.ImportCount; i++ )
				VerifyImport( i );
			unguard;
		}
		catch( char* Error )
		{
			GObj.Loaders.RemoveItem( this );
			throw( Error );
		}

		// Success.
		Success = 1;

		unguard;
	}

	// Safely verify an import.
	void VerifyImport( INT i )
	{
		guard(ULinkerLoad::VerifyImport);
		FObjectImport& Import = ImportMap(i);
		if( Import.SourceIndex != -1 )
		{
			// Already verified.
			return;
		}
		if
		(	Import.ClassPackage  != NAME_None
		&&	Import.ClassName     != NAME_None
		&&	Import.ObjectName    != NAME_None
		&&	(Ver()>=50 || Import._ObjectPackage!=NAME_None) )
		{
			UObject* Pkg=NULL;
			if( Ver()>=50 )
			{
				if( Import.PackageIndex == 0 )
				{
					check(Import.ClassName==NAME_Package);
					check(Import.ClassPackage==NAME_Core);
					UPackage* TmpPkg = GObj.CreatePackage( NULL, *Import.ObjectName );
					Import.SourceLinker = GObj.GetPackageLinker( TmpPkg, NULL, LOAD_Throw | (LoadFlags & LOAD_Propagate), NULL, NULL );
				}
				else
				{
					check(Import.PackageIndex<0);
					VerifyImport( -Import.PackageIndex-1 );
					Import.SourceLinker = ImportMap(-Import.PackageIndex-1).SourceLinker;
					check(Import.SourceLinker);
					for
					(	FObjectImport* Top = &Import
					;	Top->PackageIndex<0
					;	Top = &ImportMap(-Top->PackageIndex-1) );
					Pkg = GObj.CreatePackage( NULL, *Top->ObjectName );
				}
			}
			else
			{
				//oldver
				Pkg = GObj.CreatePackage( NULL, *Import._ObjectPackage );
				Import.SourceLinker = GObj.GetPackageLinker( Pkg, NULL, LOAD_Throw | (LoadFlags & LOAD_Propagate), NULL, NULL );
			}

			//caveat: Fast enough, but could be much faster with hashing.
			UBOOL SafeReplace = 0;
			for( INT j=0; j<Import.SourceLinker->ExportMap.Num(); j++ )
			{
				FObjectExport& Source = Import.SourceLinker->ExportMap( j );
				if
				(	Source.ObjectName	== Import.ObjectName
				&&	Source.ClassName	== Import.ClassName
				&&	Source.ClassPackage	== Import.ClassPackage )
				{
					if( Ver()>=50 && Import.SourceLinker->Ver()>=50 && Import.PackageIndex<0 )
					{
						FObjectImport& ParentImport = ImportMap(-Import.PackageIndex-1);
						if( ParentImport.SourceLinker )
						{
							if( ParentImport.SourceIndex==-1 )
							{
								if( Source.PackageIndex!=0 )
								{
									continue;
								}
							}
							else if( ParentImport.SourceIndex+1 != Source.PackageIndex )
							{
								if( Source.PackageIndex!=0 )
								{
									continue;
								}
							}
						}
					}
					if( !(Source.ObjectFlags & RF_Public) )
						appThrowf( LocalizeError("FailedImportPrivate"), *Source.ClassName, Import.SourceLinker->LinkerRoot->GetClassName(), *Source.ObjectName );
					Import.SourceIndex = j;
					break;
				}
			}
			if
			(	Import.SourceIndex==-1
			//&&	Import.SourceLinker->Ver()<58
			&&	(	Import.ClassName==NAME_Texture
				||	Import.ClassName==NAME_Sound
				||	Import.ClassName==NAME_FireTexture
				||	Import.ClassName==NAME_IceTexture
				||	Import.ClassName==NAME_WaterTexture
				||	Import.ClassName==NAME_WaveTexture
				||	Import.ClassName==NAME_WetTexture ) )//oldver
			{
				// See if there's a match without the proper package.
				for( INT j=0; j<Import.SourceLinker->ExportMap.Num(); j++ )
				{
					FObjectExport& Source = Import.SourceLinker->ExportMap( j );
					if
					(	Source.ObjectName	== Import.ObjectName
					&&	Source.ClassName	== Import.ClassName
					&&	Source.ClassPackage	== Import.ClassPackage )
					{
						Import.SourceIndex = j;
						break;
					}
				}
			}
			if( Import.SourceIndex==-1 && (Ver()<50 || Pkg!=NULL) )
			{
				// If not found in file, see if it's a public transient class.
				UObject* ClassPackage = FindObject<UPackage>( NULL, *Import.ClassPackage );
				if( ClassPackage )
				{
					UClass* FindClass = FindObject<UClass>( ClassPackage, *Import.ClassName );
					if( FindClass )
					{
						UObject* FindObject = GObj.FindObject( FindClass, Pkg, *Import.ObjectName );
						if
						(	(FindObject)
						&&	(FindObject->GetFlags() & RF_Public)
						&&	(FindObject->GetFlags() & RF_Transient) )
						{
							Import.Object = FindObject;
						}
						else if( FindClass->ClassFlags & CLASS_SafeReplace )
						{
							SafeReplace = 1;
						}
					}
				}
				if( !Import.Object && !SafeReplace )
				{
					FString S = "";
					for( INT j=-i-1; j!=0; j=ImportMap(-j-1).PackageIndex )
					{
						if( j != -i-1 )
							S = FString(".") + S;
						S = FString(*ImportMap(-j-1).ObjectName) + S;
					}
					appThrowf( LocalizeError("FailedImport"), *Import.ClassName, *S, Import.SourceLinker->Filename );
				}
			}
		}
		unguard;
	}

	// Load all objects; all errors here are fatal.
	void LoadAllObjects()
	{
		guard(ULinkerLoad::LoadAllObjects);

		// Load all exports.
		guard(LoadExports);
		if( Summary.ExportCount > 0 )
		{
			//debugf( NAME_Log, "Loading all objects: %i objects", Summary.ExportCount );
			for( INT i=0; i<Summary.ExportCount; i++ )
				CreateExport( i );
		}
		unguard;

		// Success.
		unguardobj;
	}

	// Create a single object.
	UObject* Create( UClass* ObjectClass, FName ObjectName, DWORD LoadFlags, UBOOL Checked )
	{
		guard(ULinkerLoad::Create);
		//caveat: Fast enough, but could be much faster with hashing.
		for( INT i=0; i<ExportMap.Num(); i++ )
		{
			if
			(	ExportMap(i).ObjectName   == ObjectName
			&&	ExportMap(i).ClassPackage == ObjectClass->GetParent()->GetFName()
			&&	ExportMap(i).ClassName    == ObjectClass->GetFName() )
			{
				// Found it.
				if( !(LoadFlags & LOAD_Verify) )
					return CreateExport( i );
				else
					return (UObject*)-1;
			}
		}
		if( Checked )
			appThrowf( LocalizeError("FailedCreate"), ObjectClass->GetName(), *ObjectName );
		return NULL;
		unguard;
	}
	void Preload( UObject* Object )
	{
		guard(ULinkerLoad::Preload);
		check(Object);
		if( Object->GetLinker() )
		{
			// If this is a struct, preload its super.
			if(	Object->IsA(UStruct::StaticClass) )
				if( ((UStruct*)Object)->SuperField )
					Object->GetLinker()->Preload( ((UStruct*)Object)->SuperField );

			// Preload the object if necessary.
			if( Object->GetFlags() & RF_NeedLoad )
			{
				// Load the local object now.
				Object->GetLinker()->LoadObject( Object );
			}
			else if( Object->GetFlags() & RF_Preloading )
			{
				// Warning for internal development.
				//debugf( "Object preload reentrancy: %s", Object->GetFullName() );
			}
		}
		unguard;
	}

private:
	// Return the loaded object corresponding to an export index; any errors are fatal.
	UObject* CreateExport( INT Index )
	{
		guard(ULinkerLoad::CreateExport);

		// Map the object into our table.
		FObjectExport& Export = ExportMap( Index );
		if( Export._Object )
		{
			return Export._Object;
		}
		else if
		(	(Export.ObjectFlags & ContextFlags)
		&&	(Ver()>=57 || appStricmp(*Export.ClassName,"Camera")) )
		{
			check(Export._Object==NULL);
			check(Export.ObjectName!=NAME_None);

			// Get the object's class.
			UClass* LoadClass = (UClass*)IndexToObject( Export.ClassIndex );
			if( !LoadClass )
				LoadClass = UClass::StaticClass;
			check(LoadClass);
			check(LoadClass->GetClass()==UClass::StaticClass);
			Preload( LoadClass );

			// Get the object's package.
			UObject* ThisParent = Export.PackageIndex ? IndexToObject(Export.PackageIndex) : LinkerRoot;

			// If preloading the class caused this object to load, return it now.
			if( Export._Object )
				return Export._Object;

			// Create the export object.
			Export._Object = GObj.ConstructObject
			(
				LoadClass,
				ThisParent,
				Export.ObjectName,
				(Export.ObjectFlags & RF_Load) | RF_NeedLoad | RF_NeedPostLoad,
				NULL
			);
			debugfSlow( NAME_DevLoad, "Created %s", Export._Object->GetFullName() );
			Export._Object->SetLinker( this, Index );

			// If it's a struct or class, set its parent.
			if( Export._Object->IsA(UStruct::StaticClass) && Export.ParentIndex!=0 )
				((UStruct*)Export._Object)->SuperField = (UStruct*)IndexToObject( Export.ParentIndex );

			// If it's a class, set its vtable.
			if( Export._Object->IsA( UClass::StaticClass ) )
				((UClass*)Export._Object)->Bind();

			return Export._Object;
		}
		else return NULL;
		unguardf(( "(%s %i %08X)", *ExportMap(Index).ObjectName, Tell(), (DWORD)File ));
	}

	// Return the loaded object corresponding to an import index; any errors are fatal.
	UObject* CreateImport( INT Index )
	{
		guard(ULinkerLoad::CreateImport);

		FObjectImport& Import = ImportMap( Index );
		if( !Import.Object && (LoadFlags & LOAD_KeepImports) )
		{
			// If keeping existing imports, try to map to existing object.
			UObject* ClassPackage = FindObject<UPackage>( NULL, *Import.ClassPackage );
			if( ClassPackage )
			{
				UClass* FindClass = FindObject<UClass>( ClassPackage, *Import.ClassName );
				if( FindClass )
				{
					UObject* ObjectPackage;
					if( Ver()>=50 && Import.PackageIndex )
					{
						check(Import.PackageIndex<0);
						ObjectPackage = IndexToObject( Import.PackageIndex );
					}
					else if( Ver()>=50 )
					{
						ObjectPackage = NULL;
					}
					else
					{
						//oldver
						ObjectPackage = FindObject<UPackage>( NULL, *Import._ObjectPackage );
					}
					if( ObjectPackage )
					{
						UObject* Found = GObj.FindObject( FindClass, ObjectPackage, *Import.ObjectName );
						if( Found && Found->GetLinker() )
						{
							//debugf( "Imported existing %s", Found->GetFullName() );
							Import.Object = Found;
						}
					}
				}
			}
		}
		if( !Import.Object && Import.SourceIndex>=0 )
		{
			check(Import.SourceLinker);
			//debugf( "Imported new %s %s.%s", *Import.ClassName, *Import.ObjectPackage, *Import.ObjectName );
			Import.Object = Import.SourceLinker->CreateExport( Import.SourceIndex );
		}
		return Import.Object;
		unguard;
	}

	// Load an object; any errors here are fatal.
	void LoadObject( UObject* Object )
	{
		guard(ULinkerLoad::LoadObject);
		check(IsValid());
		check(Object->GetLinker()==this);
		check(Object->GetFlags() & RF_NeedLoad);

		// Update flags.
		Object->ClearFlags( RF_NeedLoad );
		Object->SetFlags( RF_Preloading );

		// Find export definition.
		FObjectExport& Export = ExportMap( Object->GetLinkerIndex() );
		check(Export._Object==Object);

		// Go to the object's position.
		FFileStatus SavedStatus;
		BYTE NewBuffer[LoadBufferSize];
		Push( SavedStatus, NewBuffer );
		Seek( Export.SerialOffset, Export.SerialSize );

		// Load the object.
		Object->Serialize( *this );
		//debugf(NAME_Log,"    %s: %i", Object->GetFullName(), Export.Size );

		// Make sure we serialized the right amount of stuff.
		//if( Tell()-Export.SerialOffset != Export.SerialSize )
		//	appErrorf( LocalizeError("SerialSize"), Object->GetFullName(), Tell()-Export.SerialOffset, Export.SerialSize );

		// Restore position.
		Pop( SavedStatus );
		Object->ClearFlags( RF_Preloading );
		unguardf(( "(%s %i==%i/%i (%i %i) %08X)", Object->GetFullName(), Tell(), Pos, Eof, ExportMap( Object->GetLinkerIndex() ).SerialOffset, ExportMap( Object->GetLinkerIndex() ).SerialSize, (DWORD)File ));
	}

	// Map an import/export index to an object; all errors here are fatal.
	UObject* IndexToObject( INT Index )
	{
		guard(IndexToObject);
		if( Index > 0 )
		{
			if( !ExportMap.IsValidIndex( Index-1 ) )
				appErrorf( LocalizeError("ExportIndex"), Index-1, ExportMap.Num() );			
			return CreateExport( Index-1 );
		}
		else if( Index < 0 )
		{
			if( !ImportMap.IsValidIndex( -Index-1 ) )
				appErrorf( LocalizeError("ImportIndex"), -Index-1, ImportMap.Num() );
			return CreateImport( -Index-1 );
		}
		else return NULL;
		unguard;
	}

	// UObject interface.
	void Destroy()
	{
		guard(ULinkerLoad::Destroy);
		debugf( "Unloading: %s", LinkerRoot->GetFullName() );

		// Detach all objects linked with this linker.
		for( INT i=0; i<ExportMap.Num(); i++ )
		{
			FObjectExport& E = ExportMap(i);
			if( E._Object )
			{
				if( !E._Object->IsValid() )
					appErrorf( "Linker object %s %s.%s is invalid", *E.ClassName, LinkerRoot->GetName(), *E.ObjectName );
				if( E._Object->GetLinker()!=this )
					appErrorf( "Linker object %s %s.%s mislinked", *E.ClassName, LinkerRoot->GetName(), *E.ObjectName );
				if( E._Object->GetLinkerIndex()!=i )
					appErrorf( "Linker object %s %s.%s misindexed", *E.ClassName, LinkerRoot->GetName(), *E.ObjectName );
				ExportMap(i)._Object->SetLinker( NULL, INDEX_NONE );
			}
		}

		// Remove from object manager, if it has been added.
		GObj.Loaders.RemoveItem( this );

		ULinker::Destroy();
		unguardobj;
	}

	// FArchive interface.
	FArchive& operator<<( UObject*& Object )
	{
		guard(ULinkerLoad<<UObject);

		INT Index;
		*this << AR_INDEX(Index);
		Object = IndexToObject( Index );

		return *this;
		unguardf(( "(%s %i %08X))", GetFullName(), Tell(), (DWORD)File ));
	}
	FArchive& operator<<( FName& Name )
	{
		guard(ULinkerLoad<<FName);

		NAME_INDEX NameIndex;
		*this << AR_INDEX(NameIndex);

		if( !NameMap.IsValidIndex(NameIndex) )
			appErrorf( "Bad name index %i/%i", NameIndex, NameMap.Num() );	
		Name = NameMap( NameIndex );

		return *this;
		unguardf(( "(%s %i %08X))", GetFullName(), Tell(), (DWORD)File ));
	}
};

/*----------------------------------------------------------------------------
	ULinkerSave.
----------------------------------------------------------------------------*/

//
// A file saver.
//
class ULinkerSave : public ULinker, public FArchiveFileSave
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ULinkerSave,ULinker,CLASS_Transient);
	NO_DEFAULT_CONSTRUCTOR(ULinkerSave);

	// Variables.
	TArray<INT> ObjectIndices;
	TArray<INT> NameIndices;

	// Constructor.
	ULinkerSave( UObject* InParent, const char* InFilename )
	:	ULinker( InParent, InFilename )
	,	FArchiveFileSave( InFilename )
	{
		// Set main summary info.
		Summary.Tag           = PACKAGE_FILE_TAG;
		Summary.FileVersion	  = PACKAGE_FILE_VERSION;
		Summary.PackageFlags  = Cast<UPackage>(LinkerRoot) ? Cast<UPackage>(LinkerRoot)->PackageFlags : 0;

		// Set status info.
		ArIsSaving  = 1;
		ArForEdit   = GIsEditor;
		ArForClient = 1;
		ArForServer = 1;

		// Allocate indices.
		ObjectIndices.AddZeroed( GObj.Objects.Num() );
		NameIndices  .AddZeroed( FName::GetMaxNames() );

		// Success.
		Success=1;
	}

	// FArchive interface.
	INT MapName( FName* Name )
	{
		guardSlow(ULinkerSave::MapName);
		return NameIndices(Name->GetIndex());
		unguardobjSlow;
	}
	INT MapObject( UObject* Object )
	{
		guardSlow(ULinkerSave::MapObject);
		return Object ? ObjectIndices(Object->GetIndex()) : 0;
		unguardobjSlow;
	}
	FArchive& operator<<( FName& Name )
	{
		guardSlow(ULinkerSave<<FName);
		INT Save = NameIndices(Name.GetIndex());
		return *this << AR_INDEX(Save);
		unguardobjSlow;
	}
	FArchive& operator<<( UObject*& Obj )
	{
		guardSlow(ULinkerSave<<UObject);
		INT Save = Obj ? ObjectIndices(Obj->GetIndex()) : 0;
		return *this << AR_INDEX(Save);
		unguardobjSlow;
	}
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
