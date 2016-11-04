#include "pb_journey.h"
#include "pb_global.h"



PB_Journey::PB_Journey()
{
	currentOriginal = 0;
}


PB_Journey::~PB_Journey()
{
	pathList.clear(); 
}


PB_Path* PB_Journey::getNextPath() 
// returns the next path of the journey
{ 
	int pathId = pathList.back();
	pathList.pop_back();
//	return getPath( pathId );
	currentOriginal = getPath( pathId );
	currentCopy = *currentOriginal;
	return &currentCopy; 
}
	

void PB_Journey::savePathData()
// copies back the path to the original in mapgraph
{
	assert( currentOriginal != 0 );
	*currentOriginal = currentCopy;
}


void PB_Journey::cancel() 
// cancels the journey (continues() will return false)
{ 
	pathList.clear(); 
	currentOriginal = 0;
}