/*=============================================================================
	UnType.h: Unreal engine base type definitions.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

/*-----------------------------------------------------------------------------
	UProperty.
-----------------------------------------------------------------------------*/

//
// An UnrealScript variable.
//
class CORE_API UProperty : public UField
{
	DECLARE_ABSTRACT_CLASS(UProperty,UField,0)

	// Persistent variables.
	INT		ArrayDim;
	DWORD	PropertyFlags;
	FName	Category;
	_WORD	RepOffset;

	// In memory variables.
	INT		Offset;

	// Constructors.
	UProperty();
	UProperty( ECppProperty, INT InOffset, const char* InCategory, DWORD InFlags )
	:	UField( NULL )
	,	ArrayDim( 1 )
	,	PropertyFlags( InFlags )
	,	Category( InCategory )
	,	Offset( InOffset )
	{
		Next = GetParentStruct()->Children;
		GetParentStruct()->Children = this;
	}

	// UObject interface.
	void Serialize( FArchive& Ar );

	// UProperty interface.
	virtual UBOOL Identical( const void* A, const void* B ) const=0;
	virtual void ExportCPP( FOutputDevice& Out, UBOOL IsLocal, UBOOL IsParm );
	virtual void ExportCPPItem( FOutputDevice& Out ) const=0;
	virtual INT GetElementSize() const=0;
	virtual void SerializeItem( FArchive& Ar, void* Value ) const=0;
	virtual void ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable )=0;
	virtual const char* ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const=0;
	virtual UBOOL ExportText( INT ArrayElement, char* ValueStr, BYTE* Data, BYTE* Delta, UBOOL HumanReadable );
	virtual void Link( FArchive& Ar, UProperty* Prev )=0;
	virtual void ExecLet( void* Var, FFrame& Stack );
	virtual UBOOL Port()
	{
		return 
		(	GetSize()
		&&	(Category!=NAME_None || !(PropertyFlags & (CPF_Transient | CPF_Intrinsic)))
		&&	GetFName()!=NAME_Class );
	}
	virtual BYTE GetID()
	{
		return GetClass()->GetFName().GetIndex();
	}

	// Inlines.
	UBOOL Matches( const void* A, const void* B, INT ArrayIndex )
	{
		INT Ofs = Offset + ArrayIndex * GetElementSize();
		return Identical( (BYTE*)A + Ofs, B ? (BYTE*)B + Ofs : NULL );
	}
	UStruct* GetParentStruct()
	{
		check(CastChecked<UStruct>(GetParent())!=NULL);
		return (UStruct*)GetParent();
	}
	INT GetSize()
	{
		return ArrayDim * GetElementSize();
	}
};

/*-----------------------------------------------------------------------------
	UByteProperty.
-----------------------------------------------------------------------------*/

//
// Describes an unsigned byte value or 255-value enumeration variable.
//
class CORE_API UByteProperty : public UProperty
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UByteProperty,UProperty,0)

	// Variables.
	UEnum* Enum;

	// Constructors.
	UByteProperty()
	{}
	UByteProperty( ECppProperty, INT InOffset, const char* InCategory, DWORD InFlags, UEnum* InEnum=NULL )
	:	UProperty( EC_CppProperty, InOffset, InCategory, InFlags )
	,	Enum( InEnum )
	{}

	// UObject interface.
	void Serialize( FArchive& Ar );

	// UProperty interface.
	INT GetElementSize() const
	{
		return sizeof(BYTE);
	}
	void Link( FArchive& Ar, UProperty* Prev )
	{
		Offset = Align( GetParentStruct()->PropertiesSize, sizeof(BYTE) );
	}
	UBOOL Identical( const void* A, const void* B ) const
	{
		return *(BYTE*)A == (B ? *(BYTE*)B : 0);
	}
	void SerializeItem( FArchive& Ar, void* Value ) const
	{
		Ar << *(BYTE*)Value;
	}
	void ExportCPPItem( FOutputDevice& Out ) const;
	void ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable );
	const char* ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const;
};

/*-----------------------------------------------------------------------------
	UIntProperty.
-----------------------------------------------------------------------------*/

//
// Describes a 32-bit signed integer variable.
//
class CORE_API UIntProperty : public UProperty
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UIntProperty,UProperty,0)

	// Constructors.
	UIntProperty()
	{}
	UIntProperty( ECppProperty, INT InOffset, const char* InCategory, DWORD InFlags )
	:	UProperty( EC_CppProperty, InOffset, InCategory, InFlags )
	{}

	// UProperty interface.
	INT GetElementSize() const
	{
		return sizeof(INT);
	}
	void Link( FArchive& Ar, UProperty* Prev )
	{
		Offset = Align( GetParentStruct()->PropertiesSize, sizeof(INT) );
	}
	UBOOL Identical( const void* A, const void* B ) const
	{
		return *(INT*)A == (B ? *(INT*)B : 0);
	}
	void SerializeItem( FArchive& Ar, void* Value ) const
	{
		Ar << *(INT*)Value;
	}
	void ExportCPPItem( FOutputDevice& Out ) const;
	void ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable );
	const char* ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const;
};

/*-----------------------------------------------------------------------------
	UBoolProperty.
-----------------------------------------------------------------------------*/

//
// Describes a single bit flag variable residing in a 32-bit unsigned double word.
//
class CORE_API UBoolProperty : public UProperty
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UBoolProperty,UProperty,0)

	// Variables.
	DWORD BitMask;

	// Constructors.
	UBoolProperty()
	{}
	UBoolProperty( ECppProperty, INT InOffset, const char* InCategory, DWORD InFlags )
	:	UProperty( EC_CppProperty, InOffset, InCategory, InFlags )
	,	BitMask( 1 )
	{}

	// UObject interface.
	void Serialize( FArchive& Ar );

	// UProperty interface.
	INT GetElementSize() const
	{
		return sizeof(DWORD);
	}
	void Link( FArchive& Ar, UProperty* Prev )
	{
		UBoolProperty* PrevBool = Cast<UBoolProperty>( Prev );
		if( GetParentStruct()->MergeBools() && PrevBool && NEXT_BITFIELD(PrevBool->BitMask) )
		{
			Offset  = Prev->Offset;
			BitMask = NEXT_BITFIELD(PrevBool->BitMask);
		}
		else
		{
			Offset  = Align(GetParentStruct()->PropertiesSize,sizeof(DWORD));
			BitMask = FIRST_BITFIELD;
		}
	}
	UBOOL Identical( const void* A, const void* B ) const
	{
		return ((*(DWORD*)A ^ (B ? *(DWORD*)B : 0)) & BitMask) == 0;
	}
	void SerializeItem( FArchive& Ar, void* Value ) const
	{
		BYTE B = (*(DWORD*)Value & BitMask) ? 1 : 0;
		Ar << B;
		if( B ) *(DWORD*)Value |=  BitMask;
		else    *(DWORD*)Value &= ~BitMask;
	}
	void ExportCPPItem( FOutputDevice& Out ) const;
	void ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable );
	const char* ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const;
	void ExecLet( void* Var, FFrame& Stack );
};

/*-----------------------------------------------------------------------------
	UFloatProperty.
-----------------------------------------------------------------------------*/

//
// Describes an IEEE 32-bit floating point variable.
//
class CORE_API UFloatProperty : public UProperty
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UFloatProperty,UProperty,0)

	// Constructors.
	UFloatProperty()
	{}
	UFloatProperty( ECppProperty, INT InOffset, const char* InCategory, DWORD InFlags )
	:	UProperty( EC_CppProperty, InOffset, InCategory, InFlags )
	{}

	// UProperty interface.
	INT GetElementSize() const
	{
		return sizeof(FLOAT);
	}
	void Link( FArchive& Ar, UProperty* Prev )
	{
		Offset = Align( GetParentStruct()->PropertiesSize, sizeof(FLOAT) );
	}
	UBOOL Identical( const void* A, const void* B ) const
	{
		return *(FLOAT*)A == (B ? *(FLOAT*)B : 0);
	}
	void SerializeItem( FArchive& Ar, void* Value ) const
	{
		Ar << *(FLOAT*)Value;
	}
	void ExportCPPItem( FOutputDevice& Out ) const;
	void ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable );
	const char* ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const;
};

/*-----------------------------------------------------------------------------
	UObjectProperty.
-----------------------------------------------------------------------------*/

//
// Describes a reference variable to another object which may be nil.
//
class CORE_API UObjectProperty : public UProperty
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UObjectProperty,UProperty,0)

	// Variables.
	class UClass* PropertyClass;
	UObjectProperty* NextReference;

	// Constructors.
	UObjectProperty()
	{}
	UObjectProperty( ECppProperty, INT InOffset, const char* InCategory, DWORD InFlags, UClass* InClass )
	:	UProperty( EC_CppProperty, InOffset, InCategory, InFlags )
	,	PropertyClass( InClass )
	{}

	// UObject interface.
	void Serialize( FArchive& Ar );

	// UProperty interface.
	INT GetElementSize() const
	{
		return sizeof(UObject*);
	}
	void Link( FArchive& Ar, UProperty* Prev )
	{
		Offset = Align( GetParentStruct()->PropertiesSize, sizeof(UObject*) );
	}
	UBOOL Identical( const void* A, const void* B ) const
	{
		return *(UObject**)A == (B ? *(UObject**)B : NULL);
	}
	void SerializeItem( FArchive& Ar, void* Value ) const
	{
		Ar << *(UObject**)Value;
	}
	void ExportCPPItem( FOutputDevice& Out ) const;
	void ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable );
	const char* ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const;
};

/*-----------------------------------------------------------------------------
	UObjectProperty.
-----------------------------------------------------------------------------*/

//
// Describes a reference variable to another object which may be nil.
//
class CORE_API UClassProperty : public UObjectProperty
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UClassProperty,UObjectProperty,0)

	// Variables.
	class UClass* MetaClass;

	// Constructors.
	UClassProperty()
	{}
	UClassProperty( ECppProperty, INT InOffset, const char* InCategory, DWORD InFlags, UClass* InMetaClass )
	:	UObjectProperty( EC_CppProperty, InOffset, InCategory, InFlags, UClass::StaticClass )
	,	MetaClass( InMetaClass )
	{}

	// UObject interface.
	void Serialize( FArchive& Ar );

	// UProperty interface.
	const char* ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const;
	BYTE GetID()
	{
		return NAME_ObjectProperty;
	}
};

/*-----------------------------------------------------------------------------
	UNameProperty.
-----------------------------------------------------------------------------*/

//
// Describes a name variable pointing into the global name table.
//
class CORE_API UNameProperty : public UProperty
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UNameProperty,UProperty,0)

	// Constructors.
	UNameProperty()
	{}
	UNameProperty( ECppProperty, INT InOffset, const char* InCategory, DWORD InFlags )
	:	UProperty( EC_CppProperty, InOffset, InCategory, InFlags )
	{}

	// UProperty interface.
	INT GetElementSize() const
	{
		return sizeof(FName);
	}
	void Link( FArchive& Ar, UProperty* Prev )
	{
		Offset = Align( GetParentStruct()->PropertiesSize, sizeof(FName) );
	}
	UBOOL Identical( const void* A, const void* B ) const
	{
		return *(FName*)A == (B ? *(FName*)B : NAME_None);
	}
	void SerializeItem( FArchive& Ar, void* Value ) const
	{
		Ar << *(FName*)Value;
	}
	void ExportCPPItem( FOutputDevice& Out ) const;
	void ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable );
	const char* ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const;
};

/*-----------------------------------------------------------------------------
	UStringProperty.
-----------------------------------------------------------------------------*/

//
// Describes a string variable.
//
class CORE_API UStringProperty : public UProperty
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UStringProperty,UProperty,0)

	// Variables.
	INT StringSize;

	// Constructors.
	UStringProperty()
	{}
	UStringProperty( ECppProperty, INT InOffset, const char* InCategory, DWORD InFlags, INT InStringSize )
	:	UProperty( EC_CppProperty, InOffset, InCategory, InFlags )
	,	StringSize( InStringSize )
	{}

	// UObject interface.
	void Serialize( FArchive& Ar );

	// UProperty interface.
	INT GetElementSize() const
	{
		return StringSize;
	}
	void Link( FArchive& Ar, UProperty* Prev )
	{
		Offset = Align( GetParentStruct()->PropertiesSize, sizeof(char) );
	}
	UBOOL Identical( const void* A, const void* B ) const
	{
		return appStricmp((const char*)A,B ? (const char*)B : "")==0;
	}
	void SerializeItem( FArchive& Ar, void* Value ) const
	{
		Ar.String( (char*)Value, StringSize );
	}
	void ExportCPPItem( FOutputDevice& Out ) const;
	void ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable );
	const char* ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const;
	void ExecLet( void* Var, FFrame& Stack );
};

/*-----------------------------------------------------------------------------
	UStructProperty.
-----------------------------------------------------------------------------*/

//
// Describes a structure variable embedded in (as opposed to referenced by) 
// an object.
//
class CORE_API UStructProperty : public UProperty
{
	DECLARE_CLASS(UStructProperty,UProperty,0)

	// Variables.
	class UStruct* Struct;
	UStructProperty* NextStruct;

	// Constructors.

	UStructProperty( ECppProperty, INT InOffset, const char* InCategory, DWORD InFlags, UStruct* InStruct )
	:	UProperty( EC_CppProperty, InOffset, InCategory, InFlags )
	,	Struct( InStruct )
	{}

	// UObject interface.
	void Serialize( FArchive& Ar );

	// UProperty interface.
	INT GetElementSize() const
	{
		return Struct->PropertiesSize;
	}
	void Link( FArchive& Ar, UProperty* Prev )
	{
		Ar.Preload( Struct );
		if( GetElementSize()==2 )
			Offset = Align( GetParentStruct()->PropertiesSize, 2 );
		else if( GetElementSize()>=4 )
			Offset = Align( GetParentStruct()->PropertiesSize, 4 );
		else
			Offset = GetParentStruct()->PropertiesSize;
	}
	UBOOL Identical( const void* A, const void* B ) const
	{
		for( TFieldIterator<UProperty> It(Struct); It; ++It )
			for( INT i=0; i<It->ArrayDim; i++ )
				if( !It->Matches(A,B,i) )
					return 0;
		return 1;
	}
	void SerializeItem( FArchive& Ar, void* Value ) const
	{
		Ar.Preload( Struct );
		Struct->SerializeBin( Ar, (BYTE*)Value );
	}
	void ExportCPPItem( FOutputDevice& Out ) const;
	void ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable );
	const char* ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const;
};

/*-----------------------------------------------------------------------------
	Field templates.
-----------------------------------------------------------------------------*/

//
// Find a typed field in a struct.
//
template <class T> T* FindField( UStruct* Owner, const char* FieldName )
{
	guard(FindField);
	for( TFieldIterator<T>It( Owner ); It; ++It )
		if( appStricmp( It->GetName(), FieldName )==0 )
			return *It;
	return NULL;
	unguard;
}

/*-----------------------------------------------------------------------------
	UObject accessors that depend on UClass.
-----------------------------------------------------------------------------*/

//
// See if this object belongs to the specified class.
//
inline UBOOL UObject::IsA( class UClass* SomeBase ) const
{
	guardSlow(UObject::IsA);
	for( UClass* TempClass=Class; TempClass; TempClass=(UClass*)TempClass->SuperField )
		if( TempClass==SomeBase )
			return 1;
	return SomeBase==NULL;
	unguardSlow;
}

//
// See if this object is in a certain package.
//
inline UBOOL UObject::IsIn( class UObject* SomeParent ) const
{
	guardSlow(UObject::IsA);
	for( UObject* It=GetParent(); It; It=It->GetParent() )
		if( It==SomeParent )
			return 1;
	return SomeParent==NULL;
	unguardSlow;
}

//
// Return whether an object wants to receive a named probe message.
//
inline UBOOL UObject::IsProbing( FName ProbeName )
{
	guardSlow(UObject::IsProbing);
	return	(ProbeName.GetIndex() <  NAME_PROBEMIN)
	||		(ProbeName.GetIndex() >= NAME_PROBEMAX)
	||		(!MainFrame)
	||		(MainFrame->ProbeMask & ((QWORD)1 << (ProbeName.GetIndex() - NAME_PROBEMIN)));
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	UStruct inlines.
-----------------------------------------------------------------------------*/

//
// UStruct inline comparer.
//
inline UBOOL UStruct::StructCompare( const void* A, const void* B )
{
	guardSlow(UStruct::StructCompare);
	for( TFieldIterator<UProperty> It(this); It; ++It )
		for( INT i=0; i<It->ArrayDim; i++ )
			if( !It->Matches(A,B,i) )
				return 0;
	unguardSlow;
	return 1;
}

/*-----------------------------------------------------------------------------
	C++ property macros.
-----------------------------------------------------------------------------*/

#define CPP_PROPERTY(name) \
	EC_CppProperty, (BYTE*)&((ThisClass*)NULL)->name - (BYTE*)NULL

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
