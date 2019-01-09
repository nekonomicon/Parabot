#include "parabot.h"

/*
PB_Journey::PB_Journey()
{
	currentOriginal = 0;
}

PB_Journey::~PB_Journey()
{
	pathList.clear(); 
}
*/

bool
journey_continues(JOURNEY *journey)
{
	return (!journey->pathlist.empty());
}

PATH *
journey_getnextpath(JOURNEY *journey)
// returns the next path of the journey
{ 
	int pathId = journey->pathlist.back();
	journey->pathlist.pop_back();
//	return getPath( pathId );
	journey->currentoriginal = getPath( pathId );
	journey->currentcopy = *journey->currentoriginal;
	return &journey->currentcopy;
}

void
journey_savepathdata(JOURNEY *journey)
// copies back the path to the original in mapgraph
{
	assert(journey->currentoriginal != 0);
	*journey->currentoriginal = journey->currentcopy;
}

void
journey_cancel(JOURNEY *journey)
// cancels the journey (continues() will return false)
{ 
	journey->pathlist.clear();
	journey->currentoriginal = 0;
}
