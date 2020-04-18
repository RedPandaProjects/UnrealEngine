/*=============================================================================
	UnObjBas.h: Unreal object base class.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Core enumerations.
-----------------------------------------------------------------------------*/

//
// Flags for loading objects.
//
enum ELoadFlags
{
	LOAD_None			= 0x0000,	// No flags.
	LOAD_NoFail			= 0x0001,	// Critical error if load fails.
	LOAD_NoWarn			= 0x0002,	// Don't display warning if load fails.
	LOAD_KeepImports	= 0x0004,	// Keep existing objects in other packages rather than reloading them.
	LOAD_Throw			= 0x0008,	// Throw exceptions upon failure.
	LOAD_Verify			= 0x0010,	// Only verify existance; don't actually load.
	LOAD_AllowDll		= 0x0020,	// Allow plain DLLs.
	LOAD_DisallowFiles  = 0x0040,	// Don't load from file.
	LOAD_Propagate      = LOAD_KeepImports,
};

//
// Package flags.
//
enum EPackageFlags
{
	PKG_AllowDownload	= 0x0001,	// Allow downloading package.
	PKG_Optional        = 0x0002,	// Purely optional for clients.
	PKG_Need			= 0x8000,	// Client needs to download this package.
};

//
// Internal enums.
//
enum EIntrinsicConstructor {EC_IntrinsicConstructor};
enum EInternal             {EC_Internal};
enum ECppProperty          {EC_CppProperty};

//
// Special canonical package for FindObject, ParseObject.
//
#define ANY_PACKAGE ((UPackage*)-1)

//
// Result of GotoState.
//
enum EGotoState
{
	GOTOSTATE_NotFound		= 0,
	GOTOSTATE_Success		= 1,
	GOTOSTATE_Preempted		= 2,
};

/*-----------------------------------------------------------------------------
	Core flags.
-----------------------------------------------------------------------------*/

//
// Flags describing a class.
//
enum EClassFlags
{
	// Base flags.
	CLASS_Abstract          = 0x00001,  // Class is abstract and can't be instantiated directly.
	CLASS_Compiled			= 0x00002,  // Script has been compiled successfully.
	CLASS_Config			= 0x00004,  // Load object configuration at construction time.
	CLASS_Transient			= 0x00008,	// This object type can't be saved; null it out at save time.
	CLASS_Parsed            = 0x00010,	// Successfully parsed.
	CLASS_Localized         = 0x00020,  // Class contains localized text.
	CLASS_SafeReplace       = 0x00040,  // Objects of this class can be safely replaced with default or NULL.

	// Flags to inherit from parent class.
	CLASS_Inherit           = CLASS_Transient | CLASS_Config | CLASS_Localized | CLASS_SafeReplace,
};

//
// Flags associated with each property in a class, overriding the
// property's default behavior.
//
enum EPropertyFlags
{
	// Regular flags.
	CPF_Edit		 = 0x00000001, // Property is user-settable in the editor.
	CPF_Const		 = 0x00000002, // Actor's property always matches class's default actor property.
	CPF_Input		 = 0x00000004, // Variable is writable by the input system.
	CPF_ExportObject = 0x00000008, // Object can be exported with actor.
	CPF_OptionalParm = 0x00000010, // Optional parameter (if CPF_Param is set).
	CPF_Net			 = 0x00000020, // Property is relevant to network replication.
	CPF_NetReliable  = 0x00000040, // Property must be sent reliably in network replication.
	CPF_Parm		 = 0x00000080, // Function/When call parameter.
	CPF_OutParm		 = 0x00000100, // Value is copied out after function call.
	CPF_SkipParm	 = 0x00000200, // Property is a short-circuitable evaluation function parm.
	CPF_ReturnParm	 = 0x00000400, // Return value.
	CPF_CoerceParm	 = 0x00000800, // Coerce args into this function parameter.
	CPF_Intrinsic    = 0x00001000, // Property is intrinsic: C++ code is responsible for serializing it.
	CPF_Transient    = 0x00002000, // Property is transient: shouldn't be saved, zero-filled at load time.
	CPF_Config       = 0x00004000, // Property should be loaded/saved as permanent profile.
	CPF_Localized    = 0x00008000, // Property should be loaded as localizable text.
	CPF_Travel       = 0x00010000, // Property travels across levels/servers.
	CPF_EditConst    = 0x00020000, // Property is uneditable in the editor.
	CPF_GlobalConfig = 0x00040000, // Load config from base class, not subclass.
	CPF_NetAlways    = 0x00080000, // Always replicated.

	// Temporary flags.
	CPF_Replicated   = 0x80000000, // Replication condition is true.

	// Combinations of flags.
	CPF_ParmFlags           = CPF_OptionalParm | CPF_Parm | CPF_OutParm | CPF_SkipParm | CPF_ReturnParm | CPF_CoerceParm,
	CPF_PropagateFromStruct = CPF_Const | CPF_Intrinsic | CPF_Transient,
	CPF_NetFlags            = CPF_Net | CPF_NetReliable | CPF_NetAlways,
};

//
// Flags describing an object instance.
//
enum EObjectFlags
{
	RF_Transactional    = 0x00000001,   // Object is transactional.
	RF_Unreachable		= 0x00000002,	// Object is not reachable on the object graph.
	RF_Public			= 0x00000004,	// Object is visible outside its package.
	RF_TagImp			= 0x00000008,	// Temporary import tag in load/save.
	RF_TagExp			= 0x00000010,	// Temporary export tag in load/save.
	RF_SourceModified   = 0x00000020,   // Modified relative to source files.
	RF_TagGarbage		= 0x00000040,	// Check during garbage collection.
	RF_TransData		= 0x00000080,	// Used by transaction tracking system to track changed data.
	RF_Trans			= 0x00000100,	// Used by transaction tracking system to track changed objects.
	RF_NeedLoad			= 0x00000200,   // During load, indicates object needs loading.
	RF_HighlightedName  = 0x00000400,	// A hardcoded name which should be syntax-highlighted.
	RF_InSingularFunc   = 0x00000800,	// In a singular function.
	RF_Suppress         = 0x00001000,	// Suppressed log name.
	RF_StateChanged     = 0x00001000,   // Object did a state change.
	RF_InEndState       = 0x00002000,   // Within an EndState call.
	RF_Transient        = 0x00004000,	// Don't save object.
	RF_Preloading       = 0x00008000,   // Data is being preloaded from file.
	RF_LoadForClient	= 0x00010000,	// In-file load for client.
	RF_LoadForServer	= 0x00020000,	// In-file load for client.
	RF_LoadForEdit		= 0x00040000,	// In-file load for client.
	RF_Standalone       = 0x00080000,   // Keep object around for editing even if unreferenced.
	RF_NotForClient		= 0x00100000,	// Don't load this object for the game client.
	RF_NotForServer		= 0x00200000,	// Don't load this object for the game server.
	RF_NotForEdit		= 0x00400000,	// Don't load this object for the editor.
	RF_Destroyed        = 0x00800000,	// Object Destroy has already been called.
	RF_NeedPostLoad		= 0x01000000,   // Object needs to be postloaded.
	RF_HasStack         = 0x02000000,	// Has execution stack.
	RF_Intrinsic        = 0x04000000,   // Intrinsic (UClass only).
	RF_Marked			= 0x08000000,   // Marked (for debugging).
	RF_ErrorShutdown    = 0x10000000,	// ShutdownAfterError called.
	RF_DebugPostLoad    = 0x20000000,   // For debugging Serialize calls.
	RF_DebugSerialize   = 0x40000000,   // For debugging Serialize calls.
	RF_DebugDestroy     = 0x80000000,   // For debugging Destroy calls.
	RF_ContextFlags		= RF_NotForClient | RF_NotForServer | RF_NotForEdit, // All context flags.
	RF_LoadContextFlags	= RF_LoadForClient | RF_LoadForServer | RF_LoadForEdit, // Flags affecting loading.
	RF_Load  			= RF_ContextFlags | RF_LoadContextFlags | RF_Public | RF_Standalone | RF_Intrinsic | RF_SourceModified | RF_Transactional | RF_HasStack, // Flags to load from Unrealfiles.
	RF_Keep             = RF_Intrinsic | RF_Marked, // Flags to persist across loads.
};

/*----------------------------------------------------------------------------
	Core types.
----------------------------------------------------------------------------*/

//
// Class for handling undo/redo among objects.
//
class CORE_API FTransactionTracker
{
public:
	virtual void NoteObject( UObject* Res )=0;
	virtual void NoteSingleChange( UObject* Res, INT Index )=0;
	virtual void Reset( const char* Action )=0;
	virtual void Begin( class ULevel* Level, const char* SessionName )=0;
	virtual void End()=0;
};

//
// Globally unique identifier.
//
class CORE_API FGuid
{
public:
	DWORD A,B,C,D;
	FGuid()
	{}
	FGuid( DWORD InA, DWORD InB, DWORD InC, DWORD InD )
	: A(InA), B(InB), C(InC), D(InD)
	{}
	friend UBOOL operator==(const FGuid& X, const FGuid& Y)
		{return X.A==Y.A && X.B==Y.B && X.C==Y.C && X.D==Y.D;}
	friend UBOOL operator!=(const FGuid& X, const FGuid& Y)
		{return X.A!=Y.A || X.B!=Y.B || X.C!=Y.C || X.D!=Y.D;}
	friend FArchive& operator<<( FArchive& Ar, FGuid& G )
	{
		guard(FGuid<<);
		return Ar << G.A << G.B << G.C << G.D;
		unguard;
	}
	char* String( char* String64 ) const
	{
		appSprintf( String64, "%08X%08X%08X%08X", A, B, C, D );
		return String64;
	}
};

//
// COM IUnknown interface.
//
class CORE_API FUnknown
{
public:
	virtual DWORD STDCALL QueryInterface( const FGuid& RefIID, void** InterfacePtr ) {return 0;}
	virtual DWORD STDCALL AddRef() {return 0;}
	virtual DWORD STDCALL Release() {return 0;}
};

//
// Class for serializing objects in a compactly, mapping small values
// to fewer bytes.
//
class CORE_API FCompactIndex
{
public:
	INT Value;
	CORE_API friend FArchive& operator<<( FArchive& Ar, FCompactIndex& I );
};

//
// Ordered information of linker file requirements.
//
class CORE_API FPackageInfo
{
public:
	// Variables.
	char			URL[256];		// URL of the package file we need to request.
	ULinkerLoad*	Linker;			// Pointer to the linker, if loaded.
	UObject*		Parent;			// The parent package.
	FGuid			Guid;			// Package identifier.
	INT				FileSize;		// File size.
	INT				ObjectBase;		// Net index of first object.
	INT				NameBase;		// Net index of first name.
	DWORD			PackageFlags;	// Package flags.
	inline bool operator==(const FPackageInfo&right)const { return appMemcmp(this, &right, sizeof(FPackageInfo))==0; }
	// Functions.
	FPackageInfo( ULinkerLoad* InLinker=NULL );
	CORE_API friend FArchive& operator<<( FArchive& Ar, FPackageInfo& I );
};

//
// Maps objects and names to and from indices for network communication.
//
class CORE_API FPackageMap : public TArray<FPackageInfo>
{
public:
	// Functions.
	INT NameToIndex( FName Name );
	INT ObjectToIndex( UObject* Object );
	FName IndexToName( INT Index );
	UObject* IndexToObject( INT Index, UBOOL Load );
	void Init( UObject* Base );
	void AddLinker( ULinkerLoad* Linker );
	void Compute();

private:
	TArray<INT> NameIndices;
};

//
// Information about a driver class.
//
class CORE_API FRegistryObjectInfo
{
public:
	char Object[64];
	char Class[64];
	char MetaClass[64];
	char Autodetect[32];
	FRegistryObjectInfo()
	{*Object=*MetaClass=*Autodetect=0;}
};

//
// Information about a preferences menu item.
//
class CORE_API FPreferencesInfo
{
public:
	char Caption[64];
	char ParentCaption[64];
	char Class[64];
	FName Category;
	UBOOL Immediate;
	FPreferencesInfo()
	{*Caption=*ParentCaption=*Class=0; Immediate=0; Category=NAME_None;}
};

/*----------------------------------------------------------------------------
	Core macros.
----------------------------------------------------------------------------*/

// Macro to serialize an integer as a compact index.
#define AR_INDEX(intref) \
	(*(FCompactIndex*)&(intref))

// Define private default constructor.
#define NO_DEFAULT_CONSTRUCTOR(cls) \
	protected: cls() {} public:

// Guard macros.
#define unguardobjSlow		unguardfSlow(( "(%s)", GetFullName() ))
#define unguardobj			unguardf(( "(%s)", GetFullName() ))

// Verify the a class definition and C++ definition match up.
#define VERIFY_CLASS_OFFSET(Pre,ClassName,Member) \
	{for( TFieldIterator<UProperty> It( FindObjectChecked<UClass>( Pre##ClassName::StaticClass->GetParent(), #ClassName ) ); It; ++It ) \
		if( appStricmp(It->GetName(),#Member)==0 ) \
			if( It->Offset != STRUCT_OFFSET(Pre##ClassName,Member) ) \
				appErrorf("Class %s Member %s problem: Script=%i C++=%i", #ClassName, #Member, It->Offset, STRUCT_OFFSET(Pre##ClassName,Member) );}

// Foward declaration macro.
#define PREDECLARE_CLASS(objclass) \
	class objclass; \
	FArchive& operator<<( FArchive&, objclass*& ); \

// Declare the base UObject class.
#define DECLARE_BASE_CLASS( TClass, TSuperClass, TStaticFlags ) \
public: \
	/* Identification */ \
	enum {StaticClassFlags=TStaticFlags}; \
	static UClass* StaticClass; \
	typedef TSuperClass Super;\
	typedef TClass ThisClass;\
	/* Create a new object of this class */ \
	void* operator new( size_t Size, UObject* Parent=(UObject*)GObj.GetTransientPackage(), FName Name=NAME_None, DWORD SetFlags=0 ) \
		{ return GObj.AllocateObject( StaticClass, Parent, Name, SetFlags ); } \

// Declare a concrete class.
#define DECLARE_CLASS_WITHOUT_CONSTRUCT( TClass, TSuperClass, TStaticFlags ) \
	DECLARE_BASE_CLASS( TClass, TSuperClass, TStaticFlags ) \
	/* Typesafe pointer archiver */ \
	friend FArchive &operator<<( FArchive& Ar, TClass*& Res ) \
		{ return Ar << *(UObject**)&Res; } \
	/* Empty virtual destructor */ \
	virtual ~TClass() \
		{ ConditionalDestroy(); } \
	void* operator new( size_t Size, EInternal* Mem ) \
		{ return (void*)Mem; } \
	static void InternalConstructor( void* X ) \
		{ new( (EInternal*)X )TClass(); } \

#define DECLARE_CLASS( TClass, TSuperClass, TStaticFlags ) \
	DECLARE_BASE_CLASS( TClass, TSuperClass, TStaticFlags ) \
	/* Typesafe pointer archiver */ \
	TClass(){}\
	friend FArchive &operator<<( FArchive& Ar, TClass*& Res ) \
		{ return Ar << *(UObject**)&Res; } \
	/* Empty virtual destructor */ \
	virtual ~TClass() \
		{ ConditionalDestroy(); } \
	void* operator new( size_t Size, EInternal* Mem ) \
		{ return (void*)Mem; } \
	static void InternalConstructor( void* X ) \
		{ new( (EInternal*)X )TClass(); } \

// Declare an abstract class.
#define DECLARE_ABSTRACT_CLASS( TClass, TSuperClass, TStaticFlags ) \
	DECLARE_BASE_CLASS( TClass, TSuperClass, TStaticFlags | CLASS_Abstract ) \
	/* Typesafe pointer archiver */ \
	friend FArchive &operator<<( FArchive& Ar, TClass*& Res ) \
		{ return Ar << *(UObject**)&Res; } \
	/* Empty virtual destructor */ \
	virtual ~TClass() \
		{ ConditionalDestroy(); } \
	enum {InternalConstructor=0}; \

// Register a class at startup time.
#define IMPLEMENT_CLASS(ClassName) \
	/* Register this class globally */ \
	extern "C" DLL_EXPORT UClass autoclass##ClassName \
	( \
		sizeof(ClassName), \
		ClassName::StaticRecordSize, \
		ClassName::StaticClassFlags, \
		ClassName::Super::StaticClass, \
		FGuid(ClassName::GUID1,ClassName::GUID2,ClassName::GUID3,ClassName::GUID4), \
		#ClassName, \
		FName(GPackage), \
		(void(*)(void*))ClassName::InternalConstructor, \
		(void(*)(UClass*))ClassName::InternalClassInitializer \
	); \
	/* Static variable. */ \
	UClass* ClassName::StaticClass = &autoclass##ClassName;

// Define the package of the current DLL being compiled.
#define IMPLEMENT_PACKAGE(pkg) \
	extern "C" DLL_EXPORT char GPackage[] = #pkg; \
	IMPLEMENT_PACKAGE_PLATFORM(pkg)

/*----------------------------------------------------------------------------
	FObjectManager.
----------------------------------------------------------------------------*/

//
// The global object manager.  This tracks all information about all
// active objects, names, types, and files.
//
class CORE_API FObjectManager : public FUnknown
{
public:
	// Friends.
	friend class UObject;
	friend class FObjectIterator;
	friend class FArchiveTagUsed;
	friend class ULinker;
	friend class ULinkerLoad;
	friend class ULinkerSave;
	friend class FPackageMap;

	// FObjectManager interface.
	virtual void Init();
	virtual void Exit();
	virtual UBOOL Exec( const char* Cmd, FOutputDevice* Out=GSystem );
	virtual void Tick();
	virtual void PreRegister( UObject* InObject, FName InPackage );
	virtual void AddToRoot( UObject* Res );
	virtual void RemoveFromRoot( UObject* Res );
	virtual UObject* LoadPackage( UObject* InParent, const char* Filename, DWORD LoadFlags );
	virtual UBOOL SavePackage( UObject* InParent, UObject* Base, DWORD TopLevelFlags, const char* Filename, UBOOL NoWarn=0 );
	virtual void CollectGarbage( FOutputDevice* Out, DWORD KeepFlags );
	virtual UBOOL IsReferenced( UObject*& Res, DWORD KeepFlags, UBOOL IgnoreReference );
	virtual UBOOL AttemptDelete( UObject*& Res, DWORD KeepFlags, UBOOL IgnoreReference );
	virtual UObject* FindObject( UClass* Class, UObject* Parent, const char* Name, UBOOL ExactClass=0 );
	virtual UObject* FindObjectChecked( UClass* Class, UObject* Parent, const char* Name, UBOOL ExactClass=0 );
	virtual UObject* LoadObject( UClass* Class, UObject* InParent, const char* Name, const char* Filename, DWORD LoadFlags, FPackageMap* Sandbox );
	virtual UClass* LoadClass( UClass* BaseClass, UObject* InParent, const char* Name, const char* Filename, DWORD LoadFlags, FPackageMap* Sandbox );
	virtual void BeginLoad();
	virtual void EndLoad();
	virtual void InitProperties( UClass* Class, BYTE* Data, INT DataCount, UClass* DefaultsClass, BYTE* Defaults, INT DefaultsCount );
	virtual void ResetLoaders( UObject* InParent );
	virtual UObject* ConstructObject( UClass* Class, UObject* Parent=(UObject*)GObj.GetTransientPackage(), FName Name=NAME_None, DWORD SetFlags=0, UObject* Template=NULL );
	virtual UObject* AllocateObject( UClass* Class, UObject* Parent=(UObject*)GObj.GetTransientPackage(), FName Name=NAME_None, DWORD SetFlags=0, UObject* Template=NULL );
	virtual UPackage* CreatePackage( UObject* InParent, const char* PkgName );
	virtual ULinkerLoad* GetPackageLinker( UObject* InParent, const char* Filename, DWORD LoadFlags, FPackageMap* Sandbox, FGuid* CompatibleGuid );
	virtual UObject* ImportObjectFromFile( UClass* Class, UObject* InParent, FName Name, const char* Filename, FFeedbackContext* Warn=GSystem );
	virtual void ShutdownAfterError();
	virtual UObject* GetIndexedObject( INT Index );
	virtual void GlobalSetProperty( const char* Value, UClass* Class, UProperty* Property, INT Offset, UBOOL Immediate );
	virtual void ExportProperties( UClass* ObjectClass, BYTE* Object, FOutputDevice* Out, INT Indent, UClass* DiffClass, BYTE* Diff );
	virtual void ResetConfig( UClass* Class, const char* SrcFilename=NULL, const char* DestFilename=NULL );
	virtual void GetRegistryObjects( TArray<FRegistryObjectInfo>& Results, UClass* Class, UClass* MetaClass, UBOOL ForceRefresh );
	virtual void GetPreferences( TArray<FPreferencesInfo>& Results, const char* Category, UBOOL ForceRefresh );

	// Accessors.
	virtual UBOOL GetInitialized() {return Initialized;}
	virtual UPackage* GetTransientPackage() {return TransientPackage;}
	FName GetTempState() {return TempState;}//oldver
	FName GetTempGroup() {return TempGroup;}//oldver
	INT GetTempNum() {return TempNum;}//oldver
	INT GetTempMax() {return TempMax;}//oldver

private:
	// Variables.
	static UBOOL				Initialized;		// Whether initialized.
	static INT					BeginLoadCount;		// Count for BeginLoad multiple loads.
	static UObject*				ObjHash[4096];		// Object hash.
	static UObject*				AutoRegister;		// Objects to automatically register.
	static TArray<UObject*>		Root;				// Top of active object graph.
	static TArray<UObject*>		Objects;			// List of all objects.
	static TArray<INT>          Available;			// Available object indices.
	static TArray<UObject*>		Loaders;			// Array of loaders.
	static UPackage*			TransientPackage;	// Transient package.

	// Temporary.
	FName TempState, TempGroup; //oldver
	INT TempNum, TempMax; //oldver

	// Internal functions.
	ULinkerLoad* GetLoader( INT i ) {return (ULinkerLoad*)Loaders(i);}
	FName MakeUniqueObjectName( UObject* Parent, UClass* Class );
	void Register( UObject* InObject, FName InPackageName );
	void AddObject( UObject* Res, INT Index );
	void HashObject( UObject* Res );
	void UnhashObject( UObject* Res );
	UBOOL ResolveName( UObject*& ObjectParent, const char*& Name, UBOOL Create, UBOOL Throw );
	void SafeLoadError( DWORD LoadFlags, const char* Error, const char* Fmt, ... );
	void PurgeGarbage( FOutputDevice* Out );
};

/*-----------------------------------------------------------------------------
	UObject.
-----------------------------------------------------------------------------*/

//
// The base class of all objects.
//
class CORE_API UObject : public FUnknown
{
	DECLARE_BASE_CLASS(UObject,UObject,CLASS_Abstract)

	// Identification.
	enum {GUID1=0,GUID2=0,GUID3=0,GUID4=0};
	enum {StaticRecordSize=0};
	enum {InternalConstructor=0};

	// Friends.
	friend class FObjectManager;

private:
	// Variables maintained by FObjectManager.
	INT			    Index;			// Index of object into FObjectManager's Objects table.
	UObject*		HashNext;		// Next object in this hash bin.
	FMainFrame*		MainFrame;		// Main script execution stack.
	ULinkerLoad*	Linker;			// Linker it came from, or NULL if none.
	DWORD			LinkerIndex;	// Index of this object in the linker's export map.
	UObject*		Parent;			// Package the object belongs to.
	DWORD			ObjectFlags;	// Private EObjectFlags used by object manager.
	FName			Name;			// Name of the object.
	UClass*			Class;	  		// Class the object belongs to.

	// Disable the copy constructor and assignment operator.
	UObject& operator=( const UObject& )
		{ appErrorf( "UObject assignment operator called" ); return *this; }
	UObject( const UObject& )
		{ appErrorf( "UObject copy constructor operator called" ); }
public:

	// Constructors.
	UObject();
	UObject( EIntrinsicConstructor, UClass* InClass, FName InName, FName InPackageName );
	static void InternalClassInitializer( UClass* Class );

	// Destructors.
	virtual ~UObject();
	void operator delete( void* Object, size_t Size )
		{guard(UObject::operator delete); appFree( Object ); unguard;}

	// FUnknown interface.
	virtual DWORD STDCALL QueryInterface( const FGuid& RefIID, void** InterfacePtr );
	virtual DWORD STDCALL AddRef();
	virtual DWORD STDCALL Release();

	// UObject interface.
	virtual void ProcessEvent( UFunction* Function, void* Parms );
	virtual void ProcessState( FLOAT DeltaSeconds );
	virtual UBOOL ProcessRemoteFunction( UFunction* Function, void* Parms, FFrame* Stack );
	virtual void ProcessInternal( FFrame& Stack );
	virtual INT ExportToFile( const char* Filename );
	virtual INT MemUsage();
	virtual void Modify();
	virtual void Export( FOutputDevice& Out, const char* FileType, int Indent );
	virtual void PostLoad();
	virtual void Destroy();
	virtual void Serialize( FArchive& Ar );
	virtual UBOOL IsPendingKill() {return 0;}
	virtual EGotoState GotoState( FName State );
	virtual INT GotoLabel( FName Label );
	virtual void InitExecution();
	virtual void ShutdownAfterError();
	virtual void PostEditChange();
	virtual void CallFunction( FFrame& Stack, BYTE*& Result, UFunction* Function );
	virtual UBOOL ScriptConsoleExec( const char* Cmd, FOutputDevice* Out );

	// Functions.
	const char* GetFullName( char* Str=NULL ) const;
	const char* GetPathName( UObject* StopParent=NULL, char* Str=NULL ) const;
	UBOOL IsValid();
	void ConditionalDestroy();
	void ConditionalPostLoad();
	void ConditionalShutdownAfterError();
	UBOOL IsA( UClass* Parent ) const;
	UBOOL IsIn( UObject* SomeParent ) const;
	void SetLinker( ULinkerLoad* L, INT I );
	UBOOL IsProbing( FName ProbeName );
	void Rename();
	UField* FindField( FName InName, UBOOL Global=0 );
	UFunction* FindFunction( FName InName, UBOOL Global=0 );
	UFunction* FindFunctionChecked( FName InName, UBOOL Global=0 );
	UState* FindState( FName InName );
	void SaveConfig( DWORD Flags=CPF_Config, const char* Filename=NULL );
	void LoadConfig( FName Type, UClass* Class=NULL, const char* Filename=NULL );
	void FixOld();//oldver

	// Accessors.
	UClass* GetClass() const
	{
		// Note: Must be safe with class-default metaobjects.
		return Class;
	}
	void SetClass( UClass* In )
	{
		// Note: Must be safe with class-default metaobjects.
		Class=In;
	}
	const char* GetClassName() const
	{
		// Note: Must be safe with class-default metaobjects.
		return ((UObject*)Class)->GetName();
	}
	INT	GetHash() const
	{
		return Name.GetIndex() & (ARRAY_COUNT(FObjectManager::ObjHash)-1);
	}
	DWORD GetFlags() const
	{
		return ObjectFlags;
	}
	void SetFlags( DWORD NewFlags )
	{
		ObjectFlags |= NewFlags;
	}
	void ClearFlags( DWORD NewFlags )
	{
		ObjectFlags &= ~NewFlags;
	}
	const char* GetName() const
	{
		return *Name;
	}
	 FName GetFName() 
	{
		return Name;
	}
	const FName GetFName() const
	{
		return Name;
	}
	UObject* GetParent() const
	{
		return Parent;
	}
	DWORD GetIndex() const
	{
		return Index;
	}
	INT GetLinkerIndex()
	{
		return LinkerIndex;
	}
	ULinkerLoad* GetLinker()
	{
		return Linker;
	}
	FMainFrame* GetMainFrame()
	{
		return MainFrame;
	}

	// UnrealScript intrinsics.
	#define INTRINSIC(func) void func( FFrame& Stack, BYTE*& Result );
	INTRINSIC(execUndefined)
	INTRINSIC(execLocalVariable)
	INTRINSIC(execInstanceVariable)
	INTRINSIC(execDefaultVariable)
	INTRINSIC(execArrayElement)
	INTRINSIC(execBoolVariable)
	INTRINSIC(execClassDefaultVariable)
	INTRINSIC(execEndFunctionParms)
	INTRINSIC(execNothing)
	INTRINSIC(execStop)
	INTRINSIC(execEndCode)
	INTRINSIC(execSwitch)
	INTRINSIC(execCase)
	INTRINSIC(execJump)
	INTRINSIC(execJumpIfNot)
	INTRINSIC(execAssert)
	INTRINSIC(execGotoLabel)
	INTRINSIC(execLet)
	INTRINSIC(execLet1)
	INTRINSIC(execLet4)
	INTRINSIC(execLetBool)
	INTRINSIC(execLetString)
	INTRINSIC(execSelf)
	INTRINSIC(execContext)
	INTRINSIC(execVirtualFunction)
	INTRINSIC(execFinalFunction)
	INTRINSIC(execGlobalFunction)
	INTRINSIC(execStructCmpEq)
	INTRINSIC(execStructCmpNe)
	INTRINSIC(execStructMember)
	INTRINSIC(execIntConst)
	INTRINSIC(execFloatConst)
	INTRINSIC(execStringConst)
	INTRINSIC(execObjectConst)
	INTRINSIC(execNameConst)
	INTRINSIC(execByteConst)
	INTRINSIC(execIntZero)
	INTRINSIC(execIntOne)
	INTRINSIC(execTrue)
	INTRINSIC(execFalse)
	INTRINSIC(execNoObject)
	INTRINSIC(execIntConstByte)
	INTRINSIC(execResizeString)
	INTRINSIC(execDynamicCast)
	INTRINSIC(execMetaCast)
	INTRINSIC(execByteToInt)
	INTRINSIC(execByteToBool)
	INTRINSIC(execByteToFloat)
	INTRINSIC(execByteToString)
	INTRINSIC(execIntToByte)
	INTRINSIC(execIntToBool)
	INTRINSIC(execIntToFloat)
	INTRINSIC(execIntToString)
	INTRINSIC(execBoolToByte)
	INTRINSIC(execBoolToInt)
	INTRINSIC(execBoolToFloat)
	INTRINSIC(execBoolToString)
	INTRINSIC(execFloatToByte)
	INTRINSIC(execFloatToInt)
	INTRINSIC(execFloatToBool)
	INTRINSIC(execFloatToString)
	INTRINSIC(execObjectToBool)
	INTRINSIC(execObjectToString)
	INTRINSIC(execNameToBool)
	INTRINSIC(execNameToString)
	INTRINSIC(execStringToByte)
	INTRINSIC(execStringToInt)
	INTRINSIC(execStringToBool)
	INTRINSIC(execStringToFloat)
	INTRINSIC(execNot_PreBool)
	INTRINSIC(execEqualEqual_BoolBool)
	INTRINSIC(execNotEqual_BoolBool)
	INTRINSIC(execAndAnd_BoolBool)
	INTRINSIC(execXorXor_BoolBool)
	INTRINSIC(execOrOr_BoolBool)
	INTRINSIC(execMultiplyEqual_ByteByte)
	INTRINSIC(execDivideEqual_ByteByte)
	INTRINSIC(execAddEqual_ByteByte)
	INTRINSIC(execSubtractEqual_ByteByte)
	INTRINSIC(execAddAdd_PreByte)
	INTRINSIC(execSubtractSubtract_PreByte)
	INTRINSIC(execAddAdd_Byte)
	INTRINSIC(execSubtractSubtract_Byte)
	INTRINSIC(execComplement_PreInt)
	INTRINSIC(execSubtract_PreInt)
	INTRINSIC(execMultiply_IntInt)
	INTRINSIC(execDivide_IntInt)
	INTRINSIC(execAdd_IntInt)
	INTRINSIC(execSubtract_IntInt)
	INTRINSIC(execLessLess_IntInt)
	INTRINSIC(execGreaterGreater_IntInt)
	INTRINSIC(execLess_IntInt)
	INTRINSIC(execGreater_IntInt)
	INTRINSIC(execLessEqual_IntInt)
	INTRINSIC(execGreaterEqual_IntInt)
	INTRINSIC(execEqualEqual_IntInt)
	INTRINSIC(execNotEqual_IntInt)
	INTRINSIC(execAnd_IntInt)
	INTRINSIC(execXor_IntInt)
	INTRINSIC(execOr_IntInt)
	INTRINSIC(execMultiplyEqual_IntFloat)
	INTRINSIC(execDivideEqual_IntFloat)
	INTRINSIC(execAddEqual_IntInt)
	INTRINSIC(execSubtractEqual_IntInt)
	INTRINSIC(execAddAdd_PreInt)
	INTRINSIC(execSubtractSubtract_PreInt)
	INTRINSIC(execAddAdd_Int)
	INTRINSIC(execSubtractSubtract_Int)
	INTRINSIC(execRand)
	INTRINSIC(execMin)
	INTRINSIC(execMax)
	INTRINSIC(execClamp)
	INTRINSIC(execSubtract_PreFloat)
	INTRINSIC(execMultiplyMultiply_FloatFloat)
	INTRINSIC(execMultiply_FloatFloat)
	INTRINSIC(execDivide_FloatFloat)
	INTRINSIC(execPercent_FloatFloat)
	INTRINSIC(execAdd_FloatFloat)
	INTRINSIC(execSubtract_FloatFloat)
	INTRINSIC(execLess_FloatFloat)
	INTRINSIC(execGreater_FloatFloat)
	INTRINSIC(execLessEqual_FloatFloat)
	INTRINSIC(execGreaterEqual_FloatFloat)
	INTRINSIC(execEqualEqual_FloatFloat)
	INTRINSIC(execNotEqual_FloatFloat)
	INTRINSIC(execComplementEqual_FloatFloat)
	INTRINSIC(execMultiplyEqual_FloatFloat)
	INTRINSIC(execDivideEqual_FloatFloat)
	INTRINSIC(execAddEqual_FloatFloat)
	INTRINSIC(execSubtractEqual_FloatFloat)
	INTRINSIC(execAbs)
	INTRINSIC(execSin)
	INTRINSIC(execCos)
	INTRINSIC(execTan)
	INTRINSIC(execAtan)
	INTRINSIC(execExp)
	INTRINSIC(execLoge)
	INTRINSIC(execSqrt)
	INTRINSIC(execSquare)
	INTRINSIC(execFRand)
	INTRINSIC(execFMin)
	INTRINSIC(execFMax)
	INTRINSIC(execFClamp)
	INTRINSIC(execLerp)
	INTRINSIC(execSmerp)
	INTRINSIC(execConcat_StringString)
	INTRINSIC(execLess_StringString)
	INTRINSIC(execGreater_StringString)
	INTRINSIC(execLessEqual_StringString)
	INTRINSIC(execGreaterEqual_StringString)
	INTRINSIC(execEqualEqual_StringString)
	INTRINSIC(execNotEqual_StringString)
	INTRINSIC(execComplementEqual_StringString)
	INTRINSIC(execLen)
	INTRINSIC(execInStr)
	INTRINSIC(execMid)
	INTRINSIC(execLeft)
	INTRINSIC(execRight)
	INTRINSIC(execCaps)
	INTRINSIC(execChr)
	INTRINSIC(execAsc)
	INTRINSIC(execEqualEqual_ObjectObject)
	INTRINSIC(execNotEqual_ObjectObject)
	INTRINSIC(execEqualEqual_NameName)
	INTRINSIC(execNotEqual_NameName)
	INTRINSIC(execLog)
	INTRINSIC(execWarn)
	INTRINSIC(execClassIsChildOf)
	INTRINSIC(execClassContext)
	INTRINSIC(execGoto)
	INTRINSIC(execGotoState)
	INTRINSIC(execIsA)
	INTRINSIC(execEnable)
	INTRINSIC(execDisable)
	INTRINSIC(execIterator)
	INTRINSIC(execLocalize)
	INTRINSIC(execIntrinsicParm)
	INTRINSIC(execGetPropertyText)
	INTRINSIC(execSetPropertyText)
	INTRINSIC(execSaveConfig)
	INTRINSIC(execResetConfig)
	INTRINSIC(execGetEnum)
	INTRINSIC(execDynamicLoadObject)
	INTRINSIC(execIsInState)
	INTRINSIC(execGetStateName)
	INTRINSIC(execHighIntrinsic0)
	INTRINSIC(execHighIntrinsic1)
	INTRINSIC(execHighIntrinsic2)
	INTRINSIC(execHighIntrinsic3)
	INTRINSIC(execHighIntrinsic4)
	INTRINSIC(execHighIntrinsic5)
	INTRINSIC(execHighIntrinsic6)
	INTRINSIC(execHighIntrinsic7)
	INTRINSIC(execHighIntrinsic8)
	INTRINSIC(execHighIntrinsic9)
	INTRINSIC(execHighIntrinsic10)
	INTRINSIC(execHighIntrinsic11)
	INTRINSIC(execHighIntrinsic12)
	INTRINSIC(execHighIntrinsic13)
	INTRINSIC(execHighIntrinsic14)
	INTRINSIC(execHighIntrinsic15)
    void BeginState()
    {
        ProcessEvent(FindFunctionChecked(NAME_BeginState),NULL);
    }
    void EndState()
    {
        ProcessEvent(FindFunctionChecked(NAME_EndState),NULL);
    }
};

/*----------------------------------------------------------------------------
	Core object templates.
----------------------------------------------------------------------------*/

// Parse an object name in the input stream.
template< class T > UBOOL ParseObject( const char* Stream, const char* Match, T*& Obj, UObject* Parent )
{
	return ParseObject( Stream, Match, T::StaticClass, *(UObject **)&Obj, Parent );
}

// Find an optional object.
template< class T > T* FindObject( UObject* Parent, const char* Name, UBOOL ExactClass=0 )
{
	return (T*)GObj.FindObject( T::StaticClass, Parent, Name, ExactClass );
}

// Find an object, no failure allowed.
template< class T > T* FindObjectChecked( UObject* Parent, const char* Name, UBOOL ExactClass=0 )
{
	return (T*)GObj.FindObjectChecked( T::StaticClass, Parent, Name, ExactClass );
}

// Dynamically cast an object type-safely.
template< class T > T* Cast( UObject* Src )
{
	return Src && Src->IsA(T::StaticClass) ? (T*)Src : NULL;
}
template< class T, class U > T* CastChecked( U* Src )
{
	if( !Src || !Src->IsA(T::StaticClass) )
		appErrorf( "Cast of %s to %s failed", Src ? Src->GetFullName() : "NULL", T::StaticClass->GetName() );
	return (T*)Src;
}

// Import an object using a UFactory.
template< class T > T* ImportObjectFromFile( UObject* Parent, FName Name, const char* Filename, FFeedbackContext* Warn=GSystem )
{
	return (T*)GObj.ImportObjectFromFile( T::StaticClass, Parent, Name, Filename, Warn );
}

// Construct an object of a particular class.
template< class T > T* ConstructClassObject( UClass* Class, UObject* Parent=(UObject*)GObj.GetTransientPackage(), FName Name=NAME_None, DWORD SetFlags=0 )
{
	check(Class->IsChildOf(T::StaticClass));
	return (T*)GObj.ConstructObject( Class );
}

// Load an object.
template< class T > T* LoadObject( UObject* InParent, const char* Name, const char* Filename, DWORD LoadFlags, FPackageMap* Sandbox )
{
	return (T*)GObj.LoadObject( T::StaticClass, InParent, Name, Filename, LoadFlags, Sandbox );
}

// Load a class object.
template< class T > UClass* LoadClass( UObject* InParent, const char* Name, const char* Filename, DWORD LoadFlags, FPackageMap* Sandbox )
{
	return GObj.LoadClass( T::StaticClass, InParent, Name, Filename, LoadFlags, Sandbox );
}

// Get default object of a class.
template< class T > T* GetDefault()
{
	return (T*)&T::StaticClass->Defaults(0);
}

/*----------------------------------------------------------------------------
	Object iterators.
----------------------------------------------------------------------------*/

//
// Class for iterating through all objects.
//
class FObjectIterator
{
public:
	FObjectIterator( UClass* InClass=UObject::StaticClass )
	:	Class( InClass ), Index( -1 )
	{
		check(Class);
		++*this;
	}
	void operator++()
	{
		while( ++Index<GObj.Objects.Num() && (!GObj.Objects(Index) || !GObj.Objects(Index)->IsA(Class)) );
	}
	UObject* operator*()
	{
		debug(Index<GObj.Objects.Num());
		debug(GObj.Objects(Index));
		return GObj.Objects(Index);
	}
	UObject* operator->()
	{
		debug(Index<GObj.Objects.Num());
		debug(GObj.Objects(Index));
		return GObj.Objects(Index);
	}
	operator UBOOL()
	{
		return Index<GObj.Objects.Num();
	}
protected:
	UClass* Class;
	INT Index;
};

//
// Class for iterating through all objects which inherit from a
// specified parent class.
//
template< class T > class TObjectIterator : public FObjectIterator
{
public:
	TObjectIterator()
	:	FObjectIterator( T::StaticClass )
	{}
	T* operator* ()
	{
		return (T*)FObjectIterator::operator*();
	}
	T* operator-> ()
	{
		return (T*)FObjectIterator::operator->();
	}
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
