#if !defined( PBT_DYNARRAY_CPP )
#define PBT_DYNARRAY_CPP

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#if !defined( _DEBUG )
#define NDEBUG			// no assert!
#endif

#include <assert.h>
#include <malloc.h>
#include "PBT_DynArray.h"




template <class T> 
PBT_DynArray<T>::PBT_DynArray( int maxCapacity, int bankCapacity )
{
	numBanks = 0;
	capacity = 0;
	numElements = 0;

	bankSize = bankCapacity;
	maxBanks = ((maxCapacity-1) / bankSize) + 1;
	typedef T* TA;
	bank = new TA[maxBanks];
}


template <class T>
PBT_DynArray<T>::~PBT_DynArray()
{
	clear();
	delete [] bank;
}


template <class T>
T& PBT_DynArray<T>::operator[]( int index )
{
	assert( index < capacity );
	int b    = index / bankSize;
	int ofs  = index % bankSize;
	return bank[b][ofs];
}


template <class T>
void PBT_DynArray<T>::set( int index, T data )
{
	assert( index < capacity );
	int b    = index / bankSize;
	int ofs  = index % bankSize;
	bank[b][ofs] = data;
}


template <class T>
void PBT_DynArray<T>::add( int index, T data )
{
	int b    = index / bankSize;
	int ofs  = index % bankSize;

	assert( b <= numBanks );
	if (b == numBanks) {
		// get memory for new bank
		assert( b < maxBanks );
		bank[b] = new T[bankSize];
		capacity += bankSize;
		numBanks++;
	}
	
	bank[b][ofs] = data;
	if ( index >= numElements ) numElements = index+1;
}


template <class T>
void PBT_DynArray<T>::clear()
{
	for (int i=0; i<numBanks; i++) delete [] bank[i];
	
	numBanks = 0;
	capacity = 0;
	numElements = 0;
}


template <class T>
void PBT_DynArray<T>::push_back( const T& data )
{
	add( numElements, data );
}


#endif