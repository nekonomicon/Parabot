#pragma once
#if !defined(PBT_PRIORITY_QUEUE_H)
#define PBT_PRIORITY_QUEUE_H

#define MAX_QUEUE_ELEMS 8191	
// maximal number of elements that queue can handle

#define EMPTY_KEY	MAX_QUEUE_ELEMS
#define NOT_IN_HEAP -1

typedef struct priority_queue {
	short	heap[MAX_QUEUE_ELEMS];		// heap with node indices
	float	weight[MAX_QUEUE_ELEMS + 1];	// cost array
	float	value[MAX_QUEUE_ELEMS + 1];	// e.g. costs without heuristics
	short	heappos[MAX_QUEUE_ELEMS];	// heap indices
	int	numelements;			// number of elements in heap
	short	freepos[MAX_QUEUE_ELEMS];	// list with free heap positions
	int	numfree;			// number of elements in freePos
} PRIORITY_QUEUE;

bool	priorityqueue_empty(PRIORITY_QUEUE *pqueue);
int	priorityqueue_size(PRIORITY_QUEUE *pqueue);
bool	priorityqueue_nevercontained(PRIORITY_QUEUE *pqueue, short index);
float	priorityqueue_getweight(PRIORITY_QUEUE *pqueue, short index);
float	priorityqueue_getvalue(PRIORITY_QUEUE *pqueue, short index);
short	priorityqueue_getfirst(PRIORITY_QUEUE *pqueue);
void	priorityqueue_addorupdate(PRIORITY_QUEUE *pqueue, short index, float newWeight, float newValue=0.0);
#endif
