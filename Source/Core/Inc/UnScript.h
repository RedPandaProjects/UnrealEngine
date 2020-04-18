/*=============================================================================
	UnScript.h: UnrealScript execution engine.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Constants & types.
-----------------------------------------------------------------------------*/

// Sizes.
enum {MAX_STRING_CONST_SIZE	= 256      };
enum {MAX_CONST_SIZE        = 16       };
enum {MAX_FUNC_PARMS        = 16       };

//
// guardexec mechanism for script debugging.
//
#define unguardexecSlow  unguardfSlow(( "(%s @ %s : %04X)", Stack.Object->GetFullName(), Stack.Node->GetFullName(), Stack.Code - &Stack.Node->Script(0) ))
#define unguardexec      unguardf    (( "(%s @ %s : %04X)", Stack.Object->GetFullName(), Stack.Node->GetFullName(), Stack.Code - &Stack.Node->Script(0) ))

//
// State flags.
//
enum EStateFlags
{
	// State flags.
	STATE_Editable		= 0x0001,	// State should be user-selectable in UnrealEd.
	STATE_Auto			= 0x0002,	// State is automatic (the default state).
	STATE_Simulated     = 0x0004,   // State executes on client side.
};

//
// Function flags.
//
enum EFunctionFlags
{
	// Function flags.
	FUNC_Final			= 0x0001,	// Function is final (prebindable, non-overridable function).
	FUNC_Defined		= 0x0002,	// Function has been defined (not just declared).
	FUNC_Iterator		= 0x0004,	// Function is an iterator.
	FUNC_Latent		    = 0x0008,	// Function is a latent state function.
	FUNC_PreOperator	= 0x0010,	// Unary operator is a prefix operator.
	FUNC_Singular       = 0x0020,   // Function cannot be reentered.
	FUNC_Net            = 0x0040,   // Function is network-replicated.
	FUNC_NetReliable    = 0x0080,   // Function should be sent reliably on the network.
	FUNC_Simulated		= 0x0100,	// Function executed on the client side.
	FUNC_Exec		    = 0x0200,	// Executable from command line.
	FUNC_Intrinsic		= 0x0400,	// Intrinsic function.
	FUNC_Event          = 0x0800,   // Event function.
	FUNC_Operator       = 0x1000,   // Operator function.
	FUNC_Static         = 0x2000,   // Static function.

	// Combinations of flags.
	FUNC_FuncInherit        = FUNC_Exec | FUNC_Event,
	FUNC_FuncOverrideMatch	= FUNC_Exec | FUNC_Final | FUNC_Latent | FUNC_PreOperator | FUNC_Iterator | FUNC_Static,
	FUNC_NetFuncFlags       = FUNC_Net | FUNC_NetReliable,
};

//
// Evaluatable expression item types.
//
enum EExprToken
{
	// Variable references which map onto propery bin numbers.
	EX_LocalVariable		= 0x00,	// A local variable.
	EX_InstanceVariable		= 0x01,	// An object variable.
	EX_DefaultVariable		= 0x02,	// Default variable for a concrete object.

	// Tokens.
	EX_Return				= 0x04,	// Return from function.
	EX_Switch				= 0x05,	// Switch.
	EX_Jump					= 0x06,	// Goto a local address in code.
	EX_JumpIfNot			= 0x07,	// Goto if not expression.
	EX_Stop					= 0x08,	// Stop executing state code.
	EX_Assert				= 0x09,	// Assertion.
	EX_Case					= 0x0A,	// Case.
	EX_Nothing				= 0x0B,	// No operation.
	EX_LabelTable			= 0x0C,	// Table of labels.
	EX_GotoLabel			= 0x0D,	// Goto a label.
	EX_ValidateObject       = 0x0E, // Object variable.
	EX_Let					= 0x0F,	// Assign an arbitrary size value to a variable.
	EX_ClassContext         = 0x12, // Class default metaobject context.
	EX_MetaCast             = 0x13, // Metaclass cast.
	EX_BeginFunction		= 0x14,	// Beginning of function in code.
	EX_EndCode				= 0x15,	// End of code block.
	EX_EndFunctionParms		= 0x16,	// End of function call parameters.
	EX_Self					= 0x17,	// Self object.
	EX_Skip					= 0x18,	// Skippable expression.
	EX_Context				= 0x19,	// Call a function through an object context.
	EX_ArrayElement			= 0x1A,	// Array element.
	EX_VirtualFunction		= 0x1B,	// A function call with parameters.
	EX_FinalFunction		= 0x1C,	// A prebound function call with parameters.
	EX_IntConst				= 0x1D,	// Int constant.
	EX_FloatConst			= 0x1E,	// Floating point constant.
	EX_StringConst			= 0x1F,	// String constant.
	EX_ObjectConst		    = 0x20,	// An object constant.
	EX_NameConst			= 0x21,	// A name constant.
	EX_RotationConst		= 0x22,	// A rotation constant.
	EX_VectorConst			= 0x23,	// A vector constant.
	EX_ByteConst			= 0x24,	// A byte constant.
	EX_IntZero				= 0x25,	// Zero.
	EX_IntOne				= 0x26,	// One.
	EX_True					= 0x27,	// Bool True.
	EX_False				= 0x28,	// Bool False.
	EX_IntrinsicParm        = 0x29, // Intrinsic parameter offset.
	EX_NoObject				= 0x2A,	// NoObject.
	EX_ResizeString			= 0x2B,	// Resize a string's length.
	EX_IntConstByte			= 0x2C,	// Int constant that requires 1 byte.
	EX_BoolVariable			= 0x2D,	// A bool variable which requires a bitmask.
	EX_DynamicCast			= 0x2E,	// Safe dynamic class casting.
	EX_Iterator             = 0x2F, // Begin an iterator operation.
	EX_IteratorPop          = 0x30, // Pop an iterator level.
	EX_IteratorNext         = 0x31, // Go to next iteration.
	EX_StructCmpEq          = 0x32,	// Struct binary compare-for-equal.
	EX_StructCmpNe          = 0x33,	// Struct binary compare-for-unequal.
	EX_StructConst			= 0x34,	// Struct constant.
	EX_StructMember         = 0x36, // Struct member.
	EX_GlobalFunction		= 0x38, // Call non-state version of a function.

	// Intrinsic conversions.
	EX_MinConversion		= 0x39,	// Minimum conversion token.
	EX_RotationToVector		= 0x39,
	EX_ByteToInt			= 0x3A,
	EX_ByteToBool			= 0x3B,
	EX_ByteToFloat			= 0x3C,
	EX_IntToByte			= 0x3D,
	EX_IntToBool			= 0x3E,
	EX_IntToFloat			= 0x3F,
	EX_BoolToByte			= 0x40,
	EX_BoolToInt			= 0x41,
	EX_BoolToFloat			= 0x42,
	EX_FloatToByte			= 0x43,
	EX_FloatToInt			= 0x44,
	EX_FloatToBool			= 0x45,
	EX_ObjectToBool			= 0x47,
	EX_NameToBool			= 0x48,
	EX_StringToByte			= 0x49,
	EX_StringToInt			= 0x4A,
	EX_StringToBool			= 0x4B,
	EX_StringToFloat		= 0x4C,
	EX_StringToVector		= 0x4D,
	EX_StringToRotation		= 0x4E,
	EX_VectorToBool			= 0x4F,
	EX_VectorToRotation		= 0x50,
	EX_RotationToBool		= 0x51,
	EX_ByteToString			= 0x52,
	EX_IntToString			= 0x53,
	EX_BoolToString			= 0x54,
	EX_FloatToString		= 0x55,
	EX_ObjectToString		= 0x56,
	EX_NameToString			= 0x57,
	EX_VectorToString		= 0x58,
	EX_RotationToString		= 0x59,
	EX_MaxConversion		= 0x5A,	// Maximum conversion token.

	// Intrinsics.
	EX_ExtendedIntrinsic	= 0x60,
	EX_FirstIntrinsic		= 0x70,
	EX_FirstPhysics			= 0xF80,
	EX_Max					= 0x1000,
};

//
// Latent functions.
//
enum EPollSlowFuncs
{
	EPOLL_Sleep			      = 384,
	EPOLL_FinishAnim	      = 385,
	EPOLL_FinishInterpolation = 302,
};

/*-----------------------------------------------------------------------------
	Intrinsic functions.
-----------------------------------------------------------------------------*/

//
// Intrinsic function table.
//
extern CORE_API void (UObject::*GIntrinsics[])( FFrame& Stack, BYTE*& Result );
BYTE CORE_API GRegisterIntrinsic( int iIntrinsic, void* Func );

//
// Registering an intrinsic function.
//
#define AUTOREGISTER_INTRINSIC(cls,num,func) \
	extern "C" DLL_EXPORT void (cls::*int##cls##func)( FFrame& Stack, BYTE*& Result ) =&cls::func; \
	static BYTE func##Temp = GRegisterIntrinsic(num,*(void**)&int##cls##func);

/*-----------------------------------------------------------------------------
	Macros.
-----------------------------------------------------------------------------*/

//
// Macros for grabbing parameters for intrinsic functions.
//
#define P_GET_INT(var)              INT           var;   {INT *Ptr=&var;       Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_INT_OPT(var,def)      INT       var=def;   {INT *Ptr=&var;       Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_INT_REF(var)          INT   a##var=0,*var=&a##var;              {Stack.Step( Stack.Object, *(BYTE**)&var );          }
#define P_GET_UBOOL(var)            DWORD         var;   {DWORD *Ptr=&var;     Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_UBOOL_OPT(var,def)    DWORD     var=def;   {DWORD *Ptr=&var;     Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_UBOOL_REF(var)        DWORD a##var=0,*var=&a##var;              {Stack.Step( Stack.Object, *(BYTE**)&var );          }
#define P_GET_FLOAT(var)            FLOAT         var;   {FLOAT *Ptr=&var;     Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_FLOAT_OPT(var,def)    FLOAT     var=def;   {FLOAT *Ptr=&var;     Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_FLOAT_REF(var)        FLOAT a##var=0.0,*var=&a##var;            {Stack.Step( Stack.Object, *(BYTE**)&var );          }
#define P_GET_BYTE(var)             BYTE          var;   {BYTE *Ptr=&var;      Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_BYTE_OPT(var,def)     BYTE      var=def;   {BYTE *Ptr=&var;      Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_BYTE_REF(var)         BYTE  a##var=0,*var=&a##var;              {Stack.Step( Stack.Object, *(BYTE**)&var );          }
#define P_GET_NAME(var)             FName         var;   {FName *Ptr=&var;     Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_NAME_OPT(var,def)     FName     var=def;   {FName *Ptr=&var;     Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_NAME_REF(var)         FName a##var=NAME_None,*var=&a##var;      {Stack.Step( Stack.Object, *(BYTE**)&var );          }
#define P_GET_ACTOR(var)            AActor       *var;   {AActor **Ptr=&var;   Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_ACTOR_OPT(var,def)    AActor   *var=def;   {AActor **Ptr=&var;   Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_ACTOR_REF(var)        AActor *a##var=NULL,**var=&a##var;        {Stack.Step( Stack.Object, *(BYTE**)&var );          }
#define P_GET_VECTOR(var)           FVector       var;   {FVector *Ptr=&var;   Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_VECTOR_OPT(var,def)   FVector   var=def;   {FVector *Ptr=&var;   Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_VECTOR_REF(var)       FVector a##var(0,0,0),*var=&a##var;       {Stack.Step( Stack.Object, *(BYTE**)&var );          }
#define P_GET_ROTATOR(var)          FRotator      var;   {FRotator *Ptr=&var; Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_ROTATOR_OPT(var,def)  FRotator  var=def;   {FRotator *Ptr=&var; Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_ROTATOR_REF(var)      FRotator  a##var(0,0,0),*var=&a##var;     {Stack.Step( Stack.Object, *(BYTE**)&var );          }
#define P_GET_OBJECT(cls,var)       cls          *var;   {cls**Ptr=&var;       Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_OBJECT_OPT(var,def)   UObject*var=def;     {UObject**Ptr=&var;   Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_OBJECT_REF(var)       UObject*a##var=NULL,**var=&a##var;        {Stack.Step( Stack.Object, *(BYTE**)&var );          }
#define P_GET_STRING(var)           CHAR var##T[MAX_STRING_CONST_SIZE], *var=var##T; {Stack.Step( Stack.Object,*(BYTE**)&var);     }
#define P_GET_STRING_OPT(var,def)   CHAR var##T[MAX_STRING_CONST_SIZE]=def, *var=var##T; {Stack.Step( Stack.Object,*(BYTE**)&var); }
#define P_GET_STRING_REF(var)       CHAR a##var[MAX_STRING_CONST_SIZE],*var=a##var; {Stack.Step( Stack.Object, *(BYTE**)&var );          }
#define P_GET_STRUCT(typ,var)       typ var; {typ *Ptr=&var; Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_STRUCT_OPT(typ,var,def) typ var=def; {typ *Ptr=&var; Stack.Step( Stack.Object, *(BYTE**)&Ptr ); var=*Ptr;}
#define P_GET_STRUCT_REF(typ,var)   typ a##var,*var=&a##var; {Stack.Step( Stack.Object, *(BYTE**)&var );}
#define P_GET_SKIP_OFFSET(var)      _WORD var; {debug(*Stack.Code==EX_Skip); Stack.Code++; var=*(_WORD*)Stack.Code; Stack.Code+=2; }
#define P_FINISH                    {Stack.Code++;}

//
// Iterator macros.
//
#define PRE_ITERATOR \
	INT wEndOffset = Stack.ReadWord(); \
	BYTE B=0, *Addr, Buffer[MAX_CONST_SIZE]; \
	BYTE *StartCode = Stack.Code; \
	do {
#define POST_ITERATOR \
		while( (B=*Stack.Code)!=EX_IteratorPop && B!=EX_IteratorNext ) \
			Stack.Step( Stack.Object, Addr=Buffer ); \
		if( *Stack.Code++==EX_IteratorNext ) \
			Stack.Code = StartCode; \
	} while( B != EX_IteratorPop );

/*-----------------------------------------------------------------------------
	FFrame implementation.
-----------------------------------------------------------------------------*/

inline FFrame::FFrame( UObject* InObject )
:	Node		(InObject ? InObject->GetClass() : NULL)
,	Object		(InObject)
,	Code		(NULL)
,	Locals		(NULL)
{}
inline FFrame::FFrame( UObject* InObject, UStruct* InNode, INT CodeOffset, BYTE* InLocals )
:	Node		(InNode)
,	Object		(InObject)
,	Code		(&InNode->Script(CodeOffset))
,	Locals		(InLocals)
{}
inline void FFrame::Step( UObject* Context, BYTE*& Result )
{
	guardSlow(FFrame::Step);
	BYTE B = *Code++;
	(Context->*GIntrinsics[B])( *this, Result );
	unguardSlow;
}
inline INT FFrame::ReadInt()
{
	INT Result = *(INT*)Code;
	Code += sizeof(INT);
	return Result;
}
inline FLOAT FFrame::ReadFloat()
{
	FLOAT Result = *(FLOAT*)Code;
	Code += sizeof(FLOAT);
	return Result;
}
inline INT FFrame::ReadWord()
{
	INT Result = *(_WORD*)Code;
	Code += sizeof(_WORD);
	return Result;
}
inline FName FFrame::ReadName()
{
	FName Result = *(FName*)Code;
	Code += sizeof(FName);
	return Result;
}
CORE_API void GInitRunaway();

/*-----------------------------------------------------------------------------
	FMainFrame implementation.
-----------------------------------------------------------------------------*/

inline FMainFrame::FMainFrame( UObject* InObject )
:	FFrame	( InObject )
,	StateNode	( InObject->GetClass() )
,	ProbeMask	( ~(QWORD)0 )
{}
inline const char* FMainFrame::Describe()
{
	return Node ? Node->GetFullName() : "None";
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
