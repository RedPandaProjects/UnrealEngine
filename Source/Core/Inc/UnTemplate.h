/*=============================================================================
	UnTemplate.h: Unreal templates.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Standard templates.
-----------------------------------------------------------------------------*/

template< class T > inline T Abs( const T A )
{
	return (A>=(T)0) ? A : -A;
}
template< class T > inline T Sgn( const T A )
{
	return (A>0) ? 1 : ((A<0) ? -1 : 0);
}
template< class T > inline T Max( const T A, const T B )
{
	return (A>=B) ? A : B;
}
template< class T > inline T Min( const T A, const T B )
{
	return (A<=B) ? A : B;
}
template< class T > inline T Square( const T A )
{
	return A*A;
}
template< class T > inline T Clamp( const T X, const T Min, const T Max )
{
	return X<Min ? Min : X<Max ? X : Max;
}
template< class T > inline T Align( const T Ptr, INT Alignment )
{
	return (T)(((DWORD)Ptr + Alignment - 1) & ~(Alignment-1));
}
template< class T > inline void Exchange( T& A, T& B )
{
	const T Temp = A;
	A = B;
	B = Temp;
}
template< class T > T Lerp( T& A, T& B, FLOAT Alpha )
{
	return A + Alpha * (B-A);
}

/*----------------------------------------------------------------------------
	Standard macros.
----------------------------------------------------------------------------*/

// Number of elements in an array.
#define ARRAY_COUNT( array ) \
	( sizeof(array) / sizeof((array)[0]) )

// Offset of a struct member.
#define STRUCT_OFFSET( struc, member ) \
	( (int)&((struc*)NULL)->member )

/*-----------------------------------------------------------------------------
	Dynamic array template.
-----------------------------------------------------------------------------*/

//
// Base dynamic array.
//
class CORE_API FArray
{
public:
	void* GetData()
	{
		return Data;
	}
	void Remove( INT Index, INT Count, INT ElementSize );

	void Realloc( INT ElementSize );
protected:
	FArray( INT InNum, INT ElementSize )
	:	ArrayNum( InNum )
	,	ArrayMax( InNum )
	,	Data    ( NULL  )
	{
		Realloc( ElementSize );
	}
	~FArray()
	{
		if( Data )
			appFree( Data );
	}
	void* Data;
public:
	INT	  ArrayNum;
	INT	  ArrayMax;
};

//
// Templated dynamic array.
//
#define checkarray debug /* For debugging array code. */
template< class T > class TArray : public FArray
{
public:
	TArray( INT InNum=0 )
	:	FArray( InNum, sizeof(T) )
	{}
	TArray( const TArray& Other )
	:	FArray( Other.ArrayNum, sizeof(T) )
	{
		guardSlow(TArray::copyctor);
		ArrayNum=0;
		for( INT i=0; i<Other.ArrayNum; i++ )
			new(*this)T(Other(i));
		unguardSlow;
	}
	TArray& operator=( const TArray& Other )
	{
		guardSlow(TArray::operator=);
		if( this != &Other )
		{
			ArrayNum = 0;
			ArrayMax = Other.ArrayNum;
			Realloc( sizeof(T) );
			for( INT i=0; i<Other.ArrayNum; i++ )
				new(*this)T(Other(i));
		}
		return *this;
		unguardSlow;
	}
	~TArray()
	{
		checkarray(ArrayNum>=0);
		checkarray(ArrayMax>=ArrayNum);
		Remove( 0, ArrayNum );
	}
    T& operator()( int i )
	{
		guardSlow(TArray::());
		checkarray(i>=0);
		checkarray(i<=ArrayNum);
		checkarray(ArrayMax>=ArrayNum);
		return ((T*)Data)[i];
		unguardSlow;
	}
	const T& operator()( int i ) const
	{
		checkarray(i>=0);
		checkarray(i<=ArrayNum);
		checkarray(ArrayMax>=ArrayNum);
		return ((T*)Data)[i];
	}
	INT Num() const
	{
		checkarray(ArrayNum>=0);
		checkarray(ArrayMax>=ArrayNum);
		return ArrayNum;
	}
	UBOOL IsValidIndex( int i ) const
	{
		return i>=0 && i<ArrayNum;
	}
	void Empty()
	{
		guardSlow(TArray::Empty);
		Remove( 0, ArrayNum );
		unguardSlow;
	}
	void Shrink()
	{
		guardSlow(TArray::Shrink);
		checkarray(ArrayNum>=0);
		checkarray(ArrayMax>=ArrayNum);
		if( ArrayMax != ArrayNum )
		{
			ArrayMax = ArrayNum;
			Realloc( sizeof(T) );
		}
		unguardSlow;
	}
	INT AddItem( const T& Item )
	{
		guardSlow(TArray::AddItem);
		INT Index=Add();
		(*this)(Index)=Item;
		return Index;
		unguardSlow;
	}
	INT Add( INT n=1 )
	{
		guardSlow(TArray::Add);
		checkarray(n>=0);
		checkarray(ArrayNum>=0);
		checkarray(ArrayMax>=ArrayNum);
		INT Index=ArrayNum;
		if( (ArrayNum+=n)>ArrayMax )
		{
			ArrayMax = ArrayNum + ArrayNum/4 + 32;
			Realloc( sizeof(T) );
		}
		return Index;
		unguardSlow;
	}
	INT AddZeroed( INT n=1 )
	{
		guardSlow(TArray::AddZeroed);
		INT Index = Add(n);
		appMemset( &(*this)(Index), 0, n*sizeof(T) );
		return Index;
		unguardSlow;
	}
	INT AddUniqueItem( const T& Item )
	{
		guardSlow(TArray::AddUniqueItem);
		for( INT Index=0; Index<ArrayNum; Index++ )
			if( (*this)(Index)==Item )
				return Index;
		return AddItem( Item );
		unguardSlow;
	}
	UBOOL FindItem( const T& Item, INT& Index ) const
	{
		guardSlow(TArray::FindItem);
		for( Index=0; Index<ArrayNum; Index++ )
			if( (*this)(Index)==Item )
				return 1;
		return 0;
		unguardSlow;
	}
	inline INT RemoveItem( const T& Item )
	{
		guardSlow(TArray::RemoveItem);
		INT OriginalNum=ArrayNum;
		for( INT Index=0; Index<ArrayNum; Index++ )
			if( (*this)(Index)==Item )
				Remove( Index-- );
		return OriginalNum - ArrayNum;
		unguardSlow;
	}
	void Remove( int Index, int Count=1 );
	template< class T >
	friend	inline FArchive& operator<<(FArchive& Ar, TArray<T>& A);
	void SetNum( INT NewSize )
	{
		guardSlow(TArray::SetNum);
		if( ArrayNum!=NewSize || ArrayMax!=NewSize )
		{
			ArrayNum = ArrayMax = NewSize;
			Realloc( sizeof(T) );
		}
		unguardSlow;
	}
};

template <class T> void* operator new( size_t Size, TArray<T>& Array )
{
	guardSlow(TArray::operator new);
	INT Index = Array.Add();
	return &Array(Index);
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	Dynamic strings.
-----------------------------------------------------------------------------*/

//
// A dynamically sizeable string.
//
#define checkstr debug
class CORE_API FString : protected TArray<char>
{
public:
	// Constructors.
	FString()
	:	TArray<char>( 1 )
	{
		(*this)(0)=0;
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
	}
	FString( const char* In )
	{
		checkstr(In);
		Add( appStrlen(In)+1 );
		appMemcpy( &(*this)(0), In, Num() );
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
	}
	FString( const FString& Other )
	:	TArray<char>( Other.Num() )
	{
		checkstr(Other.Num());
		appMemcpy( &(*this)(0), &Other(0), Num() );
		checkstr((*this)(Num()-1)==0);
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
	}
	FString& operator=( const FString& Other )
	{
		if( this != &Other )
		{
			checkstr(Other.Num());
			checkstr(Other(Other.Num()-1)==0);
			SetNum( Other.Num() );
			appMemcpy( &(*this)(0), &Other(0), Num() );
			checkstr((*this)(Num()-1)==0);
		}
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
		return *this;
	}

	// TArray overrides.
	void Empty()
	{
		TArray<char>::Empty();
		AddItem(0);
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
	}
	void Shrink()
	{
		checkstr(Num());
		checkstr((*this)(Num()-1)==0);
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
		TArray<char>::Shrink();
	}

	// Conversions.
	const char* operator*() const
	{
		checkstr(Num());
		checkstr((*this)(Num()-1)==0);
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
		return &(*this)(0);
	}
	operator UBOOL() const
	{
		return (*this)(0) != 0;
	}

	// Operators.
	FString& operator +=( const char* Str )
	{
		checkstr(Str);
		checkstr(Num());
		INT Index = Num()-1;
		checkstr((*this)(Index)==0);
		Add(appStrlen(Str));
		appStrcpy( &(*this)(Index), Str );
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
		return *this;
	}
	FString operator +( const char* Str )
	{
		return FString( *this ) += Str;
	}
	FString& operator +=( const FString& Str )
	{
		checkstr(Str.Num());
		checkstr(Num());
		INT Index = Num()-1;
		checkstr((*this)(Index)==0);
		Add(Str.Length());
		appMemcpy( &(*this)(Index), &Str(0), Str.Num() );
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
		return *this;
	}
	FString operator +( const FString& Str )
	{
		return FString( *this ) += Str;
	}
	UBOOL operator==( const char* Other ) const
	{
		checkstr(Num());
		checkstr(Other);
		checkstr((*this)(Num()-1)==0);
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
		return appStricmp( &(*this)(0), Other )==0;
	}
	UBOOL operator==( const FString& Other ) const
	{
		checkstr(Num());
		checkstr(Other.Num());
		checkstr((*this)(Num()-1)==0);
		checkstr(Other(Num()-1)==0);
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
		return appStricmp( &(*this)(0), &Other(0) )==0;
	}
	UBOOL operator!=( const char* Other ) const
	{
		checkstr(Num());
		checkstr(Other);
		checkstr((*this)(Num()-1)==0);
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
		return appStricmp( &(*this)(0), Other )!=0;
	}
	UBOOL operator!=( const FString& Other ) const
	{
		checkstr(Num());
		checkstr(Other.Num());
		checkstr((*this)(Num()-1)==0);
		checkstr(Other(Num()-1)==0);
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
		return appStricmp( &(*this)(0), &Other(0) )!=0;
	}

	// Functions.
	INT Length() const
	{
		checkstr(Num());
		checkstr((*this)(Num()-1)==0);
		checkstr(Num()==(int)appStrlen(&(*this)(0))+1);
		return Num()-1;
	}
	UBOOL Parse( const char* Stream, const char* Match );
	void Appendf( const char* Fmt, ... );
	void Setf( const char* Fmt, ... );

	// Friends.
	friend FArchive& operator<<( FArchive& Ar, FString& S )
	{
		return Ar << (TArray<char>&) S;
	}
};

/*-------------------------------------------------------------------------------
	Dynamic array iterator.
-------------------------------------------------------------------------------*/

//
// Array iterators.
//
template <class T> class TIterator
{
public:
	TIterator( TArray<T>& InArray ) : Array(InArray), Index(-1) { ++*this;      }
	void operator++()      { ++Index;                                           }
	INT GetIndex()   const { return Index;                                      }
	operator UBOOL() const { return Index < Array.Num();                        }
	T& operator*()   const { return Array(Index);                               }
	T* operator->()  const { return &Array(Index);                              }
	T& GetCurrent()  const { return Array( Index );                             }
	T& GetPrev()     const { return Array( Index ? Index-1 : Array.Num()-1 );   }
	T& GetNext()     const { return Array( Index<Array.Num()-1 ? Index+1 : 0 ); }
private:
	TArray<T>& Array;
	INT Index;
};
template <class T> class TPtrIterator
{
public:
	TPtrIterator( TArray<T*>& InArray ) : Array(InArray), Index(-1) { ++*this;  }
	void operator++()      { ++Index;                                           }
	INT GetIndex()   const { return Index;                                      }
	operator UBOOL() const { return Index < Array.Num();                        }
	T* operator*()   const { return Array(Index);                               }
	T* operator->()  const { return Array(Index);                               }
	T& GetCurrent()  const { return Array( Index );                             }
	T& GetPrev()     const { return Array( Index ? Index-1 : Array.Num()-1 );   }
	T& GetNext()     const { return Array( Index<Array.Num()-1 ? Index+1 : 0 ); }
private:
	TArray<T*>& Array;
	INT Index;
};
template <class T> class TConstIterator
{
public:
	TConstIterator( const TArray<T>& InArray ) : Array(InArray), Index(-1) { ++*this; }
	void operator++()            { ++Index;                                           }
	INT GetIndex()         const { return Index;                                      }
	operator UBOOL()       const { return Index < Array.Num();                        }
	const T& operator*()   const { return Array(Index);                               }
	const T* operator->()  const { return &Array(Index);                              }
	const T& GetCurrent()  const { return Array( Index );                             }
	const T& GetPrev()     const { return Array( Index ? Index-1 : Array.Num()-1 );   }
	const T& GetNext()     const { return Array( Index<Array.Num()-1 ? Index+1 : 0 ); }
private:
	const TArray<T>& Array;
	INT Index;
};

/*----------------------------------------------------------------------------
	TMap.
----------------------------------------------------------------------------*/

//
// Maps unique keys to values.
//
template< class TK, class TI > class TMap
{
public:
	INT Size()
	{
		return Pairs.ArrayNum;
	}
	TI&operator[](INT i)
	{
		return Pairs(i).Value;
	}
	const TI&operator[](INT i)const
	{
		return Pairs(i).Value;
	}
	void Add( const TK& Key, const TI& Value )
	{
		for( INT i=0; i<Pairs.Num(); i++ )
			if( Pairs(i).Key==Key )
				break;
		if( i==Pairs.Num() )
			new(Pairs)FPair;
		Pairs(i).Key   = Key;
		Pairs(i).Value = Value;
	}
	void Empty()
	{
		Pairs.Empty();
	}
	UBOOL Find( const TK& Key, TI& Value ) const
	{
		for( INT i=0; i<Pairs.Num(); i++ )
			if( Pairs(i).Key==Key )
				break;
		if( i<Pairs.Num() )
			Value = Pairs(i).Value;
		return i<Pairs.Num();
	}
	UBOOL Find(const TK& Key, TI*& Value) 
	{
		for (INT i = 0; i < Pairs.Num(); i++)
			if (Pairs(i).Key == Key)
				break;
		if (i < Pairs.Num())
			Value = &Pairs(i).Value;
		else
			Value = 0;
		return i < Pairs.Num();
	}
	TI* Find(const TK& Key) 
	{
		for (INT i = 0; i < Pairs.Num(); i++)
			if (Pairs(i).Key == Key)
				break;
		if (i < Pairs.Num())
			return & Pairs(i).Value;
		check(false);
		return 0;
	}
private:
	struct FPair
	{
		TK Key;
		TI Value;
	};
	TArray<FPair> Pairs;
};

/*----------------------------------------------------------------------------
	FRainbowPtr.
----------------------------------------------------------------------------*/

//
// A union of pointers of all base types.
//
union CORE_API FRainbowPtr
{
	// All pointers.
	void*  PtrVOID;
	BYTE*  PtrBYTE;
	_WORD* PtrWORD;
	DWORD* PtrDWORD;
	QWORD* PtrQWORD;
	FLOAT* PtrFLOAT;

	// Conversion constructors.
	FRainbowPtr() {}
	FRainbowPtr( void* Ptr ) : PtrVOID(Ptr) {};
};

/*----------------------------------------------------------------------------
	Global constants.
----------------------------------------------------------------------------*/

enum {MAXBYTE		= 0xff       };
enum {MAXWORD		= 0xffffU    };
enum {MAXDWORD		= 0xffffffffU};
enum {MAXSBYTE		= 0x7f       };
enum {MAXSWORD		= 0x7fff     };
enum {MAXINT		= 0x7fffffff };
enum {INDEX_NONE	= -1         };

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
