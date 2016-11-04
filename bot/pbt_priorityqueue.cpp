#include <float.h>
#include <stdio.h>
#include "PBT_PriorityQueue.h"




PBT_PriorityQueue::PBT_PriorityQueue()
{
	weight[EMPTY_KEY] = FLT_MAX;
	init();
}


void PBT_PriorityQueue::init()
{
	numElements = 0;
	numFree = 0;
	for (int i=0; i<MAX_QUEUE_ELEMENTS; i++) {
		heapPos[i] = NOT_IN_HEAP;
	//	heap[i] = EMPTY_KEY;
	}
}


short PBT_PriorityQueue::getFirst()
{
	short first = heap[0];

	int hPos = 0;
	int l, r;
	do {
		l = hPos+hPos+1;
		r = l+1;
		if (weight[heap[l]] < weight[heap[r]]) {
			heap[hPos] = heap[l];
			if (heap[hPos] == EMPTY_KEY) {
				freePos[numFree++] = (short)hPos;
				break;
			}
			else if (l >= ((MAX_QUEUE_ELEMENTS-1)/2)) {
				heap[l] = EMPTY_KEY;
				freePos[numFree++] = (short)l;
				break;
			}
			hPos=l;
		}
		else {
			heap[hPos] = heap[r];
			if (heap[hPos] == EMPTY_KEY) {
				freePos[numFree++] = (short)hPos;
				break;
			}
			else if (r >= ((MAX_QUEUE_ELEMENTS-1)/2)) {
				heap[r] = EMPTY_KEY;
				freePos[numFree++] = (short)r;
				break;
			}
			hPos=r;
		}
	} while (0==0);
	
	//heapPos[first] = NOT_IN_HEAP;		UNDONE: indices that have been in must be known!
	
	numElements--;
	return first;
}


void PBT_PriorityQueue::addOrUpdate( short index, float newWeight, float newValue )
{
	short hPos = heapPos[index];

	if ( hPos == NOT_IN_HEAP) {
		// find position in heap too store new node
		if (numFree>0) hPos = freePos[--numFree];	
		else {
			hPos = numElements;
			if (hPos < ((MAX_QUEUE_ELEMENTS-1)/2)) {
				// set empty keys:
				int l = hPos+hPos+1;
				int r = l+1;
				heap[l] = EMPTY_KEY;
				heap[r] = EMPTY_KEY;
			}
		}
		weight[index] = FLT_MAX;
		numElements++;
	}

	if (newWeight < weight[index]) {
		// update weight
		short pre;
		while ( (hPos > 0) && (newWeight < weight[heap[(pre=(hPos-1)/2)]]) ) {
			// swap indices
			heap[hPos] = heap[pre];
			heapPos[heap[pre]] = hPos;
			hPos = pre;
		}
		heap[hPos] = index;
		weight[index] = newWeight;
		value[index] = newValue;
		heapPos[index] = hPos;
	}
}
