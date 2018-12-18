#include <float.h>
#include <stdio.h>
#include "priorityqueue.h"

bool
priorityqueue_empty(PRIORITY_QUEUE *pqueue)
{
	return (pqueue->numelements == 0);
}

int
priorityqueue_size(PRIORITY_QUEUE *pqueue)
{
	return pqueue->numelements;
}

bool
priorityqueue_nevercontained(PRIORITY_QUEUE *pqueue, short index)
{
	return (pqueue->heappos[index] == NOT_IN_HEAP);
}

float
priorityqueue_getweight(PRIORITY_QUEUE *pqueue, short index)
{
	return pqueue->weight[index];
}

float
priorityqueue_getvalue(PRIORITY_QUEUE *pqueue, short index)
{
	return pqueue->value[index];
}

short
priorityqueue_getfirst(PRIORITY_QUEUE *pqueue)
{
	short first = pqueue->heap[0];

	int hPos = 0;
	int l, r, m;
	do {
		l = hPos + hPos + 1;
		r = l + 1;
		m = (pqueue->weight[pqueue->heap[l]] < pqueue->weight[pqueue->heap[r]]) ? l : r;
		pqueue->heap[hPos] = pqueue->heap[m];
		if (pqueue->heap[hPos] == EMPTY_KEY) {
			pqueue->freepos[++pqueue->numfree] = (short)hPos;
			break;
		} else if (m >= ((MAX_QUEUE_ELEMS - 1) / 2)) {
			pqueue->heap[m] = EMPTY_KEY;
			pqueue->freepos[++pqueue->numfree] = (short)m;
			break;
		}
		hPos = m;
	} while (true);

	// heapPos[first] = NOT_IN_HEAP;		UNDONE: indices that have been in must be known!

	--pqueue->numelements;
	return first;
}

void
priorityqueue_addorupdate(PRIORITY_QUEUE *pqueue, short index, float newWeight, float newValue)
{
	short hPos = pqueue->heappos[index];

	if (hPos == NOT_IN_HEAP) {
		// find position in heap too store new node
		if (pqueue->numfree > 0)
			hPos = pqueue->freepos[--pqueue->numfree];
		else {
			hPos = pqueue->numelements;
			if (hPos < ((MAX_QUEUE_ELEMS - 1) / 2)) {
				// set empty keys:
				int l = hPos + hPos + 1;
				int r = l + 1;
				pqueue->heap[l] = EMPTY_KEY;
				pqueue->heap[r] = EMPTY_KEY;
			}
		}
		pqueue->weight[index] = FLT_MAX;
		++pqueue->numelements;
	}

	if (newWeight < pqueue->weight[index]) {
		// update weight
		short pre;
		while ((hPos > 0) && (newWeight < pqueue->weight[pqueue->heap[(pre = (hPos - 1) / 2)]])) {
			// swap indices
			pqueue->heap[hPos] = pqueue->heap[pre];
			pqueue->heappos[pqueue->heap[pre]] = hPos;
			hPos = pre;
		}
		pqueue->heap[hPos] = index;
		pqueue->weight[index] = newWeight;
		pqueue->value[index] = newValue;
		pqueue->heappos[index] = hPos;
	}
}
