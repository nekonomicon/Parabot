#pragma once
#if !defined(PBT_DYNARRAY_H)
#define PBT_DYNARRAY_H

template <class T>
class PBT_DynArray  
{


public:

	PBT_DynArray( int maxCapacity, int bankCapacity );
	//  maxCapacity  = maximum number of elements the array can store
	//  bankCapacity = number of elements per bank
	 
	virtual ~PBT_DynArray();

	T& operator[]( int index );		// read like normal array
	void add( int index, T data );	// add new entries
	void set( int index, T data );	// set with this function

	// STL compatibility:
	int size() const { return numElements; }
	void clear();
	void push_back( const T& data );
	


private:

	T	**bank;
	int numBanks;					// number of allocated banks
	int	capacity;					// current capacity
	int numElements;				// current size

	int bankSize;					// number of elements per bank
	int maxBanks;					// maximum number of banks


};

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
