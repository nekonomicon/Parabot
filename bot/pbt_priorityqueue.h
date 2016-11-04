#if !defined( PBT_PRIORITY_QUEUE_H )
#define PBT_PRIORITY_QUEUE_H

#if _MSC_VER > 1000
#pragma once
#endif




class PBT_PriorityQueue
{


#define MAX_QUEUE_ELEMENTS 8191	
// maximal number of elements that queue can handle

#define EMPTY_KEY	MAX_QUEUE_ELEMENTS
#define NOT_IN_HEAP -1


public:

	PBT_PriorityQueue();
	void init();

	bool empty()						{ return (numElements == 0);				}
	int size()							{ return numElements;						}
	bool neverContained( short index )	{ return (heapPos[index] == NOT_IN_HEAP);	}
	float getWeight( short index )		{ return weight[index];						}
	float getValue ( short index )		{ return value[index];						}

	short getFirst();
	void addOrUpdate( short index, float newWeight, float newValue=0.0 );
	


private:

	short	heap[MAX_QUEUE_ELEMENTS];		// heap with node indices
	float	weight[MAX_QUEUE_ELEMENTS+1];	// cost array
	float	value[MAX_QUEUE_ELEMENTS+1];	// e.g. costs without heuristics
	short	heapPos[MAX_QUEUE_ELEMENTS];	// heap indices
	int		numElements;					// number of elements in heap
	short	freePos[MAX_QUEUE_ELEMENTS];	// list with free heap positions
	int		numFree;						// number of elements in freePos


};

#endif