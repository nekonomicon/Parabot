#include <float.h>
#include <stdio.h>
#include "pbt_priorityqueue.h"
#include "string.h"

PBT_PriorityQueue::PBT_PriorityQueue()
{
	weight[EMPTY_KEY] = FLT_MAX;
	init();
}

void PBT_PriorityQueue::init()
{
	numElements = 0;
	numFree = 0;
	memset( &heapPos, NOT_IN_HEAP, sizeof heapPos );
}

short PBT_PriorityQueue::getFirst()
{
	short first = heap[0];

	int hPos = 0;
	int l, r, m;
	do {
		l = hPos + hPos + 1;
		r = l + 1;
		m = (weight[heap[l]] < weight[heap[r]]) ? l : r;
		heap[hPos] = heap[m];
		if (heap[hPos] == EMPTY_KEY) {
			freePos[numFree++] = (short)hPos;
			break;
		} else if (m >= ((MAX_QUEUE_ELEMS - 1) / 2)) {
			heap[m] = EMPTY_KEY;
			freePos[numFree++] = (short)m;
			break;
		}
		hPos = m;
	} while (true);
	
	//heapPos[first] = NOT_IN_HEAP;		UNDONE: indices that have been in must be known!
	
	numElements--;
	return first;
}

void PBT_PriorityQueue::addOrUpdate( short index, float newWeight, float newValue )
{
	short hPos = heapPos[index];

	if ( hPos == NOT_IN_HEAP) {
		// find position in heap too store new node
		if (numFree > 0)
			hPos = freePos[--numFree];
		else {
			hPos = numElements;
			if (hPos < ((MAX_QUEUE_ELEMS - 1) / 2)) {
				// set empty keys:
				int l = hPos + hPos + 1;
				int r = l + 1;
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
		while ((hPos > 0) && (newWeight < weight[heap[(pre = (hPos - 1) / 2)]]) ) {
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
