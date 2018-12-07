#pragma once
#if !defined(PB_JOURNEY_H)
#define PB_JOURNEY_H

#include "pb_path.h"
#include <stack>

typedef struct journey {
	// DATA:
	std::deque<int>	 pathlist; // need access from mapGraph
	PB_Path		*currentoriginal;
	PB_Path		 currentcopy;
} JOURNEY;

bool	 journey_continues(JOURNEY *journey);
// returns true if there are more paths to go

PB_Path	*journey_getnextpath(JOURNEY *journey);
// returns the next path of the journey

void	 journey_savepathdata(JOURNEY *journey);
// copies back the path to the original in mapgraph

void	 journey_cancel(JOURNEY *journey);
// cancels the journey (continues() will return false)
#endif
