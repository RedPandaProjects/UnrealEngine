/*=============================================================================
	UnScrCom.h: UnrealScript compiler.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Constants & types.
-----------------------------------------------------------------------------*/

// Max nesting.
enum {MAX_NEST_LEVELS = 16};

//
// Token types.
//
enum ETokenType
{
	TOKEN_None				= 0x00,		// No token.
	TOKEN_Identifier		= 0x01,		// Alphanumeric identifier.
	TOKEN_Symbol			= 0x02,		// Symbol.
	TOKEN_Const				= 0x03,		// A constant.
	TOKEN_Max				= 0x0D
};

//
// Code nesting types.
//
enum ENestType
{
	NEST_None				=0x0000,	//  No nesting.
	NEST_Class				=0x0001,	//  Class/EndClass.
	NEST_State				=0x0002,	//	State/EndState.
	NEST_Function			=0x0003,	//	Function/EndFunction.
	NEST_If					=0x0005,	//  If/ElseIf/EndIf.
	NEST_Loop				=0x0006,	//  While/Do/Loop/Until.
	NEST_Switch				=0x0007,	//  Switch.
	NEST_For				=0x0008,	//  For.
	NEST_ForEach            =0x000A,    //  ForEach.
	NEST_Max				=0x000A
};

//
// Types of statements to allow within a particular nesting block.
//
enum ENestAllowFlags
{
	ALLOW_StateCmd			= 0x0001,	// Allow commands that reside in states only.
	ALLOW_Cmd				= 0x0002,	// Allow commands that take 0 cycle time.
	ALLOW_Function			= 0x0004,	// Allow Event declarations at this level.
	ALLOW_State				= 0x0008,	// Allow State declarations at this level.
	ALLOW_ElseIf			= 0x0010,	// Allow ElseIf commands at this level.
	ALLOW_VarDecl			= 0x0040,	// Allow variable declarations at this level.
	ALLOW_Class				= 0x0080,	// Allow class definition heading.
	ALLOW_Case				= 0x0100,	// Allow 'case' statement.
	ALLOW_Default			= 0x0200,	// Allow 'default' case statement.
	ALLOW_Return			= 0x0400,	// Allow 'return' within a function.
	ALLOW_Break				= 0x0800,	// Allow 'break' from loop.
	ALLOW_Label				= 0x2000,	// Allow any label.
	ALLOW_Ignores			= 0x4000,	// Allow function masks like 'Ignores'.
	ALLOW_Instance          = 0x8000,   // Allow instance variables and functions.
};

/*-----------------------------------------------------------------------------
	FPropertyBase.
-----------------------------------------------------------------------------*/

//
// Property data type enums.
//warning: Script compiler has a hardcoded table based on these enum indices.
//
enum EPropertyType
{
	CPT_None			= 0,
	CPT_Byte			= 1,
	CPT_Int				= 2,
	CPT_Bool			= 3,
	CPT_Float			= 4,
	CPT_ObjectReference	= 5,
	CPT_Name			= 6,
	CPT_String			= 7,
	CPT_Struct			= 10,
	CPT_Vector          = 11,
	CPT_Rotation        = 12,
	CPT_MAX				= 13,
};

//
// Basic information describing a type.
//
class FPropertyBase
{
public:
	// Variables.
	EPropertyType Type;
	INT ArrayDim;
	DWORD PropertyFlags;
	union
	{
		class UEnum* Enum;
		class UClass* PropertyClass;
		class UStruct* Struct;
		DWORD BitMask;
		INT StringSize;
	};
	UClass* MetaClass;

	// Constructors.
	FPropertyBase( EPropertyType InType )
	:	Type(InType), ArrayDim(1), PropertyFlags(0), BitMask(0), MetaClass(NULL)
	{}
	FPropertyBase( UEnum* InEnum )
	:	Type(CPT_Byte), ArrayDim(1), PropertyFlags(0), Enum(InEnum), MetaClass(NULL)
	{}
	FPropertyBase( UClass* InClass )
	:	Type(CPT_ObjectReference), ArrayDim(1), PropertyFlags(0), PropertyClass(InClass), MetaClass(NULL)
	{}
	FPropertyBase( UStruct* InStruct )
	:	Type(CPT_Struct), ArrayDim(1), PropertyFlags(0), Struct(InStruct), MetaClass(NULL)
	{}
	FPropertyBase( UProperty* Property )
	{
		guard(FPropertyBase::FPropertyBase);
		if( Property->GetClass()==UByteProperty::StaticClass )
		{
			*this = FPropertyBase(CPT_Byte);
			Enum = Cast<UByteProperty>(Property)->Enum;
		}
		else if( Property->GetClass()==UIntProperty::StaticClass )
		{
			*this = FPropertyBase(CPT_Int);
		}
		else if( Property->GetClass()==UBoolProperty::StaticClass )
		{
			*this = FPropertyBase(CPT_Bool);
			BitMask = Cast<UBoolProperty>(Property)->BitMask;
		}
		else if( Property->GetClass()==UFloatProperty::StaticClass )
		{
			*this = FPropertyBase(CPT_Float);
		}
		else if( Property->GetClass()==UClassProperty::StaticClass )
		{
			*this = FPropertyBase(CPT_ObjectReference);
			PropertyClass = Cast<UClassProperty>(Property)->PropertyClass;
			MetaClass = Cast<UClassProperty>(Property)->MetaClass;
		}
		else if( Property->GetClass()==UObjectProperty::StaticClass )
		{
			*this = FPropertyBase(CPT_ObjectReference);
			PropertyClass = Cast<UObjectProperty>(Property)->PropertyClass;
		}
		else if( Property->GetClass()==UNameProperty::StaticClass )
		{
			*this = FPropertyBase(CPT_Name);
		}
		else if( Property->GetClass()==UStringProperty::StaticClass )
		{
			*this = FPropertyBase(CPT_String);
			StringSize = Cast<UStringProperty>(Property)->StringSize;
		}
		else if( Property->GetClass()==UStructProperty::StaticClass )
		{
			*this = FPropertyBase(CPT_Struct);
			Struct = Cast<UStructProperty>(Property)->Struct;
		}
		else
		{
			appErrorf( "Unknown property type '%s'", Property->GetFullName() );
		}
		ArrayDim = Property->ArrayDim;
		PropertyFlags = Property->PropertyFlags;
		unguard;
	}

	// Functions.
	INT GetSize() const
	{
		return GetElementSize() * ArrayDim;
	}
	INT GetElementSize() const
	{
		static const INT ElementSizes[CPT_MAX] =
		{
			0,
			sizeof(BYTE),
			sizeof(INT),
			sizeof(DWORD),
			sizeof(FLOAT),
			sizeof(UObject*),
			sizeof(FName),
			0,
			0,
			0,
			0
		};
		if( Type!=CPT_Struct && Type!=CPT_String )
			return ElementSizes[ Type ];
		else if( Type==CPT_Struct )
			return Struct->GetPropertiesSize();
		else
			return StringSize;
	}
	UBOOL IsVector() const
	{
		return Type==CPT_Struct && appStricmp(*Struct->GetFName(),"Vector")==0;
	}
	UBOOL IsRotator() const
	{
		return Type==CPT_Struct && appStricmp(*Struct->GetFName(),"Rotator")==0;
	}
	UBOOL MatchesType( const FPropertyBase& Other, UBOOL Identity ) const
	{
		check(Type!=CPT_None || !Identity);

		// If converting to an l-value, we require an exact match with an l-value.
		if( PropertyFlags & CPF_OutParm )
		{
			if( (Other.PropertyFlags & CPF_Const) || !(Other.PropertyFlags & CPF_OutParm) )
				return 0;
			Identity = 1;
		}

		// Check everything.
		if( Type==CPT_None && (Other.Type==CPT_None || !Identity) )
		{
			// If Other has no type, accept anything.
			return 1;
		}
		else if( Type != Other.Type )
		{
			// Mismatched base types.
			return 0;
		}
		else if( ArrayDim != Other.ArrayDim )
		{
			// Mismatched array dimensions.
			return 0;
		}
		else if( Type==CPT_Byte )
		{
			// Make sure enums match, or we're generalizing.
			return Enum==Other.Enum || (Enum==NULL && !Identity);
		}
		else if( Type==CPT_ObjectReference )
		{
			check(PropertyClass!=NULL);

			// Make sure object types match, or we're generalizing.
			if( Identity )
			{
				// Exact match required.
				return PropertyClass==Other.PropertyClass && MetaClass==Other.MetaClass;
			}
			else if( Other.PropertyClass==NULL )
			{
				// Cannonical 'None' matches all object classes.
				return 1;
			}
			else
			{
				// Generalization is ok.
				if( Other.PropertyClass->IsChildOf(PropertyClass) )
					if( PropertyClass!=UClass::StaticClass || Other.MetaClass->IsChildOf(MetaClass) )
						return 1;
				return 0;
			}
		}
		else if( Type==CPT_Struct )
		{
			check(Struct!=NULL);
			check(Other.Struct!=NULL);

			// Make sure struct types match, or we're generalizing.
			if( Identity ) return Struct==Other.Struct;            // Exact match required.
			else           return Other.Struct->IsChildOf(Struct); // Generalization is ok.
		}
		else if( Type==CPT_String )
		{
			// Make sure lengths match, or we're generalizing (widening).
			if( Identity )  return StringSize == Other.StringSize;
			else            return StringSize >= Other.StringSize;
		}
		else
		{
			// General match.
			return 1;
		}
	}
};

/*-----------------------------------------------------------------------------
	FToken.
-----------------------------------------------------------------------------*/

//
// Information about a token that was just parsed.
//
class FToken : public FPropertyBase
{
public:
	// Variables.
	ETokenType	TokenType;		// Type of token.
	FName		TokenName;		// Name of token.
	int			StartPos;		// Starting position in script where this token came from.
	int			StartLine;		// Starting line in script.
	char		Identifier[NAME_SIZE]; // Always valid.
	union
	{
		// TOKEN_Const values.
		BYTE	Byte;								// If CPT_Byte.
		INT		Int;								// If CPT_Int.
		UBOOL	Bool;								// If CPT_Bool.
		FLOAT	Float;								// If CPT_Float.
		UObject* Object;							// If CPT_ObjectReference.
		CHAR	NameBytes[sizeof(FName)];			// If CPT_Name.
		CHAR	String[MAX_STRING_CONST_SIZE];		// If CPT_String.
		CHAR	VectorBytes[sizeof(FVector)];		// If CPT_Struct && IsVector().
		CHAR	RotationBytes[sizeof(FRotator)];	// If CPT_Struct && IsRotator().
		BYTE	StructBytes[MAX_STRING_CONST_SIZE]; // If CPT_Struct.
	};

	// Constructors.
	FToken()
	: FPropertyBase( CPT_None )
	{
		InitToken( CPT_None );
	}
	FToken( const FPropertyBase& InType )
	: FPropertyBase( CPT_None )
	{
		InitToken( CPT_None );
		(FPropertyBase&)*this = InType;
	}

	// Inlines.
	void InitToken( EPropertyType InType )
	{
		(FPropertyBase&)*this = FPropertyBase(InType);
		TokenType		= TOKEN_None;
		TokenName		= NAME_None;
		StartPos		= 0;
		StartLine		= 0;
	}
	int Matches( const char *Str )
	{
		return (TokenType==TOKEN_Identifier || TokenType==TOKEN_Symbol) && appStricmp(Identifier,Str)==0;
	}
	int Matches( FName Name )
	{
		return TokenType==TOKEN_Identifier && TokenName==Name;
	}
	void AttemptToConvertConstant( const FPropertyBase& NewType )
	{
		check(TokenType==TOKEN_Const);
		switch( NewType.Type )
		{
			case CPT_Int:		{INT        V(0);           if( GetConstInt     (V) ) SetConstInt     (V); break;}
			case CPT_Bool:		{UBOOL      V(0);           if( GetConstBool    (V) ) SetConstBool    (V); break;}
			case CPT_Float:		{FLOAT      V(0.f);         if( GetConstFloat   (V) ) SetConstFloat   (V); break;}
			case CPT_Name:		{FName      V(NAME_None);   if( GetConstName    (V) ) SetConstName    (V); break;}
			case CPT_Struct:
			{
				if( NewType.IsVector() )
				{
					FVector V( 0.f, 0.f, 0.f );
					if( GetConstVector( V ) )
						SetConstVector( V );
				}
				else if( NewType.IsRotator() )
				{
					FRotator V( 0, 0, 0 );
					if( GetConstRotation( V ) )
						SetConstRotation( V );
				}
				//!!struct conversion
				break;
			}
			case CPT_String:
			{
				if( Type==CPT_String )
				{
					check(NewType.StringSize>0);
					if( NewType.StringSize < StringSize )
						String[NewType.StringSize-1]=0;
					StringSize = NewType.StringSize;
				}
				break;
			}
			case CPT_Byte:
			{
				BYTE V=0;
				if( NewType.Enum==NULL && GetConstByte(V) )
					SetConstByte(NULL,V); 
				break;
			}
			case CPT_ObjectReference:
			{
				UObject* Ob=NULL; 
				if( GetConstObject( NewType.PropertyClass, Ob ) )
					SetConstObject( Ob ); 
				break;
			}
		}
	}

	// Setters.
	void SetConstByte( UEnum* InEnum, BYTE InByte )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_Byte);
		Enum			= InEnum;
		Byte			= InByte;
		TokenType		= TOKEN_Const;
	}
	void SetConstInt( INT InInt )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_Int);
		Int				= InInt;
		TokenType		= TOKEN_Const;
	}
	void SetConstBool( UBOOL InBool )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_Bool);
		Bool 			= InBool;
		TokenType		= TOKEN_Const;
	}
	void SetConstFloat( FLOAT InFloat )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_Float);
		Float			= InFloat;
		TokenType		= TOKEN_Const;
	}
	void SetConstObject( UObject* InObject )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_ObjectReference);
		PropertyClass	= InObject ? InObject->GetClass() : NULL;
		Object			= InObject;
		TokenType		= TOKEN_Const;
		if( PropertyClass==UClass::StaticClass )
			MetaClass = CastChecked<UClass>(InObject);
	}
	void SetConstName( FName InName )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_Name);
		*(FName *)NameBytes = InName;
		TokenType		= TOKEN_Const;
	}
	void SetConstString( char *InString, int MaxLength = MAX_STRING_CONST_SIZE )
	{
		check(MaxLength>0);
		(FPropertyBase&)*this = FPropertyBase(CPT_String);
		if( InString != String )
			appStrncpy( String, InString, MaxLength );
		StringSize = appStrlen(String)+1;
		TokenType = TOKEN_Const;
	}
	void SetConstVector( FVector &InVector )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_Struct);
		Struct                  = FindObjectChecked<UStruct>( ANY_PACKAGE, "Vector" );//!!
		*(FVector *)VectorBytes = InVector;
		TokenType		        = TOKEN_Const;
	}
	void SetConstRotation( FRotator &InRotation )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_Struct);
		Struct          = FindObjectChecked<UStruct>( ANY_PACKAGE, "Rotator" );//!!
		*(FRotator *)RotationBytes	= InRotation;
		TokenType		= TOKEN_Const;
	}
	//!!struct constants

	// Getters.
	int GetConstByte( BYTE &B )
	{
		if( TokenType==TOKEN_Const && Type==CPT_Byte )
		{
			B = Byte;
			return 1;
		}
		else if( TokenType==TOKEN_Const && Type==CPT_Int && Int>=0 && Int<255 )
		{
			B = Int;
			return 1;
		}
		else if( TokenType==TOKEN_Const && Type==CPT_Float && Float>=0 && Float<255 && Float==(INT)Float)
		{
			B = Float;
			return 1;
		}
		else return 0;
	}
	int GetConstInt( INT &I )
	{
		if     ( TokenType==TOKEN_Const && Type==CPT_Int )
		{
			I = Int;
			return 1;
		}
		else if( TokenType==TOKEN_Const && Type==CPT_Byte )
		{
			I = Byte;
			return 1;
		}
		else if( TokenType==TOKEN_Const && Type==CPT_Float && Float==(INT)Float)
		{
			I=Float;
			return 1;
		}
		else return 0;
	}
	int GetConstBool( UBOOL &B )
	{
		if( TokenType==TOKEN_Const && Type==CPT_Bool )
		{
			B = Bool;
			return 1;
		}
		else return 0;
	}
	int GetConstFloat( FLOAT &R )
	{
		if( TokenType==TOKEN_Const && Type==CPT_Float )
		{
			R = Float;
			return 1;
		}
		else if( TokenType==TOKEN_Const && Type==CPT_Int )
		{
			R = Int;
			return 1;
		}
		else if( TokenType==TOKEN_Const && Type==CPT_Byte )
		{
			R = Byte;
			return 1;
		}
		else return 0;
	}
	int GetConstObject( UClass *DesiredClass, UObject *&Ob )
	{
		if( TokenType==TOKEN_Const && Type==CPT_ObjectReference && (DesiredClass==NULL || PropertyClass->IsChildOf(DesiredClass)) )
		{
			Ob = Object;
			return 1;
		}
		return 0;
	}
	int GetConstName( FName &n )
	{
		if( TokenType==TOKEN_Const && Type==CPT_Name )
		{
			n = *(FName *)NameBytes;
			return 1;
		}
		return 0;
	}
	int GetConstString( char *&s )
	{
		if( TokenType==TOKEN_Const && Type==CPT_String )
		{
			s = String;
			return 1;
		}
		return 0;
	}
	int GetConstVector( FVector &v )
	{
		if( TokenType==TOKEN_Const && IsVector() )
		{
			v = *(FVector *)VectorBytes;
			return 1;
		}
		return 0;
	}
	//!!struct constants
	int GetConstRotation( FRotator &r )
	{
		if( TokenType==TOKEN_Const && IsRotator() )
		{
			r = *(FRotator *)RotationBytes;
			return 1;
		}
		return 0;
	}
};

/*-----------------------------------------------------------------------------
	Retry points.
-----------------------------------------------------------------------------*/

//
// A point in the script compilation state that can be set and returned to
// using InitRetry() and PerformRetry().  This is used in cases such as testing
// to see which overridden operator should be used, where code must be compiled
// and then "undone" if it was found not to match.
//
// Retries are not allowed to cross command boundaries (and thus nesting 
// boundaries).  Retries can occur across a single command or expressions and
// subexpressions within a command.
//
struct FRetryPoint
{
	const char* Input;
	INT InputPos;
	INT InputLine;
	INT CodeTop;
};

/*-----------------------------------------------------------------------------
	FNestInfo.
-----------------------------------------------------------------------------*/

//
// Types of code offset fixups we can perform.
//
enum EFixupType
{
	FIXUP_SwitchEnd		= 0, // Address past end of Switch construct.
	FIXUP_IfEnd			= 1, // Address past end of If construct.
	FIXUP_LoopStart		= 2, // Address of loop start.
	FIXUP_LoopEnd		= 3, // Address past end of Loop construct.
	FIXUP_ForStart		= 4, // Address of for start.
	FIXUP_ForEnd		= 5, // Address past end of For construct.
	FIXUP_Label			= 6, // Address of a label.
	FIXUP_IteratorEnd   = 7, // Address of end of iterator.
	FIXUP_MAX			= 8, // Maximum value.
};

//
// A fixup request.
//
struct FNestFixupRequest
{
	// Variables.
	EFixupType			Type;			// Type of fixup request.
	INT					iCode;			// Address in script code to apply the fixup.
	FName				Name;			// Label name, if FIXUP_Label.
	FNestFixupRequest*	Next;			// Next fixup request in nest info's linked list.

	// Constructor.
	FNestFixupRequest( EFixupType InType, INT iInCode, FName InName, FNestFixupRequest *InNext )
	:	Type	(InType)
	,	iCode	(iInCode)
	,	Name	(InName)
	,	Next	(InNext)
	{}
};

//
// Temporary compiler information about a label, stored at a nest level.
//
struct FLabelRecord : public FLabelEntry
{
	// Variables.
	FLabelRecord* Next; // Next label in the nest info's linked list of labels.

	// Constructor.
	FLabelRecord( FName InName, INT iInCode, FLabelRecord *InNext )
	:	FLabelEntry		( InName, iInCode )
	,	Next			( InNext )
	{}
};

//
// Information about a function we're compiling.
//
struct FFuncInfo
{
	// Variables;
	FToken		Function;		// Name of the function or operator.
	INT			Precedence;		// Binary operator precedence.
	DWORD		FunctionFlags;	// Function flags.
	INT			iIntrinsic;		// Index of intrinsic function.
	INT			ExpectParms;	// Number of parameters expected for operator.

	// Constructor.
	FFuncInfo()
	:	Function		()
	,	Precedence		(0)
	,	FunctionFlags   (0)
	,	iIntrinsic		(0)
	,	ExpectParms		(0)
	{}
};

//
// Information for a particular nesting level.
//
struct FNestInfo
{
	// Information for all nesting levels.
	UStruct*		Node;               // Link to the stack node.
	ENestType		NestType;			// Statement that caused the nesting.
	INT				Allow;				// Types of statements to allow at this nesting level.

	// Information for nesting levels.
	INT				Fixups[FIXUP_MAX];	// Fixup addresses for PopNest to use.
	FNestFixupRequest* FixupList;		// Pending fixup requests.
	FLabelRecord*	LabelList;			// Linked list of labels.
	INT				iCodeChain;			// Code index for next command, i.e. in strings of if/elseif/elseif...

	// Command specific info.
	FToken			SwitchType;			// Type of Switch statement.
	FRetryPoint		ForRetry;			// Retry point (used in For command).

	// Set a fixup address.
	void SetFixup( EFixupType Type, INT iCode )
	{
		check(Fixups[Type]==MAXWORD);
		Fixups[Type] = iCode;
	}
};

/*-----------------------------------------------------------------------------
	FScriptCompiler.
-----------------------------------------------------------------------------*/

//
// Little class for writing to scripts.
//
class FScriptWriter : public FArchive
{
public:
	FScriptWriter( class FScriptCompiler& InCompiler )
	: Compiler( InCompiler )
	{}
	FArchive& Serialize( void* V, int Length );
	FArchive& operator<<( class FName& N )
		{NAME_INDEX W=N.GetIndex(); return *this << W;}
	FArchive& operator<<( class UObject*& Res )
		{DWORD D = (DWORD)Res; return *this << D;}
	FArchive& operator<<( char* S )
		{return Serialize(S,appStrlen(S)+1);}
	FArchive& operator<<( enum EExprToken E )
		{BYTE B=E; return *this<<B;}
	FArchive& operator<<( enum EPropertyType E )
		{BYTE B=E; return *this<<B;}
	FScriptCompiler& Compiler;
};

//
// Script compiler class.
//
class FScriptCompiler
{
public:
	// Variables.
	UClass*			Class;					// Actor class info while compiling is happening.
	UTextBuffer*	ErrorText;				// Error text buffer.
	FScriptWriter	Writer;					// Script writer.
	FMemStack*		Mem;					// Pool for temporary allocations.
	const char*		Input;					// Input text.
	INT				InputPos;				// Current position in text.
	INT				InputLine;				// Current line in text.
	INT				PrevPos;				// Position previous to last GetChar() call.
	INT				PrevLine;				// Line previous to last GetChar() call.
	INT				StatementsCompiled;		// Number of statements compiled.
	INT				LinesCompiled;			// Total number of lines compiled.
	INT				GotAffector;			// Got an expression that has a side effect?
	INT				GotIterator;			// Got an iterator.
	INT				AllowIterator;			// Allow iterators.
	INT				Booting;				// Bootstrap compiling classes.
	INT				Pass;					// Compilation pass.
	INT				NestLevel;				// Current nest level, starts at 0.
	FNestInfo*		TopNest;				// Top nesting level.
	UStruct*		TopNode;				// Top stack node.
	FNestInfo		Nest[MAX_NEST_LEVELS];	// Information about all nesting levels.
	INT             OriginalPropertiesSize;	// Original intrinsic properties size before compile.
	UBOOL			ShowDep;				// Show dependencies when recompiling.

	// Constructor.
	FScriptCompiler()
	: Writer( *this )
	{}

	// Precomputation.
	void			PostParse( UStruct* Node );

	// High-level compiling functions.
	INT				CompileScript( UClass* Class, FMemStack* Mem, UBOOL Booting, INT Pass );
	INT				CompileDeclaration( FToken& Token, UBOOL& NeedSemicolon );
	void			CompileCommand( FToken& Token, UBOOL& NeedSemicolon );
	INT				CompileStatement();
	void			CompileStatements();
	INT				CompileExpr( const FPropertyBase RequiredType, const char* ErrorTag=NULL, FToken* ResultType=NULL, INT MaxPrecedence=MAXINT, FPropertyBase* HintType=NULL );
	UBOOL			CompileFieldExpr( UStruct* Scope, FPropertyBase& RequiredType, FToken Token, FToken& ResultType, UBOOL IsSelf, UBOOL IsConcrete );
	INT				CompileDynamicCast( FToken Token, FToken& ResultType );
	void			CompileAssignment( const char* Tag );
	void			CompileAffector();
	void			CompileDirective();
	void			CompileSecondPass( UStruct* Node );
	UEnum*			CompileEnum( UStruct* Owner );
	UStruct*		CompileStruct( UStruct* Owner );
	void			CompileConst( UStruct* Owner );

	// High-level parsing functions.
	INT				GetToken( FToken& Token, const FPropertyBase* Hint=NULL, INT NoConsts=0 );
	INT				GetRawToken( FToken& Token );
	void			UngetToken( FToken& Token );
	INT				GetIdentifier( FToken& Token, INT NoConsts=0 );
	UClass*			GetQualifiedClass( const char* Thing );
	UBOOL			GetSymbol( FToken& Token );
	void			CheckAllow( const char* Thing, int AllowFlags );
	void			CheckInScope( UObject* Obj );
	UField*			FindField( UStruct* InScope, const char* InIdentifier, UClass* FieldClass=UField::StaticClass, const char* Thing=NULL );
	INT				ConversionCost( const FPropertyBase& Dest, const FPropertyBase& Source );
	void			SkipStatements( int SubCount, const char* ErrorTag );
	UBOOL			GetVarType( UStruct* Scope, FPropertyBase& VarProperty, DWORD& ObjectFlags, DWORD Disallow, const char* Thing );
	UProperty*      GetVarNameAndDim( UStruct* Struct, FPropertyBase& VarProperty, DWORD ObjectFlags, UBOOL NoArrays, UBOOL IsFunction, const char* HardcodedName, const char* Thing, FName Category, UBOOL Skip );

	// Low-level parsing functions.
	const char*		NestTypeName( ENestType NestType );
	char			GetChar( UBOOL Literal=0 );
	char			PeekChar();
	char			GetLeadingChar();
	void			UngetChar();
	INT				IsEOL( char c );
	void VARARGS	AddResultText( char* Fmt, ... );
	INT				GetConstInt( int& Result, const char* Tag=NULL );
	INT				GetConstFloat( FLOAT& Result, const char* Tag=NULL );
	const char*		FunctionNameCpp( UFunction* Function );

	// Nest management functions.
	void			PushNest( ENestType NestType, FName ThisName, UStruct* InNode );
	void			PopNest( ENestType NestType, const char* Descr );
	INT				FindNest( ENestType NestType );

	// Matching predefined text.
	INT				MatchIdentifier( const char* Match );
	INT				MatchSymbol( const char* Match );
	INT				PeekSymbol( const char* Match );
	INT				PeekIdentifier( const char* Match );

	// Requiring predefined text.
	void			RequireIdentifier( const char* Match, const char* Tag );
	void			RequireSymbol( const char* Match, const char* Tag );
	void			RequireSizeOfParm( FToken& TypeToken, const char* Tag );

	// Retry functions.
	void			InitRetry( FRetryPoint& Retry );
	void			PerformRetry( FRetryPoint& Retry, UBOOL Binary=1, UBOOL Text=1 );
	void			CodeSwitcheroo( FRetryPoint& LowRetry, FRetryPoint& HighRetry );

	// Emitters.
	void			EmitConstant( FToken& ConstToken );
	void			EmitStackNodeLinkFunction( UFunction* Node, UBOOL ForceFinal, UBOOL Global );
	void			EmitAddressToFixupLater( FNestInfo* Nest, EFixupType Type, FName Name );
	void			EmitAddressToChainLater( FNestInfo* Nest );
	void			EmitChainUpdate( FNestInfo* Nest );
	void			EmitSize( int Size, const char* Tag );
	void			EmitLet( const FPropertyBase& Type, const char* Tag );
};

/*-----------------------------------------------------------------------------
	FScriptWriter inlines.
-----------------------------------------------------------------------------*/

inline FArchive& FScriptWriter::Serialize( void* V, int Length )
{
	INT iStart = Compiler.TopNode->Script.Add( Length );
	appMemcpy( &Compiler.TopNode->Script(iStart), V, Length );
	return *this;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
