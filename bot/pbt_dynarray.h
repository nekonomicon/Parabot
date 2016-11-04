#if !defined( PBT_DYNARRAY_H )
#define PBT_DYNARRAY_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




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


#endif 
