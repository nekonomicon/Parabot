#if !defined( PB_JOURNEY_H )
#define PB_JOURNEY_H


#include "pb_path.h"
#include <stack>


class PB_Journey
{

public:

	PB_Journey();

	~PB_Journey();

	bool continues() { return ( !pathList.empty() ); }
	// returns true if there are more paths to go

	PB_Path* getNextPath();
	// returns the next path of the journey

	void savePathData();
	// copies back the path to the original in mapgraph

	void cancel();
	// cancels the journey (continues() will return false)

	// DATA:
	std::deque<int>	pathList;	// need access from mapGraph


private:

	PB_Path		*currentOriginal;
	PB_Path		currentCopy;
};

#endif